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

#include "EditorTool.h"
#include "ToolButton.h"
#include "../gui/MatrixWidget.h"
#include "../gui/MainWindow.h"

MatrixWidget *EditorTool::matrixWidget = qnullptr;
MainWindow *EditorTool::_mainWindow = qnullptr;

EditorTool::EditorTool() : Tool() {
    mouseX = 0;
    mouseY = 0;
    etool_selected = false;
    mouseIn = false;
}

EditorTool::EditorTool(EditorTool &other):Tool(other) {
    mouseX = other.mouseX;
    mouseY = other.mouseY;
    etool_selected = other.etool_selected;
    mouseIn = other.mouseIn;
}

int EditorTool::type() const {
    return Type;
}

void EditorTool::draw(QPainter *painter) {
    Q_UNUSED(painter);
    return;
}

bool EditorTool::press(bool leftClick) {
    Q_UNUSED(leftClick);
    return false;
}

bool EditorTool::pressKey(int key) {
    Q_UNUSED(key);
    return false;
}

bool EditorTool::releaseKey(int key) {
    Q_UNUSED(key);
    return false;
}

bool EditorTool::release() {
    return false;
}

bool EditorTool::move(qreal mouseX, qreal mouseY) {
    if (_settings.antialiasing) {
        this->mouseX = mouseX;
        this->mouseY = mouseY;
    } else {
        this->mouseX = short(mouseX);
        this->mouseY = short(mouseY);
    }
    return false;
}

void EditorTool::exit() {
    mouseIn = false;
    return;
}

void EditorTool::enter() {
    mouseIn = true;
    return;
}

void EditorTool::deselect() {
    etool_selected = false;
    if (_button) {
        _button->setChecked(false);
    }
}

void EditorTool::select() {
    etool_selected = true;
    if (_button) {
        _button->setChecked(true);
    }
    _mainWindow->toolChanged();
}

bool EditorTool::selected() {
    return etool_selected;
}

void EditorTool::buttonClick() {
    if (_currentTool) {
        _currentTool->deselect();
    }
    _currentTool = this;
    select();
}

ProtocolEntry *EditorTool::copy() {
    return new EditorTool(*this);
}

void EditorTool::reloadState(ProtocolEntry *entry) {

    Tool::reloadState(entry);

    EditorTool *other = protocol_cast<EditorTool*>(entry);
    if (!other) {
        return;
    }
    etool_selected = other->etool_selected;
    mouseIn = other->mouseIn;
}

void EditorTool::setMatrixWidget(MatrixWidget *w) {
    matrixWidget = w;
}

void EditorTool::setMainWindow(MainWindow *mw) {
    _mainWindow = mw;
}

bool EditorTool::pointInRect(qreal x, qreal y, qreal x_start, qreal y_start, qreal x_end, qreal y_end) {
    return x >= x_start && x < x_end && y >= y_start && y <= y_end;
}

bool EditorTool::releaseOnly() {
    return false;
}
