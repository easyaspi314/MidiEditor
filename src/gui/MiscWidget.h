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

#ifndef MISCWIDGET_H
#define MISCWIDGET_H

#include "PaintWidget.h"

class MatrixWidget;
class MidiEvent;
class MidiFile;
class SelectTool;
class NoteOnEvent;

#include <QPair>
#include <QList>

class MiscWidget : public PaintWidget {

	Q_OBJECT

	public:
		enum MiscWidgetEditMode {
			SingleMode = 0,
			LineMode,
			FreehandMode
		};
		enum MiscWidgetMode {
			VelocityEditor = 0,
			ControlEditor,
			PitchBendEditor,
			KeyPressureEditor,
			ChannelPressureEditor,
			MiscModeEnd,
		};

		MiscWidget(MatrixWidget *mw, QWidget *parent = Q_NULLPTR);

		static QString modeToString(int mode);
		void setMode(int mode);
		void setEditMode(int mode);
		void setFile(MidiFile *midiFile);

	public slots:
		void setChannel(int);
		void setControl(int ctrl);
		void redraw();

	protected:
		void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
		void keyPressEvent(QKeyEvent* e) Q_DECL_OVERRIDE;
		void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

		void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
		void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
		void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

	private:
		MatrixWidget *matrixWidget;

		// Mode is SINGLE_MODE or LINE_MODE
		int edit_mode;
		int mode;
		int channel;
		int controller;

		void resetState();

		QList<QPair<int, int> > getTrack(QList<MidiEvent*> *accordingEvents = Q_NULLPTR);
		void computeMinMax();
		QPair<int, int> processEvent(MidiEvent *e, bool *ok);
		qreal interpolate(QList<QPair<qreal, qreal> > track, qreal x);
		qreal value(double y);
		bool filter(MidiEvent *e);

		int _max, _default;

		// single
		int dragY;
		bool dragging;
		NoteOnEvent *aboveEvent;
		SelectTool *_dummyTool;
		int trackIndex;

		// free hand
		QList<QPair<qreal, qreal> > freeHandCurve;
		bool isDrawingFreehand;

		// line
		qreal lineX, lineY;
		bool isDrawingLine;

		MidiFile *file;
		QPixmap *pixmap;
		bool inited;

		const int WIDTH = 7;
};

#endif
