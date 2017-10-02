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

/**
 * \file midi/MidiChannel.cpp
 *
 * \brief Implements the class MidiChannel.
 */


#include "MidiChannel.h"

#include <QColor>

#include "MidiFile.h"
#include "MidiTrack.h"
#include "../gui/EventWidget.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/ProgChangeEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"

QColor MidiChannel::colorByChannelNumber(int number) {
	switch(number){
		case 0: { return QColor(241, 70, 57, 255); }
		case 1: { return QColor(205, 241, 0, 255); }
		case 2: { return QColor(50, 201, 20, 255); }
		case 3: { return QColor(107, 241, 231, 255); }
		case 4: { return QColor(127, 67, 255, 255); }
		case 5: { return QColor(241, 127, 200, 255); }
		case 6: { return QColor(170, 212, 170, 255); }
		case 7: { return QColor(222, 202, 170, 255); }
		case 8: { return QColor(241, 201, 20, 255); }
		case 9: { return QColor(80, 80, 80, 255); }
		case 10: { return QColor(202, 50, 127, 255); }
		case 11: { return QColor(0, 132, 255, 255); }
		case 12: { return QColor(102, 127, 37, 255); }
		case 13: { return QColor(241, 164, 80, 255); }
		case 14: { return QColor(107, 30, 107, 255); }
		case 15: { return QColor(50, 127, 127, 255); }
		default: { return QColor(50, 50, 255, 255); }
	}
}

MidiChannel::MidiChannel(MidiFile *f, int num){

	_midiFile = f;
	_num = num;

	_visible = true;
	_mute = false;
	_solo = false;

	_events = new QMultiMap<int, MidiEvent*>;

	// the color only depends on the number
	_color = new QColor(colorByChannelNumber(num));
}

MidiChannel::MidiChannel(const MidiChannel &other) : ProtocolEntry(other) {
	_midiFile = other._midiFile;
	_visible = other._visible;
	_mute = other._mute;
	_solo = other._solo;
	_events = new QMultiMap<int, MidiEvent*>(*(other._events));
	//_events->swap(*(other._events));
	_num = other._num;
	_color = other._color;
}

ProtocolEntry *MidiChannel::copy(){
	return new MidiChannel(*this);
}

void MidiChannel::reloadState(ProtocolEntry *entry){
	MidiChannel *other = qobject_cast<MidiChannel*>(entry);
	if(!other){
		return;
	}
	_midiFile = other->_midiFile;
	_visible = other->_visible;
	_mute = other->_mute;
	_solo = other->_solo;
	_events = new QMultiMap<int, MidiEvent*>(*(other->_events));
	//_color = other->_color;
	_num = other->_num;
	_color = other->_color;
}

MidiFile *MidiChannel::file(){
	return _midiFile;
}

bool MidiChannel::visible(){
	if(_num > 16){
		return _midiFile->channel(16)->visible();
	}
	return _visible;
}

void MidiChannel::setVisible(bool b){
	ProtocolEntry *toCopy = copy();
	_visible = b;
	protocol(toCopy, this);
}

bool MidiChannel::mute(){
	return _mute;
}

void MidiChannel::setMute(bool b){
	ProtocolEntry *toCopy = copy();
	_mute = b;
	protocol(toCopy, this);
}

bool MidiChannel::solo(){
	return _solo;
}

void MidiChannel::setSolo(bool b){
	ProtocolEntry *toCopy = copy();
	_solo = b;
	protocol(toCopy, this);
}

int MidiChannel::number(){
	return _num;
}

QMultiMap<int, MidiEvent*> *MidiChannel::eventMap(){
	return _events;
}

QColor MidiChannel::color(){
	return *_color;
}

NoteOnEvent* MidiChannel::insertNote(int note, int startTick, int endTick,int velocity, MidiTrack *track){
	ProtocolEntry *toCopy = copy();
	NoteOnEvent *onEvent = new NoteOnEvent(note, velocity, number(), track);

	OffEvent *off = new OffEvent(number(), 127-note, track);

	off->setFile(file());
	off->setMidiTime(endTick, false);
	onEvent->setFile(file());
	onEvent->setMidiTime(startTick, false);

	protocol(toCopy, this);

	return onEvent;
}

bool MidiChannel::removeEvent(MidiEvent *event){

	// if its once TimeSig / TempoChange at 0, dont delete event
	if(number()==18 || number()==17){
		if((event->midiTime()==0) && (_events->count(0) == 1)){
			return false;
		}
	}

	// remove from track if its the trackname
	if(number() == 16 && (MidiEvent*)(event->track()->nameEvent()) == event){
		event->track()->setNameEvent(Q_NULLPTR);
	}

	ProtocolEntry *toCopy = copy();
	_events->remove(event->midiTime(), event);
	OnEvent *on = qobject_cast<OnEvent*>(event);
	if(on && on->offEvent()){
		_events->remove(on->offEvent()->midiTime(), on->offEvent());
	}
	protocol(toCopy, this);

	//if(MidiEvent::eventWidget()->events().contains(event)){
	//	MidiEvent::eventWidget()->removeEvent(event);
	//}
	return true;
}

void MidiChannel::insertEvent(MidiEvent *event, int tick){
	//ProtocolEntry *toCopy = copy();
	event->setFile(file());
	event->setMidiTime(tick, false);
	protocol(copy(), this);
}

void MidiChannel::deleteAllEvents(){
	ProtocolEntry *toCopy = copy();
	_events->clear();
	protocol(toCopy, this);
}

int MidiChannel::progAtTick(int tick){

	// search for the last ProgChangeEvent in the channel
	QMultiMap<int, MidiEvent*>::iterator it = _events->upperBound(tick);
	if(it == _events->end()){
		it--;
	}
	if(_events->size() ) {
		while(it != _events->begin()){
			ProgChangeEvent *ev = qobject_cast<ProgChangeEvent*>(it.value());
			if(ev && it.key()<=tick){
				return ev->program();
			}
			it--;
		}
	}

	// default: first
	foreach(MidiEvent *event, *_events){
		ProgChangeEvent *ev = qobject_cast<ProgChangeEvent*>(event);
		if(ev){
			return ev->program();
		}
	}
	return 0;
}
