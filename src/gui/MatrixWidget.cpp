// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

#include "MatrixWidget.h"
#include <QSizePolicy>
#include "../midi/MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../midi/MidiChannel.h"
#include "../tool/Tool.h"
#include "../tool/EditorTool.h"
#include "../tool/EventTool.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiPlayer.h"
#include "../midi/PlayerThread.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiTrack.h"
#include "../tool/Selection.h"
#include <QDebug>

#include <QList>
#include <QPixmapCache>
#include <QtCore/qmath.h>
#include <QBuffer>
#include <QTime>
#include <QPicture>
#include "CrashHandler.h"

/*
 * TODO:
 * - Remove old variables that we don't need
 * - Make painting even more efficient:
 *     > Further decrease CPU usage.
 *     > Hardware acceleration?
 *     > Decrease update frequency
 *     > Only repaint areas that are updated.
 *     > Render in chunks?
 *     > Set backgrounds when appropriate
 *     > Maybe get rid of QPixmap entirely?
 * - Remove unnecessary checks
 * - Make sure everything that should be
 *   changed...changes, and everything that
 *   shouldn't...shouldn't.
 *     > Divs don't update properly
 *     > MiscWidget doesn't update properly.
 * - PianoWidget interaction
 */
MatrixWidget::MatrixWidget(QWidget *parent) : PaintWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    screen_locked = false;
    setFixedHeight(NUM_LINES * PIXEL_PER_LINE);
    endTimeX = 0;
    // Double the cache space as we use it a lot.
    QPixmapCache::setCacheLimit(20480);
    totalRepaint = true;
    pixmap = new QPixmap(width(), height());
    file = qnullptr;
    scaleX = 1;
    scaleY = 1;

    pianoEvent = new NoteOnEvent(0, 100, 0, qnullptr);
    currentTempoEvents = new QList<MidiEvent *>;
    currentTimeSignatureEvents = new QList<TimeSignatureEvent *>;
    msOfFirstEventInList = 0;
    objects = new QList<MidiEvent *>;
    velocityObjects = new QList<MidiEvent *>;
    EditorTool::setMatrixWidget(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);

    setRepaintOnMouseMove(false);
    setRepaintOnMousePress(false);
    setRepaintOnMouseRelease(false);

    connect(MidiPlayer::player(), &PlayerThread::timeMsChanged, this, &MatrixWidget::timeMsChanged);
    redraw();
}

void MatrixWidget::setScreenLocked(bool b) {
    screen_locked = b;
}

bool MatrixWidget::screenLocked() {
    return screen_locked;
}

void MatrixWidget::timeMsChanged(int ms, bool ignoreLocked) {
    if (!file) {
        return;
    }
    int x = xPosOfMs(ms);
    int startTimeXTemp = msOfXPos(relativeRect().left());
    int endTimeXTemp = msOfXPos(relativeRect().right());

    if ((!screen_locked || ignoreLocked) && (x < 0 || ms < startTimeXTemp
              || ms > endTimeXTemp
              || x > (relativeRect().left() + relativeRect().width() + 100))) {

        // return if the last tick is already shown
        if (file->maxTime() <= endTimeXTemp && ms >= startTimeXTemp) {
            redraw();
            return;
        }

        // sets the new position and repaints
        emit scrollChanged(qRound(relativeRect().right() + (relativeRect().width() *
                                          0.9)), relativeRect().top());
    } else {
        redraw();
    }
}

/*
 * FIXME: Divs don't update after undo/redo
 */
