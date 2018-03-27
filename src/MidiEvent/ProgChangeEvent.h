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

#ifndef PROGCHANGEEVENT_H_
#define PROGCHANGEEVENT_H_

#include "MidiEvent.h"

class ProgChangeEvent : public MidiEvent {


    public:
        ProgChangeEvent(ubyte channel, ubyte prog, MidiTrack *track);
        ProgChangeEvent(const ProgChangeEvent &other);
        int type() const qoverride;
        enum {
            Type = ProgramChangeEventType
        };
        ubyte line() qoverride;

        const QString toMessage() qoverride;
        const QByteArray save() qoverride;

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;

        const QString typeString() qoverride;

        ubyte program();
        void setProgram(ubyte prog);

    private:
        ubyte _program;
};

#endif
