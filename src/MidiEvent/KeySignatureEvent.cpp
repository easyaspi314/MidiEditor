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

#include "KeySignatureEvent.h"

KeySignatureEvent::KeySignatureEvent(ubyte channel, ubyte tonality, bool minor, MidiTrack *track) : MidiEvent(channel, track){
    _tonality = tonality;
    _minor = minor;
}

KeySignatureEvent::KeySignatureEvent(const KeySignatureEvent &other) : MidiEvent(other){
    _tonality = other._tonality;
    _minor = other._minor;
}

int KeySignatureEvent::type() const {
    return Type;
}

ubyte KeySignatureEvent::line(){
    return MidiEventLine::KeySignatureEventLine;
}

const QString KeySignatureEvent::toMessage(){
    return QString();
}

const QByteArray KeySignatureEvent::save(){
    QByteArray array = QByteArray();
    append(array, 0xFF);
    append(array, 0x59 | channel());
    append(array, 0x02);
    append(array, tonality());
    if(_minor){
        append(array, 0x01);
    } else {
        append(array, 0x00);
    }
    return array;
}

ProtocolEntry *KeySignatureEvent::copy(){
    return new KeySignatureEvent(*this);
}

void KeySignatureEvent::reloadState(ProtocolEntry *entry){
    KeySignatureEvent *other = protocol_cast<KeySignatureEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    _tonality = other->_tonality;
    _minor = other->_minor;
}

const QString KeySignatureEvent::typeString(){
    return "Key Signature Event";
}

ubyte KeySignatureEvent::tonality(){
    return _tonality;
}

bool KeySignatureEvent::minor(){
    return _minor;
}

void KeySignatureEvent::setTonality(ubyte t){
    ProtocolEntry *toCopy = copy();
    _tonality = t;
    protocol(toCopy, this);
}

void KeySignatureEvent::setMinor(bool minor){
    ProtocolEntry *toCopy = copy();
    _minor = minor;
    protocol(toCopy, this);
}

QString KeySignatureEvent::toString(ubyte tonality, bool minor){

    QString text;

    if(!minor){
        switch(byte(tonality)){
            case 0: {
                text = "C";
                break;
            }
            case 1: {
                text = "G";
                break;
            }
            case 2: {
                text = "D";
                break;
            }
            case 3: {
                text = "A";
                break;
            }
            case 4: {
                text = "E";
                break;
            }
            case 5: {
                text = "B";
                break;
            }
            case 6: {
                text = "F sharp";
                break;
            }
            case -1: {
                text = "F";
                break;
            }
            case -2: {
                text = "B flat";
                break;
            }
            case -3: {
                text = "E flat";
                break;
            }
            case -4: {
                text = "A flat";
                break;
            }
            case -5: {
                text = "D flat";
                break;
            }
            case -6: {
                text = "G flat";
                break;
            }
        }
        text += " major";
    } else {
        switch(byte(tonality)){
            case 0: {
                text = "a";
                break;
            }
            case 1: {
                text = "e";
                break;
            }
            case 2: {
                text = "b";
                break;
            }
            case 3: {
                text = "f sharp";
                break;
            }
            case 4: {
                text = "c sharp";
                break;
            }
            case 5: {
                text = "g sharp";
                break;
            }
            case 6: {
                text = "d sharp";
                break;
            }
            case -1: {
                text = "d";
                break;
            }
            case -2: {
                text = "g";
                break;
            }
            case -3: {
                text = "c";
                break;
            }
            case -4: {
                text = "f";
                break;
            }
            case -5: {
                text = "b flat";
                break;
            }
            case -6: {
                text = "e flat";
                break;
            }
        }
        text += " minor";
    }
    return text;
}
