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

#include "TempoChangeEvent.h"
#include "../midi/MidiFile.h"

TempoChangeEvent::TempoChangeEvent(ubyte channel, int value, MidiTrack *track) : MidiEvent(channel, track){
    if (value == 0) {
        qWarning("Error! TempoChangeEvent is trying to divide by zero!");
        _beats = 60000000/120;
    } else {
        _beats = 60000000/value;
    }
}

TempoChangeEvent::TempoChangeEvent(const TempoChangeEvent &other) : MidiEvent(other){
    _beats = other._beats;
}

EventType TempoChangeEvent::type() const {
    return TempoChangeEventType;
}

int TempoChangeEvent::beatsPerQuarter(){
    return _beats;
}

int TempoChangeEvent::msPerTick(){
    int quarters_per_second = int(_beats)/60;
    int ticks_per_second = int(file()->ticksPerQuarter()) * quarters_per_second;
    if (ticks_per_second == 0) {
        qWarning("TempoChangeEvent::msPerTick(): Trying to divide by zero!");
        return 1000/192;
    }
    return 1000/(ticks_per_second);
}

ProtocolEntry *TempoChangeEvent::copy(){
    return new TempoChangeEvent(*this);
}

void TempoChangeEvent::reloadState(ProtocolEntry *entry){
    TempoChangeEvent *other = qobject_cast<TempoChangeEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    _beats = other->_beats;
}

ubyte TempoChangeEvent::line(){
    return MidiEventLine::TempoChangeEventLine;
}

const QByteArray TempoChangeEvent::save(){
    QByteArray array = QByteArray();

    append(array, 0xFF);
    append(array, 0x51);
    append(array, 0x03);
    int value;
    if (_beats == 0) {
        qWarning("TempoChangeEvent::save(): Trying to divide by zero!");
        value = 60000000/120;
    } else {
        value = 60000000/_beats;
    }
    for(int i = 2; i >= 0; i--){
        append(array, (value) & (0xFF << 8*i) >>8*i);
    }

    return array;
}

void TempoChangeEvent::setBeats(int beats){
    ProtocolEntry *toCopy = copy();
    _beats = beats;
    file()->calcMaxTime();
    protocol(toCopy, this);
}

const QString TempoChangeEvent::typeString(){
    return "Tempo Change Event";
}
