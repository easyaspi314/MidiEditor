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

#include "MiscWidget.h"
#include "MatrixWidget.h"
#include "../midi/MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiTrack.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../tool/EventTool.h"
#include "../tool/SelectTool.h"
#include "../tool/NewNoteTool.h"
#include "../protocol/Protocol.h"
#include "../tool/Selection.h"
#include "../midi/MidiOutput.h"

#include <QPixmapCache>
#include <QStringBuilder>

#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/PitchBendEvent.h"
#include "../MidiEvent/KeyPressureEvent.h"
#include "../MidiEvent/ChannelPressureEvent.h"

/*
 * FIXME:
 * - Kill all the repaint hacks
 * - Notes when created k
 * TODO:
 * - Set everything that doesn't need to be
 *   repainted to the background, and what does
 *   to the foreground.
 * - Color the channel lines?
 * - Smooth creation of lines; i.e. you click and
 *   drag for instant feedback.
 * - Find the proper update() time
 * - update() is regional
 */
MiscWidget::MiscWidget(MatrixWidget *mw, QWidget *parent) : PaintWidget(parent) {
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // FIXME: hack
    inited = false;
    setRepaintOnMouseMove(false);
    setRepaintOnMousePress(true);
    setRepaintOnMouseRelease(true);
    setMouseTracking(true);
    matrixWidget = mw;
    edit_mode = MiscWidgetEditMode::SingleMode;
    mode = MiscWidgetMode::VelocityEditor;
    channel = 0;
    QPixmapCache::setCacheLimit(20480);
    pixmap = new QPixmap(size());
    controller = 0;
    file = qnullptr;
    dragY = 0;
    isDrawingFreehand = false;
    isDrawingLine = false;
    resetState();
    computeMinMax();
    setFixedWidth(matrixWidget->width());
    _dummyTool = new SelectTool(SelectionType::Single);
    setFocusPolicy(Qt::ClickFocus);
}

/*
 * TODO: Hopefully get rid of this
 */
void MiscWidget::redraw() {
    inited = false;
    delete pixmap;
    pixmap = new QPixmap(size());
    update();
}

void MiscWidget::resizeEvent(QResizeEvent *event) {
    PaintWidget::resizeEvent(event);
    if (event->oldSize() != event->size()) {
        inited = false;

        delete pixmap;
        pixmap = new QPixmap(event->size());

    };
}

void MiscWidget::setFile(MidiFile *midiFile) {
    file = midiFile;
    inited = false;
    redraw();
    connect(matrixWidget, &MatrixWidget::objectListChanged, this, &MiscWidget::redraw);
}
const QString MiscWidget::modeToString(ubyte mode) {
    switch (MiscWidgetMode(mode)) {
        case MiscWidgetMode::VelocityEditor:
            return "Velocity";
        case MiscWidgetMode::ControlEditor:
            return "Control Change";
        case MiscWidgetMode::PitchBendEditor:
            return "Pitch Bend";
        case MiscWidgetMode::KeyPressureEditor:
            return "Key Pressure";
        case MiscWidgetMode::ChannelPressureEditor:
            return "Channel Pressure";
        default:
            return QString();
    }
}
void MiscWidget::startProtocol() {
    QString text;
    switch (mode) {
        case MiscWidgetMode::ControlEditor: {
            text = "Edited Control Change Events";
            break;
        }
        case MiscWidgetMode::PitchBendEditor: {
            text = "Edited Pitch Bend Events";
            break;
        }
        case MiscWidgetMode::KeyPressureEditor: {
            text = "Edited Key Pressure Events";
            break;
        }
        case MiscWidgetMode::ChannelPressureEditor: {
            text = "Edited Channel Pressure Events";
            break;
        }
    }

    file->protocol()->startNewAction(text);
}

void MiscWidget::setMode(MiscWidgetMode mode) {
    this->mode = mode;
    resetState();
    computeMinMax();
}

void MiscWidget::setEditMode(MiscWidgetEditMode mode) {
    this->edit_mode = mode;
    resetState();
    computeMinMax();
}

void MiscWidget::setChannel(int channel) {
    this->channel = ubyte(channel);
    resetState();
    computeMinMax();
}

void MiscWidget::setControl(int ctrl) {
    this->controller = ubyte(ctrl);
    resetState();
    computeMinMax();
}

void MiscWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    if (!matrixWidget || !file || !file->protocol() || paintingActive() || matrixWidget->divs().isEmpty()
            ||  matrixWidget->divs().size() <= 0) {
        return;
    }
    QPainter painter(this);
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
        painter.setClipRect(relativeRect());
    }
    if (width() != matrixWidget->width() && matrixWidget->width() != 0) {
        setFixedWidth(matrixWidget->width());
    }
    QPixmap background;
    if (!QPixmapCache::find(_("MiscWidget_%1").arg(height()), background)) {
        background = QPixmap(1, height());
        background.fill(Qt::transparent);
        QPainter bgpainter(&background);
        QColor c(234, 246, 255);
        if (_settings.antialiasing) {
            bgpainter.setRenderHint(QPainter::Antialiasing);
        }
        bgpainter.setPen(c);
        bgpainter.setBrush(c);
        bgpainter.drawRect(qRectF(0, 0, 0, relativeRect().height() ));

        bgpainter.setPen(QColor(194, 230, 255));
        for (ubyte i = 0; i < 8; i++) {
            bgpainter.drawLine(qLineF(0, (i * height()) >> 3, 1, (i * height()) >> 3));
        }
        bgpainter.end();
        QPixmapCache::insert(_("MiscWidget_%1").arg(height()), background);
        QPalette palette;
        palette.setBrush(backgroundRole(), QBrush(background));
        setPalette(palette);
    }
    QByteArray array("Misc");
    array += QByteArray::number(file->protocol()->currentStepId(), 36);
    array += QByteArray::number(height(), 36);
    array += QByteArray::number(ubyte(mode), 36);
    array += QByteArray::number(_settings.div);
    array += QByteArray::number(int(matrixWidget->scaleX * 10), 36);
    QString notesId = QString::fromUtf8(array);

    // FIXME: Hacks.
    bool totalRepaint = !inited || !pixmap || !QPixmapCache::find(notesId, pixmap);
    if (totalRepaint || dragging) {
        pixmap->fill(Qt::transparent);
        QPainter pixpainter(pixmap);
        if (_settings.antialiasing) {
            pixpainter.setRenderHint(QPainter::Antialiasing);
        }
        //pixpainter.fillRect(0, 0, width(), height(), QBrush(background));
        pixpainter.setBrush(QColor(234, 246, 255));
        pixpainter.setPen(QColor(194, 230, 255));

        for (const QPair<qreal, int> &p : matrixWidget->divs()) {
            pixpainter.drawLine(qLineF(p.first , 0, p.first, height()));
        }

        // draw contents
        if (mode == MiscWidgetMode::VelocityEditor) {
            const QList<MidiEvent *> *list = matrixWidget->velocityEvents();
            for (MidiEvent *event : *list) {

                if (!file->channel(event->channel())->visible()) {
                    continue;
                }

                if (event->track()->hidden()) {
                    continue;
                }

                QColor c = file->channel(event->channel())->color();
                if (!matrixWidget->colorsByChannel()) {
                    c = *event->track()->color();
                }

                ubyte velocity = 0;
                NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
                if (noteOn) {
                    velocity = noteOn->velocity();
                    int h;
                    if (velocity > 0) {

                        if (_settings.gba_mode) {
                            h = (height() * sqrt(velocity * 127))/128;
                        } else {
                            h = (height()*velocity)/128;
                        }

                        pixpainter.setBrush(c);
                        pixpainter.setPen(Qt::lightGray);
                        pixpainter.drawRoundedRect(qRectF(event->x(),
                                                          height() - h, WIDTH, h), 1, 1);
                    }
                }
            }

            // paint selected events above all others
            EventTool *t = qobject_cast<EventTool *>(Tool::currentTool());
            if (t && t->showsSelection()) {
                for (MidiEvent *event : Selection::instance()->selectedEvents()) {

                    if (!file->channel(event->channel())->visible()) {
                        continue;
                    }

                    if (event->track()->hidden()) {
                        continue;
                    }

                    ubyte velocity = 0;
                    qreal velocityMultiplier = 1.0;
                    NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);

                    if (noteOn && noteOn->midiTime() >= matrixWidget->minVisibleMidiTime()
                            && noteOn->midiTime() <= matrixWidget->maxVisibleMidiTime()) {
                        velocity = noteOn->velocity();
                        if (aboveEvent) {
                            velocityMultiplier = velocity / aboveEvent->velocity();
                        }

                        int h;
                        if (velocity > 0) {

                            if (_settings.gba_mode) {
                                h = (height() * sqrt(velocity * 127))/128;
                            } else {
                                h = (height()*velocity)/128;
                            }

                            if (edit_mode == MiscWidgetEditMode::SingleMode && dragging) {
                                h = velocityMultiplier * (h + (dragY - mouseY));
                            }
                            pixpainter.setBrush(Qt::darkBlue);
                            pixpainter.setPen(Qt::lightGray);
                            pixpainter.drawRoundedRect(qRectF(event->x(),
                                                              height() - h, WIDTH, h), 1, 1);
                        }
                    }
                }
            }
        }


        // draw content track
        if (mode > MiscWidgetMode::VelocityEditor) {

            QColor c = file->channel(0)->color();
            QPen pen(c);
            pen.setWidth(3);
            pixpainter.setPen(pen);

            QPen circlePen(Qt::darkGray);
            circlePen.setWidth(1);

            QList<MidiEvent *> accordingEvents;
            QList<QPair<qreal, ushort> > track = getTrack(&accordingEvents);

            qreal xOld;
            qreal yOld;

            for (int i = 0; i < track.size(); i++) {

                qreal xPix = track.at(i).first;
                qreal yPix;

                if (_settings.gba_mode && mode == MiscWidgetMode::ControlEditor && controller == 7 /* Channel Volume */) {
                    yPix = height()-(sqrt(_max * track.at(i).second)/_max)*height();
                } else {
                    yPix = height()-(track.at(i).second/_max)*height();
                }

                if (edit_mode == MiscWidgetEditMode::SingleMode) {
                    if (i == trackIndex) {
                        if (dragging) {
                            yPix = yPix + mouseY - dragY;
                        }
                    }
                }
                if (i > 0) {
                    pixpainter.drawLine(qLineF(xOld, yOld, xPix, yOld));
                    pixpainter.drawLine(qLineF(xPix, yOld, xPix, yPix));
                }
                xOld = xPix;
                yOld = yPix;
                if (edit_mode == MiscWidgetEditMode::SingleMode && (dragging || mouseOver)) {

                    if (accordingEvents.at(i)
                            && Selection::instance()->selectedEvents().contains(accordingEvents.at(i))) {
                        pixpainter.setBrush(Qt::darkBlue);
                    }
                    pixpainter.setPen(circlePen);
                    if (i == trackIndex) {
                        pixpainter.setBrush(Qt::gray);
                    }
                    pixpainter.drawEllipse(qRectF(xPix - 4, yPix - 4, 8, 8));
                    pixpainter.setPen(pen);
                    pixpainter.setBrush(Qt::NoBrush);
                }
}
            pixpainter.drawLine(qLineF(xOld, yOld, width(), yOld));

        }

        // draw freehand track
        if (edit_mode == MiscWidgetEditMode::FreehandMode && isDrawingFreehand && freeHandCurve.size() > 0) {

            qreal xOld;
            qreal yOld;

            QPen pen(Qt::darkBlue);
            pen.setWidth(3);
            pixpainter.setPen(pen);

            for (int i = 0; i < freeHandCurve.size(); i++) {
                qreal xPix = freeHandCurve.at(i).first;
                qreal yPix = freeHandCurve.at(i).second;
                if (i > 0) {
                    pixpainter.drawLine(qLineF(xOld, yOld, xPix, yPix));
                }
                xOld = xPix;
                yOld = yPix;
            }
        }

        // draw line
        if (edit_mode == MiscWidgetEditMode::LineMode && isDrawingLine) {
            QPen pen(Qt::darkBlue);
            pen.setWidth(3);
            pixpainter.setPen(pen);

            pixpainter.drawLine(qLineF(lineX, lineY, mouseX, mouseY));
        }
        pixpainter.end();

        QPixmapCache::insert(notesId, *pixmap);

        inited = true;
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
            QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(relativeRect().center(), relativeRect());

            painter.drawPixmapFragments(&frag, 1, *pixmap);
            //painter.drawPixmap(0, 0, *pixmap);
    }
}

