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

	file = Q_NULLPTR;
	timer = Q_NULLPTR;
	timeoutSinceLastSignal = 0;
	time = Q_NULLPTR;

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

	if(!timer){
		timer = new QTimer(this);
	}
	if(time){
		delete time;
		time = Q_NULLPTR;
	}

	events = file->playerData();

	if(file->pauseTick() >= 0){
		position = file->msOfTick(file->pauseTick());
	} else {
		position = file->msOfTick(file->cursorTick());
	}

	emit playerStarted();

	// Reset all Controllers
	for(int i = 0; i<16; i++){
		QByteArray array;
		array.append(byte(0xB0) | byte(i));
		array.append(byte(121));
		array.append(byte(0));
		MidiOutput::instance()->sendCommand(array);
	}
	MidiOutput::instance()->playedNotes.clear();

	// All Events before position should be sent, progChanges and ControlChanges
	QMultiMap<int, MidiEvent*>::iterator it = events->begin();
	while(it!=events->end()){
		if(it.key()>=position){
			break;
		}
		MidiOutput::instance()->sendCommand(it.value());
		it++;
	}

	setInterval(INTERVAL_TIME);

	connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	timer->start(interval);

	stopped = false;

	QList<TimeSignatureEvent*> *list = Q_NULLPTR;

	int tickInMeasure = 0;
	measure = file->measure(file->cursorTick(), file->cursorTick(), &list, &tickInMeasure);
	emit(measureChanged(measure, tickInMeasure));

	if(exec() == 0){
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
	if(!time) {
		time = new QTime();
		time->start();
	}

	if(stopped) {
		disconnect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
		// AllNotesOff // All SoundsOff
		for (int i = 0; i < 16; i++) {
			// value (third number) should be 0, but doesnt work
			QByteArray array;
			array.append(0xB0 | byte(i));
			array.append(byte(123));
			array.append(byte(127));
			MidiOutput::instance()->sendCommand(array);
		}
		if(MidiOutput::isAlternativePlayer()) {
			foreach(int channel, MidiOutput::instance()->playedNotes.keys()){
				foreach(int note, MidiOutput::instance()->playedNotes.value(channel)) {
					QByteArray array;
					array.append(0x80 | byte(channel));
					array.append(byte(note));
					array.append(byte(0));
					MidiOutput::instance()->sendCommand(array);
				}
			}
		}
		quit();

	} else {

		int newPos = qRound(position + time->elapsed()*MidiPlayer::instance()->speedScale());
		int tick = file->tick(newPos);
		QList<TimeSignatureEvent*> *list = Q_NULLPTR;
		int ickInMeasure = 0;

		int new_measure = file->measure(tick, tick, &list, &ickInMeasure);
		delete list;
		// compute current pos

		if(new_measure > measure){
			emit measureChanged(new_measure, ickInMeasure);
			measure = new_measure;
		}
		time->restart();
		QMultiMap<int, MidiEvent*>::iterator it = events->lowerBound(position);

		while(it!=events->end() && it.key()<newPos){

			// save events for the given tick
			QList<MidiEvent*> onEv, offEv;
			int sendPosition = it.key();

			do {
				if(it.value()->isOnEvent()){
					onEv.append(it.value());
				} else {
					offEv.append(it.value());
				}
				it++;
			} while(it!=events->end() && it.key() == sendPosition);

			foreach(MidiEvent *ev, offEv){
				MidiOutput::instance()->sendCommand(ev);
			}
			foreach(MidiEvent *ev, onEv){
				if(ev->line() == MidiEvent::KEY_SIGNATURE_EVENT_LINE){
					KeySignatureEvent *keySig = qobject_cast<KeySignatureEvent*>(ev);
					if(keySig){
						emit tonalityChanged(keySig->tonality());
					}
				}
				if(ev->line() == MidiEvent::TIME_SIGNATURE_EVENT_LINE){
					TimeSignatureEvent *timeSig = qobject_cast<TimeSignatureEvent*>(ev);
					if(timeSig){
						emit meterChanged(timeSig->num(), timeSig->denom());
					}
				}
				MidiOutput::instance()->sendCommand(ev);
			}

			//MidiOutput::sendCommand(it.value());
			//it++;
		}

		// end if it was last event, but only if not recording
		if(it == events->end() && !MidiInput::instance()->recording()){
			stop();
		}
		position = newPos;
		timeoutSinceLastSignal++;
		MidiInput::instance()->setTime(position);
		if(timeoutSinceLastSignal==TIMEOUTS_PER_SIGNAL){
			emit timeMsChanged(position);
			emit measureUpdate(measure, ickInMeasure);
			timeoutSinceLastSignal = 0;
		}
	}
	timer->blockSignals(false);

	//connect(timer, SIGNAL(timeout()), this, SLOT(timeout()), Qt::DirectConnection);
}
int PlayerThread::timeMs(){
	return position;
}
