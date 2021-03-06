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

#ifndef OFFEVENT_H_
#define OFFEVENT_H_

#include "MidiEvent.h"
#include <QMultiMap>
#include <QList>

class OnEvent;

class OffEvent : public MidiEvent{

	Q_OBJECT

	public:
		OffEvent(int ch, int line, MidiTrack *track);
		OffEvent(const OffEvent &other);
		int type() const Q_DECL_OVERRIDE;

		void setOnEvent(OnEvent *event);
		OnEvent *onEvent();

		static void enterOnEvent(OnEvent *event);
		static void clearOnEvents();
		static void removeOnEvent(OnEvent *event);
		static QList<OnEvent*> corruptedOnEvents();
		void paint(QPainter *painter,
			   const QStyleOptionGraphicsItem *option,
			   QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;
		int line() Q_DECL_OVERRIDE;
		QByteArray save() Q_DECL_OVERRIDE;
		QString toMessage() Q_DECL_OVERRIDE;

		ProtocolEntry *copy() Q_DECL_OVERRIDE;
		void reloadState(ProtocolEntry *entry) Q_DECL_OVERRIDE;

		void setMidiTime(int t, bool toProtocol=true) Q_DECL_OVERRIDE;

		virtual bool isOnEvent() Q_DECL_OVERRIDE;
	protected:
		OnEvent *_onEvent;

		// Saves all openes and not closed onEvents. When an offEvent is created,
		// it searches his onEvent in onEvents and removes it from onEvents.
		static QMultiMap<int, OnEvent*> *onEvents;

		// needs to save the line, because offEvents are bound to their onEvents.
		// Setting the line is necessary to find the onEvent in the QMap
		int _line;
};

#endif