void MiscWidget::mouseMoveEvent(QMouseEvent *event) {
    if (edit_mode == MiscWidgetEditMode::SingleMode) {
        if (mode == MiscWidgetMode::VelocityEditor) {
            bool above = dragging;
            if (!above) {
                const QList<MidiEvent *> *list = matrixWidget->velocityEvents();
                for (MidiEvent *event : *list) {

                    if (!event->file()->channel(event->channel())->visible()) {
                        continue;
                    }

                    if (event->track()->hidden()) {
                        continue;
                    }

                    int velocity = 0;
                    NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
                    if (noteOn) {
                        velocity = noteOn->velocity();

                        if (velocity > 0) {
                            int h;
                            if (_settings.gba_mode)
                                h = (height() * sqrt(velocity * 127))/127;
                            else
                                h = (height()*velocity)/128;
                            if (!dragging && mouseInRect(qRectF(event->x(), height() - h - 5, WIDTH, 10))) {
                                above = true;
                                aboveEvent = noteOn;
                                break;
                            }
                        }
                    }
                }
            }
            if (above) {
                setCursor(Qt::SizeVerCursor);
            } else {
                setCursor(Qt::ArrowCursor);
            }

        } else {

            //other modes
            if (!dragging) {
                trackIndex = -1;
                QList<QPair<qreal, ushort> > track = getTrack();
                for (int i = 0; i < track.size(); i++) {

                    qreal xPix = track.at(i).first;
                    qreal yPix;
                    if (_settings.gba_mode && mode == MiscWidgetMode::ControlEditor && controller == 7)
                        yPix = height()-(sqrt(qreal(_max) * track.at(i).second)/qreal(_max))*height();
                    else
                        yPix = height()-(track.at(i).second/qreal(_max))*height();

                    if (mouseInRect(xPix - 4, yPix - 4, 8, 8)) {
                        trackIndex = i;
                        //setCursor(Qt::SizeVerCursor);
                        setCursor(Qt::ArrowCursor);
                        break;
                    }
                }

                if (trackIndex == -1) {
                    setCursor(Qt::ArrowCursor);
                }
            } else {
                setCursor(Qt::SizeVerCursor);
            }
        }
    }
    if (edit_mode == MiscWidgetEditMode::FreehandMode) {
        if (isDrawingFreehand) {
            bool ok = true;
            for (int i = 0; i < freeHandCurve.size(); i++) {
                if (freeHandCurve.at(i).first >= qPointF(event->localPos()).x()) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                freeHandCurve.append(QPair<qreal, qreal>(qPointF(event->localPos()).x(), qPointF(event->localPos()).y()));
            }
        }
    }
    PaintWidget::mouseMoveEvent(event);
}

