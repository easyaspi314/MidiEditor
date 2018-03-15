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

#ifndef MIDIFILE_H_
#define MIDIFILE_H_

#include "../protocol/ProtocolEntry.h"

#include <QDataStream>
#include <QMultiMap>
#include <QObject>
#include <QList>

#include "../Utils.h"

class MidiEvent;
class TimeSignatureEvent;
class TempoChangeEvent;
class Protocol;
class MidiChannel;
class MidiTrack;

class MidiFile : public ProtocolEntry {

    Q_OBJECT

    public:
        MidiFile(const QString &path, bool *ok, QStringList *log = qnullptr);
        MidiFile();
        // needed to protocol fileLength
        MidiFile(int maxTime, Protocol *p);
        bool save(const QString &path);
        QByteArray writeDeltaTime(int time);
        int maxTime();
        int endTick();
        int timeMS(int midiTime);
        int measure(int midiTime, int &midiTimeInMeasure);
        QMap<int, MidiEvent*> *tempoEvents();
        QMap<int, MidiEvent*> *timeSignatureEvents();
        void calcMaxTime();
        int tick(int ms);
        int tick(int startms, int endms, QList<MidiEvent*> **events, int *endTick, int *msOfFirstEvent);
        int measure(int startTick, int endTick, QList<TimeSignatureEvent*> **eventList, int *tickInMeasure = qnullptr);
        int msOfTick(int tick, QList<MidiEvent*> *events = qnullptr, int msOfFirstEventInList = 0);

        QList<MidiEvent*> *eventsBetween(int start, int end);
        int ticksPerQuarter();
        QMultiMap<int, MidiEvent*> *channelEvents(ubyte channel);

        Protocol *protocol();
        MidiChannel *channel(ubyte i);
        void preparePlayerData(int tickFrom);
        QMultiMap<int, MidiEvent*> *playerData();

        static const QString instrumentName(ubyte prog);
        static const QString controlChangeName(ubyte control);
        int cursorTick();
        int pauseTick();
        void setCursorTick(int tick);
        void setPauseTick(int tick);
        QString path();
        bool modified();

        void setPath(const QString &path);
        bool channelMuted(ubyte ch);
        ushort numTracks();
        const QList<MidiTrack *> *tracks() const;
        void addTrack();
        void setMaxLengthMs(int ms);

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;
        MidiFile *file() qoverride;
        bool removeTrack(MidiTrack *track);
        MidiTrack *track(ushort number);

        ubyte tonalityAt(int tick);
        void meterAt(int tick, int *num, int *denum);

        static int variableLengthvalue(QDataStream *content);
        static QByteArray writeVariableLengthValue(int value);

        void registerCopiedTrack(MidiTrack *source, MidiTrack *destination, MidiFile *fileFrom);
        MidiTrack *getPasteTrack(MidiTrack *source, MidiFile *fileFrom);

        QList<int> quantization(int fractionSize);

        void insertEventInChannel(ubyte mChannel, MidiEvent *event, int tick);

    public slots:
        void setModified(bool b);

    signals:
        void cursorPositionChanged();
        void recalcWidgetSize();
        void trackChanged();

    private:
        bool readMidiFile(QDataStream *content, QStringList *log);
        bool readTrack(QDataStream *content, ushort num, QStringList *log);
        void printLog(QStringList *log);
        QList<MidiChannel *> channels;

        QString _path;
        Protocol *prot;
        QMultiMap<int, MidiEvent*> *playerMap;

        QList<MidiTrack*> *_tracks;
        QHash<MidiFile*, QHash<MidiTrack*, MidiTrack*> > pasteTracks;
        int midiTicks, maxTimeMS, _cursorTick, _pauseTick;

        short timePerQuarter;

        #ifdef NO_BIT_PACK
            ubyte _midiFormat;
            ubyte _modified;
        #else
            ubyte _midiFormat : 3;
            bool _modified : 1;
        #endif
};

#endif
