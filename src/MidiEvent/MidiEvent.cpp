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

#include "MidiEvent.h"
#include "NoteOnEvent.h"
#include "OffEvent.h"
#include "UnknownEvent.h"
#include "TempoChangeEvent.h"
#include "TimeSignatureEvent.h"
#include "ProgChangeEvent.h"
#include "ControlChangeEvent.h"
#include "ChannelPressureEvent.h"
#include "KeyPressureEvent.h"
#include "TextEvent.h"
#include "../midi/MidiFile.h"
#include "../gui/EventWidget.h"
#include "KeySignatureEvent.h"
#include "PitchBendEvent.h"
#include "SysExEvent.h"
#include "../midi/MidiTrack.h"

#include <QByteArray>

#include "../midi/MidiChannel.h"

ubyte MidiEvent::_startByte = 0;
EventWidget *MidiEvent::_eventWidget = qnullptr;

MidiEvent::MidiEvent(ubyte channel, MidiTrack *track) :/* ProtocolEntry(),*/
    GraphicObject() {
    _track = track;
    numChannel = channel;
    timePos = 0;
    midiFile = qnullptr;
    _tempID = 0;
}

MidiEvent::MidiEvent(const MidiEvent &other) : /*ProtocolEntry(other), */GraphicObject() {
    _track = other._track;
    numChannel = other.numChannel;
    timePos = other.timePos;
    midiFile = other.midiFile;
    _tempID = other._tempID;
}

EventType MidiEvent::type() const {
    return MidiEventType;
}

