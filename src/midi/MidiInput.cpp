// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MidiInput.h"

#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/KeyPressureEvent.h"
#include "../gui/MainWindow.h"

#include "MidiTrack.h"

#include <QTextStream>
#include <QByteArray>

#include <cstdlib>

#include "rtmidi/RtMidi.h"

#include "MidiOutput.h"

#include "../Singleton.h"

MidiInput::MidiInput() : QObject() {
    _midiIn = qnullptr;
    _inPort = QString();
    _messages = new QMultiMap<int, std::vector<ubyte> >;
    _currentTime = 0;
    _recording = false;
}

void MidiInput::init() {
    // RtMidiIn constructor
    try {
        _midiIn = new RtMidiIn(RtMidi::UNSPECIFIED, "MidiEditor input");
        //_midiIn->setQueueSizeLimit(65535);
        _midiIn->ignoreTypes(false, true, true);
        _midiIn->setCallback(&receiveMessage);
    }
    catch (RtMidiError &error) {
        error.printMessage();
    }
    // alert MainWindow that the input is ready.
    MainWindow::getMainWindow()->ioReady(true);
}

void MidiInput::receiveMessage(qreal deltatime, std::vector<ubyte>
        *message, void *userData)
{
    Q_UNUSED(deltatime)
    Q_UNUSED(userData)

    if (message->size() > 1) {
        MidiInput::instance()->_messages->insert(MidiInput::instance()->_currentTime, *message);
    }

    if (_settings.thru) {
        QByteArray a;

        for (ulong i = 0; i < message->size(); i++) {
            // check channel
            if (i == 0) {
                switch (message->at(i) & 0xF0) {
                    case 0x80: {
                        append(a, 0x80 | MidiOutput::instance()->standardChannel());
                        continue;
                    }
                    case 0x90: {
                        append(a, 0x90 | MidiOutput::instance()->standardChannel());
                        continue;
                    }
                    case 0xD0: {
                        append(a, 0xD0 | MidiOutput::instance()->standardChannel());
                        continue;
                    }
                    case 0xC0: {
                        append(a, 0xC0 | MidiOutput::instance()->standardChannel());
                        continue;
                    }
                    case 0xB0: {
                        append(a, 0xB0 | MidiOutput::instance()->standardChannel());
                        continue;
                    }
                    case 0xA0: {
                        append(a, 0xA0 | MidiOutput::instance()->standardChannel());
                        continue;
                    }
                    case 0xE0: {
                        append(a, 0xE0 | MidiOutput::instance()->standardChannel());
                        continue;
                    }
                }
            }
            append(a, message->at(i));
        }
        MidiOutput::instance()->sendCommand(a);
    }
}

const QStringList MidiInput::inputPorts() {

    QStringList ports;

    // Check outputs.
    uint nPorts = _midiIn->getPortCount();
    if (nPorts > INT_MAX) {
        qWarning("MidiInput::inputPorts(): Somehow had more than INT_MAX input ports!");
        nPorts = INT_MAX;
    }
    ports.reserve(int(nPorts));
    for (uint i = 0; i < nPorts; i++) {

        try {
            ports.append(QString::fromStdString(_midiIn->getPortName(i)));
        } catch (RtMidiError &error) {
            error.printMessage();
        }
    }

    return ports;
}

bool MidiInput::setInputPort(const QString &name) {

    // try to find the port
    uint nPorts = _midiIn->getPortCount();

    for (uint i = 0; i < nPorts; i++) {
        try {
            // if the current port has the given name, select it and close
            // current port
            if (name == QString::fromStdString(_midiIn->getPortName(i))) {

                _midiIn->closePort();
                _midiIn->openPort(i);
                _inPort = name;
                return true;
            }

        } catch (RtMidiError &error) {
            error.printMessage();
        }
    }

    // port not found
    return false;
}

QString MidiInput::inputPort() {
    return _inPort;
}

void MidiInput::startInput() {

    // clear eventlist
    _messages->clear();

    _recording = true;
}

