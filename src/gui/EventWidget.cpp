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

#include "EventWidget.h"

#include <QBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QtCore/qmath.h>
#include <QMessageBox>

#include "../MidiEvent/MidiEvent.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "../midi/MidiChannel.h"

#include "../MidiEvent/ChannelPressureEvent.h"
#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/KeyPressureEvent.h"
#include "../MidiEvent/KeySignatureEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/PitchBendEvent.h"
#include "../MidiEvent/ProgChangeEvent.h"
#include "../MidiEvent/SysExEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TextEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/UnknownEvent.h"

#include "DataEditor.h"

/* TODO: Clean up this file. It needs it a lot. */

QSize EventWidgetDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    EventWidget::EditorField field = EventWidget::EditorField(index.data(Qt::UserRole).toInt());
    if (field == EventWidget::MidiEventData) {
        int min = 80;
        QSize normal = QStyledItemDelegate::sizeHint(option, index);
        if (normal.height()<min) {
            return QSize(0, min);
        }
        return normal;
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }

}

QWidget *EventWidgetDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{

    EventWidget::EditorField field = EventWidget::EditorField(index.data(Qt::UserRole).toInt());

    switch(field) {
        case EventWidget::MidiEventTick:
        case EventWidget::MidiEventNote:
        case EventWidget::NoteEventOffTick:
        case EventWidget::NoteEventVelocity:
        case EventWidget::NoteEventDuration:
        case EventWidget::MidiEventChannel:
        case EventWidget::TimeSignatureNum:
        case EventWidget::MidiEventValue: {
            return new QSpinBox(parent);
        }
        case EventWidget::MidiEventTrack:
        case EventWidget::ControlChangeControl:
        case EventWidget::ProgramChangeProgram:
        case EventWidget::KeySignatureKey:
        case EventWidget::TimeSignatureDenom:
        case EventWidget::TextType: {
            return new QComboBox(parent);
        }
        case EventWidget::TextText: {
            return new QTextEdit(parent);
        }
        case EventWidget::UnknownType: {
            return new QLineEdit(parent);
        }
        case EventWidget::MidiEventData: {
            return new DataEditor(parent);
        }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void EventWidgetDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{
    EventWidget::EditorField field = EventWidget::EditorField(index.data(Qt::UserRole).toInt());

    switch(field) {
        case EventWidget::MidiEventTick: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            spin->setMaximum(0);
            spin->setMaximum(INT_MAX);
            if (index.data().canConvert(QVariant::Int)) {
                spin->setValue(index.data().toInt());
            }
            break;
        }
        case EventWidget::MidiEventTrack: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            int i = 0;
            const QList<MidiTrack *> *tracks = eventWidget->file()->tracks();
            for (MidiTrack *track : *tracks) {
                QString text = _("Track %1: %2").arg(QString::number(track->number()), track->name());
                box->addItem(text);
                if (!text.compare(index.data().toString())) {
                    i = track->number();
                }
            }
            box->setCurrentIndex(i);
            break;
        }
        case EventWidget::MidiEventNote: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            spin->setMaximum(0);
            spin->setMaximum(127);
            if (index.data().canConvert(QVariant::Int)) {
                spin->setValue(index.data().toInt());
            }
            break;
        }
        case EventWidget::NoteEventOffTick: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            spin->setMaximum(0);
            spin->setMaximum(INT_MAX);
            if (index.data().canConvert(QVariant::Int)) {
                spin->setValue(index.data().toInt());
            }
            break;
        }
        case EventWidget::NoteEventVelocity: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            spin->setMaximum(0);
            spin->setMaximum(127);
            if (index.data().canConvert(QVariant::UInt)) {
                spin->setValue(index.data().toInt());
            }
            break;
        }
        case EventWidget::NoteEventDuration: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            spin->setMaximum(0);
            spin->setMaximum(INT_MAX);
            if (index.data().canConvert(QVariant::Int)) {
                spin->setValue(index.data().toInt());
            }
            break;
        }
        case EventWidget::MidiEventChannel: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            spin->setMaximum(0);
            spin->setMaximum(15);
            if (index.data().canConvert(QVariant::Int)) {
                spin->setValue(index.data().toInt());
            }
            break;
        }
        case EventWidget::MidiEventValue: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            spin->setMinimum(0);
            int type = eventWidget->type();
            switch(type) {
                case MidiEvent::PitchBendEventType: {
                    spin->setMaximum(16383);
                    break;
                }
                case MidiEvent::TempoChangeEventType: {
                    spin->setMaximum(1000);
                    break;
                }
                default: {
                    spin->setMaximum(127);
                    break;
                }
            }

            if (index.data().canConvert(QVariant::Int)) {
                spin->setValue(index.data().toInt());
            }
            break;
        }
        case EventWidget::ControlChangeControl: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            int current = 0;
            for (ubyte i = 0; i < 128; i++) {
                QString text = _("%1: %2").arg(QString::number(i), MidiFile::controlChangeName(i));
                box->addItem(text);
                if (!text.compare(index.data().toString())) {
                    current = i;
                }
            }
            box->setCurrentIndex(current);
            break;
        }
        case EventWidget::ProgramChangeProgram: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            int current = 0;
            for (ubyte i = 0; i < 128; i++) {
                QString text = _("%1: %2").arg(QString::number(i), MidiFile::instrumentName(i));
                box->addItem(text);
                if (!text.compare(index.data().toString())) {
                    current = i;
                }
            }
            box->setCurrentIndex(current);
            break;
        }
        case EventWidget::KeySignatureKey: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            int current = 0;
            int i = 0;
            const QStringList keyStrings = eventWidget->keyStrings();
            for (const QString &key : keyStrings) {
                box->addItem(key);
                if (!key.compare(index.data().toString())) {
                    current = i;
                }
                i++;
            }
            box->setCurrentIndex(current);
            break;
        }
        case EventWidget::TimeSignatureNum: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            spin->setMaximum(1);
            spin->setMaximum(99);
            if (index.data().canConvert(QVariant::Int)) {
                spin->setValue(index.data().toInt());
            }
            break;
        }
        case EventWidget::TimeSignatureDenom: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            int current = 0;
            for (ubyte p = 0; p < 5; p++) {
                box->addItem(QString::number(p * p));
                if ((index.data().toInt()) == p * p) {
                    current = p;
                }
            }
            box->setCurrentIndex(current);
            break;
        }
        case EventWidget::TextType: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            int current = 0;
            for (ubyte p = 1; p < 8; p++) {
                box->addItem(TextEvent::textTypeString(p));
                if (!index.data().toString().compare(TextEvent::textTypeString(p))) {
                    current = p - 1;
                }
            }
            box->setCurrentIndex(current);
            break;
        }
        case EventWidget::TextText: {
            QTextEdit *edit = qobject_cast<QTextEdit*>(editor);
            if (index.data().canConvert(QVariant::String)) {
                edit->setPlainText(index.data().toString());
            }
            break;
        }
        case EventWidget::UnknownType: {
            QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
            edit->setInputMask("HH");
            if (index.data().canConvert(QVariant::String)) {
                edit->setText(index.data().toString().right(2));
            }
            break;
        }
        case EventWidget::MidiEventData: {
            DataEditor *edit = qobject_cast<DataEditor*>(editor);
            if (index.data(Qt::UserRole+2).canConvert(QVariant::ByteArray)) {
                edit->setData(index.data(Qt::UserRole + 2).toByteArray());
            }
            break;
        }
    }
}

void EventWidgetDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const{

    auto field = EventWidget::EditorField(index.data(Qt::UserRole).toInt());

    const QList<MidiEvent*> events = eventWidget->events();
    // set values
    eventWidget->file()->protocol()->startNewAction(_("Edited %1").arg(index.data(Qt::UserRole + 1).toString().toLower()));
    switch(field) {
        case EventWidget::MidiEventTick: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);

            for (MidiEvent *event : events) {
                if (NoteOnEvent *noteOn = protocol_cast<NoteOnEvent*>(event)) {
                    if (spin->value() >= noteOn->offEvent()->midiTime()) {
                        noteOn->offEvent()->setMidiTime(spin->value() + 10);
                    }
                }
                event->setMidiTime(spin->value());
            }

            break;
        }
        case EventWidget::MidiEventTrack: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            MidiTrack *track = eventWidget->file()->track(box->currentIndex());
            for (MidiEvent *event : events) {
                MidiTrack *oldTrack = event->track();
                if (NoteOnEvent *noteOn = protocol_cast<NoteOnEvent*>(event)) {
                    noteOn->offEvent()->setTrack(track);
                }
                event->setTrack(track);

                if (TextEvent *text = protocol_cast<TextEvent*>(event)) {
                    if (text->textType() == TextType::TrackNameTextEventType) {
                        oldTrack->setNameEvent(qnullptr);
                        text->track()->setNameEvent(text);
                    }
                }
            }
            break;
        }
        case EventWidget::MidiEventNote: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            ubyte value = ubyte(spin->value());

            for (MidiEvent *event : events) {
                if (NoteOnEvent *noteOn = protocol_cast<NoteOnEvent*>(event)) {
                    noteOn->setNote(value);
                } else if (KeyPressureEvent *key = protocol_cast<KeyPressureEvent*>(event)) {
                    key->setNote(value);
                }
            }
            break;
        }
        case EventWidget::NoteEventOffTick: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            int value = spin->value();
            for (MidiEvent *event : events) {
                if (NoteOnEvent *noteOn = protocol_cast<NoteOnEvent*>(event)) {
                    noteOn->offEvent()->setMidiTime(value);
                }
            }
            break;
        }
        case EventWidget::NoteEventVelocity: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            ubyte value = ubyte(spin->value());
            for (MidiEvent *event : events) {
                if (NoteOnEvent *noteOn = protocol_cast<NoteOnEvent*>(event)) {
                    noteOn->setVelocity(value);
                }
            }
            break;
        }
        case EventWidget::NoteEventDuration: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            int value = spin->value();
            for (MidiEvent *event : events) {
                if (NoteOnEvent *noteOn = protocol_cast<NoteOnEvent*>(event)) {
                    noteOn->offEvent()->setMidiTime(noteOn->midiTime() + value);
                }
            }
            break;
        }
        case EventWidget::MidiEventChannel: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            ubyte value = ubyte(spin->value());
            for (MidiEvent *ev : events) {
                ev->moveToChannel(value);
            }
            break;
        }
        case EventWidget::MidiEventValue: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            int value = spin->value();
            for (MidiEvent *event : events) {
                if (ChannelPressureEvent *ch = protocol_cast<ChannelPressureEvent*>(event)) {
                    ch->setValue(ubyte(value));
                } else if (ControlChangeEvent *ctrl = protocol_cast<ControlChangeEvent*>(event)) {
                    ctrl->setValue(ubyte(value));
                } else if (KeyPressureEvent *key = protocol_cast<KeyPressureEvent*>(event)) {
                    key->setValue(ubyte(value));
                } else if (PitchBendEvent *pitch = protocol_cast<PitchBendEvent*>(event)) {
                    pitch->setValue(ushort(value));
                } else if (TempoChangeEvent *tempo = protocol_cast<TempoChangeEvent*>(event)) {
                    tempo->setBeats(spin->value());
                }
            }
            break;
        }
        case EventWidget::ControlChangeControl: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            ubyte value = ubyte(box->currentIndex());
            for (MidiEvent *event : events) {
                if (ControlChangeEvent *c = protocol_cast<ControlChangeEvent*>(event)) {
                    c->setControl(value);
                }
            }
            break;
        }
        case EventWidget::ProgramChangeProgram: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            ubyte value = ubyte(box->currentIndex());
            for (MidiEvent *event : events) {
                if (ProgChangeEvent *c = protocol_cast<ProgChangeEvent*>(event)) {
                    c->setProgram(value);
                }
            }
            break;
        }
        case EventWidget::KeySignatureKey: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            ubyte tonality;
            bool minor;
            eventWidget->getKey(ubyte(box->currentIndex()), &tonality, &minor);

            for (MidiEvent *event : events) {

                if (KeySignatureEvent *c = protocol_cast<KeySignatureEvent*>(event)) {
                    c->setTonality(tonality);
                    c->setMinor(minor);
                }
            }
            break;
        }
        case EventWidget::TimeSignatureNum: {
            QSpinBox *spin = qobject_cast<QSpinBox*>(editor);
            ubyte value = ubyte(spin->value());
            for (MidiEvent *event : events) {
                if (TimeSignatureEvent *ev = protocol_cast<TimeSignatureEvent*>(event)) {
                    ev->setNumerator(value);
                }
            }
            break;
        }
        case EventWidget::TimeSignatureDenom: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            ubyte value = ubyte(box->currentIndex());
            for (MidiEvent *event : events) {
                if (TimeSignatureEvent *c = protocol_cast<TimeSignatureEvent*>(event)) {
                    c->setDenominator(value);
                }
            }
            break;
        }
        case EventWidget::TextType: {
            QComboBox *box = qobject_cast<QComboBox*>(editor);
            for (MidiEvent *event : events) {
                if (TextEvent *c = protocol_cast<TextEvent*>(event)) {
                    TextType oldType = c->textType();
                    TextType newType = TextType(box->currentIndex()+1);
                    c->setTextType(newType);
                    if ((oldType == TextType::TrackNameTextEventType) && (oldType != newType)) {
                        event->track()->setNameEvent(qnullptr);
                    }
                    if ((newType == TextType::TrackNameTextEventType) && (oldType != newType)) {
                        event->track()->setNameEvent(c);
                    }
                }
            }
            break;
        }
        case EventWidget::TextText: {
            QTextEdit *ed = qobject_cast<QTextEdit*>(editor);
            for (MidiEvent *event : events) {
                if (TextEvent *c = protocol_cast<TextEvent*>(event)) {
                    c->setText(ed->toPlainText());
                }
            }
            break;
        }
        case EventWidget::UnknownType: {
            QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
            bool ok;
            ubyte type = ubyte(edit->text().toUInt(&ok, 16));
            QMap<int, QString> registeredTypes = MidiEvent::knownMetaTypes();

            if (registeredTypes.contains(type)) {
                QMessageBox::warning(eventWidget, "Error",
                    _("The entered type refers to known Meta Event (%1)").arg(registeredTypes.value(type)));
                return;
            }

            for (MidiEvent *event : events) {
                if (UnknownEvent *c = protocol_cast<UnknownEvent*>(event)) {
                    c->setUnknownType(type);
                }
            }
            break;
        }
        case EventWidget::MidiEventData: {
            DataEditor *edit = qobject_cast<DataEditor*>(editor);
            QByteArray data = edit->data();
            if (eventWidget->type() == MidiEvent::SystemExclusiveEventType) {
                if (data.contains(byte(0xF7))) {
                    QMessageBox::warning(eventWidget, "Error", "The data must not contain byte 0xF7 (End of SysEx)");
                    return;
                }
            }
            for (MidiEvent *event : events) {
                if (UnknownEvent *c = protocol_cast<UnknownEvent*>(event)) {
                    c->setData(data);
                } else if (SysExEvent *s = protocol_cast<SysExEvent*>(event)) {
                    s->setData(data);
                }
            }

            break;
        }
    }
    eventWidget->file()->protocol()->endAction();

    // refresh model
    if (field != EventWidget::MidiEventData) {
        model->setData(index, eventWidget->fieldContent(field));
    } else {
        DataEditor *edit = qobject_cast<DataEditor*>(editor);
        QByteArray data = edit->data();
        model->setData(index, data, Qt::UserRole+2);
        model->setData(index, EventWidget::dataToString(data));
    }
}

