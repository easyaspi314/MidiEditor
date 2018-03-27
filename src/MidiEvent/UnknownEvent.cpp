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

#include "UnknownEvent.h"

#include "../midi/MidiFile.h"

UnknownEvent::UnknownEvent(ubyte channel, ubyte type, QByteArray data, MidiTrack *track) : MidiEvent(channel, track){
    _data = data;
    _type = type;
}

UnknownEvent::UnknownEvent(const UnknownEvent &other) : MidiEvent(other) {
    _data = other._data;
    _type = other._type;
}

int UnknownEvent::type() const {
    return Type;
}

QByteArray UnknownEvent::data(){
    return _data;
}

ubyte UnknownEvent::line(){
    return UnknownEventLine;
}

const QByteArray UnknownEvent::save(){
    QByteArray s;
    append(s, 0xFF);
    append(s, _type);
    s.append(MidiFile::writeVariableLengthValue(_data.length()));
    s.append(_data);
    return s;
}

void UnknownEvent::reloadState(ProtocolEntry *entry){
    UnknownEvent *other = protocol_cast<UnknownEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    _type = other->_type;
    _data = other->_data;
}

ProtocolEntry *UnknownEvent::copy(){
    return new UnknownEvent(*this);
}

ubyte UnknownEvent::unknownType(){
    return _type;
}

void UnknownEvent::setUnknownType(ubyte type){
    _type = type;
    protocol(copy(), this);
}

void UnknownEvent::setData(const QByteArray &d){
    _data = d;
    protocol(copy(), this);
}
