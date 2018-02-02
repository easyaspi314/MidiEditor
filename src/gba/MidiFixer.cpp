/*
 * midfix4agb
 * Copyright (C) 2014  ipatix
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

#ifndef GBA
#include "MidiFixer.h"

#include "../midi/MidiFile.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiTrack.h"
#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/TextEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/ProgChangeEvent.h"
#include "../MidiEvent/PitchBendEvent.h"

/*
 * This is ipatix's midfix4agb, found at https://github.com/ipatix/midfix4agb
 *
 * It has been translated to C++ and adapted to MidiEditor.
 *
 * This class converts a standard MIDI to the GBA's exponential volume/mod scale.
 */
void MidiFixer::addAgbCompatibleEvents(MidiFile *midiFile, int modType) {
	int *trackNumber = new int[16];

	for (int currentChannel = 0; currentChannel < 16; currentChannel++) {
		if (!midiFile->track(currentChannel))
			midiFile->addTrack();
		trackNumber[currentChannel] = currentChannel;
	}

	// add all MODT events
	for (int currentChannel = 0; currentChannel < 16; currentChannel++) {
		if (trackNumber[currentChannel] == -1) continue;    // skip track if there is no regular MIDI commands
		// add a MODT controller event in the beginning of the track and LFOS
		midiFile->channel(currentChannel)->insertEvent(new ControlChangeEvent(currentChannel, 21, 44, midiFile->track(trackNumber[currentChannel])), 0);
		midiFile->channel(currentChannel)->insertEvent(new ControlChangeEvent(currentChannel, 22, modType, midiFile->track(trackNumber[currentChannel])), 0);
	}

	qWarning("Adding BENDR Events...");

	for (int currentChannel = 0; currentChannel < 16; currentChannel++) {
		if (channelIsEmpty(midiFile, currentChannel)) continue;

		int rpMSB = 0;
		int rpLSB = 0;

		QMultiMap<int, MidiEvent*> *events = midiFile->channelEvents(currentChannel);
		QMultiMap<int, MidiEvent*>::iterator it = events->begin();

		// Adding and removing MidiEvents during an iterator can cause out-of-bounds crashes.
		QList<MidiEvent*> *eventsToRemove = new QList<MidiEvent*>();
		QMultiMap<int, MidiEvent*> *eventsToAdd = new QMultiMap<int, MidiEvent*>();

		while (it != events->end()) {
			MidiEvent *event = it.value();
			if (event) {
				ControlChangeEvent *ccEvent = qobject_cast<ControlChangeEvent*>(event);
				if (ccEvent) {
					switch (ccEvent->control()) {
						case 0x64:
							rpLSB = ccEvent->value();
							eventsToRemove->append(ccEvent);
							break;
						case 0x65:
							rpLSB = ccEvent->value();
							eventsToRemove->append(ccEvent);
							break;
						case 0x6:
							if (rpLSB == 0 && rpMSB == 0) {
								// insert new event if right parameter slots are selected
								eventsToAdd->insert(ccEvent->midiTime(), new ControlChangeEvent(currentChannel, 20, ccEvent->value(), midiFile->track(trackNumber[currentChannel])));
								eventsToRemove->append(ccEvent);
							}
							break;
						default:
							break;
					}

				}
			}
			it++;
		}
		while (!eventsToRemove->isEmpty())
			midiFile->channel(currentChannel)->removeEvent(eventsToRemove->takeFirst());
		it = eventsToAdd->begin();
		while (it != eventsToAdd->end()) {
			midiFile->channel(currentChannel)->insertEvent(it.value(), it.key());
			it++;
		}
	}
	qWarning("done");
}

void MidiFixer::addModulationScale(MidiFile *midiFile, double modScale) {
	qWarning("Adding Modulation Scale...");

	for (int currentChannel = 0; currentChannel < 16; currentChannel++) {
		if (channelIsEmpty(midiFile, currentChannel)) continue;

		QMultiMap<int, MidiEvent*> *events = midiFile->channelEvents(currentChannel);
		QMultiMap<int, MidiEvent*>::iterator it = events->begin();

		while (it != events->end()) {
			MidiEvent *event = it.value();
			// get current Event data
			ControlChangeEvent *ccEvent = qobject_cast<ControlChangeEvent*>(event);
			if (ccEvent) {
				if (ccEvent->control() == 0x1) {
					ccEvent->setValue(minMax(0, ccEvent->value() * modScale, 127));
				}
			}
			it++;
		}
	}
	qWarning("Done");
}