EventWidget::EventWidget(QWidget *parent) : QTableWidget(0, 2, parent) {

    _file = qnullptr;

    QHeaderView *headerView = new QHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(headerView);
    headerView->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    headerView->setSectionResizeMode(1, QHeaderView::Stretch);

    QStringList headers;
    headers.append("Property");
    headers.append("Value");
    setHorizontalHeaderLabels(headers);

    verticalHeader()->setVisible(false);

    setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);
    setSelectionBehavior(QAbstractItemView::SelectRows);

    setItemDelegate(new EventWidgetDelegate(this));
}

void EventWidget::setFile(MidiFile *file) {
    _file = file;
    _events.clear();
    emit selectionChanged(_events.count()>0);
    reload();
}

MidiFile *EventWidget::file() {
    return _file;
}

void EventWidget::setEvents(const QList<MidiEvent*> &events) {
    _events = events;
    emit selectionChanged(_events.count()>0);
}

QList<MidiEvent*> EventWidget::events() {
    return _events;
}

void EventWidget::removeEvent(MidiEvent *event) {
    _events.removeAll(event);
    emit selectionChanged(_events.count()>0);
}

void EventWidget::reload() {

    clear();

    QStringList headers;
    headers.append("Property");
    headers.append("Value");
    setHorizontalHeaderLabels(headers);

    if (_events.isEmpty()) {
        setRowCount(0);
        return;
    }

    // compute type to display
    _currentType = computeType();
    QList<QPair<QString, EditorField> > fields = getFields();
    setRowCount(fields.size()+1);

    int row = 0;

    // display type
    QTableWidgetItem *typeLabel = new QTableWidgetItem("Type");
    typeLabel->setFlags(Qt::ItemIsEnabled);
    QTableWidgetItem *type = new QTableWidgetItem(eventType());
    type->setFlags(Qt::ItemIsEnabled);
    setItem(row, 0, typeLabel);
    setItem(row++, 1, type);

    for (const QPair<QString, EditorField> &pair : fields) {

        QTableWidgetItem *label = new QTableWidgetItem(pair.first);
        label->setFlags(Qt::ItemIsEnabled);
        //label->setTextAlignment(Qt::Align);
        setItem(row, 0, label);

        QTableWidgetItem *value = new QTableWidgetItem;
        value->setData(0, fieldContent(pair.second));
        value->setData(Qt::UserRole, QVariant(pair.second));
        value->setData(Qt::UserRole+1, QVariant(pair.first));
        if (pair.second == MidiEventData) {
            value->setData(Qt::UserRole+2, value->data(0));
            value->setData(0, dataToString(value->data(Qt::UserRole+2).toByteArray()));
        }
        setItem(row++, 1, value);
    }

    resizeRowsToContents();
}