void MatrixWidget::paintEvent(QPaintEvent *event) {
    PaintWidget::paintEvent(event);
    if (!file || !file->protocol() || height() <= 0 || paintingActive()) {
        return;
    }
    QPainter painter(this);
    QFont font = painter.font();
    font.setPixelSize(12);
    painter.setFont(font);
    QRect viewport = relativeRect();
    //if (!staticRepaint || totalRepaint || !pixmap) {
        /*
         * Check if we are calling a flat-out repaint
         * or just updating part of the widget.
         *
         * TODO: Make further use of this, especially
         * when scrolling.
         */
        ubyte updatemode = 0;
        if (pixmap && !event->region().isNull() && !event->region().isEmpty()) {
            updatemode = 1;
            painter.setClipping(true);
            painter.setClipRegion(event->region());
        } else if (pixmap && !event->rect().isNull() && !event->rect().isEmpty()) {
            updatemode = 2;
            painter.setClipping(true);
            painter.setClipRect(event->rect());
        } else {
            painter.setClipping(true);
            painter.setClipRect(viewport);
        }
        /*QPixmap linesTexture;
        if (!QPixmapCache::find("MatrixWidget_%1".arg(scaleY), linesTexture)) {
            linesTexture = QPixmap(1, height());
            //linesTexture.fill(Qt::transparent);

            QPainter linesPainter(&linesTexture);
            if (_settings.antialiasing) {
                linesPainter.setRenderHint(QPainter::Antialiasing);
            }
            for (int i = qFloor(startLineY); i <= qFloor(endLineY); i++) {
                int startLine = yPosOfLine(i);
                QColor c(194, 230, 255);
                if (i % 2 == 0) {
                    c = QColor(234, 246, 255);
                }
                if (i > 127) {
                    c = QColor(194, 194, 194);
                    if (i % 2 == 1) {
                        c = QColor(234, 246, 255);
                    }
                }
                linesPainter.fillRect(qRectF(0, startLine, 1, startLine + lineHeight()), c);

            }
            linesPainter.end();
            QPixmapCache::insert("MatrixWidget_%1".arg(scaleY), linesTexture);
            QPalette palette;
            palette.setBrush(backgroundRole(), QBrush(linesTexture));
            setPalette(palette);
        }*/
#if 0
        // This complex QString serves as the ID of the events pixmap.
        // It stores the zoom level, measure division, and the UUID of the current ProtocolStep.
        QByteArray array("Mtrx");
        array += QByteArray::number(file->protocol()->currentStepId(), 36);
        array += QByteArray::number(int(scaleY * 10), 36);
        array += QByteArray::number(int(scaleX * 10), 36);
        array += QByteArray::number(_settings.div);
        QString pixmapId = QString::fromUtf8(array);

        bool doRepaint = totalRepaint || !QPixmapCache::find(pixmapId, pixmap);

        if (doRepaint) {
            this->pianoKeys.clear();
            //pixmap = new QPixmap(width(), height());
            //pixmap->
            if (!pixmap) {
                pixmap = new QPixmap(size());
            }
            if (pixmap->paintingActive()) {
                return;
            }
            pixmap->fill(Qt::transparent);
            QPainter pixpainter(pixmap);

            pixpainter.setBrush(Qt::transparent);
            if (_settings.antialiasing) {
                pixpainter.setRenderHint(QPainter::Antialiasing);
            }

            QFont f = pixpainter.font();
            f.setPixelSize(12);
            pixpainter.setFont(f);
            pixpainter.setClipping(false);

            for (int i = 0; i < objects->length(); i++) {
                objects->at(i)->setShown(false);
                OnEvent *onev = protocol_cast<OnEvent *>(objects->at(i));
                if (onev && onev->offEvent()) {
                    onev->offEvent()->setShown(false);
                }
            }
            objects->clear();
            velocityObjects->clear();
            currentTempoEvents->clear();
            currentTimeSignatureEvents->clear();
            currentDivs.clear();

            startTick = file->tick(startTimeX, endTimeX, &currentTempoEvents,
                                          &endTick, &msOfFirstEventInList);

            TempoChangeEvent *ev = protocol_cast<TempoChangeEvent *>(currentTempoEvents->at(0));
            if (!ev) {
                return;
            }

            // paint measures and timeline
            currentTimeSignatureEvents->clear();
            int measure = file->measure(0, file->endTick(), &currentTimeSignatureEvents);

            TimeSignatureEvent *currentEvent = currentTimeSignatureEvents->at(0);
            int i = 0;
            if (!currentEvent) {
                return;
            }
            int tick = currentEvent->midiTime();
            while (tick + currentEvent->ticksPerMeasure() <= 0) {
                tick += currentEvent->ticksPerMeasure();
            }
            qreal xfrom, xDiv, metronomeDiv;
            int measureStartTick, ticksPerDiv, startTickDiv, divTick;
            QPen oldPen, dashPen;
            while (tick < file->endTick()) {
                TimeSignatureEvent *measureEvent = currentTimeSignatureEvents->at(i);
                xfrom = xPosOfMs(msOfTick(tick));
                currentDivs.append(QPair<qreal, int>(xfrom, tick));
                measure++;
                measureStartTick = tick;
                tick += currentEvent->ticksPerMeasure();
                if (i < currentTimeSignatureEvents->length() - 1) {
                    if (currentTimeSignatureEvents->at(i + 1)->midiTime() <= tick) {
                        currentEvent = currentTimeSignatureEvents->at(i + 1);
                        tick = currentEvent->midiTime();
                        i++;
                    }
                }
                // draw measures
                if (_settings.div >= 0) {
                    metronomeDiv = 4 / qPow(2, _settings.div);
                    ticksPerDiv = metronomeDiv * file->ticksPerQuarter();
                    startTickDiv = ticksPerDiv;
                    oldPen = pixpainter.pen();
                    dashPen = QPen(Qt::lightGray, 1, Qt::DashLine);
                    pixpainter.setPen(dashPen);
                    while (startTickDiv <= measureEvent->ticksPerMeasure()) {
                        divTick = startTickDiv + measureStartTick;
                        xDiv = xPosOfMs(msOfTick(divTick));
                        currentDivs.append(QPair<qreal, int>(xDiv, divTick));
                        pixpainter.drawLine(QLineF(xDiv, 0, xDiv, height()));
                        startTickDiv += ticksPerDiv;
                    }
                    pixpainter.setPen(oldPen);
                }
            }

            pixpainter.setPen(Qt::black);

            // paint the events
            //pixpainter.setClipping(true);
            //pixpainter.setClipRect(qRectF(0, 0, width(), height()));
            for (ubyte j = 0; j < 19; j++) {
                paintChannel(&pixpainter, j);
            }
            pixpainter.end();

            //QPixmapCache::insert(pixmapId, *pixmap);
            totalRepaint = false;
            setPixmap(*pixmap);
            // Set the background of the widget to prevent further
            // repaints.
            /*QPalette palette = this->palette();
            palette.setBrush(backgroundRole(), QBrush(notesPixmap));
            setPalette(palette);*/

            // TODO: I don't think we need this.

        }
        switch (updatemode) {
            case 1: {
                QPainter::PixmapFragment *frags = new QPainter::PixmapFragment[event->region().rectCount()];
                QVector<QRect> rects = event->region().rects();
                for (int i = 0; i < event->region().rectCount(); i++) {
                    QRect rect = rects.at(i);
                    frags[i] = QPainter::PixmapFragment::create(rect.center(), rect, 1, 1, 0, 1);
                }
                painter.drawPixmapFragments(frags, event->region().rectCount(), *pixmap);
                delete[] frags;
                break;
            }
            case 2: {
                QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
                                                                event->rect().center(), event->rect());
                painter.drawPixmapFragments(&frag, 1, *pixmap);
                break;
            }
            default:
                QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(viewport.center(), viewport);

                painter.drawPixmapFragments(&frag, 1, *pixmap);
                //painter.drawPixmap(0, 0, *pixmap);
        }
   // }
