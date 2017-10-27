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

#include "SenderThread.h"

#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "MidiPlayer.h"

#include <QTimer>

// Credit to http://thesmithfam.org/blog/2010/02/07/talking-to-qt-threads/
SenderThread::SenderThread() {
	moveToThread(this);
	// We actually call this on the main thread; despite being called
	// after moveToThread().
	if (!_eventQueue || !_noteQueue) {
			_eventQueue = new AtomicQueue<QByteArray>();
			_noteQueue = new AtomicQueue<QByteArray>();
		}
}
void SenderThread::run() {
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(sendCommands()));
	timer->start(1);
	exec();
}
void SenderThread::sendCommands() {
	if (!timer) {
		timer = new QTimer(this);
		connect(timer, SIGNAL(timeout()), this, SLOT(sendCommands()));
		timer->start(1);
	}
	timer->blockSignals(true);
	QByteArray event;
	if (!_eventQueue || !_noteQueue) {
		qWarning("sendCommands: Event queue or note queue is null.");
		return;
	}
	while(_eventQueue->pop(event)){
		// send command
		MidiOutput::instance()->sendEnqueuedCommand(event);
	}
	// Now send the note events.
	while(_noteQueue->pop(event)){
		// send command
		MidiOutput::instance()->sendEnqueuedCommand(event);
	}
	timer->blockSignals(false);
}
void SenderThread::stop() {
	if(currentThread() != this) {
		QMetaObject::invokeMethod(this, "stop",
						Qt::QueuedConnection);
	} else {
		// TODO: I think this would theoretically crash.
		if (_noteQueue && _eventQueue) {
			delete _noteQueue;
			delete _eventQueue;
			_noteQueue = new AtomicQueue<QByteArray>();
			_eventQueue = new AtomicQueue<QByteArray>();
		}
		if (timer) {
			timer->stop();
			disconnect(timer, SIGNAL(timeout()), this, SIGNAL(sendCommands()));
		}
		quit();
	}
}
void SenderThread::initQueue() {

}
void SenderThread::enqueue(MidiEvent *event){
	if (!isRunning()){
		start(QThread::TimeCriticalPriority);
	}
	if (!_noteQueue || !_eventQueue) {
		_noteQueue = new AtomicQueue<QByteArray>();
		_eventQueue = new AtomicQueue<QByteArray>();
	}
	// If it is a NoteOnEvent or an OffEvent, we put it in _noteQueue.
	if (event->type() == MidiEvent::NoteOnEventType || event->type() == MidiEvent::OffEventType) {
		_noteQueue->push(event->save());
	// Otherwise, it goes into _eventQueue.
	} else {
		_eventQueue->push(event->save());
	}
}
