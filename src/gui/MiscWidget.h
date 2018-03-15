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
#include "../Utils.h"

class MatrixWidget;
class MidiEvent;
class MidiFile;
class SelectTool;
class NoteOnEvent;

#include <QPair>
#include <QList>

enum struct MiscWidgetMode : ubyte {

    VelocityEditor = 0,
    ControlEditor,
    PitchBendEditor,
    KeyPressureEditor,
    ChannelPressureEditor,
    MiscModeEnd
};

enum struct MiscWidgetEditMode : ubyte {
    SingleMode = 0,
    LineMode,
    FreehandMode
};

class MiscWidget : public PaintWidget {

    Q_OBJECT

    public:


        MiscWidget(MatrixWidget *mw, QWidget *parent = qnullptr);

        static const QString modeToString(ubyte mode);
        void setMode(MiscWidgetMode mode);
        void setEditMode(MiscWidgetEditMode mode);
        void setFile(MidiFile *midiFile);

    public slots:
        void setChannel(int);
        void setControl(int ctrl);
        void redraw();

    protected:
        void startProtocol();
        void paintEvent(QPaintEvent *event) qoverride;
        void keyPressEvent(QKeyEvent* e) qoverride;
        void keyReleaseEvent(QKeyEvent *event) qoverride;
        void resizeEvent(QResizeEvent *event) qoverride;
        void mouseReleaseEvent(QMouseEvent *event) qoverride;
        void mousePressEvent(QMouseEvent *event) qoverride;
        void leaveEvent(QEvent *event) qoverride;
        void mouseMoveEvent(QMouseEvent *event) qoverride;

    private:


        void resetState();
        QList<QPair<qreal, ushort> > getTrack(QList<MidiEvent*> *accordingEvents = qnullptr);
        void computeMinMax();
        QPair<qreal, ushort> processEvent(MidiEvent *e, bool *ok);
        qreal interpolate(const QList<QPair<qreal, qreal> > &track, qreal x);
        qreal value(qreal y);
        bool filter(MidiEvent *e);

        // line
        qreal lineX, lineY;

        // free hand
        QList<QPair<qreal, qreal> > freeHandCurve;

        MatrixWidget *matrixWidget;
        NoteOnEvent *aboveEvent;
        SelectTool *_dummyTool;
        MidiFile *file;
        QPixmap *pixmap;

        int trackIndex;
        // single
        int dragY;

        ushort _max;
        ushort _default;

        #ifdef NO_BIT_PACK
            // Mode is SINGLE_MODE or LINE_MODE
            MiscWidgetEditMode edit_mode;
            MiscWidgetMode mode;

            bool dragging;
            bool inited;
            bool isDrawingLine;

            ubyte controller;

            ubyte channel;

            bool isDrawingFreehand;

        #else
            // Mode is SINGLE_MODE or LINE_MODE
            MiscWidgetEditMode edit_mode : 2;
            MiscWidgetMode mode : 3;
            bool dragging : 1;
            bool inited : 1;
            bool isDrawingLine : 1;

            ubyte controller : 7;
            bool isDrawingFreehand : 1;

            ubyte channel : 4;

        #endif
        const ubyte WIDTH = 7;

};

#endif