int EventWidget::computeType() {
    int type = MidiEvent::MidiEventType;
    bool inited = false;
    for (MidiEvent *event : qAsConst(_events)) {
        if (!inited && event->type() != MidiEvent::OnEventType
                && event->type() != MidiEvent::OffEventType) {
            type = event->type();
        } else {
            if (type != event->type())
                type = MidiEvent::MidiEventType;
        }
        inited = true;
    }
    return type;
}

const QString EventWidget::eventType() {
    switch(_currentType) {
        case MidiEvent::MidiEventType: {
            return "Midi Event";
        }
        case MidiEvent::ChannelPressureEventType: {
            return "Channel Pressure Event";
        }
        case MidiEvent::ControlChangeEventType: {
            return "Control Change Event";
        }
        case MidiEvent::KeyPressureEventType: {
            return "Key Pressure Event";
        }
        case MidiEvent::KeySignatureEventType: {
            return "Key Signature Event";
        }
        case MidiEvent::NoteOnEventType: {
            return "Note On/Off Event";
        }
        case MidiEvent::PitchBendEventType: {
            return "Pitch Bend Event";
        }
        case MidiEvent::ProgramChangeEventType: {
            return "Program Change Event";
        }
        case MidiEvent::SystemExclusiveEventType: {
            return "Sysex Event";
        }
        case MidiEvent::TempoChangeEventType: {
            return "Temo Change Event";
        }
        case MidiEvent::TextEventType: {
            return "Text Event";
        }
        case MidiEvent::TimeSignatureEventType: {
            return "Time Signature Event";
        }
        case MidiEvent::UnknownEventType: {
            return "Unknown Event";
        }
        default: {
            return "Midi Event";
        }
    }
}