MidiEvent *MidiEvent::loadMidiEvent(QDataStream *content, bool *ok,
                                    bool *endEvent, MidiTrack *track, ubyte startByte, ubyte secondByte) {

    // first try to load the event. If this does not work try to use
    // old first byte as new first byte. This is implemented in the end of this
    // method using recursive calls.
    // if startByte (paramater) is not 0, this is the second call so first and
    // second byte must not be loaded from the stream but from the parameters.

    *ok = true;

    ubyte tempByte;
    ubyte prevStartByte = _startByte;

    if (!startByte) {
        (*content) >> tempByte;
    } else {
        tempByte = startByte;
    }
    _startByte = tempByte;

    ByteValuePair type = tempByte;
    ubyte channel = type.MSB();

    switch (type.LSB()) {
        case 0x8: {
            // Note Off
            if (!startByte) {
                (*content) >> tempByte;
            } else {
                tempByte = secondByte;
            }
            ubyte note = tempByte;
            if (note > 127) {
                *ok = false;
                return qnullptr;
            }
            // skip byte (velocity)
            (*content) >> tempByte;

            OffEvent *event = new OffEvent(channel, 127 - note, track);
            *ok = true;
            return event;
        }

        case 0x9: {
            // Note On
            if (!startByte) {
                (*content) >> tempByte;
            } else {
                tempByte = secondByte;
            }
            ubyte note = tempByte;
            if (note > 127) {
                *ok = false;
                return qnullptr;
            }
            (*content) >> tempByte;
            ubyte velocity = tempByte;
            *ok = true;

            if (velocity > 0) {
                NoteOnEvent *event = new NoteOnEvent(note, velocity, channel, track);
                return event;
            } else {
                OffEvent *event = new OffEvent(channel, 127 - note, track);
                return event;
            }
        }

        case 0xA: {
            // Key Pressure
            if (!startByte) {
                (*content) >> tempByte;
            } else {
                tempByte = secondByte;
            }
            ubyte note = tempByte;
            if (note > 127) {
                *ok = false;
                return qnullptr;
            }
            (*content) >> tempByte;
            ubyte value = tempByte;

            *ok = true;
            return new KeyPressureEvent(channel, value, note, track);
        }

        case 0xB: {
            // Controller
            if (!startByte) {
                (*content) >> tempByte;
            } else {
                tempByte = secondByte;
            }
            ubyte control = tempByte;
            (*content) >> tempByte;
            ubyte value = tempByte;
            *ok = true;
            return new ControlChangeEvent(channel, control, value, track);
        }

        case 0xC: {
            // programm change
            if (!startByte) {
                (*content) >> tempByte;
            } else {
                tempByte = secondByte;
            }
            *ok = true;
            return new ProgChangeEvent(channel, tempByte, track);
        }

        case 0xD: {
            // Key Pressure
            if (!startByte) {
                (*content) >> tempByte;
            } else {
                tempByte = secondByte;
            }
            ubyte value = tempByte;

            *ok = true;
            return new ChannelPressureEvent(channel, value, track);
        }

        case 0xE: {

            // Pitch Wheel
            if (!startByte) {
                (*content) >> tempByte;
            } else {
                tempByte = secondByte;
            }
            ubyte first = tempByte;
            (*content) >> tempByte;
            ubyte second = tempByte;

            ushort value = (second << 7) | first;

            *ok = true;

            return new PitchBendEvent(channel, value, track);
        }

        case 0xF: {
            // System Message
            channel = 16; // 16 is channel without number

            switch (type.MSB()) {
                case 0x0: {
                    // SysEx
                    QByteArray array;
                    while (tempByte != 0xF7) {
                        (*content) >> tempByte;
                        if (tempByte != 0xF7) {
                            append(array, tempByte);
                        }
                    }
                    *ok = true;
                    return new	SysExEvent(channel, array, track);
                }

                case 0xF: {
                    // MetaEvent
                    if (!startByte) {
                        (*content) >> tempByte;
                    } else {
                        tempByte = secondByte;
                    }
                    switch (tempByte) {
                        case 0x51: {
                            // TempoChange
                            //(*content)>>tempByte;
                            //if(tempByte!=3){
                            //	*ok = false;
                            //	return 0;
                            //}
                            quint32 value;
                            (*content) >> value;
                            // remove first position,
                            value -= 50331648;
                            return new TempoChangeEvent(17, int(value), track);
                        }
                        case 0x58: {
                            // TimeSignature
                            (*content) >> tempByte;
                            if (tempByte != 4) {
                                *ok = false;
                                return qnullptr;
                            }

                            (*content) >> tempByte;
                            int num = int(tempByte);
                            (*content) >> tempByte;
                            int denom = int(tempByte);
                            (*content) >> tempByte;
                            int metronome = int(tempByte);
                            (*content) >> tempByte;
                            int num32 = int(tempByte);
                            return new TimeSignatureEvent(18, num, denom, metronome, num32, track);
                        }
                        case 0x59: {
                            // keysignature
                            (*content) >> tempByte;
                            if (tempByte != 2) {
                                *ok = false;
                                return qnullptr;
                            }
                            ubyte tonality;
                            (*content) >> tonality;
                            (*content) >> tempByte;
                            bool minor = true;
                            if (tempByte == 0) {
                                minor = false;
                            }
                            return new KeySignatureEvent(channel, tonality, minor, track);
                        }
                        case 0x2F: {
                            // end Event
                            *endEvent = true;
                            *ok = true;
                            return qnullptr;
                        }
                        default: {
                            if (tempByte >= 0x01 && tempByte <= 0x07) {
                                // textevent
                                // read type
                                int length = MidiFile::variableLengthvalue(content);
                                QByteArray array;
                                for (int i = 0; i < length; i++) {
                                    (*content) >> tempByte;
                                    if (content->status() != QDataStream::Ok) {
                                        qWarning("Error! A Text Event's length passed the end of the file!");
                                        *ok = false;
                                        return qnullptr;
                                    }
                                    append(array, tempByte);
                                }

                                TextEvent *textEvent = new TextEvent(channel, track,
                                                                     TextType(tempByte),
                                                                     QString::fromUtf8(array));
                                *ok = true;
                                return textEvent;
                            } else {
                                // tempByte is meta event type
                                ubyte typeByte = tempByte;

                                // read length
                                int length = MidiFile::variableLengthvalue(content);

                                // content
                                QByteArray array;
                                for (int i = 0; i < length; i++) {
                                    (*content) >> tempByte;
                                    append(array, tempByte);
                                }
                                *ok = true;
                                return new UnknownEvent(channel, typeByte, array, track);
                            }
                        }
                    }
                }
            }
        }
    }

    // if the event could not be loaded try to use old firstByte before the new
    // data.
    // To do this, pass prefFirstByte and secondByte (the current firstByte)
    // and use it recursive.
    _startByte = prevStartByte;
    return loadMidiEvent(content, ok, endEvent, track, _startByte, tempByte);
}

