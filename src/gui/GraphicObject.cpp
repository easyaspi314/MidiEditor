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

#include "GraphicObject.h"

#include "MatrixWidget.h"
#include <QGraphicsItem>

GraphicObject::GraphicObject(QGraphicsItem *parent) : QGraphicsRectItem (QRectF(0, 0, 0, 0), parent) {
	setFlags(ItemIsSelectable|ItemIsMovable | ItemIsFocusable);

	setPos(0, 0);
	setX(0);
	setY(0);
	setWidth(0);
	setHeight(0);
}
QRectF GraphicObject::boundingRect() const {
	return QRectF(0, 0, width(), height());
}
/*qreal GraphicObject::x() {
	return _x;
}

qreal GraphicObject::y() {
	return _y;
}



void GraphicObject::setX(qreal x) {
	_x = x;
}

void GraphicObject::setY(qreal y) {
	_y = y;
}*/
qreal GraphicObject::width() const {
	return rect().width();
}

qreal GraphicObject::height() const {
	return rect().height();
}
void GraphicObject::setWidth(qreal w) {
	QRectF rectF = rect();
	rectF.setWidth(w);
	setRect(rectF);
}

void GraphicObject::setHeight(qreal h) {
	QRectF rectF = rect();
	rectF.setHeight(h);
	setRect(rectF);
}

void GraphicObject::paint(QPainter *painter,
	   const QStyleOptionGraphicsItem *option,
	   QWidget *widget) {
	Q_UNUSED(option) Q_UNUSED(widget)
	painter->drawRoundedRect(rect(), 1, 1);
	return;
}

bool GraphicObject::shown() {
	return shownInWidget;
}

void GraphicObject::setShown(bool b) {
	shownInWidget = b;
}
