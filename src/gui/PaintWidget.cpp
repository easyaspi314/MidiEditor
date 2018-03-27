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
#include "PaintWidget.h"

#include <QScrollArea>

PaintWidget::PaintWidget(QWidget *parent) : QWidget(parent) {
    this->setMouseTracking(true);
    this->mouseOver = false;
    this->mousePressed = false;
    this->mouseReleased = false;
    this->repaintOnMouseMove = false;
    this->repaintOnMousePress = false;
    this->repaintOnMouseRelease = false;
    this->repaintOnScroll = true;
    this->inDrag = false;
    this->mousePinned = false;
    this->mouseX = 0;
    this->mouseY = 0;
    this->mouseLastY = 0;
    this->mouseLastX = 0;
    this->enabled = true;
}

void PaintWidget::mouseMoveEvent(QMouseEvent *event) {

    QWidget::mouseMoveEvent(event);
    this->mouseOver = true;

    if (mousePinned) {
        // do not change mousePosition but lastMousePosition to get the
        // correct move distance
        QCursor::setPos(mapToGlobal(QPointF(mouseX, mouseY).toPoint()));
        mouseLastX = 2*mouseX-qPointF(event->localPos()).x();
        mouseLastY = 2*mouseY-qPointF(event->localPos()).y();
    } else {
        this->mouseLastX = this->mouseX;
        this->mouseLastY = this->mouseY;
        this->mouseX = qPointF(event->localPos()).x();
        this->mouseY = qPointF(event->localPos()).y();
    }
    if (mousePressed) {
        inDrag = true;
    }

    if (!enabled) {
        return;
    }

    if (this->repaintOnMouseMove) {
        this->update();
    }
}

void PaintWidget::enterEvent(QEvent *event) {
    QWidget::enterEvent(event);

    this->mouseOver = true;

    if (!enabled) {
        return;
    }

    update();
}

void PaintWidget::wheelEvent(QWheelEvent *event) {
    QWidget::wheelEvent(event);

    if (repaintOnScroll)
        update();
}

void PaintWidget::leaveEvent(QEvent *event) {
    QWidget::leaveEvent(event);

    this->mouseOver = false;

    if (!enabled) {
        return;
    }
    update();
}

void PaintWidget::mousePressEvent(QMouseEvent *event) {
    QWidget::mousePressEvent(event);

    this->mousePressed = true;
    this->mouseReleased = false;

    if (!enabled) {
        return;
    }

    if (this->repaintOnMousePress) {
        this->update();
    }
}

void PaintWidget::mouseReleaseEvent(QMouseEvent *event) {
    QWidget::mouseReleaseEvent(event);

    this->inDrag = false;
    this->mouseReleased = true;
    this->mousePressed = false;

    if (!enabled) {
        return;
    }

    if (this->repaintOnMouseRelease) {
        this->update();
    }
}

bool PaintWidget::mouseInRect(qreal x, qreal y, qreal width, qreal height) {
    return mouseBetween(x, y, x + width, y + height);
}

bool PaintWidget::mouseInRect(const QRectF &rect) {
    return mouseInRect(rect.x(), rect.y(), rect.width(), rect.height());
}

bool PaintWidget::mouseInWidget(PaintWidget *widget) {
    if (qobject_cast<QScrollArea*>(widget->parentWidget()->parentWidget())) {
        const QRect rect = widget->parentWidget()->rect();
        return widget->mouseInRect(QRectF(widget->mapFromParent(rect.topLeft()),
                                          widget->mapFromParent(rect.bottomRight())));
    }
    return false;
}

QRect PaintWidget::relativeRect() {
    QWidget *parent = parentWidget();
    // See if the parent's parent is a QScrollArea.
    // Note: We take the parent's parent.
    if (qobject_cast<QScrollArea*>(parent->parentWidget())) {
        const QRect rect = parent->frameGeometry();
        return QRect(mapFrom(parent, rect.topLeft()),
                     mapFrom(parent, rect.bottomRight()));
    }
    return rect();
}

bool PaintWidget::mouseBetween(qreal x1, qreal y1, qreal x2, qreal y2) {
    qreal temp;
    if (x1 > x2) {
        temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y1 > y2) {
        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    return mouseOver && mouseX >= x1 && mouseX <= x2 && mouseY >= y1 && mouseY <= y2;
}


qreal PaintWidget::draggedX() {
    if (!inDrag) {
        return 0;
    }
    qreal i = mouseX - mouseLastX;
    mouseLastX = mouseX;
    return i;
}

qreal PaintWidget::draggedY() {
    if (!inDrag) {
        return 0;
    }
    qreal i = mouseY - mouseLastY;
    mouseLastY = mouseY;
    return i;
}

void PaintWidget::setRepaintOnMouseMove(bool b) {
    repaintOnMouseMove = b;
}

void PaintWidget::setRepaintOnMousePress(bool b) {
    repaintOnMousePress = b;
}

void PaintWidget::setRepaintOnMouseRelease(bool b) {
    repaintOnMouseRelease = b;
}

void PaintWidget::setRepaintOnScroll(bool b) {
    repaintOnScroll = b;
}

void PaintWidget::setEnabled(bool b) {
    enabled = b;
    setMouseTracking(enabled);
    update();
}

