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

#ifndef TEXTEVENT_H_
#define TEXTEVENT_H_

#include "MidiEvent.h"
#include <QByteArray>

enum TextType {
    TextTextEventType = 0x01,
    CopyrightTextEventType,
    TrackNameTextEventType,
    InstrumentTextEventType,
    LyricTextEventType,
    MarkerTextEventType,
    CommentTextEventType
};
class TextEvent : public MidiEvent {



    public:
        TextEvent(ubyte channel, MidiTrack *track, TextType type = TextType::TextTextEventType, const QString &text = QString());
        TextEvent(const TextEvent &other);
        int type() const qoverride;
        enum {
            Type = TextEventType
        };
        const QString &text();
        void setText(const QString &text);

        TextType textType();
        void setTextType(TextType type);

        ubyte line() qoverride;

        const QByteArray save() qoverride;

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;

        const QString typeString() qoverride;
        static const QString textTypeString(ubyte type);

    private:
        QString _text;
        TextType _type;
};

#endif
