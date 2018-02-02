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

class MidiEvent;
class RtMidiIn;
class RtMidiOut;
class QStringList;
class SenderThread;

class MidiOutput : public QObject {

	Q_OBJECT

	public:
		static MidiOutput *instance();

		void sendCommand(QByteArray array);
		void sendCommand(MidiEvent *e);
		QStringList outputPorts();
		bool setOutputPort(QString name);
		QString outputPort();
		void sendEnqueuedCommand(QByteArray array);
		static bool isAlternativePlayer();
		static void setAlternativePlayer(bool enable);
		QMap<int, QList<int> > playedNotes;
		void setStandardChannel(int channel);
		int standardChannel();
		void sendProgram(int channel, int prog);
		static SenderThread *sender();
		static bool isGBAMode();
		static void setGBAMode(bool enable);

	public slots:
		void init();

	private:
		MidiOutput();
		static MidiOutput *createInstance();
		bool _gbaMode;
		QString _outPort;
		RtMidiOut *_midiOut;
		static SenderThread *_sender;
		int _stdChannel;
		bool _alternativePlayer;
};

#endif