void MidiFixer::combineVolumeAndExpression(MidiFile *midiFile) {
	qWarning("Combining Volume and Expression Events...");

	for (int currentChannel = 0; currentChannel < 16; currentChannel++) {
		if (channelIsEmpty(midiFile, currentChannel)) continue;

		int expressionLevel = 127;
		int volumeLevel = 127;
		QMultiMap<int, MidiEvent*> *events = midiFile->channelEvents(currentChannel);
		QMultiMap<int, MidiEvent*>::iterator it = events->begin();

		QList<MidiEvent*> *eventsToRemove = new QList<MidiEvent*>();
		QMultiMap<int, MidiEvent*> *eventsToAdd = new QMultiMap<int, MidiEvent*>();

		while (it != events->end())
		{
			MidiEvent *event = it.value();
			// get current Event data
			ControlChangeEvent *ccEvent = qobject_cast<ControlChangeEvent*>(event);
			if (ccEvent) {
				if (ccEvent->control() == 0x7) {
					volumeLevel = ccEvent->value();
					int newLevel = (int)(volumeLevel * expressionLevel / 127);

					ccEvent->setValue(newLevel);
				} else if (ccEvent->control() == 0xB) {
					expressionLevel = ccEvent->value();
					int newLevel = (int)(volumeLevel * expressionLevel / 127);

					eventsToAdd->insert(ccEvent->midiTime(), new ControlChangeEvent(currentChannel, 0x7, newLevel, midiFile->track(currentChannel)));
					eventsToRemove->append(ccEvent);
				}
			}
			it++;
		}

		while (!eventsToRemove->isEmpty())
			midiFile->channel(currentChannel)->removeEvent(eventsToRemove->takeFirst());

		it = eventsToAdd->begin();
		while (it != eventsToAdd->end()) {
			midiFile->channel(currentChannel)->insertEvent(it.value(), it.key());
			it++;
		}
	}
	qWarning("Done.");
}

void MidiFixer::addExponentialScale(MidiFile *midiFile) {
	qWarning("Applying exponential Volume and Velocity Scale...");

	for (int currentChannel = 0; currentChannel < 16; currentChannel++) {
		if (channelIsEmpty(midiFile, currentChannel)) continue;

		QMultiMap<int, MidiEvent*> *events = midiFile->channelEvents(currentChannel);
		QMultiMap<int, MidiEvent*>::iterator it = events->begin();

		while (it != events->end()) {
			// get current Event data
			MidiEvent *event = it.value();
			ControlChangeEvent *ccEvent = qobject_cast<ControlChangeEvent*>(event);
			if (ccEvent) {
				if (ccEvent->control() == 0x7) {
					ccEvent->setValue(expVol(ccEvent->value()));
				}
			}

			NoteOnEvent *noteOnEvent = qobject_cast<NoteOnEvent*>(event);
			if (noteOnEvent) {
				noteOnEvent->setVelocity(expVol(noteOnEvent->velocity()));
			}
			it++;
		}
	}
	qWarning("Done.");
}

void MidiFixer::removeRedundantMidiEvents(MidiFile *midiFile)
{

}

