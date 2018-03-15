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

#include "TimeSignatureEvent.h"
#include <QtMath>
#include "../midi/MidiFile.h"

TimeSignatureEvent::TimeSignatureEvent(ubyte channel, ubyte num, ubyte denom,
        ubyte midiClocks, ubyte num32In4, MidiTrack *track) : MidiEvent(channel, track)
{
    numerator = num;
    denominator = denom;
    midiClocksPerMetronome = midiClocks;
    num32In4th = num32In4;
}

TimeSignatureEvent::TimeSignatureEvent(const TimeSignatureEvent &other):
        MidiEvent(other)
{
    numerator = other.numerator;
    denominator = other.denominator;
    midiClocksPerMetronome = other.midiClocksPerMetronome;
    num32In4th = other.num32In4th;
}

EventType TimeSignatureEvent::type() const {
    return TimeSignatureEventType;
}
ubyte TimeSignatureEvent::num(){
    return numerator;
}

ubyte TimeSignatureEvent::denom(){
    return denominator;
}

ubyte TimeSignatureEvent::midiClocks(){
    return midiClocksPerMetronome;
}

ubyte TimeSignatureEvent::num32In4(){
    return num32In4th;
}

int TimeSignatureEvent::ticksPerMeasure(){
    return (4*numerator*file()->ticksPerQuarter())/qPow(2, denominator);
}

int TimeSignatureEvent::measures(int ticks, int *ticksLeft){
    //int numTicks = tick-midiTime();
    int tpm = ticksPerMeasure();
    if (tpm == 0) {
        qWarning("ticksPerMeasure is 0. We can't divide by zero now, can we?");
        return 0;
    }
    if (ticksLeft) {
        *ticksLeft = ticks % tpm;
    }

    return ticks/tpm;
}

ProtocolEntry *TimeSignatureEvent::copy(){
    return new TimeSignatureEvent(*this);
}

void TimeSignatureEvent::reloadState(ProtocolEntry *entry){
    TimeSignatureEvent *other = qobject_cast<TimeSignatureEvent*>(entry);
    if(!other){
        return;
    }
    MidiEvent::reloadState(entry);
    numerator = other->numerator;
    denominator = other->denominator;
    midiClocksPerMetronome = other->midiClocksPerMetronome;
    num32In4th = other->num32In4th;
}
ubyte TimeSignatureEvent::line(){
    return TimeSignatureEventLine;
}

void TimeSignatureEvent::setNumerator(ubyte n){
    numerator = n;
    protocol(copy(), this);
}

void TimeSignatureEvent::setDenominator(ubyte d){
    denominator = d;
    protocol(copy(), this);
}

const QByteArray TimeSignatureEvent::save(){
    QByteArray array = QByteArray();
    append(array, 0xFF);
    append(array, 0x58);
    append(array, 0x04);
    append(array, numerator);
    append(array, denominator);
    append(array, midiClocksPerMetronome);
    append(array, num32In4th);
    return array;
}

const QString TimeSignatureEvent::typeString(){
    return "Time Signature Event";
}
