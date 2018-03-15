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

#include "GraphicObject.h"

#include "../Utils.h"

GraphicObject::GraphicObject() {
    _x = 0;
    _y = 0;
    _width = 0;
    _height = 0;
}

qreal GraphicObject::x() {
    return _x;
}

qreal GraphicObject::y() {
    return _y;
}

qreal GraphicObject::width() {
    return _width;
}

qreal GraphicObject::height() {
    return _height;
}

void GraphicObject::setX(qreal x) {
    _x = x;
}

void GraphicObject::setY(qreal y) {
    _y = y;
}

void GraphicObject::setWidth(qreal w) {
    _width = w;
}

void GraphicObject::setHeight(qreal h) {
    _height = h;
}

void GraphicObject::draw(QPainter *p, QColor c) {
    Q_UNUSED(p) Q_UNUSED(c)
    return;
}

bool GraphicObject::shown() {
    return shownInWidget;
}

void GraphicObject::setShown(bool b) {
    shownInWidget = b;
}