QMultiMap<int, MidiEvent*> MidiInput::endInput(MidiTrack *track) {

    QMultiMap<int, MidiEvent*> eventList;
    QByteArray array;

    auto it = _messages->constBegin();

    bool ok = true;
    bool endEvent = false;

    _recording = false;

    QMultiMap<int, OffEvent*> emptyOffEvents;

    while (ok && it != _messages->constEnd()) {

        array.clear();

        for (ubyte b : it.value()) {
            append(array, b);
        }

        QDataStream tempStream(array);

        MidiEvent *event = MidiEvent::loadMidiEvent(&tempStream, &ok, &endEvent, track);
        OffEvent *off = protocol_cast<OffEvent*>(event);
        if (off && !off->onEvent()) {
            emptyOffEvents.insert(it.key(), off);
            ++it;
            continue;
        }
        if (ok) {
            eventList.insert(it.key(), event);
        }
        // if on event, check whether the off event has been loaded before.
        // this occurs when RTMidi fails to send the correct order
        OnEvent *on = protocol_cast<OnEvent*>(event);
        if (on && emptyOffEvents.contains(it.key())) {
            auto emptyIt = emptyOffEvents.lowerBound(it.key());
            while (emptyIt != emptyOffEvents.end() && emptyIt.key() == it.key()) {
                if (emptyIt.value()->line() == on->line()) {
                    emptyIt.value()->setOnEvent(on);
                    OffEvent::removeOnEvent(on);
                    emptyOffEvents.remove(emptyIt.key(), emptyIt.value());
                    // add offEvent
                    eventList.insert(it.key() + 100, emptyIt.value());
                    break;
                }
                ++emptyIt;
            }
        }
        ++it;
    }
    auto it2 = eventList.begin();
    while (it2 != eventList.end()) {
        OnEvent *on = protocol_cast<OnEvent*>(it2.value());
        if (on && !on->offEvent()) {
            it2 = eventList.erase(it2);
        } else {
            ++it2;
        }
    }
    _messages->clear();

    _currentTime = 0;


    // perform consistency check
    QMultiMap<int, MidiEvent*> toRemove;
    const QList<int> allTicks = eventList.uniqueKeys();

    // forward declare containers outside the for loops
    QMultiMap<int, MidiEvent*> sortedByChannel;
    QMultiMap<int, MidiEvent*> sortedByLine;
    QMap<int, MidiEvent*> byController;
    QMap<int, MidiEvent*> byNote;


    for (int tick : allTicks) {

        int id = 0;
        sortedByChannel.clear();
        auto mapIt = eventList.upperBound(tick);
        auto mEnd = eventList.lowerBound(tick);
        for (; mapIt != mEnd; ++mapIt) {
            MidiEvent *event = mapIt.value();
            event->setTemporaryRecordID(id);
            sortedByChannel.insert(event->channel(), event);
            id++;
        }

        for (int channel : toUnique(sortedByChannel.keys())) {
            sortedByLine.clear();
            mapIt = sortedByChannel.upperBound(channel);
            mEnd = sortedByChannel.lowerBound(channel);
            for (; mapIt != mEnd; ++mapIt) {
                MidiEvent *event = mapIt.value();
                if ((event->line() == MidiEventLine::ControlChangeEventLine) ||
                    (event->line() == MidiEventLine::PitchBendEventLine) ||
                    (event->line() == MidiEventLine::ChannelPressureEventLine) ||
                    (event->line() == MidiEventLine::KeyPressureEventLine)
                ) {
                    sortedByLine.insert(event->line(), event);
                }
            }
            const auto sortedByLineKeys = sortedByLine.uniqueKeys();
            for (int line : sortedByLineKeys) {

                // check for duplicates and mark all events which have been replaced
                // as toRemove
                if (sortedByLine.count(line) > 1) {

                    if (line == MidiEventLine::ControlChangeEventLine) {
                        byController.clear();
                        mapIt = sortedByLine.upperBound(line);
                        mEnd = sortedByLine.lowerBound(line);
                        for (; mapIt != mEnd; ++mapIt) {
                            ControlChangeEvent *conv = protocol_cast<ControlChangeEvent*>(mapIt.value());
                            if (!conv) {
                                continue;
                            }
                            if (byController.contains(conv->control())) {
                                if (conv->temporaryRecordID() > byController[conv->control()]->temporaryRecordID()) {
                                    toRemove.insert(tick, byController[conv->control()]);
                                    byController[conv->control()] = conv;
                                } else {
                                    toRemove.insert(tick, conv);
                                }
                            } else {
                                byController.insert(conv->control(), conv);
                            }
                        }
                    } else if ((line == MidiEventLine::PitchBendEventLine) || (line == MidiEventLine::ChannelPressureEventLine)) {

                        // search for maximum
                        MidiEvent *maxIdEvent = qnullptr;

                        mapIt = sortedByLine.upperBound(line);
                        mEnd = sortedByLine.lowerBound(line);
                        for (; mapIt != mEnd; ++mapIt) {

                            toRemove.insert(tick, mapIt.value());
                            if (!maxIdEvent || (maxIdEvent->temporaryRecordID() < mapIt.value()->temporaryRecordID())) {
                                maxIdEvent = mapIt.value();
                            }
                        }

                        if (maxIdEvent) {
                            toRemove.remove(tick, maxIdEvent);
                        }

                    } else if (line == MidiEventLine::KeyPressureEventLine) {
                        byNote.clear();
                        mapIt = sortedByLine.upperBound(line);
                        mEnd = sortedByLine.lowerBound(line);
                        for (; mapIt != mEnd; ++mapIt) {
                            KeyPressureEvent *conv = protocol_cast<KeyPressureEvent*>(mapIt.value());
                            if (!conv) {
                                continue;
                            }
                            if (byNote.contains(conv->note())) {
                                if (conv->temporaryRecordID() > byNote[conv->note()]->temporaryRecordID()) {
                                    toRemove.insert(tick, byNote[conv->note()]);
                                    byNote[conv->note()] = conv;
                                } else {
                                    toRemove.insert(tick, conv);
                                }
                            } else {
                                byNote.insert(conv->note(), conv);
                            }
                        }
                    }
                }
            }
        }
    }

    if (toRemove.size() > 0) {
        auto remIt = toRemove.begin();
        while (remIt != toRemove.end()) {
            eventList.remove(remIt.key(), remIt.value());
            ++remIt;
        }
    }

    return eventList;
}

void MidiInput::setTime(int ms) {
    _currentTime = ms;
}

bool MidiInput::recording() {
    return _recording;
}

const QList<int> MidiInput::toUnique(const QList<int> &in) {
    QList<int> out;
    for (int i : in) {
        if ((out.size() == 0) || (out.last() != i)) {
            out.append(i);
        }
    }
    return out;
}

MidiInput *MidiInput::createInstance() {
    return new MidiInput();
}
MidiInput *MidiInput::instance() {
    return Singleton<MidiInput>::instance(MidiInput::createInstance);
}
