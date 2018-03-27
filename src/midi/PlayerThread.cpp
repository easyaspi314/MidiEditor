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

#include "PlayerThread.h"
#include "MidiOutput.h"
#include <QMultiMap>
#include "MidiFile.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/KeySignatureEvent.h"
#include "MidiInput.h"
#include <QTime>
#include "MidiPlayer.h"

#define INTERVAL_TIME 15
#define TIMEOUTS_PER_SIGNAL 1

PlayerThread::PlayerThread() : QThread() {

    moveToThread(this);
    interval = INTERVAL_TIME;
    file = qnullptr;
    timer = qnullptr;
    timeoutSinceLastSignal = 0;
    time = qnullptr;

}

void PlayerThread::setFile(MidiFile *f) {
    file = f;
}

void PlayerThread::stop() {
    stopped = true;
}

void PlayerThread::setInterval(int i) {
    interval = i;
}

void PlayerThread::run() {
    if (currentThread() != this) {
        qWarning("Wrong thread!");
    }
    if (!timer) {
        timer = new QTimer(this);
    }
    if (time) {
        delete time;
        time = qnullptr;
    }

    events = file->playerData();

    if (file->pauseTick() >= 0) {
        position = file->msOfTick(file->pauseTick());
    } else {
        position = file->msOfTick(file->cursorTick());
    }

    emit playerStarted();

    // Reset all Controllers
    for (ubyte i = 0; i < 16; i++) {
        QByteArray array;
        append(array, 0xB0 | i);
        append(array, 121);
        append(array, 0);
        MidiOutput::instance()->sendCommand(array);
    }
    MidiOutput::instance()->playedNotes.clear();

    // All Events before position should be sent, progChanges and ControlChanges
    QMultiMap<int, MidiEvent*>::const_iterator it = events->constBegin();
    while (it != events->constEnd()) {
        if (it.key() >= position) {
            break;
        }
        MidiOutput::instance()->sendCommand(it.value());
        ++it;
    }

    connect(timer, &QTimer::timeout, this, &PlayerThread::timeout);
    timer->start(interval);

    stopped = false;

    QList<TimeSignatureEvent*> *list = qnullptr;

    int tickInMeasure = 0;
    measure = file->measure(file->cursorTick(), file->cursorTick(), &list, &tickInMeasure);
    emit measureChanged(measure, tickInMeasure);

    if (exec() == 0) {
        timer->stop();
        emit playerStopped();
    }
}

void PlayerThread::timeout() {
    if (!timer) {
        timer = new QTimer(this);
        timer->setInterval(interval);

    }
    // Avoid running twice.
    timer->blockSignals(true);
    if (!time) {
        time = new QTime();
        time->start();
    }

    if (stopped) {
        disconnect(timer, &QTimer::timeout, this, &PlayerThread::timeout);
        // AllNotesOff // All SoundsOff
        for (ubyte i = 0; i < 16; i++) {
            // value (third number) should be 0, but doesnt work
            QByteArray array;
            append(array, 0xB0 | i);
            append(array, 123);
            append(array, 127);
            MidiOutput::instance()->sendCommand(array);
        }
        if (_settings.alt_stop) {
            /*foreach(int channel, MidiOutput::instance()->playedNotes.keys()) {
                foreach(int note, MidiOutput::instance()->playedNotes.value(channel)) {
                    QByteArray array;
                    array.append(0x80 | byte(channel));
                    append(array, note);
                    array.append(char(0));
                    MidiOutput::instance()->sendCommand(array);
                }
            }*/
            const QMap<ubyte, QList<ubyte> > map = MidiOutput::instance()->playedNotes;
            QMap<ubyte, QList<ubyte> >::const_iterator i;
            QList<ubyte>::const_iterator j;
            // I *think* this could just be a standard iterator...
            for (i = map.constBegin(); i != map.constEnd(); ++i) {
                for (j = i.value().constBegin(); j != i.value().constEnd(); ++j) {
                    QByteArray array;
                    append(array, 0x80 | ubyte(i.key()));
                    append(array, *j);
                    append(array, 0);
                    MidiOutput::instance()->sendCommand(array);
                }
            }
        }
        quit();

    } else {

        int newPos = qRound(position + time->elapsed()*MidiPlayer::instance()->speedScale());
        int tick = file->tick(newPos);
        QList<TimeSignatureEvent*> *list = qnullptr;
        int ickInMeasure = 0;

        int new_measure = file->measure(tick, tick, &list, &ickInMeasure);
        delete list;
        // compute current pos

        if (new_measure > measure) {
            emit measureChanged(new_measure, ickInMeasure);
            measure = new_measure;
        }
        time->restart();
        QMultiMap<int, MidiEvent*>::iterator it = events->lowerBound(position);

        while(it != events->end() && it.key() < newPos) {

            // save events for the given tick
            QList<MidiEvent*> onEv, offEv;
            const int sendPosition = it.key();

            do {
                if (it.value()->isOnEvent()) {
                    onEv.append(it.value());
                } else {
                    offEv.append(it.value());
                }
                ++it;
            } while(it != events->end() && it.key() == sendPosition);

            for (MidiEvent *ev : offEv) {
                MidiOutput::instance()->sendCommand(ev);
            }
            for (MidiEvent *ev : onEv) {
                if (ev->line() == MidiEventLine::KeySignatureEventLine) {
                    KeySignatureEvent *keySig = protocol_cast<KeySignatureEvent*>(ev);
                    if (keySig) {
                        emit tonalityChanged(keySig->tonality());
                    }
                }
                if (ev->line() == MidiEventLine::TimeSignatureEventLine) {
                    TimeSignatureEvent *timeSig = protocol_cast<TimeSignatureEvent*>(ev);
                    if (timeSig) {
                        emit meterChanged(timeSig->num(), timeSig->denom());
                    }
                }
                MidiOutput::instance()->sendCommand(ev);
            }

            //MidiOutput::sendCommand(it.value());
            //it++;
        }

        // end if it was last event, but only if not recording
        if (it == events->end() && !MidiInput::instance()->recording()) {
            stop();
        }
        position = newPos;
        timeoutSinceLastSignal++;
        MidiInput::instance()->setTime(position);
        if (timeoutSinceLastSignal == TIMEOUTS_PER_SIGNAL) {
            emit timeMsChanged(position,false);
            emit measureUpdate(measure, ickInMeasure);
            timeoutSinceLastSignal = 0;
        }
    }
    timer->blockSignals(false);
}
int PlayerThread::timeMs() {
    return position;
}
