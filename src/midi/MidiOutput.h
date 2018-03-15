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

#ifndef MIDIOUTPUT_H_
#define MIDIOUTPUT_H_

#include <QObject>
#include <QList>
#include <QMap>
#include "../Utils.h"

class MidiEvent;
class RtMidiIn;
class RtMidiOut;
class QStringList;
class SenderThread;

class MidiOutput : public QObject {

    Q_OBJECT

    public:
        static MidiOutput *instance();

        void sendCommand(const QByteArray &array);
        void sendCommand(MidiEvent *e);
        const QStringList outputPorts();
        bool setOutputPort(const QString &name);
        QString outputPort();
        void sendEnqueuedCommand(const QByteArray &array);
        void sendRawCommand(std::vector<ubyte> *message);
        QMap<ubyte, QList<ubyte> > playedNotes;
        void setStandardChannel(ubyte channel);
        ubyte standardChannel();
        void sendProgram(ubyte channel, ubyte prog);
        static SenderThread *sender();

    public slots:
        void init();

    private:
        MidiOutput();
        static MidiOutput *createInstance();
        static SenderThread *_sender;

        RtMidiOut *_midiOut;
        QString _outPort;
        ubyte _stdChannel;

};

#endif