// TODO: I am sure this could be reduced.
QList<QPair<QString, EventWidget::EditorField> > EventWidget::getFields() {

    QList<QPair<QString, EditorField> > fields;
    fields.append(QPair<QString, EditorField>("On Tick", MidiEventTick));

    switch(_currentType) {
        case MidiEvent::ChannelPressureEventType: {
            fields.append(QPair<QString, EditorField>("Value", MidiEventValue));
            fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
            break;
        }
        case MidiEvent::ControlChangeEventType: {
            fields.append(QPair<QString, EditorField>("Control", ControlChangeControl));
            fields.append(QPair<QString, EditorField>("Value", MidiEventValue));
            fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
            break;
        }
        case MidiEvent::KeyPressureEventType: {
            fields.append(QPair<QString, EditorField>("Note", MidiEventNote));
            fields.append(QPair<QString, EditorField>("Value", MidiEventValue));
            fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
            break;
        }
        case MidiEvent::KeySignatureEventType: {
            fields.append(QPair<QString, EditorField>("Key", KeySignatureKey));
            break;
        }
        case MidiEvent::NoteOnEventType: {
            fields.append(QPair<QString, EditorField>("Off Tick", NoteEventOffTick));
            fields.append(QPair<QString, EditorField>("Duration", NoteEventDuration));
            fields.append(QPair<QString, EditorField>("Note", MidiEventNote));
            fields.append(QPair<QString, EditorField>("Velocity", NoteEventVelocity));
            fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
            break;
        }

        case MidiEvent::PitchBendEventType: {
            fields.append(QPair<QString, EditorField>("Value", MidiEventValue));
            fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
            break;
        }
        case MidiEvent::ProgramChangeEventType: {
            fields.append(QPair<QString, EditorField>("Program", ProgramChangeProgram));
            fields.append(QPair<QString, EditorField>("Channel", MidiEventChannel));
            break;
        }
        case MidiEvent::SystemExclusiveEventType: {
            fields.append(QPair<QString, EditorField>("Data", MidiEventData));
            break;
        }
        case MidiEvent::TempoChangeEventType: {
            fields.append(QPair<QString, EditorField>("Tempo", MidiEventValue));
            break;
        }
        case MidiEvent::TextEventType: {
            fields.append(QPair<QString, EditorField>("Type", TextType));
            fields.append(QPair<QString, EditorField>("Text", TextText));
            break;
        }
        case MidiEvent::TimeSignatureEventType: {
            fields.append(QPair<QString, EditorField>("Numerator", TimeSignatureNum));
            fields.append(QPair<QString, EditorField>("Denominator", TimeSignatureDenom));
            break;
        }
        case MidiEvent::UnknownEventType: {
            fields.append(QPair<QString, EditorField>("Type", UnknownType));
            fields.append(QPair<QString, EditorField>("Data", MidiEventData));
            break;
        }
        default: {
            Q_UNREACHABLE();
            break;
        }
    }
    fields.append(QPair<QString, EditorField>("Track", MidiEventTrack));
    return fields;
}

