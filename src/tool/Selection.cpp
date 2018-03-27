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

#include "Selection.h"

#include "../gui/EventWidget.h"
#include "../midi/MidiFile.h"

Selection *Selection::_selectionInstance = new Selection(qnullptr);
EventWidget *Selection::_eventWidget = qnullptr;

Selection::Selection(MidiFile *file) : QObject(file) {
    _file = file;
    if (_eventWidget) {
        _eventWidget->setEvents(_selectedEvents);
        _eventWidget->reload();
    }
}

Selection::Selection(Selection &other) {
    _file = other._file;
    _selectedEvents = other._selectedEvents;
}

int Selection::type() const {
    return Type;
}

ProtocolEntry *Selection::copy() {
    return new Selection(*this);
}

void Selection::reloadState(ProtocolEntry *entry) {
    if (Selection *other = protocol_cast<Selection*>(entry)) {
        _selectedEvents = other->_selectedEvents;
        if (_eventWidget) {
            _eventWidget->setEvents(_selectedEvents);
            //_eventWidget->reload();
        }
    }
}

MidiFile *Selection::file() {
    return _file;
}

Selection *Selection::instance() {
    return _selectionInstance;
}

void Selection::setFile(MidiFile *file) {

    // create new selection
    _selectionInstance = new Selection(file);
}

const QList<MidiEvent*> &Selection::selectedEvents() {
    return _selectedEvents;
}

void Selection::setSelection(const QList<MidiEvent*> &selections) {
    if (selections.isEmpty() && _selectedEvents.isEmpty()) {
        // No point in making empty protocol steps
        return;
    }
    ProtocolEntry *toCopy = copy();
    _selectedEvents = selections;
    protocol(toCopy, this);
    if (_eventWidget) {
        _eventWidget->setEvents(_selectedEvents);
        //_eventWidget->reload();
    }
    emit selectionChanged();
}

void Selection::clearSelection() {
    if (_selectedEvents.isEmpty()) {
        // No point in making empty protocol steps
        return;
    }
    ProtocolEntry *toCopy = copy();
    _selectedEvents.clear();
    protocol(toCopy, this);
    if (_eventWidget) {
        _eventWidget->setEvents(_selectedEvents);
        //_eventWidget->reload();
    }
    emit selectionChanged();
}