void MiscWidget::mousePressEvent(QMouseEvent *event) {

    if (edit_mode == MiscWidgetEditMode::SingleMode) {

        if (mode == MiscWidgetMode::VelocityEditor) {

            // check whether selection has to be changed.
            bool clickHandlesSelected = false;
            for (MidiEvent *event : Selection::instance()->selectedEvents()) {

                byte velocity = 0;
                NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);

                if (noteOn && noteOn->midiTime() >= matrixWidget->minVisibleMidiTime()
                        && noteOn->midiTime() <= matrixWidget->maxVisibleMidiTime()) {
                    velocity = noteOn->velocity();
                }
                int h;
                if (velocity > 0) {
                    if (_settings.gba_mode)
                        h = (height() * sqrt(velocity*127))/128;
                    else
                        h = (height()*velocity)/128;
                    if (!dragging
                            && mouseInRect(qRectF(event->x(), height() - h - 5, WIDTH,
                                            10))) {
                        clickHandlesSelected = true;
                        break;
                    }
                }
            }

            // find event to select
            bool selectedNew = false;
            if (!clickHandlesSelected) {
                const QList<MidiEvent *> *list = matrixWidget->velocityEvents();
                for (MidiEvent *event : *list) {

                    if (!event->file()->channel(event->channel())->visible()) {
                        continue;
                    }

                    if (event->track()->hidden()) {
                        continue;
                    }

                    byte velocity = 0;
                    NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
                    if (noteOn) {
                        velocity = noteOn->velocity();
                    }
                    if (velocity > 0) {
                        qreal h;
                        if (_settings.gba_mode)
                            h = (height()*sqrt(velocity*127))/128;
                        else
                            h = (height()*velocity)/128;
                        if (!dragging
                                && mouseInRect(qRectF(event->x(), height() - h - 5, WIDTH,
                                                10))) {
                            file->protocol()->
                            startNewAction("Selection changed",
                                            new QImage(":/run_environment/graphics/tool/select_single.png"), false);
                            ProtocolEntry *toCopy = _dummyTool->copy();
                            EventTool::selectEvent(event, true);
                            selectedNew = true;
                            _dummyTool->protocol(toCopy, _dummyTool);
                            file->protocol()->endAction();
                            break;
                        }
                    }
                }
            }

            // if nothing selected deselect all
            if (Selection::instance()->selectedEvents().size() > 0 && !clickHandlesSelected
                    && !selectedNew) {
                file->protocol()->
                startNewAction("Cleared selection", qnullptr, false);
                ProtocolEntry *toCopy = _dummyTool->copy();
                EventTool::clearSelection();
                _dummyTool->protocol(toCopy, _dummyTool);
                file->protocol()->endAction();
                matrixWidget->update();
            }

            // start drag
            if (Selection::instance()->selectedEvents().size() > 0) {
                dragY = mouseY;
                dragging = true;
            }
        } else {

            //other modes
            trackIndex = -1;
            QList<MidiEvent *> accordingEvents;
            QList<QPair<qreal, ushort> > track = getTrack(&accordingEvents);
            for (int i = 0; i < track.size(); i++) {

                qreal xPix = track.at(i).first;
                qreal yPix;
                if (_settings.gba_mode && mode == MiscWidgetMode::ControlEditor && controller == 7)
                    yPix = height()-((qreal)sqrt((qreal)_max * (qreal)track.at(i).second)/(qreal)_max)*height();
                else
                    yPix = height()-((qreal)track.at(i).second/(qreal)_max)*height();

                if (!dragging && mouseInRect(qRectF(xPix - 4, yPix - 4, 8, 8))) {
                    trackIndex = i;

                    if (accordingEvents.at(i)) {
                        file->protocol()->
                        startNewAction("Selection changed",
                                        new QImage(":/run_environment/graphics/tool/select_single.png"), false);
                        ProtocolEntry *toCopy = _dummyTool->copy();
                        EventTool::clearSelection();
                        EventTool::selectEvent(accordingEvents.at(i), true, true);
                        matrixWidget->update();
                        _dummyTool->protocol(toCopy, _dummyTool);
                        file->protocol()->endAction();
                    }
                    break;
                }
            }

            if (trackIndex > -1) {
                dragging = true;
                dragY = mouseY;
            }
        }

    } else if (edit_mode == MiscWidgetEditMode::FreehandMode) {
        freeHandCurve.clear();
        isDrawingFreehand = true;
    } else if (edit_mode == MiscWidgetEditMode::LineMode) {
        lineX = qPointF(event->localPos()).x();
        lineY = qPointF(event->localPos()).y();
        isDrawingLine = true;
    }

    if (event->button() == Qt::LeftButton) {
        PaintWidget::mousePressEvent(event);
        return;
    }
}