QVariant EventWidget::fieldContent(EditorField field) {
    switch(field) {
        case MidiEventTick: {
            int tick = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (tick == -1) {
                    tick = event->midiTime();
                } else if (tick != event->midiTime()) {
                    return QVariant();
                }
            }
            return QVariant(tick);
        }
        case MidiEventTrack: {
            MidiTrack *track = qnullptr;
            for (MidiEvent *event : qAsConst(_events)) {
                if (!track) {
                    track = event->track();
                } else if (track != event->track()) {
                    return QVariant();
                }
            }
            if (!track) {
                return QVariant();
            }
            return QVariant(_("Track %1: %2").arg(QString::number(track->number()), track->name()));
        }
        case MidiEventNote: {
            int note = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (NoteOnEvent *onEvent = protocol_cast<NoteOnEvent*>(event)) {
                    if (note == -1) {
                        note = onEvent->note();
                    } else if (note != onEvent->note()) {
                        return QVariant();
                    }
                }

                if (KeyPressureEvent *key = protocol_cast<KeyPressureEvent*>(event)) {
                    if (note == -1) {
                        note = key->note();
                    } else if (note != key->note()) {
                        return QVariant();
                    }
                }
            }
            if (note < 0) {
                return QVariant();
            }
            return QVariant(note);
        }
        case NoteEventOffTick: {
            int off = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (NoteOnEvent *onEvent = protocol_cast<NoteOnEvent*>(event)) {
                    if (off == -1) {
                        off = onEvent->offEvent()->midiTime();
                    } else if (off != onEvent->offEvent()->midiTime()) {
                        return QVariant();
                    }
                }
            }
            if (off < 0) {
                return QVariant();
            }
            return QVariant(off);
        }
        case NoteEventVelocity: {
            int velocity = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (NoteOnEvent *onEvent = protocol_cast<NoteOnEvent*>(event)) {
                    if (velocity == -1) {
                        velocity = onEvent->velocity();
                    } else if (velocity != onEvent->velocity()) {
                        return QVariant();
                    }
                }
            }
            if (velocity < 0) {
                return QVariant();
            }
            return QVariant(velocity);
        }
        case NoteEventDuration: {
            int duration = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (NoteOnEvent *onEvent = protocol_cast<NoteOnEvent*>(event)) {
                    if (duration == -1) {
                        duration = onEvent->offEvent()->midiTime()-onEvent->midiTime();
                    } else if (duration != onEvent->offEvent()->midiTime()-onEvent->midiTime()) {
                        return QVariant();
                    }
                }
            }
            if (duration < 0) {
                return QVariant();
            }
            return QVariant(duration);
        }
        case MidiEventChannel: {
            int channel = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (channel == -1) {
                    channel = event->channel();
                } else if (channel != event->channel()) {
                    return QVariant();
                }
            }
            if (Q_UNLIKELY(channel < 0)) {
                return QVariant();
            }
            return QVariant(channel);
        }

        case MidiEventValue: {
            int value = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (ChannelPressureEvent *ev = protocol_cast<ChannelPressureEvent*>(event)) {
                    if (value == -1) {
                        value = ev->value();
                    } else if (value != ev->value()) {
                        return QVariant();
                    }
                } else if (ControlChangeEvent *ev = protocol_cast<ControlChangeEvent*>(event)) {
                    if (value == -1) {
                        value = ev->value();
                    } else if (value != ev->value()) {
                        return QVariant();
                    }
                } else if (KeyPressureEvent *ev = protocol_cast<KeyPressureEvent*>(event)) {
                    if (value == -1) {
                        value = ev->value();
                    } else if (value != ev->value()) {
                        return QVariant();
                    }
                } else if (PitchBendEvent *ev = protocol_cast<PitchBendEvent*>(event)) {
                    if (value == -1) {
                        value = ev->value();
                    } else if (value != ev->value()) {
                        return QVariant();
                    }
                } else if (TempoChangeEvent *ev = protocol_cast<TempoChangeEvent*>(event)) {
                    if (value == -1) {
                        value = ev->beatsPerQuarter();
                    } else if (value != ev->beatsPerQuarter()) {
                        return QVariant();
                    }
                }

            }
            if (Q_UNLIKELY(value < 0)) {
                return QVariant();
            }
            return QVariant(value);
        }
        case ControlChangeControl: {
            int control = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (ControlChangeEvent *ev = protocol_cast<ControlChangeEvent*>(event)) {
                    if (control == -1) {
                        control = ev->control();
                    } else if (control != ev->control()) {
                        return QVariant();
                    }
                }
            }
            if (control < 0) {
                return QVariant();
            }
            return QVariant(_("%1: %2").arg(QString::number(control), MidiFile::controlChangeName(ubyte(control))));
        }
        case ProgramChangeProgram: {
            int program = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (ProgChangeEvent *ev = protocol_cast<ProgChangeEvent*>(event)) {
                    if (program == -1) {
                        program = ev->program();
                    } else if (program != ev->program()) {
                        return QVariant();
                    }
                }
            }
            if (program < 0) {
                return QVariant();
            }
            return QVariant(_("%1: %2").arg(QString::number(program), MidiFile::instrumentName(ubyte(program))));
        }
        case KeySignatureKey: {
            int key = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (KeySignatureEvent *ev = protocol_cast<KeySignatureEvent*>(event)) {
                    if (key == -1) {
                        key = keyIndex(ev->tonality(), ev->minor());
                    } else if (key != keyIndex(ev->tonality(), ev->minor())) {
                        return QVariant();
                    }
                }
            }
            if (key < 0) {
                return QVariant();
            }
            return QVariant(keyStrings().at(key));
        }
        case TimeSignatureNum: {
            int n = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (TimeSignatureEvent *ev = protocol_cast<TimeSignatureEvent*>(event)) {
                    if (n == -1) {
                        n = ev->num();
                    } else if (n != ev->num()) {
                        return QVariant();
                    }
                }
            }
            if (Q_UNLIKELY(n < 0)) {
                return QVariant();
            }
            return QVariant(n);
        }
        case TimeSignatureDenom: {
            int n = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (TimeSignatureEvent *ev = protocol_cast<TimeSignatureEvent*>(event)) {
                    if (n == -1) {
                        n = ev->denom();
                    } else if (n != ev->denom()) {
                        return QVariant();
                    }
                }
            }
            if (Q_UNLIKELY(n < 0)) {
                return QVariant();
            }
            return QVariant(int(qPow(2, n)));
        }
        case TextType: {
            int n = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (TextEvent *ev = protocol_cast<TextEvent*>(event)) {
                    if (n == -1) {
                        n = ev->textType();
                    } else if (n != ev->textType()) {
                        return QVariant();
                    }
                }
            }
            if (n < 0) {
                return QVariant();
            }
            return QVariant(TextEvent::textTypeString(ubyte(n)));
        }
        case TextText: {
            bool inited = false;
            QString n;
            for (MidiEvent *event : qAsConst(_events)) {
                if (TextEvent *ev = protocol_cast<TextEvent*>(event)) {
                    if (!inited) {
                        n = ev->text();
                    } else if (n.compare(ev->text())) {
                        return QVariant();
                    }
                }
                inited = true;
            }
            if (!inited) {
                return QVariant();
            }
            return QVariant(n);
        }
        case UnknownType: {
            int n = -1;
            for (MidiEvent *event : qAsConst(_events)) {
                if (UnknownEvent *ev = protocol_cast<UnknownEvent*>(event)) {
                    if (n == -1) {
                        n = ev->unknownType();
                    } else if (n!=ev->unknownType()) {
                        return QVariant();
                    }
                }
            }
            if (n < 0) {
                return QVariant();
            }
            QString s;
            s.sprintf("0x%02X", n);
            return QVariant(s);
        }
        case MidiEventData: {
            QByteArray data;
            bool inited = false;
            for (MidiEvent *event : qAsConst(_events)) {
                if (UnknownEvent *ev = protocol_cast<UnknownEvent*>(event)) {
                    if (!inited) {
                        data = ev->data();
                    } else if (data!=ev->data()) {
                        return QVariant();
                    }
                } else if (SysExEvent *sys = protocol_cast<SysExEvent*>(event)) {
                    if (!inited) {
                        data = sys->data();
                    } else if (data!=sys->data()) {
                        return QVariant();
                    }
                }
                inited = true;
            }
            if (!inited) {
                return QVariant();
            }
            return QVariant(data);
        }
    }
    return QVariant();
}

QStringList EventWidget::keyStrings() {
    QStringList list;
    list.reserve(24);
    for (int i = -6; i <= 6; i++) {
        list.append(KeySignatureEvent::toString(i, false));
    }
    for (int i = -6; i <= 6; i++) {
        list.append(KeySignatureEvent::toString(i, true));
    }
    return list;
}

ubyte EventWidget::keyIndex(ubyte tonality, bool minor) {
    ubyte center = 6;
    if (minor) {
        center = 19;
    }
    return center + tonality;
}

void EventWidget::getKey(ubyte index, ubyte *tonality, bool *minor) {
    if (index > 13) {
        *minor = true;
        index -= 13;
    } else {
        *minor = false;
    }
    *tonality = index - 6;
}

QString EventWidget::dataToString(const QByteArray &data) {
    QString s;
    for (byte b : data) {
        s.append(s.asprintf("0x%02X\n", ubyte(b)));
    }
    return s.trimmed();
}

void EventWidget::reportSelectionChangedByTool() {
    emit selectionChangedByTool(_events.count() > 0);
}
