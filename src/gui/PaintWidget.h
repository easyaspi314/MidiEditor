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

#ifndef PAINTWIDGET_H
#define PAINTWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QEvent>
#include <QCursor>
#include <QPoint>
#include <QPolygonF>

#include "../Utils.h"

// With all these classes, for some reason, QtMath
// (and subsequently, cmath) is not included.
#include <QtMath>

class PaintWidget : public QWidget {

    Q_OBJECT

    public:
        PaintWidget(QWidget *parent = qnullptr);
        void setRepaintOnMouseMove(bool b);
        void setRepaintOnMousePress(bool b);
        void setRepaintOnMouseRelease(bool b);
        void setRepaintOnScroll(bool b);
        void setEnabled(bool b);
        QRect relativeRect();
    protected:
        void mouseMoveEvent(QMouseEvent *event) qoverride;
        void enterEvent(QEvent *event) qoverride;
        void leaveEvent(QEvent *event) qoverride;
        void mousePressEvent(QMouseEvent *event) qoverride;
        void mouseReleaseEvent(QMouseEvent *event) qoverride;
        void wheelEvent(QWheelEvent *event) qoverride;
        inline qreal movedX() { return mouseX - mouseLastX; }
        inline qreal movedY() { return mouseY - mouseLastY; }
        qreal draggedX();
        qreal draggedY();
        bool mouseInRect(qreal x, qreal y, qreal width, qreal height);
        bool mouseInRect(const QRectF &rect);
        bool mouseInWidget(PaintWidget *widget);
        bool mouseBetween(qreal x1, qreal y1, qreal x2, qreal y2);

        inline void setMousePinned(bool b) { mousePinned = b; }


        qreal mouseX, mouseY, mouseLastX, mouseLastY;
        #ifdef NO_BIT_PACK
            bool mouseOver, mousePressed, mouseReleased, repaintOnMouseMove,
                 repaintOnMousePress, repaintOnMouseRelease, repaintOnScroll,
                 inDrag, mousePinned, enabled;
        #else
            bool mouseOver : 1, mousePressed : 1, mouseReleased : 1, repaintOnMouseMove : 1,
                 repaintOnMousePress : 1, repaintOnMouseRelease : 1, repaintOnScroll : 1,
                 inDrag : 1, mousePinned : 1, enabled : 1;
        #endif
};
#endif
