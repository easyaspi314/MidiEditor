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

#ifndef MIDIEVENT_H_
#define MIDIEVENT_H_

#include <QDataStream>
#include <QColor>
#include "../gui/GraphicObject.h"
#include "../protocol/ProtocolEntry.h"
#include <QWidget>

#include "../Utils.h"

enum MidiEventLine {
    TempoChangeEventLine = 128,
    TimeSignatureEventLine,
    KeySignatureEventLine,
    ProgramChangeEventLine,
    ControlChangeEventLine,
    KeyPressureEventLine,
    ChannelPressureEventLine,
    TextEventLine,
    PitchBendEventLine,
    SysExEventLine,
    UnknownEventLine
};

enum EventType {
    MidiEventType,
    ChannelPressureEventType,
    ControlChangeEventType,
    KeyPressureEventType,
    KeySignatureEventType,
    NoteOnEventType,
    OffEventType,
    OnEventType,
    PitchBendEventType,
    ProgramChangeEventType,
    SystemExclusiveEventType,
    TempoChangeEventType,
    TextEventType,
    TimeSignatureEventType,
    UnknownEventType
};
class MidiFile;
class QSpinBox;
class QLabel;
class QWidget;
class EventWidget;
class MidiTrack;

class MidiEvent : public ProtocolEntry, public GraphicObject {

        Q_OBJECT

    public:


        MidiEvent(ubyte channel, MidiTrack *track);
        MidiEvent(const MidiEvent &other);
        virtual EventType type() const;

        static MidiEvent *loadMidiEvent(QDataStream *content,
                                        bool *ok, bool *endEvent, MidiTrack *track, ubyte startByte = 0,
                                        ubyte secondByte = 0);

        static EventWidget *eventWidget();
        static void setEventWidget(EventWidget *widget);



        void setTrack(MidiTrack *track, bool toProtocol = true);
        MidiTrack *track();
        void setChannel(ubyte ch, bool toProtocol = true);
        ubyte channel();
        virtual void setMidiTime(int t, bool toProtocol = true);
        int midiTime();
        void setFile(MidiFile *f);
        MidiFile *file() qoverride;
        bool shownInEventWidget();

        virtual ubyte line();
        virtual const QString toMessage();
        virtual const QByteArray play();
        virtual const QByteArray save();
        virtual void draw(QPainter *p, QColor c) qoverride;

        virtual ProtocolEntry *copy() qoverride;
        virtual void reloadState(ProtocolEntry *entry) qoverride;

        virtual const QString typeString();

        virtual bool isOnEvent();

        static QMap<int, QString> knownMetaTypes();

        void setTemporaryRecordID(int id);
        int temporaryRecordID();

        virtual void moveToChannel(ubyte channel);

    protected:

        MidiFile *midiFile;
        static EventWidget *_eventWidget;
        MidiTrack *_track;
        int _tempID;
        int timePos;

        ushort numTrack;

        static ubyte _startByte;
        ubyte numChannel;
        ubyte _line;

};

#endif