#endif
    if (_settings.antialiasing) {
        painter.setRenderHint(QPainter::Antialiasing);
    }
    // draw the piano / linenames
    if (Tool::currentTool()) {
        painter.save();
        painter.setClipping(true);
        painter.setClipRect(viewport);
        Tool::currentTool()->draw(&painter);
        painter.restore();
    }

    qreal timelinePos = timelineWidget->mousePosition();
    if (enabled && timelinePos >= 0 && !MidiPlayer::instance()->isPlaying()) {
        painter.setPen(Qt::red);
        painter.drawLine(qLineF(timelinePos, 0, timelinePos, height()));
    }

    if (MidiPlayer::instance()->isPlaying()) {
        painter.setPen(Qt::red);
        qreal x = xPosOfMs(MidiPlayer::instance()->timeMs() - _settings.playbackDelay);
        if (x >= 0 && viewport.contains(QPoint(qFloor(x), viewport.center().y()))) {
            painter.drawLine(qLineF(x, 0, x, height()));
        }

    }

    // border
    //	painter.setPen(Qt::gray);
    //	painter.drawLine(qLineF(width() - 1, height() - 1, 0,
    //							 height() - 1));
    //	painter.drawLine(qLineF(width() - 1, height() - 1, width() - 1, 2));

    // if the recorder is recording, show red circle
    if (MidiInput::instance()->recording()) {
        painter.setPen(Qt::black);
        painter.setBrush(Qt::red);
        painter.drawEllipse(qPointF(viewport.right() - 20.0, 5), 15, 15);
    }

    // if MouseRelease was not used, delete it
    mouseReleased = false;

    if (totalRepaint) {
        emit objectListChanged();
    }
}

