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

#include "Protocol.h"

#include <QImage>
#include <QHash>

#include "ProtocolItem.h"
#include "ProtocolStep.h"
#include "../midi/MidiFile.h"

Protocol::Protocol(MidiFile *f) {

    _currentStep = qnullptr;

    _file = f;

    _undoSteps = new QStack<ProtocolStep*>;
    _redoSteps = new QStack<ProtocolStep*>;
}

void Protocol::enterUndoStep(ProtocolItem *item) {

    if (_currentStep) {
        _currentStep->addItem(item);
    }

    emit protocolChanged();
}

void Protocol::undo(bool emitChanged) {

    if (!_undoSteps->isEmpty()) {

        // Take last undoStep from the Stack
        ProtocolStep *step = _undoSteps->pop();
        bool modified = step->modified();

        // release it and copy it to the redo Stack
        ProtocolStep *redoAction = step->releaseStep();
        if (redoAction) {
            _redoSteps->push(redoAction);
        }

        // delete the old Step
        delete step;
        if (modified) {
            emit fileModified(true);
        }
        if (emitChanged) {
            emit protocolChanged();
            emit actionFinished();
        }
    }
}

void Protocol::redo(bool emitChanged) {

    if (!_redoSteps->isEmpty()) {

        // Take last redoSteo from the Stack
        ProtocolStep *step = _redoSteps->pop();
        bool modified = step->modified();


        // release it and copy it to the undoStack
        ProtocolStep *undoAction = step->releaseStep();
        if (undoAction) {
            _undoSteps->push(undoAction);
        }

        // delete the old Step
        delete step;
        if (modified) {
            emit fileModified(true);
        }
        if (emitChanged) {
            emit protocolChanged();
            emit actionFinished();
        }
    }
}

void Protocol::startNewAction(const QString &description, QImage *img, bool modified) {

    // When there is a new Action started the redoStack has to be cleared
    _redoSteps->clear();

    // Any old Action is ended
    endAction();

    // create a new Step
    _currentStep = new ProtocolStep(description, img, modified);
}

void Protocol::endAction() {

    bool modified = false;
    // only create the Step when it exists and its size is bigger 0
    if (_currentStep && _currentStep->items()>0) {
        _undoSteps->push(_currentStep);
        modified = _currentStep->modified();
    }

    // the action is ended so there is no currentStep
    _currentStep = qnullptr;

    if (modified) {
        // the file has been changed
        emit fileModified(true);
    }

    emit protocolChanged();
    emit actionFinished();
}

int Protocol::stepsBack() {
    return _undoSteps->count();
}

int Protocol::stepsForward() {
    return _redoSteps->count();
}

ProtocolStep *Protocol::undoStep(int i) {
    return _undoSteps->at(i);
}

ProtocolStep *Protocol::redoStep(int i) {
    return _redoSteps->at(i);
}

void Protocol::goTo(ProtocolStep *toGo) {

    if (_undoSteps->contains(toGo)) {

        // do undo() until toGo is the last Step on the undoStack
        while(_undoSteps->top()!=toGo && _undoSteps->size()>1) {
            undo(false);
        }

    } else if (_redoSteps->contains(toGo)) {

        // do redo() until toGo is the last Step on the undoStep
        while(_redoSteps->contains(toGo)) {
            redo(false);
        }
    }

    emit protocolChanged();
    emit actionFinished();
}

void Protocol::addEmptyAction(QString name) {
    _undoSteps->push(new ProtocolStep(name, qnullptr, false));
}

ushort Protocol::currentStepId() {
    if (_currentStep) {
        return _currentStep->id();
    } else if (!_undoSteps->isEmpty()) {
        return _undoSteps->top()->id();
    } else {
        return 0;
    }
}
