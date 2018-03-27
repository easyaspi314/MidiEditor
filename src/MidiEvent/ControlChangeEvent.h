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

#ifndef CONTROLCHANGEEVENT_H_
#define CONTROLCHANGEEVENT_H_

#include "MidiEvent.h"

class ControlChangeEvent : public MidiEvent {



    public:
        ControlChangeEvent(ubyte channel, ubyte contr, ubyte val, MidiTrack *track);
        ControlChangeEvent(const ControlChangeEvent &other);

        int type() const qoverride;
        enum {
            Type = ControlChangeEventType
        };

        ubyte line() qoverride;
        ubyte control();
        ubyte value();
        void setValue(ubyte v);
        void setControl(ubyte c);

        const QString toMessage() qoverride;
        const QByteArray play() qoverride;
        const QByteArray save() qoverride;

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;

        const QString typeString() qoverride;

        bool isOnEvent() qoverride;
    private:
        ubyte _control, _value;

};

#endif