void MatrixWidget::paintChannel(QPainter *painter, ubyte channel) {
    if (!file->channel(channel)->visible()) {
        return;
    }
    QColor cC = file->channel(channel)->color();

    // filter events
    QMultiMap<int, MidiEvent *> *map = file->channelEvents(channel);

    QMap<int, MidiEvent *>::const_iterator it = map->lowerBound(startTick);
    while (it != map->constEnd() && it.key() <= endTick) {
        MidiEvent *event = it.value();
        if (eventInWidget(event)) {
            // insert all Events in objects, set their coordinates
            // Only onEvents are inserted. When there is an On
            // and an OffEvent, the OnEvent will hold the coordinates
            ubyte line = event->line();

            OffEvent *offEvent = protocol_cast<OffEvent *>(event);
            OnEvent *onEvent = protocol_cast<OnEvent *>(event);

            qreal x, width;
            qreal y = yPosOfLine(line);
            qreal height = lineHeight();

            if (onEvent || offEvent) {
                if (onEvent) {
                    offEvent = onEvent->offEvent();
                } else if (offEvent) {
                    onEvent = offEvent->onEvent();
                }

                width = xPosOfMs(msOfTick(offEvent->midiTime())) -
                          xPosOfMs(msOfTick(onEvent->midiTime()));
                x = xPosOfMs(msOfTick(onEvent->midiTime()));
                event = onEvent;
                if (objects->contains(event)) {
                    ++it;
                    continue;
                }
            } else {
                width = PIXEL_PER_EVENT;
                x = xPosOfMs(msOfTick(event->midiTime()));
            }
            event->setX(x);
            event->setY(y);
            event->setWidth(width);
            event->setHeight(height);

            if (!(event->track()->hidden())) {
                if (!_colorsByChannels) {
                    cC = *event->track()->color();
                }
                event->draw(painter, cC);


                if (Selection::instance()->selectedEvents().contains(event)) {
                    painter->setPen(Qt::gray);
                    painter->drawLine(qLineF(0, y, this->width(), y));
                    painter->drawLine(qLineF(0, y + height, this->width(), y + height));
                    painter->setPen(Qt::black);

                }
                objects->prepend(event);
            }
        }

        if (!(event->track()->hidden())) {
            // append event to velocityObjects if its not a offEvent and if it
            // is in the x-Area
            OffEvent *offEvent = protocol_cast<OffEvent *>(event);
            if (!offEvent && event->midiTime() >= startTick &&
                      event->midiTime() <= endTick &&
                      !velocityObjects->contains(event)) {
                qreal mX = xPosOfMs(msOfTick(event->midiTime()));
                event->setX(mX);

                velocityObjects->prepend(event);
            }
        }
        ++it;
    }
}


