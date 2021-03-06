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

#ifndef SENDERTHREAD_H_
#define SENDERTHREAD_H_

#include "../AtomicQueue.h"
#include <QThread>
#include <QTimer>

#include "MidiOutput.h"

class SenderThread : public QThread {

	Q_OBJECT

	public:
		SenderThread();
		void run() Q_DECL_OVERRIDE;
		void enqueue(MidiEvent *event);


	public slots:
		void stop();
		void initQueue();
		void sendCommands();
	protected:
	private:
		AtomicQueue<QByteArray> *_eventQueue;
		AtomicQueue<QByteArray> *_noteQueue;
		QTimer *timer;
};

#endif
