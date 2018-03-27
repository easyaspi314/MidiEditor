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

#include "NoteOnEvent.h"

#include "../midi/MidiOutput.h"
#include "OffEvent.h"
#include <QtMath>

NoteOnEvent::NoteOnEvent(ubyte note, ubyte velocity, ubyte ch, MidiTrack *track) : OnEvent(ch, track){
    _note = note;
    _velocity = velocity;
    // has to be done here because the line is not known in OnEvents constructor
    // before
    OffEvent::enterOnEvent(this);
}

NoteOnEvent::NoteOnEvent(const NoteOnEvent &other) : OnEvent(other){
    _note = other._note;
    _velocity = other._velocity;
}

int NoteOnEvent::type() const {
    return Type;
}

ubyte NoteOnEvent::note(){
    return _note;
}

ubyte NoteOnEvent::velocity(){
    return _velocity;
}

void NoteOnEvent::setVelocity(ubyte v){
    ProtocolEntry *toCopy = copy();
    _velocity = v & 0x7F;
    protocol(toCopy, this);
}

ubyte NoteOnEvent::line(){
    return 127-_note;
}

void NoteOnEvent::setNote(ubyte n){
    ProtocolEntry *toCopy = copy();
    _note = n;
    protocol(toCopy, this);
}

ProtocolEntry *NoteOnEvent::copy(){
    return new NoteOnEvent(*this);
}

void NoteOnEvent::reloadState(ProtocolEntry *entry){
    NoteOnEvent *other = protocol_cast<NoteOnEvent*>(entry);
    if(!other){
        return;
    }
    OnEvent::reloadState(entry);

    _note = other->_note;
    _velocity = other->_velocity;
}


const QString NoteOnEvent::toMessage(){
    return _("noteon %1 %2 %3").arg(QString::number(channel()),
            QString::number(note()),QString::number(velocity()));
}

const QString NoteOnEvent::offEventMessage(){
    return _("noteoff %1 %2").arg(QString::number(channel()), QString::number(note()));
}

const QByteArray NoteOnEvent::save(){
    QByteArray array = QByteArray();
    append(array, 0x90 | channel());
    append(array, note());
    append(array, velocity());
    return array;
}

const QByteArray NoteOnEvent::play(){
    if (!_settings.gba_mode)
        return save();

    QByteArray array = QByteArray();
    append(array, 0x90 | channel());
    append(array, note());
    append(array, fromExpVal(velocity()));
    return array;
}

const QByteArray NoteOnEvent::saveOffEvent(){
    QByteArray array = QByteArray();
    append(array, 0x80 | channel());
    append(array, note());
    append(array, 0x0);
    return array;
}

const QString NoteOnEvent::typeString(){
    return "Note On Event";
}