void MatrixWidget::setFile(MidiFile *f) {

    file = f;

    scaleX = 1;
    scaleY = 1;

    // Roughly vertically center on Middle C.
    int widthNew = qRound((endTimeX / 1000) * (PIXEL_PER_S * scaleX));
    if (parentWidget()->width()
              && widthNew < parentWidget()->width()
              && parentWidget()->width() > 0) {
        if (widthNew != 0) {
            scaleX = parentWidget()->width() / widthNew;
        }
        widthNew = parentWidget()->width();
    }
    setFixedHeight(qRound(NUM_LINES * PIXEL_PER_LINE * scaleY));
    endTimeX = file->maxTime();

    setFixedWidth(widthNew);
    connect(file->protocol(), &Protocol::actionFinished, this, &MatrixWidget::redraw);

    calcSizes();

    // scroll down to see events
    int maxNote = -1;
    for (ubyte channel = 0; channel < 16; channel++) {

        QMultiMap<int, MidiEvent *> *map = file->channelEvents(channel);

        QMap<int, MidiEvent *>::const_iterator it = map->lowerBound(0);
        while (it != map->constEnd()) {
            NoteOnEvent *onev = protocol_cast<NoteOnEvent *>(it.value());
            if (onev && eventInWidget(onev)) {
                if (onev->line() < maxNote || maxNote < 0) {
                    maxNote = onev->line();
                }
            }
            ++it;
        }
    }

    if (maxNote - 5 > 0) {
        // startLineY = maxNote - 5.0;
    }
    redraw();
    emit scrollChanged(maxNote, 0);
}

void MatrixWidget::setPianoWidget(PianoWidget *widget) {
    pianoWidget = widget;
}

void MatrixWidget::setTimelineWidget(TimelineWidget *widget) {
    timelineWidget = widget;
}

void MatrixWidget::calcSizes() {
    if (!file) {
        return;
    }

    int widthOld = width();
    int heightOld = height();
    int widthNew = qRound((endTimeX / 1000) * (PIXEL_PER_S * scaleX));
    if (parentWidget()->width()
              && widthNew < parentWidget()->width()
              && parentWidget()->width() > 0) {
        if (widthNew != 0) {
            scaleX = parentWidget()->width() / widthNew;
        }
        widthNew = parentWidget()->width();
    }
    setFixedHeight(qRound(NUM_LINES * PIXEL_PER_LINE * scaleY));
    setFixedWidth(widthNew);
    delete pixmap;
    pixmap = new QPixmap(width(), height());
    totalRepaint = true;

    if (widthOld != widthNew || heightOld != height()) {
        redraw();
    }
}
QSize MatrixWidget::sizeHint() const {
    return QSize(1500, 1500);
}

MidiFile *MatrixWidget::midiFile() {
    return file;
}

void MatrixWidget::mouseMoveEvent(QMouseEvent *event) {
    PaintWidget::mouseMoveEvent(event);

    if (!enabled) {
        return;
    }

    if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {
        Tool::currentTool()->move(event->localPos().x(), event->localPos().y());
        if (mousePressed) {
            update(relativeRect());
        }
    }
}

void MatrixWidget::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    calcSizes();
}

qreal MatrixWidget::xPosOfMs(int ms) {
    return ms * qreal(width()) / endTimeX;
}

qreal MatrixWidget::yPosOfLine(ubyte line) {
    return line * lineHeight();
}

qreal MatrixWidget::lineHeight() {

    return qreal(height()) / qreal(NUM_LINES);
}

