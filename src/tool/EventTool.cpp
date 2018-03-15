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

#include "EventTool.h"

#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "../midi/MidiChannel.h"
#include "../gui/MatrixWidget.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiPlayer.h"
#include "../gui/MainWindow.h"
#include "../gui/EventWidget.h"
#include "NewNoteTool.h"
#include "Selection.h"

#include <QtCore/qmath.h>

QList<MidiEvent*> *EventTool::copiedEvents = new QList<MidiEvent*>;

ubyte EventTool::_pasteChannel = 0xFF; // -1
ushort EventTool::_pasteTrack = 0xFFFE; // -2


EventTool::EventTool() : EditorTool() {

}

EventTool::EventTool(EventTool &other) : EditorTool(other) {

}

ToolType EventTool::type() const {
    return ToolType::Event;
}

void EventTool::selectEvent(MidiEvent *event, bool single, bool ignoreStr) {

    if (!event->file()->channel(event->channel())->visible()) {
        return;
    }

    if (event->track()->hidden()) {
        return;
    }

    QList<MidiEvent*> selected = Selection::instance()->selectedEvents();

    OffEvent *offevent = qobject_cast<OffEvent*>(event);
    if (offevent) {
        return;
    }

    if (single && !QApplication::keyboardModifiers().testFlag(Qt::ShiftModifier) && (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) || ignoreStr)) {
        selected.clear();
        NoteOnEvent *on = qobject_cast<NoteOnEvent*>(event);
        if (on) {
            MidiPlayer::instance()->play(on);
        }
    }
    if (!selected.contains(event) && (!QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) || ignoreStr)) {
        selected.append(event);
    } else if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) && !ignoreStr) {
        selected.removeAll(event);
    }

    Selection::instance()->setSelection(selected);
    _mainWindow->eventWidget()->reportSelectionChangedByTool();
}

void EventTool::deselectEvent(MidiEvent *event) {

    QList<MidiEvent*> selected = Selection::instance()->selectedEvents();
    selected.removeAll(event);
    Selection::instance()->setSelection(selected);

    if (_mainWindow->eventWidget()->events().contains(event)) {
        _mainWindow->eventWidget()->removeEvent(event);
    }
}

void EventTool::clearSelection() {
    Selection::instance()->clearSelection();
    _mainWindow->eventWidget()->reportSelectionChangedByTool();
}

void EventTool::paintSelectedEvents(QPainter *painter) {
    for (MidiEvent *event : Selection::instance()->selectedEvents()) {

        bool show = event->shown();

        if (!show) {
            OnEvent *ev = qobject_cast<OnEvent*>(event);
            if (ev) {
                show = ev->offEvent() && ev->offEvent()->shown();
            }
        }

        if (event->track()->hidden()) {
            show = false;
        }
        if (!(event->file()->channel(event->channel())->visible())) {
            show = false;
        }

        if (show) {
            painter->setBrush(Qt::darkBlue);
            painter->setPen(Qt::lightGray);
            painter->drawRoundedRect(qRectF(event->x(), event->y(), event->width(),
                    event->height()), 1, 1);
        }
    }
}

void EventTool::changeTick(MidiEvent* event, qreal shiftX) {
    // TODO: if event is shown, use matrixWidget tick (more efficient)
    //int newMs = matrixWidget->msOfXPos(event->x()-shiftX);

    int newMs = file()->msOfTick(event->midiTime())-matrixWidget->timeMsOfWidth(qRound(shiftX));
    int tick = file()->tick(newMs);

    if (tick < 0) {
        tick = 0;
    }
    // with magnet: set to div value if pixel refers to this tick
    if (_settings.magnet) {
        qreal newX = matrixWidget->xPosOfMs(newMs);
        for (const QPair<qreal, int> &p : matrixWidget->divs()) {
            qreal xt = p.first;
            if (qFuzzyCompare(newX, xt)) {
                tick = p.second;
                break;
            }
        }
    }
    event->setMidiTime(tick);
}

void EventTool::copyAction() {

    if (!Selection::instance()->selectedEvents().isEmpty()) {
        // clear old copied Events
        copiedEvents->clear();

        for (MidiEvent *event : Selection::instance()->selectedEvents()) {

            // add the current Event
            MidiEvent *ev = qobject_cast<MidiEvent*>(event->copy());
            if (ev) {
                // do not append off event here
                OffEvent *off = qobject_cast<OffEvent*>(ev);
                if (!off) {
                    copiedEvents->append(ev);
                }
            }

            // if its onEvent, add a copy of the OffEvent
            OnEvent *onEv = qobject_cast<OnEvent*>(ev);
            if (onEv) {
                OffEvent *offEv = qobject_cast<OffEvent*>(onEv->offEvent()->copy());
                if (offEv) {
                offEv->setOnEvent(onEv);
                copiedEvents->append(offEv);
            }
        }
        }
        _mainWindow->copiedEventsChanged();
    }
}

