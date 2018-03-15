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

#include "RecordDialog.h"

#include <QGridLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QListWidget>

#include "../midi/MidiFile.h"
#include "../MidiEvent/ChannelPressureEvent.h"
#include "../MidiEvent/ControlChangeEvent.h"
#include "../MidiEvent/KeyPressureEvent.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/ProgChangeEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/UnknownEvent.h"
#include "../MidiEvent/TextEvent.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "../tool/NewNoteTool.h"

RecordDialog::RecordDialog(MidiFile *file, const QMultiMap<int, MidiEvent*> &data,
        QWidget *parent) : QDialog(parent)
{
    _data = data;
    _file = file;

    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);

    setWindowTitle(_("Add %1 recorded Events").arg(data.size()));
    // track
    QLabel *tracklabel = new QLabel("Add to track: ", this);
    layout->addWidget(tracklabel, 1, 0, 1, 1);
    _trackBox = new QComboBox(this);
    _trackBox->addItem("Same as selected for new events");
    for (ushort i = 0; i< _file->numTracks(); i++) {
        _trackBox->addItem(_("Track %1: %2").arg(QString::number(i) ,_file->track(i)->name()));
    }
    layout->addWidget(_trackBox, 1, 1, 1, 3);

    // channel
    QLabel *channellabel = new QLabel("Add to channel: ", this);
    layout->addWidget(channellabel, 2, 0, 1, 1);
    _channelBox = new QComboBox(this);
    _channelBox->addItem("Same as selected for new events");
    _channelBox->addItem("Keep channel");
    for (ubyte i = 0; i < 16; i++) {
        _channelBox->addItem(_("Channel %1").arg(i));
    }

    layout->addWidget(_channelBox, 2, 1, 1, 3);

    // ignore types
    QLabel *ignorelabel = new QLabel("Select events to add:", this);
    layout->addWidget(ignorelabel, 3, 0, 1, 4);

    addTypes = new QListWidget(this);
    addListItem(addTypes, "Note on/off Events", 0, true);
    addListItem(addTypes, "Control Change Events", MidiEventLine::ControlChangeEventLine, true);
    addListItem(addTypes, "Pitch Bend Events", MidiEventLine::PitchBendEventLine, true);
    addListItem(addTypes, "Channel Pressure Events", MidiEventLine::ChannelPressureEventLine, true);
    addListItem(addTypes, "Key Pressure Events", MidiEventLine::KeyPressureEventLine, true);
    addListItem(addTypes, "Program Change Events", MidiEventLine::ProgramChangeEventLine, true);
    addListItem(addTypes, "System Exclusive Events", MidiEventLine::SysExEventLine, false);
    addListItem(addTypes, "Tempo Change Events", MidiEventLine::TempoChangeEventLine, false);
    addListItem(addTypes, "Time Signature Events", MidiEventLine::TimeSignatureEventLine, false);
    addListItem(addTypes, "Key Signature Events", MidiEventLine::KeySignatureEventLine, false);
    addListItem(addTypes, "Text Events", MidiEventLine::TextEventLine, false);
    addListItem(addTypes, "Unknown Events", MidiEventLine::UnknownEventLine, false);
    layout->addWidget(addTypes, 12, 0, 1, 4);

    // buttons
    QPushButton *cancel = new QPushButton(tr("&Cancel"), this);
    layout->addWidget(cancel, 13, 0, 1, 2);
    connect(cancel, &QPushButton::clicked, this, &RecordDialog::cancel);

    QPushButton *ok = new QPushButton(tr("&OK"), this);
    layout->addWidget(ok, 13, 2, 1, 2);
    connect(ok, &QPushButton::clicked, this, &RecordDialog::enter);
}

void RecordDialog::enter() {

    ubyte channel = ubyte(_channelBox->currentIndex());
    bool ownChannel = false;
    if (channel < 2) {
        if (channel == 0) {
            channel = NewNoteTool::editChannel();
        } else {
            ownChannel = true;
        }
    } else {
        channel = channel - 2;
    }

    MidiTrack *track = qnullptr;
    ushort trackIndex = ushort(_trackBox->currentIndex());
    if (trackIndex == 0) {
        track = _file->track(NewNoteTool::editTrack());
    } else {
        track = _file->track(trackIndex - 1);
    }
    if (!track) {
        track = _file->tracks()->last();
    }

    // ignore events
    QList<ubyte> ignoredLines;
    for (ubyte i = 0; i < addTypes->count(); i++) {
        QListWidgetItem *item = addTypes->item(i);
        if (item->checkState() == Qt::Unchecked) {
            ubyte line = ubyte(item->data(Qt::UserRole).toUInt());
            ignoredLines.append(line);
        }
    }

    if (_data.size() > 0) {
        _file->protocol()->startNewAction("Added recorded events");

        // first enlarge the file ( last event + 1000 ms)
        QMultiMap<int, MidiEvent*>::const_iterator it = _data.constEnd();
        --it;
        int minLength = it.key() + 1000;
        if (minLength > _file->maxTime()) {
            _file->setMaxLengthMs(minLength);
        }

        it = _data.constBegin();
        while (it != _data.constEnd()) {

            ubyte currentChannel = it.value()->channel();
            if (!ownChannel) {
                currentChannel = channel;
            }

            // check whether to add event or not
            MidiEvent *toCheck = it.value();

            OffEvent *off = qobject_cast<OffEvent*>(toCheck);
            if (off) {
                toCheck = off->onEvent();
            }

            // note event
            ubyte l = toCheck->line();
            if (l < 128) {
                l = 0;
            }

            bool ignoreEvent = ignoredLines.contains(l);

            // set channels
            TempoChangeEvent *tempo = qobject_cast<TempoChangeEvent*>(toCheck);
            if (tempo) {
                currentChannel = 17;
            }

            TimeSignatureEvent *time = qobject_cast<TimeSignatureEvent*>(toCheck);
            if (time) {
                currentChannel = 18;
            }

            TextEvent *text = qobject_cast<TextEvent*>(toCheck);
            if (text) {
                currentChannel = 16;
            }

            if (!ignoreEvent) {
                MidiEvent *toAdd = it.value();
                toAdd->setFile(_file);
                toAdd->setChannel(currentChannel, false);
                toAdd->setTrack(track, false);
                _file->insertEventInChannel(toAdd->channel(), toAdd, _file->tick(it.key()));
            }
            ++it;
        }
        _file->protocol()->endAction();
    }
    hide();
}

void RecordDialog::cancel() {

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Cancel?");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("Do you really want to cancel? The recorded events will be lost.");
    QPushButton *connectButton = msgBox.addButton(tr("&Yes"), QMessageBox::ActionRole);
    msgBox.addButton(tr("*No"), QMessageBox::ActionRole);

     msgBox.exec();

     if (msgBox.clickedButton() == connectButton) {
         // delete events
         foreach(MidiEvent *event, _data) {
             delete event;
         }
         hide();
     }
}

void RecordDialog::addListItem(QListWidget *w, const QString &title, int line, bool enabled) {
    QListWidgetItem *item = new QListWidgetItem(w);
    item->setText(title);
    QVariant v(line);
    item->setData(Qt::UserRole, v);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    if (enabled) {
        item->setCheckState(Qt::Checked);
    } else {
        item->setCheckState(Qt::Unchecked);
    }
    w->addItem(item);
}
