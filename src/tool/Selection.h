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

#ifndef SELECTION_H
#define SELECTION_H

#include "../protocol/ProtocolEntry.h"

#include <QList>

class MidiEvent;
class EventWidget;

class Selection : public ProtocolEntry {

    Q_OBJECT

    public:

        Selection(MidiFile *file);
        Selection(Selection &other);

        virtual ProtocolEntry *copy();
        virtual void reloadState(ProtocolEntry *entry);

        virtual MidiFile *file();

        static Selection *instance();
        static void setFile(MidiFile *file);

        const QList<MidiEvent*> &selectedEvents();
        void setSelection(const QList<MidiEvent*> &selections);
        void clearSelection();

        static EventWidget *_eventWidget;
    signals:
        void selectionChanged();
    private:
        QList<MidiEvent*> _selectedEvents;
        static Selection *_selectionInstance;
        MidiFile *_file;
};

#endif