void MiscWidget::mouseReleaseEvent(QMouseEvent *event) {

    if (event->button() == Qt::LeftButton) {
        PaintWidget::mouseReleaseEvent(event);
    }

    if (edit_mode == MiscWidgetEditMode::SingleMode) {
        if (mode == MiscWidgetMode::VelocityEditor) {
            if (dragging) {

                dragging = false;

                qreal dX = dragY - mouseY;

                if (dX < -3 || dX > 3) {
                    file->protocol()->
                    startNewAction("Edited velocity");

                    qreal aboveEventVelocity = -1;
                    if (aboveEvent) {
                        aboveEventVelocity = aboveEvent->velocity();
                    }

                    qreal dV = 127 * dX / height();
                    for (MidiEvent *event : Selection::instance()->selectedEvents()) {
                        NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
                        if (noteOn) {

                            qreal v;
                            if (_settings.gba_mode)
                                v = dV+sqrt(noteOn->velocity() * 127);
                            else
                                v = dV+noteOn->velocity();
                            if (aboveEventVelocity > 0) {
                                v *= (aboveEventVelocity / noteOn->velocity());
                            }

                            if (v > 127) {
                                v = 127;
                            }
                            if (v < 0) {
                                v = 0;
                            }
                            if (_settings.gba_mode)
                                noteOn->setVelocity(ubyte(qRound(pow(v, 2) / 127)));
                            else
                                noteOn->setVelocity(ubyte(v));
                        }
                    }

                    file->protocol()->endAction();
                }
            }
        } else {
            // other modes
            if (dragging) {

                QList<MidiEvent *> accordingEvents;
                getTrack(&accordingEvents);
                MidiEvent *ev = accordingEvents.at(trackIndex);

                MidiTrack *track = file->track(NewNoteTool::editTrack());
                if (!track) {
                    return;
                }

                qreal dX = dragY - mouseY;
                if (dX < -3 || dX > 3) {

                    int v = qRound(value(mouseY));

                    startProtocol();
                    if (ev) {
                        if (v < 0) {
                            v = 0;
                        }
                        switch (mode) {
                            case MiscWidgetMode::ControlEditor: {
                                ControlChangeEvent *ctrl = qobject_cast<ControlChangeEvent *>(ev);
                                if (ctrl) {
                                    if (_settings.gba_mode && ctrl->control() == 7) {
                                        v = toExpVal(v);
                                    }
                                    if (v > 127) {
                                        v = 127;
                                    }

                                    ctrl->setValue(ubyte(v));
                                }
                                break;
                            }
                            case MiscWidgetMode::PitchBendEditor: {
                                PitchBendEvent *event = qobject_cast<PitchBendEvent *>(ev);
                                if (event) {
                                    if (v > 16383) {
                                        v = 16383;
                                    }
                                    event->setValue(ushort(v));
                                }
                                break;
                            }
                            case MiscWidgetMode::KeyPressureEditor: {
                                KeyPressureEvent *event = qobject_cast<KeyPressureEvent *>(ev);
                                if (event) {
                                    if (v > 127) {
                                        v = 127;
                                    }
                                    event->setValue(ubyte(v));
                                }
                                break;
                            }
                            case MiscWidgetMode::ChannelPressureEditor: {
                                ChannelPressureEvent *event = qobject_cast<ChannelPressureEvent *>(ev);
                                if (event) {
                                    if (v > 127) {
                                        v = 127;
                                    }
                                    event->setValue(ubyte(v));
                                }
                                break;
                            }
                        }

                    } else {

                        MidiTrack *track = file->track(NewNoteTool::editTrack());
                        if (!track) {
                            return;
                        }

                        int tick = matrixWidget->minVisibleMidiTime();

                        if (v < 0) {
                            v = 0;
                        }

                        switch (mode) {
                            case MiscWidgetMode::ControlEditor: {
                                if (controller == 7)
                                    v = toExpVal(v);
                                if (v > 127) {
                                    v = 127;
                                }

                                ControlChangeEvent *ctrl = new ControlChangeEvent(channel, controller, ubyte(v),
                                        track);
                                file->insertEventInChannel(channel, ctrl, tick);
                                break;
                            }
                            case MiscWidgetMode::PitchBendEditor: {
                                if (v > 16383) {
                                    v = 16383;
                                }
                                PitchBendEvent *event = new PitchBendEvent(channel, ushort(v), track);
                                file->insertEventInChannel(channel, event, tick);
                                break;
                            }
                            case MiscWidgetMode::KeyPressureEditor: {
                                if (v > 127) {
                                    v = 127;
                                }
                                KeyPressureEvent *event = new KeyPressureEvent(channel, ubyte(v), controller, track);
                                file->insertEventInChannel(channel, event, tick);
                                break;
                            }
                            case MiscWidgetMode::ChannelPressureEditor: {
                                if (v > 127) {
                                    v = 127;
                                }
                                ChannelPressureEvent *event = new ChannelPressureEvent(channel, v, track);
                                file->insertEventInChannel(channel, event, tick);
                                break;
                            }
                        }
                    }

                    file->protocol()->endAction();
                }

                dragging = false;
                trackIndex = -1;
            } else {

                // insert new event
                int tick = file->tick(matrixWidget->msOfXPos(mouseX));
                int v = qRound(value(mouseY));

                QString text;
                switch (mode) {
                    case MiscWidgetMode::ControlEditor: {
                        text = "Inserted Control Change Event";
                        break;
                    }
                    case MiscWidgetMode::PitchBendEditor: {
                        text = "Inserted Pitch Bend Event";
                        break;
                    }
                    case MiscWidgetMode::KeyPressureEditor: {
                        text = "Inserted Key Pressure Event";
                        break;
                    }
                    case MiscWidgetMode::ChannelPressureEditor: {
                        text = "Inserted Channel Pressure Event";
                        break;
                    }
                }

                file->protocol()->startNewAction(text);

                MidiTrack *track = file->track(NewNoteTool::editTrack());
                if (!track) {
                    return;
                }
                if (v < 0) {
                    v = 0;
                }
                if (tick < 0) {
                    tick = 0;
                }
                switch (mode) {
                    case MiscWidgetMode::ControlEditor: {
                        if (_settings.gba_mode && controller == 7)
                            v = qRound(pow(v, 2) / 127);
                        if (v > 127) {
                            v = 127;
                        }
                        ControlChangeEvent *ctrl = new ControlChangeEvent(channel, controller, v,
                                track);
                        file->insertEventInChannel(channel, ctrl, tick);
                        break;
                    }
                    case MiscWidgetMode::PitchBendEditor: {
                        if (v > 16383) {
                            v = 16383;
                        }
                        PitchBendEvent *event = new PitchBendEvent(channel, v, track);
                        file->insertEventInChannel(channel, event, tick);
                        break;
                    }
                    case MiscWidgetMode::KeyPressureEditor: {
                        if (v > 127) {
                            v = 127;
                        }
                        KeyPressureEvent *event = new KeyPressureEvent(channel, v, controller, track);
                        file->insertEventInChannel(channel, event, tick);
                        break;
                    }
                    case MiscWidgetMode::ChannelPressureEditor: {
                        if (v > 127) {
                            v = 127;
                        }
                        ChannelPressureEvent *event = new ChannelPressureEvent(channel, v, track);
                        file->insertEventInChannel(channel, event, tick);
                        break;
                    }
                }

                file->protocol()->endAction();
            }
        }
    }

    QList<QPair<qreal, qreal> > toAlign;

    if (edit_mode == MiscWidgetEditMode::FreehandMode || edit_mode == MiscWidgetEditMode::LineMode) {

        // get track data
        if (edit_mode == MiscWidgetEditMode::FreehandMode) {
            if (isDrawingFreehand) {
                isDrawingFreehand = false;
                // process data
                toAlign = freeHandCurve;
                freeHandCurve.clear();
            }
        } else if (edit_mode == MiscWidgetEditMode::LineMode) {
            if (isDrawingLine) {
                // process data
                isDrawingLine = false;
                if (lineX < mouseX) {
                    toAlign.append(QPair<qreal, qreal>(lineX, lineY));
                    toAlign.append(QPair<qreal, qreal>(mouseX, mouseY));
                } else if (lineX > mouseX) {
                    toAlign.append(QPair<qreal, qreal>(mouseX, mouseY));
                    toAlign.append(QPair<qreal, qreal>(lineX, lineY));
                }
            }
        }

        if (toAlign.size() > 0) {

            int minTick = file->tick(matrixWidget->msOfXPos(toAlign.first().first));
            int maxTick = file->tick(matrixWidget->msOfXPos(toAlign.last().first));

            // process data
            if (mode == MiscWidgetMode::VelocityEditor) {

                // when any events are selected, only use those. Else all
                // in the range
                QList<MidiEvent *> events;
                if (!Selection::instance()->selectedEvents().isEmpty()) {
                    for (MidiEvent *event : Selection::instance()->selectedEvents()) {
                        NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
                        if (noteOn) {
                            if (noteOn->midiTime() >= minTick && noteOn->midiTime() <= maxTick) {
                                events.append(event);
                            }
                        }
                    }
                } else {
                    const QList<MidiEvent *> *list = matrixWidget->velocityEvents();
                    for (MidiEvent *event : *list) {

                        if (!file->channel(event->channel())->visible()) {
                            continue;
                        }

                        if (event->track()->hidden()) {
                            continue;
                        }

                        NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
                        if (noteOn) {
                            if (noteOn->midiTime() >= minTick && noteOn->midiTime() <= maxTick) {
                                events.append(event);
                            }
                        }
                    }
                }

                if (events.size() > 0) {

                    file->protocol()->startNewAction("Changed velocity");

                    // process per event
                    for (MidiEvent *event : events) {

                        NoteOnEvent *noteOn = qobject_cast<NoteOnEvent *>(event);
                        if (noteOn) {

                            int tick = noteOn->midiTime();
                            int x = matrixWidget->xPosOfMs(file->msOfTick(tick));
                            qreal y = interpolate(toAlign, x);

                            qreal v = 127 * (height() - y) / height();

                            noteOn->setVelocity(qFloor(v));
                        }
                    }
                    file->protocol()->endAction();
                }
            } else {


                startProtocol();

                // remove old events
                QList<MidiEvent *> *list = matrixWidget->velocityEvents();
                for (int i = 0; i < list->size(); i++) {
                    if (list->at(i) && list->at(i)->channel() == channel) {
                        if (list->at(i)->midiTime() >= minTick && list->at(i)->midiTime() <= maxTick
                                && filter(list->at(i))) {
                            file->channel(channel)->removeEvent(list->at(i));
                        }
                    }
                }

                // compute events
                int stepSize = 10;

                int lastValue = -1;
                for (int tick = minTick; tick <= maxTick; tick += stepSize) {

                    qreal x = matrixWidget->xPosOfMs(file->msOfTick(
                                                         tick));
                    qreal y = interpolate(toAlign, int(x));
                    int v = qRound(value(y));
                    if ((lastValue != -1) && (lastValue == v)) {
                        continue;
                    }
                    MidiTrack *track = file->track(NewNoteTool::editTrack());
                    if (!track) {
                        return;
                    }
                    lastValue = v;
                    switch (mode) {
                        case MiscWidgetMode::ControlEditor: {
                            if (controller == 7)
                                    v = toExpVal(v);
                            ControlChangeEvent *ctrl = new ControlChangeEvent(channel, controller, v,
                                    track);
                            file->insertEventInChannel(channel, ctrl, tick);
                            break;
                        }
                        case MiscWidgetMode::PitchBendEditor: {
                            PitchBendEvent *event = new PitchBendEvent(channel, v, track);
                            file->insertEventInChannel(channel, event, tick);
                            break;
                        }
                        case MiscWidgetMode::KeyPressureEditor: {
                            KeyPressureEvent *event = new KeyPressureEvent(channel, v, controller, track);
                            file->insertEventInChannel(channel, event, tick);
                            break;
                        }
                        case MiscWidgetMode::ChannelPressureEditor: {
                            ChannelPressureEvent *event = new ChannelPressureEvent(channel, v, track);
                            file->insertEventInChannel(channel, event, tick);
                            break;
                        }
                    }
                }

                file->protocol()->endAction();
            }
        }
    }

}

