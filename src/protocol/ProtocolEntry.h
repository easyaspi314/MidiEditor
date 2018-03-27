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

#ifndef PROTOCOLENTRY_H
#define PROTOCOLENTRY_H

#include <QObject>
#include <QtDebug>
#include "../Utils.h"

class MidiFile;


/**
 * \class ProtocolEntry
 *
 * \brief ProtocolEntry is the superclass for objects to protocol.
 *
 * ProtocolEntry is a Superclass for every kind of object which can be written
 * into the Programs Protocol.
 * To protocol the state of a ProtocolEntry, the object is copied
 * (ProtocolEntry *copy())). When the Protocols Method undo() or redo() is
 * called, the ProtocolEntry has to reload his old state from the copy in the
 * Protocol (void reloadState(ProtocolEntry *entry)). Both Methods have to be
 * implemented in the Subclass.
 * Before a ProtocolEntry is changed it has to copy its old state to oldObj.
 * After changing itself, the Method protocol(ProtocolEntry *oldObj,
 * ProtocolEntry *newObj) has to be called, with "this" as newObj.
 */
class ProtocolEntry {


    public:

        virtual ~ProtocolEntry();

        /**
         * \brief copies the ProtocolEntry.
         *
         * copy() should save the ProtocolEntrys state so that it will be able
         * to reload his old state in reloadState().
         * All important data has to be saved to the returned object; there is
         * no need to save Layoutinformation because the program will relayout
         * after every call of the protocol.
         */
        virtual ProtocolEntry *copy();

        ProtocolEntry();
        ProtocolEntry(const ProtocolEntry &other);

        /**
         * \brief reloads the state of entry.
         *
         * This Method has to reload the Objects old state, written to entry in
         * copy().
         */
        virtual void reloadState(ProtocolEntry *entry);

        /**
         * \brief writes the old object oldObj and the new object newObj to the
         * protocol.
         */
        virtual void protocol(ProtocolEntry *oldObj, ProtocolEntry *newObj);

        /**
         * \brief return the entries file.
         */
        virtual MidiFile *file();

        /**
         * \brief The int enum is a flag bitset that is used for type information.
         *
         * Note that inheritance is the key in making this possible. If it weren't for that, we
         * would need a 64-bit enum, which is painful to deal with.
         */
        enum ProtocolEntryType {
            UnknownEntryType           = 1 << 0x1E,

            // Base types
            MidiFileType               = 1 << 0x01,
            MidiChannelType            = 1 << 0x02,
            MidiTrackType              = 1 << 0x03,
            ToolType                   = 1 << 0x04,
            SelectionType              = 1 << 0x05,
            MidiEventType              = 1 << 0x06,

            // All MidiEvents are subclasses of MidiEvent, so they have the MidiEventType flag.
            ChannelPressureEventType   = 1 << 0x07 | MidiEventType,
            ControlChangeEventType     = 1 << 0x08 | MidiEventType,
            KeyPressureEventType       = 1 << 0x09 | MidiEventType,
            KeySignatureEventType      = 1 << 0x0A | MidiEventType,
            OffEventType               = 1 << 0x0B | MidiEventType,
            OnEventType                = 1 << 0x0C | MidiEventType,
            NoteOnEventType            = 1 << 0x0D | OnEventType,
            PitchBendEventType         = 1 << 0x0E | MidiEventType,
            ProgramChangeEventType     = 1 << 0x0F | MidiEventType,
            SystemExclusiveEventType   = 1 << 0x10 | MidiEventType,
            TempoChangeEventType       = 1 << 0x11 | MidiEventType,
            TextEventType              = 1 << 0x12 | MidiEventType,
            TimeSignatureEventType     = 1 << 0x13 | MidiEventType,
            UnknownEventType           = 1 << 0x14 | MidiEventType,

            // All Tools are subclasses of Tool, so they have the ToolType flag.
            EditorToolType             = 1 << 0x07 | ToolType,
            EventToolType              = 1 << 0x08 | EditorToolType,
            EraserToolType             = 1 << 0x09 | EventToolType,
            EventMoveToolType          = 1 << 0x10 | EventToolType,
            NewNoteToolType            = 1 << 0x11 | EventToolType,
            SelectToolType             = 1 << 0x12 | EventToolType,
            SizeChangeToolType         = 1 << 0x13 | EventToolType,
            StandardToolType           = 1 << 0x14 | EventToolType,

        };
        /**
         * \brief Returns the type of the ProtocolEntry. This removes the QObject necessity
         * without having to use RTTI. Granted, it shamelessly copies qgraphicsitem_cast, but
         * that is ok.
         *
         * This is required in each class, and it always returns "Type".
         */
        virtual int type() const;
        /**
         * The following anonymous enum is required in each class, and the only entry is
         * "Type", and the ProtocolEntryType that it returns.
         */
        enum {
            Type = UnknownEntryType
        };


};
/**
 * \brief Casts a ProtocolEntry. Use it like any other C++ cast template.
 *
 * This was adapted from qgraphicsitem.cpp, but changed to compare flags instead of the value,
 * as the ProtocolEntry subclasses have subclasses, and if I were to declare all subclasses, we
 * would actually have 34 flags, and enums do not like to be larger than int
 * .
 *
 * With this new alternative to including the entire Q_OBJECT macro, which takes up 48b of memory,
 * we can save a lot of RAM, as we don't use the
 */
template <class T>
inline T protocol_cast(ProtocolEntry *item)
{
    /*
     * Synopsis:
     *
     * // Get an empty instance of the class used in the template. We don't care if it is a null
     * // pointer, because the Type enum member, unique for each subclass, is a constant.
     * int mType = int(static_cast<T>(0)->Type);
     *
     * if ((mType & ProtocolEntry::Type) == mType) // check if it is just the base class
     * {
     *     return 0; // Nope, can't have that.
     * }
     *
     * // Check if item is an actual instance, and check if the item->type() (which returns <class
     * // of item>::Type) has the same flags set.
     * else if (item && ((mType & item->type()) == mType))
     * {
     *     return static_cast<T>(item); // Ok, cast to the type
     * }
     * return 0; // Nope, can't have that.
     */
    return (int(static_cast<T>(0)->Type) & int(ProtocolEntry::Type)) == int(static_cast<T>(0)->Type)
        || (item && (int(static_cast<T>(0)->Type) & item->type()) == int(static_cast<T>(0)->Type)) ? static_cast<T>(item) : 0;
}

/**
 * \brief Copied from qgraphicsitem.h. Use it in place of qobject_cast.
 */
template <class T>
inline T protocol_cast(const ProtocolEntry *item)
{
    return (int(static_cast<T>(0)->Type) & int(ProtocolEntry::Type)) == int(static_cast<T>(0)->Type)
        || (item && (int(static_cast<T>(0)->Type) & item->type()) == int(static_cast<T>(0)->Type)) ? static_cast<T>(item) : 0;
}
#endif
