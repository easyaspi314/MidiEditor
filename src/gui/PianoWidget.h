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

#ifndef PIANOWIDGET_H
#define PIANOWIDGET_H

#include "PaintWidget.h"
#include <QPaintEvent>
#include <QObject>
#include "MatrixWidget.h"
#include <QSize>
#include "../MidiEvent/MidiEvent.h"

class PaintWidget;
class MatrixWidget;

class PianoWidget : public PaintWidget
{
	Q_OBJECT

	public:
		PianoWidget(QWidget *parent = Q_NULLPTR);
		void paintPianoKey(QPainter *painter, int number, qreal x, qreal y, qreal width, qreal height);
		void setMatrixWidget(MatrixWidget *widget);
		QSize sizeHint() const Q_DECL_OVERRIDE;
		void setFile(MidiFile *file);
		enum PianoKey {
			C = 0,
			C_Sharp,
			D,
			D_Sharp,
			E,
			F,
			F_Sharp,
			G,
			G_Sharp,
			A,
			A_Sharp,
			B
		};
		enum PianoKeyShape {
			Black,
			WhiteBelowBlack,
			WhiteAboveBlack,
			WhiteBetweenBlack,
			WhiteOnly
		};
	public slots:
		void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
	private:
		MatrixWidget *matrixWidget;
		//QCache<PianoKey, QPainterPath> keyCache;
};

#endif // PIANOWIDGET_H
