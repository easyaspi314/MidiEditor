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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>
#include <QList>
#include "../Utils.h"

class ProtocolItem;
class ProtocolStep;
class MidiFile;
class QImage;

/**
 * \class Protocol
 *
 * \brief Protocol uses two stacks to store the history of the program.
 *
 * Protocol stores the history of the program, using two Stacks (the Stacks
 * are implemented as Lists): the redo stack and the undo stack. Both save
 * ProtocolSteps, in the order they have to be released to reload the old
 * state of the program.
 *
 * To add a ProtocolItem, there has to be opened a ProtocolStep calling
 * startNewAction(). To use the Protocol, open a ProtocolStep,
 * add some ProtocolItems calling enterUndoStep() and call endAction() to put
 * the ProtocolStep onto the undoStack.
 *
 * Starting a new Action will clear the redo stack.
 */
class Protocol : public QObject {

	Q_OBJECT

	public:

		/**
		 * \brief creates a new Protocol for the MidiFile f.
		 */
		Protocol(MidiFile *f);

		/**
		 * \brief undo the first ProtocolStep on the undo stack.
		 *
		 * If emitChanged is true, the Protocol will emit the Signal
		 * protocolChanged()
		 */
		void undo(bool emitChanged = true);

		/**
		 * \brief redo the last ProtocolStep on the redo stack.
		 *
		 * If emitChanged is true, the Protocol will emit the Signal
		 * protocolChanged()
		 */
		void redo(bool emitChanged = true);

		/**
		 * \brief start a new Action.
		 *
		 * Needs to be called to enter undo steps. Creates a new ProtocolStep
		 * saving all items added with enterUndoStep() until calling
		 * endAction().
		 *
		 * the description and the Image are for the ProtocolList and should
		 * show the user which Tool created the Action and what it did.
		 *
		 * Clears the redo stack.
		 */
		void startNewAction(QString description, QImage *img = 0, bool modified = true);

		/**
		 * \brief closes the current ProtocolStep.
		 */
		void endAction();

		/**
		 * \brief returns the number of ProtocolSteps on the undo stack.
		 */
		int stepsBack();

		/**
		 * \brief returns the number of ProtocolSteps on the redo stack.
		 */
		int stepsForward();

		/**
		 * \brief Stores the ProtocolItem item in the current ProtocolStep.
		 *
		 * You need to call startNewAction() to add a ProtocolItem.
		 */
		void enterUndoStep(ProtocolItem *item);

		/**
		 * \brief returns the ProtocolStep of the undo Stack at Position i.
		 */
		ProtocolStep *undoStep(int i);

		/**
		 * \brief returns the ProtocolStep of the redo Stack at Position i.
		 */
		ProtocolStep *redoStep(int i);

		/**
		 * @brief Returns the unique ID for the current step. For the cache.
		 */
		QString currentStepId();

		/**
		 * \brief Goes to the given ProtocolStep.
		 *
		 * redoes/undoes as often as necessary to have toGo as last Action on
		 * undo Stack
		 */
		void goTo(ProtocolStep *toGo);

		/**
		 * \brief Adds an empty Action with the given description.
		 *
		 * This is useful to generate Actions like "File opened"
		 */
		void addEmptyAction(QString name);

	signals:
		/**
		 * \brief This Signal will be emitted when there has been an undo/redo
		 */
		void protocolChanged();
		void actionFinished();

		/**
		 * \brief This signal will be emitted if the step modified the file.
		 *
		 * For now, bool is always true, but it could be used in the future to mark
		 * the file as not modified after an undo.
		 */
		void fileModified(bool modified);

	private:
		/**
		 * \brief currentStep is the actual opened Step.
		 *
		 * It will be 0 if there is no Action opened		 *
		 */
		ProtocolStep *_currentStep;

		/**
		 * \brief The two Stacks containing undo/redo Steps.
		 */
		QStack<ProtocolStep*> *_undoSteps, *_redoSteps;

		/**
		 * \brief the MidiFile this Protocol is working with.
		 */
		MidiFile *_file;
};
#endif
