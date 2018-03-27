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

#ifndef KEYPRESSUREEVENT_H_
#define KEYPRESSUREEVENT_H_

#include "MidiEvent.h"

#include <QSpinBox>
#include <QWidget>
#include <QLabel>

class KeyPressureEvent : public MidiEvent {



    public:
        KeyPressureEvent(ubyte channel, ubyte value, ubyte note, MidiTrack *track);
        KeyPressureEvent(const KeyPressureEvent &other);
        int type() const qoverride;
        enum {
            Type = KeyPressureEventType
        };
        ubyte line() qoverride;

        const QString toMessage() qoverride;
        const QByteArray save() qoverride;

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;

        const QString typeString() qoverride;

        ubyte value();
        ubyte note();
        void setValue(ubyte v);
        void setNote(ubyte n);

    private:
        ubyte _value, _note;
};

#endif
