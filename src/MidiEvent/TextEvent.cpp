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

#include "TextEvent.h"

#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"

TextEvent::TextEvent(ubyte channel, MidiTrack *track, TextType type, const QString &text) : MidiEvent(channel, track) {
    _type = type;
    _text = text;
}

TextEvent::TextEvent(const TextEvent &other) : MidiEvent(other) {
    _type = other._type;
    _text = other._text;
}

int TextEvent::type() const {
    return Type;
}

const QString &TextEvent::text(){
    return _text;
}

void TextEvent::setText(const QString &text){
    _text = text;
    protocol(copy(), this);
}

TextType TextEvent::textType(){
    return _type;
}

void TextEvent::setTextType(TextType type){
    _type = type;
    protocol(copy(), this);
}

ubyte TextEvent::line(){
    return TextEventLine;
}

const QByteArray TextEvent::save(){
    QByteArray array;
    QByteArray textArray = _text.toUtf8();

    append(array, 0xFF);
    append(array, _type);
    array.append(MidiFile::writeVariableLengthValue(textArray.size()));
    array.append(textArray);

    return array;
}

const QString TextEvent::typeString(){
    return "Text Event";
}

ProtocolEntry *TextEvent::copy(){
    return new TextEvent(*this);
}

void TextEvent::reloadState(ProtocolEntry *entry){
    TextEvent *other = protocol_cast<TextEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    _text = other->_text;
    _type = other->_type;
}

const QString TextEvent::textTypeString(ubyte type){
    switch(TextType(type)){
        case TextTextEventType: return "General text";
        case CopyrightTextEventType: return "Copyright";
        case TrackNameTextEventType: return "Trackname";
        case InstrumentTextEventType: return "Instrument name";
        case LyricTextEventType: return "Lyric";
        case MarkerTextEventType: return "Marker";
        case CommentTextEventType: return "Comment";
    }
    return QString();
}
