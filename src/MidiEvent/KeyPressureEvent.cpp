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

#include "KeyPressureEvent.h"

KeyPressureEvent::KeyPressureEvent(ubyte channel, ubyte value, ubyte note, MidiTrack *track) :
        MidiEvent(channel, track)
{
    _value = value;
    _note = note;
}

KeyPressureEvent::KeyPressureEvent(const KeyPressureEvent &other) :
        MidiEvent(other)
{
    _value = other._value;
    _note = other._note;
}

int KeyPressureEvent::type() const {
    return Type;
}

ubyte KeyPressureEvent::line(){
    return KeyPressureEventLine;
}

const QString KeyPressureEvent::toMessage(){
    return QString();
}

const QByteArray KeyPressureEvent::save(){
    QByteArray array = QByteArray();
    append(array, 0xA0 | channel());
    append(array, _note);
    append(array, _value);
    return array;
}

ProtocolEntry *KeyPressureEvent::copy(){
    return new KeyPressureEvent(*this);
}

void KeyPressureEvent::reloadState(ProtocolEntry *entry) {
    KeyPressureEvent *other = protocol_cast<KeyPressureEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    _value = other->_value;
    _note = other->_note;
}

void KeyPressureEvent::setValue(ubyte v){
    ProtocolEntry *toCopy = copy();
    _value = v;
    protocol(toCopy, this);
}

void KeyPressureEvent::setNote(ubyte n){
    ProtocolEntry *toCopy = copy();
    _note = n;
    protocol(toCopy, this);
}

const QString KeyPressureEvent::typeString(){
    return "Key Pressure Event";
}

ubyte KeyPressureEvent::value(){
    return _value;
}

ubyte KeyPressureEvent::note(){
    return _note;
}
