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

#include "SizeChangeTool.h"

#include "../gui/MatrixWidget.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../protocol/Protocol.h"
#include "StandardTool.h"

#include "Selection.h"

SizeChangeTool::SizeChangeTool() : EventTool() {
    inDrag = false;
    xPos = 0;
    dragsOnEvent = false;
    setImage(":/change_size.png");
    setToolTipText("Change the duration of the selected event");
}

SizeChangeTool::SizeChangeTool(SizeChangeTool &other) : EventTool(other) {
    xPos = other.xPos;
    inDrag = other.inDrag;
    dragsOnEvent = other.dragsOnEvent;
    return;
}

int SizeChangeTool::type() const {
    return Type;
}

ProtocolEntry *SizeChangeTool::copy() {
    return new SizeChangeTool(*this);
}

void SizeChangeTool::reloadState(ProtocolEntry *entry) {
    SizeChangeTool *other = protocol_cast<SizeChangeTool*>(entry);
    if (!other) {
        return;
    }
    EventTool::reloadState(entry);
}

void SizeChangeTool::draw(QPainter *painter) {

    int currentX = rasteredX(mouseX);

    matrixWidget->setCursor(Qt::ArrowCursor);
    if (!inDrag) {
        paintSelectedEvents(painter);
        return;
    } else {
        painter->setPen(Qt::gray);
        painter->drawLine(qLineF(currentX, 0, currentX, matrixWidget->height()));
        painter->setPen(Qt::black);
    }
    int endEventShift = 0;
    int startEventShift = 0;
    if (dragsOnEvent) {
        startEventShift = currentX-xPos;
    } else {
        endEventShift = currentX-xPos;
    }
    for (MidiEvent* event : Selection::instance()->selectedEvents()) {
        bool show = event->shown();
        if (!show) {
            OnEvent *ev = protocol_cast<OnEvent*>(event);
            if (ev) {
                show = ev->offEvent() && ev->offEvent()->shown();
            }
        }
        if (show) {
            painter->fillRect(qRectF(event->x()+startEventShift, event->y(),
                    event->width()-startEventShift+endEventShift,
                    event->height()), Qt::black);
            if (pointInRect(mouseX, mouseY, event->x()+event->width()-2+
                    endEventShift, event->y(), event->x()+event->width()+2+
                    endEventShift, event->y()+event->height()))
            {
                matrixWidget->setCursor(Qt::SplitHCursor);
            }
            if (pointInRect(mouseX, mouseY, event->x()-2+
                    startEventShift, event->y(), event->x()+2+
                    startEventShift, event->y()+event->height()))
            {
                matrixWidget->setCursor(Qt::SplitHCursor);
            }
        }
    }
}

bool SizeChangeTool::press(bool leftClick) {

    Q_UNUSED(leftClick);

    inDrag = false;
    xPos = mouseX;
    for (MidiEvent* event : Selection::instance()->selectedEvents()) {
        if (pointInRect(mouseX, mouseY, event->x()-2, event->y(), event->x()+2,
                event->y()+event->height()))
        {
            dragsOnEvent = true;
            xPos = event->x();
            inDrag = true;
            return true;
        }
        if (pointInRect(mouseX, mouseY, event->x()+event->width()-2, event->y(),
                event->x()+event->width()+2, event->y()+event->height()))
        {
            dragsOnEvent = false;
            inDrag = true;
            xPos = event->x()+event->width();
            return true;
        }
    }

    return false;
}

bool SizeChangeTool::release() {

    int currentX = rasteredX(mouseX);

    inDrag = false;
    qreal endEventShift = 0;
    qreal startEventShift = 0;
    if (dragsOnEvent) {
        startEventShift = currentX-xPos;
    } else {
        endEventShift = currentX-xPos;
    }
    xPos = 0;
    if (Selection::instance()->selectedEvents().count()>0) {
        currentProtocol()->startNewAction("Change event duration", image());
        for (MidiEvent* event : Selection::instance()->selectedEvents()) {
            OnEvent *on = protocol_cast<OnEvent*>(event);
            OffEvent *off = protocol_cast<OffEvent*>(event);
            if (on) {
                int onTick = file()->tick(file()->msOfTick(on->midiTime())-
                        matrixWidget->timeMsOfWidth(qRound(-startEventShift)));
                int offTick = file()->tick(file()->msOfTick(on->offEvent()->
                        midiTime())-matrixWidget->timeMsOfWidth(qRound(-endEventShift)));
                if (onTick<offTick) {
                    if (dragsOnEvent) {
                        changeTick(on, -startEventShift);
                    } else {
                        changeTick(on->offEvent(), -endEventShift);
                    }
                }
            } else if (off) {
                // do nothing; endEvents are shifted when touching their OnEvent
                continue;
            } else if (dragsOnEvent) {
                // normal events will be moved as normal
                changeTick(event, -startEventShift);
            }
        }
        currentProtocol()->endAction();
    }
    matrixWidget->setCursor(Qt::ArrowCursor);
    if (_standardTool) {
        Tool::setCurrentTool(_standardTool);
        _standardTool->move(mouseX, mouseY);
        _standardTool->release();
    }
    return true;
}

bool SizeChangeTool::move(qreal mouseX, qreal mouseY) {
    EventTool::move(mouseX, mouseY);
    for (MidiEvent* event : Selection::instance()->selectedEvents()) {
        if (pointInRect(mouseX, mouseY, event->x()-2, event->y(), event->x()+2,
                event->y()+event->height()))
        {
            matrixWidget->setCursor(Qt::SplitHCursor);
            return inDrag;
        }
        if (pointInRect(mouseX, mouseY, event->x()+event->width()-2, event->y(),
                event->x()+event->width()+2, event->y()+event->height()))
        {
            matrixWidget->setCursor(Qt::SplitHCursor);
            return inDrag;
        }
    }
    matrixWidget->setCursor(Qt::ArrowCursor);
    return inDrag;
}

bool SizeChangeTool::releaseOnly() {
    inDrag = false;
    xPos = 0;
    return true;
}

bool SizeChangeTool::showsSelection() {
    return true;
}
