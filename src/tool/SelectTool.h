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

#ifndef SELECTTOOL_H_
#define SELECTTOOL_H_

#include "EventTool.h"

class MidiEvent;
enum struct SelectType : ubyte {
    Right = 0,
    Left,
    Box,
    Single
};
class SelectTool: public EventTool {

    

    public:

        SelectTool(SelectType type);
        SelectTool(SelectTool &other);

        int type() const qoverride;
        enum {
            Type = SelectToolType
        };

        void draw(QPainter *painter) qoverride;

        bool press(bool leftClick) qoverride;
        bool release() qoverride;
        bool releaseOnly() qoverride;

        bool move(qreal mouseX, qreal mouseY) qoverride;

        ProtocolEntry *copy() qoverride;
        void reloadState(ProtocolEntry *entry) qoverride;
        bool inRect(MidiEvent *event, qreal x_start, qreal y_start, qreal x_end, qreal y_end);

        bool showsSelection() qoverride;

    protected:
        qreal x_rect, y_rect;
        SelectType stool_type;

};

#endif
