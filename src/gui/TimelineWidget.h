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

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include "PaintWidget.h"
#include "MatrixWidget.h"
#include "MainWindow.h"
#include "../midi/MidiFile.h"
#include "../MidiEvent/TimeSignatureEvent.h"

#include <QObject>
class PaintWidget;
class MidiFile;

class TimelineWidget : public PaintWidget
{
	Q_OBJECT

	public:
		TimelineWidget(QWidget *parent = Q_NULLPTR);
		void setMatrixWidget(MatrixWidget *widget);

		void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
		void setFile(MidiFile *midiFile);
//		QSize sizeHint() const Q_DECL_OVERRIDE;
		qreal mousePosition();
	protected:
		void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
		void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
	private:
		MatrixWidget *matrixWidget;
		qreal timeHeight;
		MidiFile *file;
		QList<TimeSignatureEvent*> *currentTimeSignatureEvents;
};

#endif // TIMELINEWIDGET_H
