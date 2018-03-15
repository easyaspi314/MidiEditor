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

#ifndef NEWNOTETOOL_H_
#define NEWNOTETOOL_H_

#include "EventTool.h"

class NewNoteTool : public EventTool {

    Q_OBJECT

    public:
        NewNoteTool();
        NewNoteTool(NewNoteTool &other);
        ToolType type() const qoverride;

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;

        void draw(QPainter *painter) qoverride;
        bool press(bool leftClick) qoverride;
        bool release() qoverride;
        bool move(qreal mouseX, qreal mouseY) qoverride;
        bool releaseOnly() qoverride;

        static ushort editTrack();
        static ubyte editChannel();
        static void setEditTrack(ushort i);
        static void setEditChannel(ubyte i);
    private:
        qreal xPos;
        static ushort _track;
        ubyte line;
        #ifdef NO_BIT_PACK
            ubyte velocity;
            bool inDrag;
        #else
            ubyte velocity : 7;
            bool inDrag : 1;
        #endif
        static ubyte _channel;


};

#endif