void MatrixWidget::enterEvent(QEvent *event) {
    PaintWidget::enterEvent(event);
    if (Tool::currentTool()) {
        Tool::currentTool()->enter();
        if (enabled) {
            update(relativeRect());
        }
    }
}
void MatrixWidget::leaveEvent(QEvent *event) {
    PaintWidget::leaveEvent(event);
    if (Tool::currentTool()) {
        Tool::currentTool()->exit();
        if (enabled) {
            update(relativeRect());
        }
    }
}
void MatrixWidget::mousePressEvent(QMouseEvent *event) {
    PaintWidget::mousePressEvent(event);

    if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {

        if (Tool::currentTool()->press(event->buttons() == Qt::LeftButton)) {
            if (enabled) {
                totalRepaint = true;

                update(relativeRect());
            }
        }
    } /*else if (enabled && (!MidiPlayer::instance()->isPlaying()) && (mouseInRect(PianoArea))) {
        foreach (int key, pianoKeys.keys()) {
            bool inRect = mouseInRect(pianoKeys.value(key));
            if (inRect) {
                // play note
                pianoEvent->setNote(key);
                MidiPlayer::instance()->play(pianoEvent);
            }
        }
    }*/
}
void MatrixWidget::mouseReleaseEvent(QMouseEvent *event) {
    PaintWidget::mouseReleaseEvent(event);

    if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {
        if (Tool::currentTool()->release()) {
            if (enabled) {
                update();
            }
        }
    } else if (Tool::currentTool()) {
        if (Tool::currentTool()->releaseOnly()) {
            if (enabled) {
                update();
            }
        }
    }
    //emit objectListChanged();

}

void MatrixWidget::takeKeyPressEvent(QKeyEvent *event) {
    if (Tool::currentTool()) {
        if (Tool::currentTool()->pressKey(event->key())) {
            update();
        }
    }
}

void MatrixWidget::takeKeyReleaseEvent(QKeyEvent *event) {
    if (Tool::currentTool()) {
        if (Tool::currentTool()->releaseKey(event->key())) {
            update();
        }
    }
    //emit objectListChanged();

}

QList<MidiEvent *> *MatrixWidget::activeEvents() {
    return objects;
}

QList<MidiEvent *> *MatrixWidget::velocityEvents() {
    return velocityObjects;
}


int MatrixWidget::msOfXPos(qreal x) {
    return qRound((x * endTimeX) / width());
}

int MatrixWidget::msOfTick(int tick) {
    return file->msOfTick(tick, currentTempoEvents, msOfFirstEventInList);
}

int MatrixWidget::timeMsOfWidth(int w) {
    return (w * endTimeX) / width() ;
}

bool MatrixWidget::eventInWidget(MidiEvent *event) {
    NoteOnEvent *on = protocol_cast<NoteOnEvent *>(event);
    OffEvent *off = protocol_cast<OffEvent *>(event);
    if (on) {
        off = on->offEvent();
    } else if (off) {
        on = protocol_cast<NoteOnEvent *>(off->onEvent());
    }
    if (on && off) {
        ubyte line = off->line();
        int tick = off->midiTime();
        bool offIn =  line >= 0 && line <= NUM_LINES && tick >= startTick &&
                          tick <= endTick;
        line = on->line();
        tick = on->midiTime();
        bool onIn = line >= 0 && line <= NUM_LINES && tick >= startTick &&
                        tick <= endTick;

        off->setShown(offIn);
        on->setShown(onIn);

        return offIn || onIn;

    } else {
        ubyte line = event->line();
        int tick = event->midiTime();
        bool shown = line >= 0 && line <= NUM_LINES && tick >= startTick &&
                         tick <= endTick;
        event->setShown(shown);

        return shown;
    }
}

ubyte MatrixWidget::lineAtY(qreal y) {
    return qRound(y / lineHeight());
}

void MatrixWidget::zoomStd() {
    scaleX = 1;
    scaleY = 1;
    calcSizes();
}

void MatrixWidget::zoomHorIn() {
    if (scaleX < MAX_HORIZ_ZOOM) {
        scaleX += 0.1;
        calcSizes();
    }
}

void MatrixWidget::zoomHorOut() {
    if (width() * (scaleX - 0.1f) >= parentWidget()->width() && scaleX >= 0.2f) {
        scaleX -= 0.1f;
        calcSizes();
    }
}

void MatrixWidget::zoomVerIn() {
    if (scaleY < MAX_VERT_ZOOM) {
        scaleY += 0.1f;
        calcSizes();
    }
}

void MatrixWidget::zoomVerOut() {
    if (height() * (scaleY - 0.1f) >= parentWidget()->width() && scaleY >= 0.2f) {
        scaleY -= 0.1f;
        calcSizes();

    }
}

