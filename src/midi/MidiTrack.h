/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.+
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MIDITRACK_H_
#define MIDITRACK_H_

#include "../protocol/ProtocolEntry.h"

#include <QObject>
#include <QString>
#include "../Utils.h"

class TextEvent;
class MidiFile;
class QColor;

class MidiTrack : public ProtocolEntry {

    Q_OBJECT

    public:

        MidiTrack(MidiFile *file);
        MidiTrack(const MidiTrack &other);
        virtual ~MidiTrack() qoverride;

        QString name();
        void setName(const QString &name);

        ushort number();
        void setNumber(ushort number);

        void setNameEvent(TextEvent *nameEvent);
        TextEvent *nameEvent();

        MidiFile *file() qoverride;

        void assignChannel(ubyte ch);
        ubyte assignedChannel();

        void setHidden(bool hidden);
        bool hidden();

        void setMuted(bool muted);
        bool muted();

        virtual ProtocolEntry *copy() qoverride;
        virtual void reloadState(ProtocolEntry *entry) qoverride;

        QColor *color();

        MidiTrack *copyToFile(MidiFile *file);

    signals:
        void trackChanged();

    private:
        TextEvent *_nameEvent;
        MidiFile *_file;
        QColor *_color;

        ushort _number;

        #ifdef NO_BIT_PACK
            ubyte _assignedChannel;
            bool _hidden;
            bool _muted;
        #else
            ubyte _assignedChannel : 5;
            bool _hidden : 1;
            bool _muted : 1;
        #endif

};

#endif
