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

#ifndef MIDIINPUT_H_
#define MIDIINPUT_H_

#include <QObject>
#include <QProcess>
#include <QList>
#include <QMultiMap>

#include <vector>

#include "../Utils.h"

class MidiEvent;
class RtMidiIn;
class RtMidiOut;
class QStringList;
class MidiTrack;

class MidiInput : public QObject {

    Q_OBJECT

    public:
        static MidiInput *instance();

        void sendCommand(const QByteArray &array);
        void sendCommand(MidiEvent *e);

        const QStringList inputPorts();
        bool setInputPort(const QString &name);
        QString inputPort();

        void startInput();
        QMultiMap<int, MidiEvent*> endInput(MidiTrack *track);

        static void receiveMessage(double deltatime,
                    std::vector<ubyte> *message, void *userData = qnullptr);

        void setTime(int ms);

        bool recording();


    public slots:
        void init();

    private:
        const QList<int> toUnique(const QList<int> &in);
        MidiInput();
        static MidiInput *createInstance();

        QString _inPort;
        RtMidiIn *_midiIn;
        QMultiMap<int, std::vector<ubyte> > *_messages;
        int _currentTime;
        bool _recording;

};

#endif
