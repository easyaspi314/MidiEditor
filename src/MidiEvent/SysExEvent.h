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

#ifndef SYSEXEVENT_H
#define SYSEXEVENT_H

#include "MidiEvent.h"
#include <QByteArray>

class SysExEvent : public MidiEvent{

    Q_OBJECT

    public:
        SysExEvent(ubyte channel, const QByteArray &data, MidiTrack *track);
        SysExEvent(const SysExEvent &other);
        EventType type() const qoverride;

        QByteArray data();
        ubyte line() qoverride;
        const QByteArray save() qoverride;

        const QString typeString() qoverride;
        ProtocolEntry *copy() qoverride;
        virtual void reloadState(ProtocolEntry *entry) qoverride;

        void setData(const QByteArray &d);

    private:
        QByteArray _data;

};

#endif