qreal MiscWidget::value(qreal y) {
    qreal v = _max * (height() - y) / height();
    if (v > _max) {
        v = _max;
    }
    return v;
}

qreal MiscWidget::interpolate(const QList<QPair<qreal, qreal> > &track, qreal x) {

    for (int i = 0; i < track.size(); i++) {

        if (qFuzzyCompare(track.at(i).first, x)) {
            return qreal(track.at(i).second);
        }

        if (track.at(i).first > x) {

            if (i == 0) {
                return qreal(track.at(i).second);
            } else {

                return qreal(track.at(i - 1).second) +
                        qreal(track.at(i).second - track.at(i - 1).second) * qreal(x - track.at(
                                    i - 1).first) / qreal(track.at(i).first - track.at(i - 1).first);
            }
        }
    }

    return 0;
}

void MiscWidget::leaveEvent(QEvent *event) {
    resetState();
    PaintWidget::leaveEvent(event);
}

void MiscWidget::resetState() {

    dragY = -1;
    dragging = false;
    freeHandCurve.clear();
    isDrawingFreehand = false;
    isDrawingLine = false;

    qWarning("resetState");
    trackIndex = -1;
    redraw();
}

void MiscWidget::keyPressEvent(QKeyEvent *event) {
    if (Tool::currentTool()) {
        if (Tool::currentTool()->pressKey(event->key())) {
            qWarning("keyPressEvent");
            update();
        }
    }
}

