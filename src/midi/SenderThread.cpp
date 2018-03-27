// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include "rtmidi/RtMidi.h"
#include <QApplication>

#include <QTimer>

// Credit to http://thesmithfam.org/blog/2010/02/07/talking-to-qt-threads/
SenderThread::SenderThread() {
    moveToThread(this);

}
void SenderThread::run() {
    if (currentThread() == this) {
            qWarning("run on sender thread");
        }
    if (!_output) {
        _output = MidiOutput::instance();
    }
    if (!_eventQueue || !_noteQueue) {
        _eventQueue = new ReaderWriterQueue<std::vector<ubyte>* >(100);
        _noteQueue = new ReaderWriterQueue<std::vector<ubyte>* >(100);
    }
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &SenderThread::sendCommands);
    timer->start(1);
    exec();
}
void SenderThread::sendCommands() {
    if (!timer) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &SenderThread::sendCommands);
        timer->start(1);
    }
    timer->blockSignals(true);
    std::vector<ubyte> *event;
    while(_eventQueue->try_dequeue(event)) {
        // send command
        _output->sendRawCommand(event);
    }
    // Now send the note events.
    while(_noteQueue->try_dequeue(event)) {
        // send command
        _output->sendRawCommand(event);
    }
    timer->blockSignals(false);
}
void SenderThread::stop() {
    if (currentThread() != this) {
        QMetaObject::invokeMethod(this, "stop", Qt::QueuedConnection);

    } else {
        if (_noteQueue && _eventQueue) {
            delete _noteQueue;
            delete _eventQueue;
            _noteQueue = new ReaderWriterQueue<std::vector<ubyte>* >(100);
            _eventQueue = new ReaderWriterQueue<std::vector<ubyte>* >(100);

        }
        if (timer) {
            timer->stop();
            disconnect(timer, &QTimer::timeout, this, &SenderThread::sendCommands);
        }
        quit();
    }
}
void SenderThread::initQueue() {
    if (currentThread() == this) {
            qWarning("initqueue on sender thread");
        }
    _noteQueue = new ReaderWriterQueue<std::vector<ubyte>* >(100);
    _eventQueue = new ReaderWriterQueue<std::vector<ubyte>* >(100);
}

void SenderThread::enqueue(MidiEvent *event) {
    if (currentThread() == QApplication::instance()->thread()) {
        qWarning("on ui thread");

    } else if (currentThread() == this) {
        qWarning("on sender thread");
    } else {
        qWarning("On thread %p", currentThreadId());
    }

    if (!isRunning()) {
        qWarning("Running thread from enqueue");
        start(QThread::TimeCriticalPriority);
    }
    if (!_noteQueue || !_eventQueue) {
        qWarning("Trying to use an uninitialized queue!");
        if (currentThread() != this) {
            QMetaObject::invokeMethod(this, "initQueue", Qt::QueuedConnection);

        } else {
            initQueue();
        }
        return;
    }
    const QByteArray qArray = event->play();
    try {
        std::vector<ubyte> *array = new std::vector<ubyte>(qArray.constBegin(), qArray.constEnd());
        // If it is a NoteOnEvent or an OffEvent, we put it in _noteQueue.
        if (event->type() == MidiEvent::NoteOnEventType || event->type() == MidiEvent::OffEventType) {
            _noteQueue->try_enqueue(array);
            // Otherwise, it goes into _eventQueue.
        } else {
            _eventQueue->try_enqueue(array);
        }

    } catch (std::out_of_range &e) {
        qWarning("%s", e.what());
    }

}
