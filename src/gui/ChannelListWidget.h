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

#ifndef CHANNELLISTWIDGET_H_
#define CHANNELLISTWIDGET_H_

#include <QWidget>
#include <QListWidget>
#include <QPaintEvent>
#include <QList>
#include <QColor>
#include <QResizeEvent>
#include <QPushButton>
#include "../Utils.h"

class QAction;
class MidiFile;
class QLabel;

class ChannelListWidget;

const ubyte ROW_HEIGHT = 85;

class ColoredWidget : public QWidget {

    Q_OBJECT

    public:
        ColoredWidget(const QColor &color, QWidget *parent = qnullptr);
        void setColor(const QColor &c) { _color = c; update(); }

    protected:
        void paintEvent(QPaintEvent *event) qoverride;

    private:
        QColor _color;
};


class ChannelListItem : public QWidget {

    Q_OBJECT

    public:
        ChannelListItem(ubyte channel, ChannelListWidget *parent);
        void onBeforeUpdate();

    signals:
        void selectInstrumentClicked(int channel);
        void channelStateChanged();

    public slots:
        void toggleVisibility(bool visible);
        void toggleAudibility(bool audible);
        void toggleSolo(bool solo);
        void instrument();

    private:
        QLabel *instrumentLabel;
        ChannelListWidget *channelList;
        ubyte channel;
        ColoredWidget *colored;
        QAction *visibleAction, *loudAction, *soloAction;
};

class ChannelListWidget : public QListWidget {

    Q_OBJECT

    public:
        ChannelListWidget(QWidget *parent = qnullptr);
        void setFile(MidiFile *f);
        MidiFile *midiFile();

    signals:
        void channelStateChanged();
        void selectInstrumentClicked(ubyte channel);

    public slots:
        void update();

    private:
        MidiFile *file;
        QList<ChannelListItem*> items;
};

#endif
