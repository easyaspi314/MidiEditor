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

#include "SysExEvent.h"

SysExEvent::SysExEvent(ubyte channel, const QByteArray &data, MidiTrack *track) : MidiEvent(channel, track){
    _data = data;
}

SysExEvent::SysExEvent(const SysExEvent &other) : MidiEvent(other) {
    _data = other._data;
}

EventType SysExEvent::type() const {
    return SystemExclusiveEventType;
}

QByteArray SysExEvent::data(){
    return _data;
}

ubyte SysExEvent::line(){
    return SysExEventLine;
}

const QByteArray SysExEvent::save(){
    QByteArray s;
    append(s, 0xF0);
    s.append(_data);
    append(s, 0xF7);
    return s;
}

const QString SysExEvent::typeString(){
    return "System Exclusive Message (SysEx)";
}

ProtocolEntry *SysExEvent::copy(){
    return new SysExEvent(*this);
}

void SysExEvent::reloadState(ProtocolEntry *entry){
    SysExEvent *other = qobject_cast<SysExEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    _data = other->_data;
}

void SysExEvent::setData(const QByteArray &d){
    ProtocolEntry *toCopy = copy();
    _data = d;
    protocol(toCopy, this);
}
