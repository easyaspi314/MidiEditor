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

#include "KeyListener.h"

#include <QKeyEvent>
#include <QEvent>

#include "../midi/MidiPlayer.h"
KeyListener::KeyListener(QObject *parent) : QObject(parent) {
//	if (parent) {
//		moveToThread(parent->thread());
//	}
	qWarning("KeyListener start");
}
bool KeyListener::eventFilter(QObject *obj, QEvent *event) {
	Q_UNUSED(obj)
	if (event->type() == QEvent::KeyPress) {
		qWarning("KeyListener press");
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		if (keyEvent->modifiers() == (Qt::ALT | Qt::CTRL)) {
			if (keyEvent->key() == Qt::Key_P) {
				qWarning("Panic signal");
				MidiPlayer::instance()->panic();
				return true;
			}
		}
		return false;
	} else {
		return false;
	}
}
