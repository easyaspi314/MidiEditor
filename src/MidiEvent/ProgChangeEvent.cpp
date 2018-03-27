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

#include "ProgChangeEvent.h"

#include "../midi/MidiFile.h"

ProgChangeEvent::ProgChangeEvent(ubyte channel, ubyte prog, MidiTrack *track) : MidiEvent(channel, track){
    _program = prog;
}

ProgChangeEvent::ProgChangeEvent(const ProgChangeEvent &other) : MidiEvent(other){
    _program = other._program;
}

int ProgChangeEvent::type() const {
    return Type;
}

ubyte ProgChangeEvent::line(){
    return ProgramChangeEventLine;
}

const QString ProgChangeEvent::toMessage(){
    return _("prog %1 %2").arg(QString::number(channel()), QString::number(_program));
}

const QByteArray ProgChangeEvent::save(){
    QByteArray array = QByteArray();
    append(array, 0xC0 | channel());
    append(array, _program);
    return array;
}

ProtocolEntry *ProgChangeEvent::copy(){
    return new ProgChangeEvent(*this);
}

void ProgChangeEvent::reloadState(ProtocolEntry *entry){
    ProgChangeEvent *other = protocol_cast<ProgChangeEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    _program = other->_program;
}

const QString ProgChangeEvent::typeString(){
    return "Program Change Event";
}

ubyte ProgChangeEvent::program(){
    return _program;
}

void ProgChangeEvent::setProgram(ubyte p){
    ProtocolEntry *toCopy = copy();
    _program = p;
    protocol(toCopy, this);
}
