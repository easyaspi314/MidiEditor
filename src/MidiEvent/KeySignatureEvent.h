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

#ifndef KEYSIGNATUREEVENT_H_
#define KEYSIGNATUREEVENT_H_

#include "MidiEvent.h"

class KeySignatureEvent  : public MidiEvent {

    Q_OBJECT

    public:
        KeySignatureEvent(ubyte channel, ubyte tonality, bool minor, MidiTrack *track);
        KeySignatureEvent(const KeySignatureEvent &other);
        EventType type() const qoverride;

        virtual ubyte line() qoverride;

        const QString toMessage() qoverride;
        const QByteArray save() qoverride;

        virtual ProtocolEntry *copy() qoverride;
        virtual void reloadState(ProtocolEntry *entry) qoverride;

        const QString typeString() qoverride;

        ubyte tonality();
        bool minor();
        void setTonality(ubyte t);
        void setMinor(bool minor);

        static QString toString(ubyte tonality, bool minor);

    private:
        ubyte _tonality;
        bool _minor;
};

#endif
