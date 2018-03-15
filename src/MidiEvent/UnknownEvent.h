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

#ifndef UNKNOWNEVENT_H_
#define UNKNOWNEVENT_H_

#include "MidiEvent.h"
#include <QByteArray>

class UnknownEvent : public MidiEvent {

    Q_OBJECT

    public:
        UnknownEvent(ubyte channel, ubyte type, QByteArray data, MidiTrack *track);
        UnknownEvent(const UnknownEvent &other);
        EventType type() const qoverride;

        QByteArray data();
        ubyte line() qoverride;
        const QByteArray save() qoverride;
        ubyte unknownType();
        void setUnknownType(ubyte type);
        void setData(const QByteArray &d);

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;

    private:
        QByteArray _data;
        ubyte _type;

};

#endif
