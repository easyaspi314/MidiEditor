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

#ifndef MIDIPLAYER_H_
#define MIDIPLAYER_H_

#include <QObject>

class MidiFile;
class NoteOnEvent;
class PlayerThread;
class SingleNotePlayer;

class MidiPlayer : public QObject {

	Q_OBJECT

	public:
		static MidiPlayer *instance();

		bool isPlaying();
		int timeMs();
		qreal speedScale();
		void setSpeedScale(double d);
		static int playbackDelay();
		static void setPlaybackDelay(int delay);
		static PlayerThread *player();


	public slots:
		void play(MidiFile *file);
		void play(NoteOnEvent *event);
		void stop();

		/**
		 * Send all Notes off to every channel.
		 */
		void panic();

	private:
		MidiPlayer();
		static MidiPlayer *createInstance();
		bool playing;
		int _playbackDelay;
		qreal _speed;
		static PlayerThread *_playerThread;
};

#endif
