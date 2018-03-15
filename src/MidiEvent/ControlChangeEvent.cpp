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

#include "ControlChangeEvent.h"

#include "../midi/MidiOutput.h"
#include "../midi/MidiFile.h"
#include <QtMath>

ControlChangeEvent::ControlChangeEvent(ubyte channel, ubyte control, ubyte value, MidiTrack *track) :
        MidiEvent(channel, track)
{
    _control = control;
    _value = value;
}

ControlChangeEvent::ControlChangeEvent(const ControlChangeEvent &other) :
        MidiEvent(other)
{
    _value = other._value;
    _control = other._control;
}

EventType ControlChangeEvent::type() const {
    return ControlChangeEventType;
}

ubyte ControlChangeEvent::line(){
    return ControlChangeEventLine;
}

const QString ControlChangeEvent::toMessage(){
    return _("cc %1 %2 %3").arg(QString::number(channel()), QString::number(_control), QString::number(_value));
}

const QByteArray ControlChangeEvent::save(){
    QByteArray array = QByteArray();
    append(array, 0xB0 | channel());
    append(array, _control);
    append(array, _value);
    return array;
}

const QByteArray ControlChangeEvent::play() {
    if (_settings.gba_mode)
        return save();

    ubyte value = _value;
    if (_control == 1 /* Modulation */) {
        // Modulation is amplified a lot on the GBA
        if (value > 0) {
            value = 10 * value;
            if (value > 127)
                value = 127;
        }

    } else if (_control == 7 /* Channel Volume */) {
        value = qRound(sqrt(127.0 * value));
    }

    QByteArray array = QByteArray();
    append(array, 0xB0 | channel());
    append(array, _control);
    append(array, value);

    return array;
}

ProtocolEntry *ControlChangeEvent::copy(){
    return new ControlChangeEvent(*this);
}

void ControlChangeEvent::reloadState(ProtocolEntry *entry){
    ControlChangeEvent *other = qobject_cast<ControlChangeEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    _control = other->_control;
    _value = other->_value;
}

const QString ControlChangeEvent::typeString(){
    return "Control Change Event";
}

ubyte ControlChangeEvent::value(){
    return _value;
}

ubyte ControlChangeEvent::control(){
    return _control;
}

void ControlChangeEvent::setValue(ubyte v){
    ProtocolEntry *toCopy = copy();
    _value = v;
    protocol(toCopy, this);
}

void ControlChangeEvent::setControl(ubyte c){
    ProtocolEntry *toCopy = copy();
    _control = c;
    protocol(toCopy, this);
}

bool ControlChangeEvent::isOnEvent(){
    return false;
}
