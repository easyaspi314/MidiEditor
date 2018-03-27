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

#include "SelectTool.h"
#include "../MidiEvent/MidiEvent.h"
#include "../midi/MidiFile.h"
#include "../protocol/Protocol.h"
#include "../gui/MatrixWidget.h"
#include "StandardTool.h"
#include "../MidiEvent/NoteOnEvent.h"

SelectTool::SelectTool(SelectType type) : EventTool() {
    stool_type = type;
    x_rect = 0;
    y_rect = 0;
    switch(stool_type) {
        case SelectType::Box: {
            setImage(":/select_box.png");
            setToolTipText("Select Events (Box)");
            break;
        }
        case SelectType::Single: {
            setImage(":/select_single.png");
            setToolTipText("Select single Events");
            break;
        }
        case SelectType::Left: {
            setImage(":/select_left.png");
            setToolTipText("Select all Events on the left side");
            break;
        }
        case SelectType::Right: {
            setImage(":/select_right.png");
            setToolTipText("Select all Events on the right side");
            break;
        }
    }
}

SelectTool::SelectTool(SelectTool &other) : EventTool(other) {
    stool_type = other.stool_type;
    x_rect = 0;
    y_rect = 0;
}

int SelectTool::type() const {
    return Type;
}

void SelectTool::draw(QPainter *painter) {
    paintSelectedEvents(painter);
    if (stool_type == SelectType::Box && (qRound(x_rect) || qRound(y_rect))) {
        painter->setPen(Qt::gray);
        painter->setBrush(QColor(0,0,0,100));
        painter->drawRect(qRectF(x_rect, y_rect, mouseX-x_rect, mouseY-y_rect));
    } else if (stool_type == SelectType::Right || stool_type == SelectType::Left) {
        if (mouseIn) {
            painter->setPen(Qt::black);
            painter->setPen(Qt::gray);
            painter->setBrush(QColor(0,0,0,100));
            if (stool_type == SelectType::Left) {
                painter->drawRect(qRectF(0, 0, mouseX, matrixWidget->height()-1));
            } else {
                painter->drawRect(qRectF(mouseX, 0, matrixWidget->width()-1, matrixWidget->height()-1));
            }
        }
    }
}

bool SelectTool::press(bool leftClick) {
    Q_UNUSED(leftClick);
    if (stool_type == SelectType::Box) {
        y_rect = mouseY;
        x_rect = mouseX;
    }
    return true;
}

bool SelectTool::release() {

    if (!file()) {
        return false;

    }
    file()->protocol()->startNewAction("Selection changed", image(), false);
    ProtocolEntry* toCopy = copy();

    if (!QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) &&
            !QApplication::keyboardModifiers().testFlag(Qt::ControlModifier)) {
        clearSelection();
    }

    if (stool_type == SelectType::Box || stool_type == SelectType::Single) {
        qreal x_start = 0.0, y_start = 0.0, x_end = 0.0, y_end = 0.0;
        if (stool_type == SelectType::Box) {
            x_start = x_rect;
            y_start = y_rect;
            x_end = mouseX;
            y_end = mouseY;
            if (x_start>x_end) {
                qreal tmp = x_start;
                x_start = x_end;
                x_end = tmp;
            }
            if (y_start>y_end) {
                qreal tmp = y_start;
                y_start = y_end;
                y_end = tmp;
            }
        } else {
            x_start = mouseX;
            y_start = mouseY;
            x_end = mouseX+1;
            y_end = mouseY+1;
        }
        const auto *eventList = matrixWidget->activeEvents();
        for (MidiEvent* event : *eventList) {
            if (inRect(event, x_start, y_start, x_end, y_end)) {
                selectEvent(event, false);
            }
        }
    } else if (stool_type == SelectType::Left || stool_type == SelectType::Right) {
        int tick = file()->tick(matrixWidget->msOfXPos(mouseX));
        int start, end;
        if (stool_type == SelectType::Left) {
            start = 0;
            end = tick;
        } else {
            end = file()->endTick();
            start = tick;
        }
        const auto *eventsBetweenList = file()->eventsBetween(start, end);
        for (MidiEvent *event : *eventsBetweenList) {
            selectEvent(event, false);
        }
    }

    x_rect = 0;
    y_rect = 0;

    protocol(toCopy, this);
    file()->protocol()->endAction();
    if (_standardTool) {
        Tool::setCurrentTool(_standardTool);
        _standardTool->move(mouseX, mouseY);
        _standardTool->release();
    }
    return true;
}

bool SelectTool::inRect(MidiEvent *event, qreal x_start, qreal y_start, qreal x_end, qreal y_end) {
    const QRectF rect(qPointF(x_start, y_start), qPointF(x_end, y_end));
    const QRectF eventRect(event->x(), event->y(), event->width(), event->height());
    return rect.contains(eventRect);
}

bool SelectTool::move(qreal mouseX, qreal mouseY) {
    EditorTool::move(mouseX, mouseY);
    return true;
}

ProtocolEntry *SelectTool::copy() {
    return new SelectTool(*this);
}

void SelectTool::reloadState(ProtocolEntry *entry) {

    if (SelectTool *other = protocol_cast<SelectTool*>(entry)) {
        EventTool::reloadState(entry);
        x_rect = 0;
        y_rect = 0;
        stool_type = other->stool_type;
    }
}

bool SelectTool::releaseOnly() {
    return release();
}

bool SelectTool::showsSelection() {
    return true;
}