void MiscWidget::keyReleaseEvent(QKeyEvent *event) {
    if (Tool::currentTool()) {
        if (Tool::currentTool()->releaseKey(event->key())) {
            qWarning("keyReleaseEvent");
            redraw();
        }
    }
}

QList<QPair<qreal, ushort> > MiscWidget::getTrack(QList<MidiEvent *> *accordingEvents) {

    QList<QPair<qreal, ushort> > track;

    // get list of all events in window
    QList<MidiEvent *> *list = matrixWidget->velocityEvents();

    // get all events before the start tick to find out value before start
    int startTick = matrixWidget->minVisibleMidiTime();
    QMultiMap<int, MidiEvent *> *channelEvents = file->channel(channel)->eventMap();
    QMultiMap<int, MidiEvent *>::const_iterator it = channelEvents->upperBound(startTick);

    bool ok = false;
    ushort valueBefore = _default;
    MidiEvent *evBef = qnullptr;

    if (channelEvents->size() > 0) {
        bool atEnd = false;
        while (!atEnd) {
            if (it != channelEvents->constEnd() && it.key() <= startTick) {
                QPair<qreal, ushort> p = processEvent(it.value(), &ok);
                if (ok) {
                    valueBefore = p.second;
                    evBef = it.value();
                    atEnd = true;
                }
            }
            if (it == channelEvents->constBegin()) {
                atEnd = true;
            } else {
                --it;
            }
        }
    }
    track.append(QPair< qreal, ushort>(0, valueBefore));
    if (accordingEvents) {
        accordingEvents->append(evBef);
    }
    // filter and extract values
    for (int i = list->size() - 1; i >= 0; i--) {
        if (list->at(i) && list->at(i)->channel() == channel) {
            QPair<qreal, ushort> p = processEvent(list->at(i), &ok);
            if (ok) {
                if (list->at(i)->midiTime() == startTick) {
                    // remove added event
                    track.removeFirst();
                    if (accordingEvents) {
                        accordingEvents->removeFirst();
                    }
                }
                track.append(p);
                if (accordingEvents) {
                    accordingEvents->append(list->at(i));
                }
            }
        }
    }

    return track;
}

