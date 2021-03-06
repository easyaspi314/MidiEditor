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

#ifndef ONEVENT_H_
#define ONEVENT_H_

#include "MidiEvent.h"

class OffEvent;

class OnEvent : public MidiEvent{

	Q_OBJECT

	public:
		OnEvent(int ch, MidiTrack *track);
		OnEvent(const OnEvent &other);
		int type() const Q_DECL_OVERRIDE;

		void setOffEvent(OffEvent *event);
		OffEvent *offEvent();
		virtual QByteArray saveOffEvent();
		virtual QString offEventMessage();
		virtual ProtocolEntry *copy() Q_DECL_OVERRIDE;
		virtual void reloadState(ProtocolEntry *entry) Q_DECL_OVERRIDE;

		void moveToChannel(int channel) Q_DECL_OVERRIDE;
		bool hasBeenAdded;

	protected:
		OffEvent *_offEvent;

};

#endif
