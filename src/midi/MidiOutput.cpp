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

#include "MidiOutput.h"

#include "../MidiEvent/MidiEvent.h"
#include "../gui/MainWindow.h"
#include "MidiPlayer.h"

#include <QFile>
#include <QTextStream>
#include <QByteArray>

#include <vector>

#include "rtmidi/RtMidi.h"

#include "SenderThread.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"

#include "../Singleton.h"

SenderThread *MidiOutput::_sender = new SenderThread();

MidiOutput::MidiOutput() : QObject() {
    _midiOut = qnullptr;
    _outPort = QString();
    playedNotes = QMap<ubyte, QList<ubyte> >();
    _stdChannel = 0;
}

void MidiOutput::init() {
    // RtMidiOut constructor
    try {
        _midiOut = new RtMidiOut(RtMidi::UNSPECIFIED,"MidiEditor output");
    } catch (RtMidiError &error) {
        error.printMessage();
    }
    // Alert MainWindow that output is ready.
    MainWindow::getMainWindow()->ioReady(false);
}

SenderThread *MidiOutput::sender() {
    return _sender;
}

void MidiOutput::sendCommand(const QByteArray &array) {
    sendEnqueuedCommand(array);
}

void MidiOutput::sendCommand(MidiEvent *e) {
    if (!_sender->isRunning()) {
        _sender->start(QThread::TimeCriticalPriority);
    }
    if (e->channel() < 16) {
        _sender->enqueue(e);

        if (_settings.alt_stop) {
            if (e->type() == EventType::NoteOnEventType) {
                NoteOnEvent *n = qobject_cast<NoteOnEvent *>(e);
                if (n && n->velocity() > 0) {
                    playedNotes[n->channel()].append(n->note());
                } else if (n && n->velocity() == 0) {
                    playedNotes[n->channel()].removeOne(n->note());
                }
            } else if (e->type() == EventType::OffEventType) {
                OffEvent *o = qobject_cast<OffEvent *>(e);
                if (o) {
                    NoteOnEvent *n = qobject_cast<NoteOnEvent *>(o->onEvent());
                    if (n) {
                        playedNotes[n->channel()].removeOne(n->note());
                    }
                }
            }
        }
    }
}

const QStringList MidiOutput::outputPorts() {
    QStringList ports;

    // Check outputs.
    uint nPorts = _midiOut->getPortCount();
    ports.reserve(nPorts);
    for (uint i = 0; i < nPorts; i++) {

        try {
            ports.append(QString::fromStdString(_midiOut->getPortName(i)));
        } catch (RtMidiError &error) {
            error.printMessage();
        }
    }
    return ports;
}

bool MidiOutput::setOutputPort(const QString &name) {

    // try to find the port
    uint nPorts = _midiOut->getPortCount();

    for (uint i = 0; i < nPorts; i++) {

        try {
            // if the current port has the given name, select it and close
            // current port
            if (_midiOut->getPortName(i) == name.toStdString()) {
                _midiOut->closePort();
                _midiOut->openPort(i);
                _outPort = name;
                // Mainly for VirtualMIDISynth which disconnects if it is idle.
                QTimer::singleShot(10, MidiPlayer::instance(), &MidiPlayer::panic);
                return true;
            }

        } catch (RtMidiError &) {}

    }

    // port not found
    return false;
}

void MidiOutput::sendEnqueuedCommand(const QByteArray &array) {

    if (!_settings.out_port.isEmpty()) {

        // convert data to std::vector

        try {
            const ubyte *data = reinterpret_cast<const ubyte*>(array.data());
           // std::vector<ubyte> message(data, data + array.size());
            _midiOut->sendMessage(data, array.size());
        } catch (RtMidiError &error) {
            error.printMessage();
        } catch (std::out_of_range &e) {
            qWarning("%s", e.what());
        }
    }
}

void MidiOutput::sendRawCommand(std::vector<ubyte> *message) {
    if (!_settings.out_port.isEmpty()) {
        try {
            _midiOut->sendMessage(message);
        } catch (RtMidiError &error) {
            error.printMessage();
        }
    }
}

QString MidiOutput::outputPort() {
    return _outPort;
}

void MidiOutput::setStandardChannel(ubyte channel) {
    _stdChannel = channel;
}

ubyte MidiOutput::standardChannel() {
    return _stdChannel;
}

void MidiOutput::sendProgram(ubyte channel, ubyte prog) {
    QByteArray array = QByteArray();
    append(array, 0xC0 | channel);
    append(array, prog);
    sendCommand(array);
}

MidiOutput *MidiOutput::createInstance() {
    return new MidiOutput();
}

MidiOutput *MidiOutput::instance() {
    return Singleton<MidiOutput>::instance(MidiOutput::createInstance);
}
