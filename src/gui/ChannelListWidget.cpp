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

#include "ChannelListWidget.h"
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QAction>
#include <QToolBar>
#include <QPainter>
#include <QStyleFactory>
#include <QPushButton>
#include <QToolButton>

#include "../midi/MidiFile.h"
#include "../midi/MidiChannel.h"
#include "../protocol/Protocol.h"

ChannelListItem::ChannelListItem(ubyte ch, ChannelListWidget *parent) : QWidget(parent) {

    channelList = parent;
    channel = ch;

    setContentsMargins(0, 0, 0, 0);
    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);
    layout->setVerticalSpacing(1);

    colored = new ColoredWidget(MidiChannel::colorByChannelNumber(channel), this);
    layout->addWidget(colored, 0, 0, 2, 1);
    QString text = _("Channel %1").arg(channel);
    if (channel == 16) {
        text = "General Events";
    }
    QLabel *text1 = new QLabel(text, this);
    text1->setFixedHeight(15);
    layout->addWidget(text1, 0, 1, 1, 1);

    instrumentLabel = new QLabel("none", this);
    instrumentLabel->setFixedHeight(15);
    layout->addWidget(instrumentLabel, 1, 1, 1, 1);
    QToolBar *toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(12, 12));

    // macOS hack to remove unwanted gradients so we can set the background to what we want.
    toolBar->setStyleSheet("QToolBar{border:none;spacing:0;}");

    QPalette palette = toolBar->palette();
    palette.setColor(QPalette::Background, Qt::transparent);
    toolBar->setPalette(palette);

    // visibility
    visibleAction = new QAction(QIcon(":/run_environment/graphics/channelwidget/visible.png"), "Channel visible", toolBar);
    visibleAction->setCheckable(true);
    visibleAction->setChecked(true);
    toolBar->addAction(visibleAction);
    toolBar->widgetForAction(visibleAction)->setProperty("SegmentedMacButton", "left");
    connect(visibleAction, &QAction::toggled, this, &ChannelListItem::toggleVisibility);

    // audibility
    if (channel < 16) {
        loudAction = new QAction(QIcon(":/run_environment/graphics/channelwidget/loud.png"), "Channel audible", toolBar);
        loudAction->setCheckable(true);
        loudAction->setChecked(true);
        toolBar->addAction(loudAction);
        toolBar->widgetForAction(loudAction)->setProperty("SegmentedMacButton", "center");

        connect(loudAction, &QAction::toggled, this, &ChannelListItem::toggleAudibility);

        // solo
        soloAction = new QAction(QIcon(":/run_environment/graphics/channelwidget/solo.png"), "Solo mode", toolBar);
        soloAction->setCheckable(true);
        soloAction->setChecked(false);
        toolBar->addAction(soloAction);
        toolBar->widgetForAction(soloAction)->setProperty("SegmentedMacButton", "right");

        connect(soloAction, &QAction::toggled, this, &ChannelListItem::toggleSolo);

        toolBar->addSeparator();

        // instrument
        QAction *instrumentAction = new QAction(QIcon(":/run_environment/graphics/channelwidget/instrument.png"),
                                                "Select instrument", toolBar);
        toolBar->addAction(instrumentAction);
        connect(instrumentAction, &QAction::triggered, this, &ChannelListItem::instrument);
    }

    layout->addWidget(toolBar, 2, 1, 1, 1);

    layout->setRowStretch(2,1);
    setContentsMargins(5, 1, 5, 0);

    setFixedHeight(ROW_HEIGHT);
}

void ChannelListItem::toggleVisibility(bool visible) {
    QString text = "Hide channel";
    if (visible) {
        text = "Show channel";
    }
    channelList->midiFile()->protocol()->startNewAction(text, qnullptr, false);
    channelList->midiFile()->channel(channel)->setVisible(visible);
    channelList->midiFile()->protocol()->endAction();
    emit channelStateChanged();
}

void ChannelListItem::toggleAudibility(bool audible) {
    QString text = "Mute channel";
    if (audible) {
        text = "Channel audible";
    }
    channelList->midiFile()->protocol()->startNewAction(text, qnullptr, false);
    channelList->midiFile()->channel(channel)->setMute(!audible);
    channelList->midiFile()->protocol()->endAction();
    emit channelStateChanged();
}