void MidiEvent::setTrack(MidiTrack *track, bool toProtocol) {
    ProtocolEntry *toCopy = copy();

    _track = track;
    if (toProtocol) {
        protocol(toCopy, this);
    } else {
        delete toCopy;
    }
}

MidiTrack *MidiEvent::track() {
    return _track;
}

void MidiEvent::setChannel(ubyte ch, bool toProtocol) {
    ubyte oldChannel = channel();
    ProtocolEntry *toCopy = copy();
    numChannel = ch;
    if (toProtocol) {
        protocol(toCopy, this);
        file()->channelEvents(oldChannel)->remove(midiTime(), this);
        // tells the new channel to add this event
        setMidiTime(midiTime(), toProtocol);
    } else {
        delete toCopy;
    }
}

ubyte MidiEvent::channel() {
    return numChannel;
}

const QString MidiEvent::toMessage() {
    return QString();
}

const QByteArray MidiEvent::play() {
    return save();
}

const QByteArray MidiEvent::save() {
    return QByteArray();
}

void MidiEvent::setMidiTime(int t, bool toProtocol) {

    // if its once TimeSig / TempoChange at 0, dont delete event
    if (toProtocol && (channel() == 18 || channel() == 17)) {
        if (midiTime() == 0 && midiFile->channel(channel())->eventMap()->count(0)
                == 1) {
            return;
        }
    }

    ProtocolEntry *toCopy = copy();
    file()->channelEvents(numChannel)->remove(timePos, this);
    timePos = t;
    if (timePos > file()->endTick()) {
        file()->setMaxLengthMs(file()->msOfTick(timePos) + 100);
    }
    if (toProtocol) {
        protocol(toCopy, this);
    } else {
        delete toCopy;
    }

    file()->channelEvents(numChannel)->insert(timePos, this);
}

int MidiEvent::midiTime() {
    return timePos;
}

void MidiEvent::setFile(MidiFile *f) {
    midiFile = f;
}

MidiFile *MidiEvent::file() {
    return midiFile;
}

ubyte MidiEvent::line() {
    return 0;
}

void MidiEvent::draw(QPainter *p, QColor c) {
    p->setPen(Qt::gray);
    p->setBrush(c);
    p->drawRoundedRect(qRectF(x(), y(), width(), height()), 1, 1);
}

ProtocolEntry *MidiEvent::copy() {
    return new MidiEvent(*this);
}

void MidiEvent::reloadState(ProtocolEntry *entry) {

    MidiEvent *other = qobject_cast<MidiEvent *>(entry);
    if (!other) {
        return;
    }
    _track = other->_track;
    file()->channelEvents(numChannel)->remove(timePos, this);
    numChannel = other->numChannel;
    file()->channelEvents(numChannel)->remove(timePos, this);
    timePos = other->timePos;
    file()->channelEvents(numChannel)->insert(timePos, this);
    midiFile = other->midiFile;
}

const QString MidiEvent::typeString() {
    return "Midi Event";
}

void MidiEvent::setEventWidget(EventWidget *widget) {
    _eventWidget = widget;
}

EventWidget *MidiEvent::eventWidget() {
    return _eventWidget;
}

bool MidiEvent::shownInEventWidget() {
    if (!_eventWidget) {
        return false;
    }
    return _eventWidget->events().contains(this);
}

bool MidiEvent::isOnEvent() {
    return true;
}

QMap<int, QString> MidiEvent::knownMetaTypes() {
    QMap<int, QString> meta;
    for (ubyte i = 1; i < 8; i++) {
        meta.insert(i, "Text Event");
    }
    meta.insert(0x51, "Tempo Change Event");
    meta.insert(0x58, "Time Signature Event");
    meta.insert(0x59, "Key Signature Event");
    meta.insert(0x2F, "End of Track");
    return meta;
}

void MidiEvent::setTemporaryRecordID(int id) {
    _tempID = id;
}

int MidiEvent::temporaryRecordID() {
    return _tempID;
}

void MidiEvent::moveToChannel(ubyte ch) {

    ubyte oldChannel = channel();

    if (oldChannel > 15) {
        return;
    }

    if (oldChannel == ch) {
        return;
    }

    midiFile->channel(oldChannel)->removeEvent(this);

    ProtocolEntry *toCopy = copy();

    numChannel = ch;

    protocol(toCopy, this);

    midiFile->insertEventInChannel(ch, this, midiTime());
}