bool MiscWidget::filter(MidiEvent *e) {
    switch (mode) {
        case MiscWidgetMode::ControlEditor: {
            ControlChangeEvent *ctrl = qobject_cast<ControlChangeEvent *>(e);
            if (ctrl && ctrl->control() == controller) {
                return true;
            } else {
                return false;
            }
        }
        case MiscWidgetMode::PitchBendEditor: {
            PitchBendEvent *pitch = qobject_cast<PitchBendEvent *>(e);
            if (pitch) {
                return true;
            } else {
                return false;
            }
        }
        case MiscWidgetMode::KeyPressureEditor: {
            KeyPressureEvent *pressure = qobject_cast<KeyPressureEvent *>(e);
            if (pressure && pressure->note() == controller) {
                return true;
            } else {
                return false;
            }
        }
        case MiscWidgetMode::ChannelPressureEditor: {
            ChannelPressureEvent *pressure = qobject_cast<ChannelPressureEvent *>(e);
            if (pressure) {
                return true;
            } else {
                return false;
            }
        }
    }
    return false;
}

QPair<qreal, ushort> MiscWidget::processEvent(MidiEvent *e, bool *isOk) {

    *isOk = false;
    QPair<qreal, ushort> pair(-1, 0xFFFF);
    switch (mode) {
        case MiscWidgetMode::ControlEditor: {
            ControlChangeEvent *ctrl = qobject_cast<ControlChangeEvent *>(e);
            if (ctrl && ctrl->control() == controller) {
                qreal x = ctrl->x();
                ubyte y = ctrl->value();
                pair.first = x;
                pair.second = y;
                *isOk = true;
            }
            break;
        }
        case MiscWidgetMode::PitchBendEditor: {
            PitchBendEvent *pitch = qobject_cast<PitchBendEvent *>(e);
            if (pitch) {
                qreal x = pitch->x();
                ushort y = pitch->value();
                pair.first = x;
                pair.second = y;
                *isOk = true;
            }
            break;
        }
        case MiscWidgetMode::KeyPressureEditor: {
            KeyPressureEvent *pressure = qobject_cast<KeyPressureEvent *>(e);
            if (pressure && pressure->note() == controller) {
                qreal x = pressure->x();
                ushort y = pressure->value();
                pair.first = x;
                pair.second = y;
                *isOk = true;
            }
            break;
        }
        case MiscWidgetMode::ChannelPressureEditor: {
            ChannelPressureEvent *pressure = qobject_cast<ChannelPressureEvent *>(e);
            if (pressure) {
                qreal x = pressure->x();
                ushort y = pressure->value();
                pair.first = x;
                pair.second = y;
                *isOk = true;
            }
            break;
        }
    }
    return pair;
}

void MiscWidget::computeMinMax() {
    switch (mode) {
        case MiscWidgetMode::ControlEditor: {
            _max = 127;
            _default = 0;
            break;
        }
        case MiscWidgetMode::PitchBendEditor: {
            _max = 0x3FFF;
            _default = 0x2000;
            break;
        }
        case MiscWidgetMode::KeyPressureEditor: {
            _max = 127;
            _default = 0;
            break;
        }
        case MiscWidgetMode::ChannelPressureEditor: {
            _max = 127;
            _default = 0;
            break;
        }
    }
}