void EventTool::pasteAction() {

    if (copiedEvents->size() == 0) {
        return;
    }

    // TODO what happends to TempoEvents??

    // copy copied events to insert unique events
    QList<MidiEvent*> copiedCopiedEvents;
    for (MidiEvent *event : *copiedEvents) {

        // add the current Event
        MidiEvent *ev = qobject_cast<MidiEvent*>(event->copy());
        if (ev) {
            // do not append off event here
            OffEvent *off = qobject_cast<OffEvent*>(ev);
            if (!off) {
                copiedCopiedEvents.append(ev);
            }
        }

        // if its onEvent, add a copy of the OffEvent
        OnEvent *onEv = qobject_cast<OnEvent*>(ev);
        if (onEv) {
            OffEvent *offEv = qobject_cast<OffEvent*>(onEv->offEvent()->copy());
            if (offEv) {
            offEv->setOnEvent(onEv);
            copiedCopiedEvents.append(offEv);
            }
        }
    }

    if (copiedCopiedEvents.count()>0) {

        // Begin a new ProtocolAction
        currentFile()->protocol()->startNewAction(_("Paste %n events").arg(copiedCopiedEvents.count()));

        qreal tickscale = 1;
        if (currentFile() != copiedEvents->constFirst()->file()) {
            tickscale = qreal(currentFile()->ticksPerQuarter()) / qreal(copiedEvents->constFirst()->file()->ticksPerQuarter());
        }

        // get first Tick of the copied events
        int firstTick = -1;
        for (MidiEvent *event : copiedCopiedEvents) {
            if (qRound(tickscale*event->midiTime()) < firstTick || firstTick < 0) {
                firstTick = qRound(tickscale*event->midiTime());
            }
        }
        if (firstTick < 0) {
            firstTick = 0;
        }
        // calculate the difference of old/new events in MidiTicks
        int diff = currentFile()->cursorTick() - firstTick;

        // set the Positions and add the Events to the channels
        clearSelection();

        for (MidiEvent *event : copiedCopiedEvents) {

            // get channel
            ubyte channel = event->channel();
            if (_pasteChannel == 0xFE) {
                channel = NewNoteTool::editChannel();
            }
            if ((_pasteChannel >= 0) && (channel < 16)) {
                channel = _pasteChannel;
            }

            // get track
            MidiTrack *track = event->track();
            if (pasteTrack() == -2) {
                track = currentFile()->track(NewNoteTool::editTrack());
            } else if ((pasteTrack() >= 0) && (pasteTrack() < currentFile()->numTracks())) {
                track = currentFile()->track(pasteTrack());
            } else if (event->file() != currentFile() || !currentFile()->tracks()->contains(track)) {
                track = currentFile()->getPasteTrack(event->track(), event->file());
                if (!track) {
                    track = event->track()->copyToFile(currentFile());
                }
            }

            if ((!track) || (track->file() != currentFile())) {
                track = currentFile()->track(0);
            }

            event->setFile(currentFile());
            event->setChannel(channel, false);
            event->setTrack(track, false);
            currentFile()->insertEventInChannel(channel, event,
                    qRound(tickscale*event->midiTime()) + diff);
            selectEvent(event, false, true);
        }

        currentFile()->protocol()->endAction();
    }
}

bool EventTool::showsSelection() {
    return false;
}

void EventTool::setPasteTrack(ushort track) {
    _pasteTrack = track;
}

ushort EventTool::pasteTrack() {
    return _pasteTrack;
}

void EventTool::setPasteChannel(ubyte channel) {
    _pasteChannel = channel;
}

ubyte EventTool::pasteChannel() {
    return _pasteChannel;
}

int EventTool::rasteredX(qreal x, int *tick) {
    if (!_settings.magnet) {
        if (tick) {
            *tick = -1;
        }
        return qRound(x);
    }
    for (QPair<qreal, int> p : matrixWidget->divs()) {
        qreal xt = p.first;
        if (qAbs(xt - x) <= 5.0) {
            if (tick) {
                *tick = p.second;
            }
            return qRound(xt);
        }
    }
    if (tick) {
        *tick = -1;
    }
    return qRound(x);
}
