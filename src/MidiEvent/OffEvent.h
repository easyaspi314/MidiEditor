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

#ifndef OFFEVENT_H_
#define OFFEVENT_H_

#include "MidiEvent.h"
#include <QMultiMap>
#include <QList>

class OnEvent;

class OffEvent : public MidiEvent{

    Q_OBJECT

    public:
        OffEvent(ubyte ch, ubyte line, MidiTrack *track);
        OffEvent(const OffEvent &other);
        EventType type() const qoverride;

        void setOnEvent(OnEvent *event);
        OnEvent *onEvent();

        static void enterOnEvent(OnEvent *event);
        static void clearOnEvents();
        static void removeOnEvent(OnEvent *event);
        static const QList<OnEvent*> corruptedOnEvents();
        void draw(QPainter *p, QColor c) qoverride;
        ubyte line() qoverride;
        const QByteArray save() qoverride;
        const QString toMessage() qoverride;

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;

        void setMidiTime(int t, bool toProtocol = true) qoverride;

        virtual bool isOnEvent() qoverride;
    protected:
        OnEvent *_onEvent;
};

#endif