int MatrixWidget::minVisibleMidiTime() {
    return startTick;
}

int MatrixWidget::maxVisibleMidiTime() {
    return endTick;
}
void MatrixWidget::keyPressEvent(QKeyEvent *event) {
    takeKeyPressEvent(event);
}

void MatrixWidget::keyReleaseEvent(QKeyEvent *event) {
    takeKeyReleaseEvent(event);
}

void MatrixWidget::setColorsByChannel() {
    _colorsByChannels = true;
}
void MatrixWidget::setColorsByTracks() {
    _colorsByChannels = false;
}

bool MatrixWidget::colorsByChannel() {
    return _colorsByChannels;
}

void MatrixWidget::setDiv(ubyte div) {
    _settings.div = div;
    redraw();
}

const QList<QPair<qreal, int> > MatrixWidget::divs() {
    return currentDivs;
}

void MatrixWidget::redraw() {
    if (!file ||  paintingActive() || pixmap->paintingActive()) {
        return;
    }
    //delete pixmap;
    //pixmap = new QPixmap(size());
    totalRepaint = true;
    QPixmap linesTexture;
    if (!QPixmapCache::find(_("MatrixWidget_%1").arg(scaleY), linesTexture)) {
        linesTexture = QPixmap(1, NUM_LINES);
        //linesTexture.fill(Qt::transparent);

        QPainter linesPainter(&linesTexture);
        if (_settings.antialiasing) {
            linesPainter.setRenderHint(QPainter::Antialiasing);
        }
        for (ubyte i = 0; i <= NUM_LINES; i++) {
           // ubyte startLine = yPosOfLine(i);
            QColor c(194, 230, 255);
            if (i % 2 == 0) {
                c = QColor(234, 246, 255);
            }
            if (i > 127) {
                c = QColor(194, 194, 194);
                if (i % 2 == 1) {
                    c = QColor(234, 246, 255);
                }
            }
            linesPainter.fillRect(qRectF(0, i, 1, i + 1), c);

        }
        linesPainter.end();
        /*QPixmapCache::insert("MatrixWidget_%1".arg(scaleY), linesTexture);
        QPalette palette;
        palette.setBrush(backgroundRole(), QBrush(linesTexture));
        setPalette(palette);*/
    }


    if (pixmap) {
        //pixmap = new QPixmap(size());
        delete pixmap;
        pixmap = qnullptr;
    }
    pixmap = new QPixmap(linesTexture.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::FastTransformation));

    if (pixmap->paintingActive()) {
        return;
    }


    // This struct serves as the ID of the events pixmap.
    // It stores the zoom level, measure division, and the UUID of the current ProtocolStep.
    // It is just one fancy unsigned int.
    MatrixStepKey keyStruct;
    keyStruct.data.mCurrStepId = file->protocol()->currentStepId();
    keyStruct.data.mScaleY = ubyte(scaleY * 10);
    keyStruct.data.mScaleX = ubyte(scaleX * 10);
    keyStruct.data.mDiv = _settings.div;
    keyStruct.data.mAntiAliased = _settings.antialiasing;

    // convert the keyStruct to a raw int.
    int key = keyStruct.rawValue;

    if (pictureCache.contains(key)) {
        qWarning("This repaint was brought to you by QCache!");

        QByteArray *array = pictureCache.object(key);
        QBuffer buffer(array);
        QPicture picture;

        buffer.open(QIODevice::ReadOnly);
        picture.load(&buffer);
        buffer.close();

        QPainter pixPainter(pixmap);
        pixPainter.drawPicture(QPointF(0, 0), picture);
        pixPainter.end();
    } else {
        qWarning("This is a raw repaint!");
        QPicture picture;
        QPainter picturePainter(&picture);

        picturePainter.setBrush(Qt::transparent);
        if (_settings.antialiasing) {
            picturePainter.setRenderHint(QPainter::Antialiasing);
        }

        QFont f = picturePainter.font();
        f.setPixelSize(12);
        picturePainter.setFont(f);
        picturePainter.setClipping(false);

        for (int i = 0; i < objects->length(); i++) {
            objects->at(i)->setShown(false);
            OnEvent *onev = protocol_cast<OnEvent *>(objects->at(i));
            if (onev && onev->offEvent()) {
                onev->offEvent()->setShown(false);
            }
        }
        objects->clear();
        velocityObjects->clear();
        currentTempoEvents->clear();
        currentTimeSignatureEvents->clear();
        currentDivs.clear();

        startTick = file->tick(0, endTimeX, &currentTempoEvents,
                                      &endTick, &msOfFirstEventInList);

        TempoChangeEvent *ev = protocol_cast<TempoChangeEvent *>(currentTempoEvents->at(0));
        if (!ev) {
            return;
        }

        // paint measures and timeline
        currentTimeSignatureEvents->clear();
        int measure = file->measure(0, file->endTick(), &currentTimeSignatureEvents);

        TimeSignatureEvent *currentEvent = currentTimeSignatureEvents->at(0);
        int i = 0;
        if (!currentEvent) {
            return;
        }
        int tick = currentEvent->midiTime();
        while (tick + currentEvent->ticksPerMeasure() <= 0) {
            tick += currentEvent->ticksPerMeasure();
        }
        qreal xfrom, xDiv, metronomeDiv;
        int measureStartTick, ticksPerDiv, startTickDiv, divTick;
        QPen oldPen, dashPen;
        while (tick < file->endTick()) {
            TimeSignatureEvent *measureEvent = currentTimeSignatureEvents->at(i);
            xfrom = xPosOfMs(msOfTick(tick));
            currentDivs.append(QPair<qreal, int>(xfrom, tick));
            measure++;
            measureStartTick = tick;
            tick += currentEvent->ticksPerMeasure();
            if (i < currentTimeSignatureEvents->length() - 1) {
                if (currentTimeSignatureEvents->at(i + 1)->midiTime() <= tick) {
                    currentEvent = currentTimeSignatureEvents->at(i + 1);
                    tick = currentEvent->midiTime();
                    i++;
                }
            }
            // draw measures
            if (_settings.div >= 0) {
                metronomeDiv = 4 / qPow(2, _settings.div);
                ticksPerDiv = metronomeDiv * file->ticksPerQuarter();
                startTickDiv = ticksPerDiv;
                oldPen = picturePainter.pen();
                dashPen = QPen(Qt::lightGray, 1, Qt::DashLine);
                picturePainter.setPen(dashPen);
                while (startTickDiv <= measureEvent->ticksPerMeasure()) {
                    divTick = startTickDiv + measureStartTick;
                    xDiv = xPosOfMs(msOfTick(divTick));
                    currentDivs.append(QPair<qreal, int>(xDiv, divTick));
                    picturePainter.drawLine(qLineF(xDiv, 0, xDiv, height()));
                    startTickDiv += ticksPerDiv;
                }
                picturePainter.setPen(oldPen);
            }
        }

        picturePainter.setPen(Qt::black);

        // paint the events
        //pixpainter.setClipping(true);
        //pixpainter.setClipRect(qRectF(0, 0, width(), height()));
        for (ubyte j = 0; j < 19; j++) {
            paintChannel(&picturePainter, j);
        }
        picturePainter.end();

        QPainter pixPainter(pixmap);
        pixPainter.drawPicture(QPointF(0, 0), picture);
        pixPainter.end();

        QByteArray *array = new QByteArray();
        QBuffer buffer(array);
        buffer.open(QIODevice::ReadWrite);
        picture.save(&buffer);
        buffer.close();
        pictureCache.insert(key, array);
        //QPixmapCache::insert(pixmapId, *pixmap);
        totalRepaint = false;
        //setPixmap(*pixmap);
        // Set the background of the widget to prevent further
        // repaints.
    }
        QPalette palette = this->palette();
        palette.setBrush(backgroundRole(), QBrush(*pixmap));
        setPalette(palette);

        // TODO: I don't think we need this.


  //  update();
}
