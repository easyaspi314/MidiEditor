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

#include "TrackListWidget.h"
#include "ChannelListWidget.h"

#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiChannel.h"

#include <QLineEdit>
#include <QInputDialog>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QAction>
#include <QToolBar>
#include <QPainter>

TrackListItem::TrackListItem(MidiTrack *track, TrackListWidget *parent) : QWidget(parent) {

    trackList = parent;
    this->track = track;

    setContentsMargins(0, 0, 0, 0);
    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);
    layout->setVerticalSpacing(1);

    colored = new ColoredWidget(*(track->color()), this);
    layout->addWidget(colored, 0, 0, 2, 1);
    QString text = _("Track %1").arg(track->number());
    QLabel *text1 = new QLabel(text, this);
    text1->setFixedHeight(15);
    layout->addWidget(text1, 0, 1, 1, 1);

    trackNameLabel = new QLabel("New Track", this);
    trackNameLabel->setFixedHeight(15);
    layout->addWidget(trackNameLabel, 1, 1, 1, 1);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setIconSize(QSize(12, 12));
    // Stylesheet needed to hide macOS gradient.
    toolBar->setStyleSheet("QToolBar{border:none;spacing:0}");

    // visibility
    visibleAction = new QAction(QIcon(":/run_environment/graphics/trackwidget/visible.png"), "Track visible", toolBar);
    visibleAction->setCheckable(true);
    visibleAction->setChecked(true);
    toolBar->addAction(visibleAction);
    toolBar->widgetForAction(visibleAction)->setProperty("SegmentedMacButton", "left");
    connect(visibleAction, &QAction::toggled, this, &TrackListItem::toggleVisibility);

    // audibility
    loudAction = new QAction(QIcon(":/run_environment/graphics/trackwidget/loud.png"), "Track audible", toolBar);
    loudAction->setCheckable(true);
    loudAction->setChecked(true);
    toolBar->addAction(loudAction);
    toolBar->widgetForAction(loudAction)->setProperty("SegmentedMacButton", "right");
    connect(loudAction, &QAction::toggled, this, &TrackListItem::toggleAudibility);

    toolBar->addSeparator();

    // name
    QAction *renameAction = new QAction(QIcon(":/run_environment/graphics/trackwidget/rename.png"),
                                        "Rename track", toolBar);
    toolBar->addAction(renameAction);
    connect(renameAction, &QAction::triggered, this, &TrackListItem::renameTrack);

    // remove
    QAction *removeAction = new QAction(QIcon(":/run_environment/graphics/trackwidget/remove.png"),
                                        "Remove track", toolBar);
    toolBar->addAction(removeAction);
    connect(removeAction, &QAction::triggered, this, &TrackListItem::removeTrack);

    layout->addWidget(toolBar, 2, 1, 1, 1);

    layout->setRowStretch(2,1);
    setContentsMargins(5, 1, 5, 0);
    setFixedHeight(ROW_HEIGHT);
}

void TrackListItem::toggleVisibility(bool visible) {
    QString text = "Hide track";
    if (visible) {
        text = "Show track";
    }
    trackList->midiFile()->protocol()->startNewAction(text, qnullptr, false);
    track->setHidden(!visible);
    trackList->midiFile()->protocol()->endAction();
}

void TrackListItem::toggleAudibility(bool audible) {
    QString text = "Mute track";
    if (audible) {
        text = "Track audible";
    }
    trackList->midiFile()->protocol()->startNewAction(text, qnullptr, false);
    track->setMuted(!audible);
    trackList->midiFile()->protocol()->endAction();
}

void TrackListItem::renameTrack() {
    emit trackRenameClicked(track->number());
}

void TrackListItem::removeTrack() {
    emit trackRemoveClicked(track->number());
}

void TrackListItem::onBeforeUpdate() {

    trackNameLabel->setText(track->name());

    if (visibleAction->isChecked() == track->hidden()) {
        disconnect(visibleAction, &QAction::toggled, this, &TrackListItem::toggleVisibility);
        visibleAction->setChecked(!track->hidden());
        connect(visibleAction, &QAction::toggled, this, &TrackListItem::toggleVisibility);
    }

    if (loudAction->isChecked() == track->muted()) {
        disconnect(loudAction, &QAction::toggled, this, &TrackListItem::toggleAudibility);
        loudAction->setChecked(!track->muted());
        connect(loudAction, &QAction::toggled, this, &TrackListItem::toggleAudibility);
    }
}

TrackListWidget::TrackListWidget(QWidget *parent) : QListWidget(parent) {

    setSelectionMode(QAbstractItemView::SingleSelection);
    setStyleSheet( "QListWidget::item { border-bottom: 1px solid lightGray; }" );
    file = qnullptr;
    connect(this, &TrackListWidget::itemClicked, this, &TrackListWidget::chooseTrack);
}

void TrackListWidget::setFile(MidiFile *f) {
    file = f;
    connect(file->protocol(), &Protocol::actionFinished, this, &TrackListWidget::update);
    update();
}

void TrackListWidget::chooseTrack(QListWidgetItem *item) {

    int t = item->data(Qt::UserRole).toInt();
    MidiTrack *track = trackorder.at(t);
    emit trackClicked(track);
}

void TrackListWidget::update() {

    if (!file) {
        clear();
        items.clear();
        trackorder.clear();
        QListWidget::update();
        return;
    }

    bool rebuild = false;
    const QList<MidiTrack*> oldTracks = trackorder;
    const QList<MidiTrack*> realTracks = *(file->tracks());

    if (oldTracks.size() != realTracks.size()) {
        rebuild = true;
    } else {
        for (int i = 0; i < oldTracks.size(); i++) {
            if (oldTracks.at(i) != realTracks.at(i)) {
                rebuild = true;
                break;
            }
        }
    }

    if (rebuild) {
        clear();
        items.clear();
        trackorder.clear();

        for (MidiTrack *track : realTracks) {
            TrackListItem *widget = new TrackListItem(track, this);
            QListWidgetItem *item = new QListWidgetItem();
            item->setSizeHint(QSize(0, ROW_HEIGHT));
            item->setData(Qt::UserRole, track->number());
            addItem(item);
            setItemWidget(item,widget);
            items.insert(track, widget);
            trackorder.append(track);
            connect(widget, &TrackListItem::trackRenameClicked, this, &TrackListWidget::trackRenameClicked);
            connect(widget, &TrackListItem::trackRemoveClicked, this, &TrackListWidget::trackRemoveClicked);
        }
    }

    for (TrackListItem *item : items.values()) {
        item->onBeforeUpdate();
    }

    QListWidget::update();
}

MidiFile *TrackListWidget::midiFile() {
    return file;
}