void MidiFixer::fixLoopCarryBack(MidiFile *midiFile) {
	qWarning("Fixing Loop Carryback Errors...");
	// first of all we need to check if the MIDI actually loops
	// we only have to check the first track for the [ ] brackets in the marker Events
	bool hasLoopStart = false;
	long loopStartTick = 0;
	bool hasLoopEnd = false;
	long loopEndTick = 0;

	QMultiMap<int, MidiEvent*> *events16 = midiFile->channelEvents(16);
	QMultiMap<int, MidiEvent*>::iterator it16 = events16->begin();
	while (it16 != events16->end()) {
		MidiEvent *event = it16.value();
		TextEvent *txtEvent = qobject_cast<TextEvent*>(event);

		if (txtEvent) {   // if event is META
			if (txtEvent->textType() == TextEvent::MarkerTextEventType) {    // if event is Marker
				if (txtEvent->text() == "[" || txtEvent->text() == "loopStart") {
					if (txtEvent->text() == "loopStart")
						txtEvent->setText("[");
					hasLoopStart = true;
					loopStartTick = txtEvent->midiTime();
				} else if (txtEvent->text() == "]" || txtEvent->text() == "loopEnd") {
					if (txtEvent->text() == "loopEnd")
						txtEvent->setText("]");
					hasLoopEnd = true;
					loopEndTick = txtEvent->midiTime();
				}
			}
		}
		it16++;
	}
	// we now got the loop points if there are any
	if (hasLoopStart == false || hasLoopEnd == false)
	{
		qWarning("MIDI is not looped!");
		return;
	}
	// now the carryback prevention is done because the program will return if there is no loop to fix
	for (int currentChannel = 0; currentChannel < 16; currentChannel++)
	{
		if (channelIsEmpty(midiFile, currentChannel)) continue;
		QMultiMap<int, MidiEvent*> *events = midiFile->channelEvents(currentChannel);
		QMultiMap<int, MidiEvent*>::iterator it = events->begin();

		agbControllerState loopStartState;
		agbControllerState loopEndState;

		int eventAtLoopStart = events->size() - 1;

		while (it != events->end()) {
			MidiEvent *event = it.value();
			// now all events get recorded on continously update the loop start state
			if (event->midiTime() > loopStartTick) {
				eventAtLoopStart = event->midiTime();
				break;
			}
			TempoChangeEvent *tempoEvent = qobject_cast<TempoChangeEvent*>(event);
			if (tempoEvent) {
				loopStartState.Tempo = tempoEvent->beatsPerQuarter();
				continue;
			}
			ControlChangeEvent *ccEvent = qobject_cast<ControlChangeEvent*>(event);
			if (ccEvent) {  // is event a controller event?

				switch(ccEvent->control()) {
				case 0x1: // modulation wheel
					loopStartState.Mod = ccEvent->value();  // save mod state
					break;
				case 0x7: // volume
					loopStartState.Volume = ccEvent->value();   // save volume state
					break;
				case 0xA: // pan
					loopStartState.Pan = ccEvent->value();  // save pan position
					break;
				case 0x14: // bend range
					loopStartState.BendR = ccEvent->value();  // save pseudo AGB bendr
					break;
				default:
					break;
				}
				continue;
			}
			ProgChangeEvent *pcEvent = qobject_cast<ProgChangeEvent*>(event);
			if (pcEvent) {     // if voice change event
				loopStartState.Voice = pcEvent->program();
				continue;
			}
			PitchBendEvent *pbEvent = qobject_cast<PitchBendEvent*>(event);
			if (pbEvent) {      // if pitch bend

				loopStartState.Bend = pbEvent->value();
				continue;
			}
			it++;
		}

		loopEndState = loopStartState;  // override all changes from the loop start state to the loop end state so we can continue on with checking the trackdata for carryback errors we need to correct
		// recorded loop start state, now record until

		int eventAtLoopEnd = midiFile->channelEvents(currentChannel)->size();

		it = events->find(eventAtLoopStart);
		while (it != events->end()) {
			\
			MidiEvent *event = it.value();
			// now all events get recorded on continously update the loop start state
			if (event->midiTime() >= loopEndTick) {
				eventAtLoopEnd = event->midiTime();      // if the loop end occurs before the end of the event data set it's value manually
				break;
			}
			TempoChangeEvent *tempoEvent = qobject_cast<TempoChangeEvent*>(event);
			if (tempoEvent) {   // check if META tempo event occurs
				loopEndState.Tempo = tempoEvent->beatsPerQuarter();
				continue;
			}
			ControlChangeEvent *ccEvent = qobject_cast<ControlChangeEvent*>(event);
			if (ccEvent) {  // is event a controller event?
				switch(ccEvent->control()) {
				case 0x1: // modulation wheel
					loopEndState.Mod = ccEvent->value();  // save mod state
					break;
				case 0x7: // volume
					loopEndState.Volume = ccEvent->value();   // save volume state
					break;
				case 0xA: // pan
					loopEndState.Pan = ccEvent->value();  // save pan position
					break;
				case 0x14: // bend range
					loopEndState.BendR = ccEvent->value();  // save pseudo AGB bendr
					break;
				default:
					break;
				}
				continue;
			}
			ProgChangeEvent *pcEvent = qobject_cast<ProgChangeEvent*>(event);
			if (pcEvent) {     // if voice change event
				loopEndState.Voice = pcEvent->program();
				continue;
			}
			PitchBendEvent *pbEvent = qobject_cast<PitchBendEvent*>(event);
			if (pbEvent) {     // if pitch bend
				loopEndState.Bend = pbEvent->value();
				continue;
			}
			it++;
		}

		// now we need to fill in the events at the loop end event slot "eventAtLoopEnd"
		// check if the values vary and set them accordingly
		if (loopStartState.Tempo != loopEndState.Tempo) { // check if tempo is the same
			if (loopStartState.Tempo != 0) {   // don't fix it if the tempo is not set to a valid value
				if (eventAtLoopStart >= events->size()) {
					midiFile->channel(currentChannel)->insertEvent(new TempoChangeEvent(currentChannel, loopStartState.Tempo, midiFile->track(currentChannel)), loopStartTick);
				} else {
					TempoChangeEvent *tcEvent = qobject_cast<TempoChangeEvent*>(events->value(eventAtLoopStart));
					if (tcEvent) {
						tcEvent->setBeats(loopStartState.Tempo);
					}
					else {
						qWarning("tcEvent is not a TempoChangeEvent!");
					}
				}
			}
		}
		if (currentChannel != -1) {    // only do this fixing if the midi channel is actually defined (META only tracks are skipped here)
			if (loopStartState.Voice != loopEndState.Voice) {
				if (loopStartState.Voice != 0xFF) {  // only fix if voice is a valid voice number (0-127)
					if (eventAtLoopStart >= events->size()) {
						midiFile->channel(currentChannel)->insertEvent(new ProgChangeEvent(currentChannel, loopStartState.Voice, midiFile->track(currentChannel)), loopStartTick);
					} else {
						ProgChangeEvent *pcEvent = qobject_cast<ProgChangeEvent*>(events->value(eventAtLoopStart));
						if (pcEvent) {
							pcEvent->setProgram(loopStartState.Voice);
						}
						else {
							qWarning("pcEvent is not a ProgChangeEvent!");
						}
					}
				}
			}
			// why be redundant
			ControlChangeEvent *ccEvent = qobject_cast<ControlChangeEvent*>(events->value(eventAtLoopStart));
			if (loopStartState.Volume != loopEndState.Volume) {     // fix volume
				if (loopStartState.Volume != 0xFF) {
					if (eventAtLoopStart >= events->size()) {
						midiFile->channel(currentChannel)->insertEvent(new ControlChangeEvent(currentChannel, 0x7, loopStartState.Volume, midiFile->track(currentChannel)), loopStartTick);
					} else if (ccEvent && ccEvent->control() == 0x7) {
						ccEvent->setValue(loopStartState.Voice);
					} else {
						qWarning("ccEvent is not a Volume ControlChangeEvent!");
					}

				}
			}
			if (loopStartState.Pan != loopEndState.Pan) {     // fix PAN
				if (loopStartState.Pan != 0xFF) {
					if (eventAtLoopStart >= events->size()) {
						midiFile->channel(currentChannel)->insertEvent(new ControlChangeEvent(currentChannel, 0xA, loopStartState.Pan, midiFile->track(currentChannel)), loopStartTick);
					} else if (ccEvent && ccEvent->control() == 0xA) {
						ccEvent->setValue(loopStartState.Pan);
					} else {
						qWarning("ccEvent is not a Pan ControlChangeEvent!");
					}

				}
			}
			if (loopStartState.BendR != loopEndState.BendR) {      // fix BENDR
				if (loopStartState.BendR != 0xFF) {
					if (eventAtLoopStart >= events->size()) {
						midiFile->channel(currentChannel)->insertEvent(new ControlChangeEvent(currentChannel, 20, loopStartState.BendR, midiFile->track(currentChannel)), loopStartTick);
					} else if (ccEvent && ccEvent->control() == 20) {
						ccEvent->setValue(loopStartState.BendR);
					} else {
						qWarning("ccEvent is not a BendR ControlChangeEvent!");
					}
				}
			}

		if (loopStartState.Mod != loopEndState.Mod) {      // fix MOD
			if (loopStartState.Mod != 0xFF) {
				if (eventAtLoopStart >= events->size()) {
					midiFile->channel(currentChannel)->insertEvent(new ControlChangeEvent(currentChannel, 0x1, loopStartState.Mod, midiFile->track(currentChannel)), loopStartTick);
				} else if (ccEvent && ccEvent->control() == 0x1) {
					ccEvent->setValue(loopStartState.Mod);
				} else {
					qWarning("ccEvent is not a Mod ControlChangeEvent!");
				}

			}
		}
		if (loopStartState.Bend != loopEndState.Bend) {     // fix BEND
			if (loopStartState.Bend != 0xFFFF) { // we don't need to check MSB because if this one isn't 0xFF the other one won't be 0xFF either
				if (eventAtLoopStart >= events->size()) {
					midiFile->channel(currentChannel)->insertEvent(new PitchBendEvent(currentChannel, loopStartState.Bend, midiFile->track(currentChannel)), loopStartTick);
				} else {
					PitchBendEvent *pbEvent = qobject_cast<PitchBendEvent*>(events->value(eventAtLoopStart));
					if (pbEvent) {
						pbEvent->setValue(loopStartState.Bend);
					} else {
						qWarning("pbEvent is not a Pitch Bend event!");
					}
				}
			}
		}
	}
	}
	qWarning("Done");
}


int MidiFixer::minMax(double minVal, double val, double maxVal) {
	if (val < minVal)
		val = minVal;
	else if (val > maxVal)
		val = maxVal;
	return (int)round(val);
}

int MidiFixer::expVol(int volume) {
	double returnValue = volume;
	if (returnValue == 0)
		return 0;
	returnValue /= 127;
	returnValue = pow(returnValue, 10.0 / 6.0);
	return (int)round(returnValue * 127);
}

bool MidiFixer::channelIsEmpty(MidiFile *midiFile, int channel) {
	return midiFile->channelEvents(channel)->isEmpty();
}

#endif