void ChannelListItem::toggleSolo(bool solo) {
    QString text = "Enter solo mode";
    if (!solo) {
        text = "Exited solo mode";
    }
    channelList->midiFile()->protocol()->startNewAction(text, qnullptr, false);
    channelList->midiFile()->channel(channel)->setSolo(solo);
    channelList->midiFile()->protocol()->endAction();
    emit channelStateChanged();
}

void ChannelListItem::instrument() {
    emit selectInstrumentClicked(channel);
}

void ChannelListItem::onBeforeUpdate() {

    QString text = MidiFile::instrumentName(channelList->midiFile()->channel(channel)->
            progAtTick(channelList->midiFile()->cursorTick()));
    if (channel == 16) {
        text = "Events affecting all channels";
    }
    instrumentLabel->setText(text);

    if (channelList->midiFile()->channel(channel)->eventMap()->isEmpty()) {
        colored->setColor(Qt::lightGray);
    } else {
        colored->setColor(MidiChannel::colorByChannelNumber(channel));
    }

    if (visibleAction->isChecked() != channelList->midiFile()->channel(channel)->visible()) {
        disconnect(visibleAction, &QAction::toggled, this, &ChannelListItem::toggleVisibility);
        visibleAction->setChecked(channelList->midiFile()->channel(channel)->visible());
        connect(visibleAction, &QAction::toggled, this, &ChannelListItem::toggleVisibility);
        emit channelStateChanged();
    }

    if (channel < 16 && (loudAction->isChecked()) == (channelList->midiFile()->channel(channel)->mute())) {
        disconnect(loudAction, &QAction::toggled, this, &ChannelListItem::toggleAudibility);
        loudAction->setChecked(!channelList->midiFile()->channel(channel)->mute());
        connect(loudAction, &QAction::toggled, this, &ChannelListItem::toggleAudibility);
        emit channelStateChanged();
    }

    if (channel < 16 && (soloAction->isChecked()) != (channelList->midiFile()->channel(channel)->solo())) {
        disconnect(soloAction, &QAction::toggled, this, &ChannelListItem::toggleSolo);
        soloAction->setChecked(channelList->midiFile()->channel(channel)->solo());
        connect(soloAction, &QAction::toggled, this, &ChannelListItem::toggleSolo);
        emit channelStateChanged();
    }
}

ChannelListWidget::ChannelListWidget(QWidget *parent) : QListWidget(parent) {

    setSelectionMode(QAbstractItemView::NoSelection);
    setStyleSheet("QListWidget::item{border-bottom:1px solid lightGray}");
    items.reserve(17);
    for (ubyte channel = 0; channel < 17; channel++) {
        ChannelListItem *widget = new ChannelListItem(channel, this);
        QListWidgetItem *item = new QListWidgetItem();
        item->setSizeHint(QSize(0, ROW_HEIGHT));
        addItem(item);
        setItemWidget(item,widget);
        items.append(widget);

        connect(widget, &ChannelListItem::channelStateChanged, this, &ChannelListWidget::channelStateChanged);
        connect(widget, &ChannelListItem::selectInstrumentClicked, this, &ChannelListWidget::selectInstrumentClicked);
    }

    file = qnullptr;
}

void ChannelListWidget::setFile(MidiFile *f) {
    file = f;
    connect(file->protocol(), &Protocol::actionFinished, this, &ChannelListWidget::update);
    update();
}

void ChannelListWidget::update() {

    for (ChannelListItem *item : qAsConst(items)) {
        item->onBeforeUpdate();
    }

    QListWidget::update();
}

MidiFile *ChannelListWidget::midiFile() {
    return file;
}

ColoredWidget::ColoredWidget(const QColor &color, QWidget *parent) : QWidget(parent) {
    _color = color;
    setFixedWidth(30);
    setContentsMargins(0,0,0,0);
}

void ColoredWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)

    if (paintingActive()) return;

    QPainter p;
    int l = width()-1;
    int x = 0;
    int y = (height() - 1 - l) / 2;
    if (l>height()-1) {
        l = height()-1;
        y = 0;
        x = (width() - 1 - l) / 2;
    }

    p.begin(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(0,0,width(), height(), Qt::white);
    p.setPen(Qt::lightGray);
    p.setBrush(_color);
    p.drawRoundRect(x,y,l,l, 30, 30);
    p.end();
}
