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

#ifndef EVENTWIDGET_H_
#define EVENTWIDGET_H_

#include <QTableWidget>
#include <QStyledItemDelegate>
#include "../MidiEvent/MidiEvent.h"
class MidiFile;
//class MidiEvent;
class EventWidget;

class EventWidgetDelegate : public QStyledItemDelegate {

    Q_OBJECT

    public:
        EventWidgetDelegate(EventWidget *w, QWidget *parent = qnullptr) : QStyledItemDelegate(parent) { eventWidget = w; }
        QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const qoverride;
        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const qoverride;
        void setEditorData(QWidget *editor, const QModelIndex &index) const qoverride;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const qoverride;

    private:
        EventWidget *eventWidget;

};

class EventWidget : public QTableWidget {

    Q_OBJECT

    public:
        EventWidget(QWidget *parent = qnullptr);

        void setEvents(const QList<MidiEvent*> &events);
        QList<MidiEvent*> events();
        void removeEvent(MidiEvent *event);

        void setFile(MidiFile *file);
        MidiFile *file();

        enum EditorField {
            MidiEventTick,
            MidiEventTrack,
            MidiEventChannel,
            MidiEventNote,
            NoteEventOffTick,
            NoteEventVelocity,
            NoteEventDuration,
            MidiEventValue,
            ControlChangeControl,
            ProgramChangeProgram,
            KeySignatureKey,
            TimeSignatureDenom,
            TimeSignatureNum,
            TextType,
            TextText,
            UnknownType,
            MidiEventData
        };
        QVariant fieldContent(EditorField field);

        inline EventType type() {
            return _currentType;
        }

        QStringList keyStrings();
        ubyte keyIndex(ubyte tonality, bool minor);
        void getKey(ubyte index, ubyte *tonality, bool *minor);

        static QString dataToString(const QByteArray &data);

        void reportSelectionChangedByTool();

    public slots:
        void reload();

    signals:
        void selectionChanged(bool);
        void selectionChangedByTool(bool);

    private:
        EventType computeType();
        const QString eventType();
        QList<QPair<QString, EditorField> > getFields();

        QList<MidiEvent*> _events;
        MidiFile *_file;

        EventType _currentType;

};

#endif
