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

#ifndef NOTEONEVENT_H_
#define NOTEONEVENT_H_

#include "OnEvent.h"

class OffEvent;

class NoteOnEvent : public OnEvent{

    Q_OBJECT

    public:
        NoteOnEvent(ubyte note, ubyte velocity, ubyte ch, MidiTrack *track);
        NoteOnEvent(const NoteOnEvent &other);
        EventType type() const qoverride;

        ubyte note();
        ubyte velocity();
        ubyte line() qoverride;

        void setNote(ubyte n);
        void setVelocity(ubyte v);
        virtual ProtocolEntry *copy() qoverride;
        virtual void reloadState(ProtocolEntry *entry) qoverride;
        const QString toMessage() qoverride;
        const QString offEventMessage() qoverride;
        const QByteArray save() qoverride;
        const QByteArray play() qoverride;
        const QByteArray saveOffEvent() qoverride;

        const QString typeString() qoverride;

    protected:
        ubyte _note, _velocity;
};

#endif
