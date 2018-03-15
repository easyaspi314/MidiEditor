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

#ifndef TIMESIGNATUREEVENT_H_
#define TIMESIGNATUREEVENT_H_

#include "MidiEvent.h"

class TimeSignatureEvent : public MidiEvent {

    Q_OBJECT

    public:
        TimeSignatureEvent(ubyte channel, ubyte num, ubyte denom, ubyte midiClocks,
                ubyte num32In4, MidiTrack *track);
        TimeSignatureEvent(const TimeSignatureEvent &other);
        EventType type() const qoverride;

        ubyte num();
        ubyte denom();
        ubyte midiClocks();
        ubyte num32In4();
        int measures(int tick, int *ticksLeft = qnullptr);
        int ticksPerMeasure();

        virtual ProtocolEntry *copy() qoverride;
        virtual void reloadState(ProtocolEntry *entry) qoverride;
        ubyte line() qoverride;
        const QByteArray save() qoverride;

        void setDenominator(ubyte d);
        void setNumerator(ubyte n);

        const QString typeString() qoverride;

    private:
        ubyte numerator, denominator, midiClocksPerMetronome, num32In4th;
};

#endif
