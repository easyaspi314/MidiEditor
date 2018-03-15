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

#ifndef MATRIXWIDGET_H_
#define MATRIXWIDGET_H_

#include "PaintWidget.h"
#include "TimelineWidget.h"
#include "PianoWidget.h"

#include <QWidget>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QMap>
#include <QColor>
#include <QPixmap>
#include <QApplication>
#include <QPalette>
#include <QPixmapCache>
#include <QTimer>
#include <QCache>

class MidiFile;
class TempoChangeEvent;
class TimeSignatureEvent;
class MidiEvent;
class GraphicObject;
class NoteOnEvent;
class TimelineWidget;
class PianoWidget;

const static ubyte NUM_LINES = 139;
const static ubyte PIXEL_PER_S = 100;
const static ubyte PIXEL_PER_LINE = 11;
const static ubyte PIXEL_PER_EVENT = 15;
const static ubyte MAX_HORIZ_ZOOM = 10;
const static ubyte MAX_VERT_ZOOM = 3;

class MatrixWidget : public PaintWidget {

    Q_OBJECT

    public:

        MatrixWidget(QWidget *parent = qnullptr);
        void setFile(MidiFile *file);
        MidiFile *midiFile();
        QList<MidiEvent*> *activeEvents();
        QList<MidiEvent*> *velocityEvents();

        qreal lineHeight();
        ubyte lineAtY(qreal y);
        int msOfXPos(qreal x);
        int timeMsOfWidth(int w);
        bool eventInWidget(MidiEvent *event);
        qreal yPosOfLine(ubyte line);
        void setScreenLocked(bool b);
        bool screenLocked();
        int minVisibleMidiTime();
        int maxVisibleMidiTime();

        void setColorsByChannel();
        void setColorsByTracks();
        bool colorsByChannel();


        int msOfTick(int tick);
        qreal xPosOfMs(int ms);
        const QList<QPair<qreal, int> > divs();

        QSize sizeHint() const  qoverride;

        void setPianoWidget(PianoWidget *widget);
        void setTimelineWidget(TimelineWidget *widget);
        QList<QPair<qreal, int> > currentDivs;
        float scaleX, scaleY;

    public slots:
        void zoomHorIn();
        void zoomHorOut();
        void zoomVerIn();
        void zoomVerOut();
        void zoomStd();
        void calcSizes();
        void timeMsChanged(int ms, bool ignoreLocked = false);
        void takeKeyPressEvent(QKeyEvent *event);
        void takeKeyReleaseEvent(QKeyEvent *event);
        void setDiv(ubyte div);
        void redraw();
    signals:
       // void sizeChanged(int maxScrollTime, double maxScrollLine, int valueX,
       //         double valueY);
        void objectListChanged();
        void scrollChanged(int x, int y);

    protected:
        void paintEvent(QPaintEvent *event) qoverride;
        void mouseMoveEvent(QMouseEvent *event) qoverride;
        void resizeEvent(QResizeEvent *event) qoverride;
        void enterEvent(QEvent *event) qoverride;
        void leaveEvent(QEvent *event) qoverride;
        void mousePressEvent(QMouseEvent *event) qoverride;
        void mouseReleaseEvent(QMouseEvent *event) qoverride;
        void keyPressEvent(QKeyEvent* event) qoverride;
        void keyReleaseEvent(QKeyEvent *event) qoverride;

    private:
        void paintChannel(QPainter *painter, ubyte channel);


        QCache<int, QByteArray /* QPicture */> pictureCache;
        MidiFile *file;

        // pixmap is the painted widget (without tools and cursorLines).
        // it will be zero if it needs to be repainted
        QPixmap *pixmap;

        // saves all TempoEvents from one before the first shown tick to the
        // last in the window
        QList<MidiEvent*> *currentTempoEvents;
        QList<TimeSignatureEvent*> *currentTimeSignatureEvents;

        // All Events to show in the velocityWidget are saved in velocityObjects
        QList<MidiEvent*> *objects, *velocityObjects;

        // To play the pianokeys, there is one NoteOnEvent
        NoteOnEvent *pianoEvent;

        TimelineWidget *timelineWidget;
        PianoWidget *pianoWidget;

        int startTick, endTick, endTimeX, msOfFirstEventInList;

        // This union represents the cache key for the QPicture cache.
        // see pictureCache.
        // Note that even with NO_BIT_PACK, we still use that here.
        union MatrixStepKey {
                uint rawValue;
                struct Key { // 32 bits
                    ushort mCurrStepId : 13;
                    ushort mDiv : 3;
                    ushort mScaleX : 8;
                    ushort mScaleY : 6;
                    bool mAntiAliased : 1;
                } data;
            };
        #ifdef NO_BIT_PACK
            bool _colorsByChannels;
            bool totalRepaint;
            bool screen_locked;
        #else

            bool _colorsByChannels : 1;
            bool totalRepaint : 1;
            bool screen_locked : 1;
        #endif
};

#endif
