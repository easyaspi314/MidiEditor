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

#include "ProtocolWidget.h"
#include "../midi/MidiFile.h"
#include "../midi/MidiChannel.h"
#include "../protocol/Protocol.h"
#include "../protocol/ProtocolStep.h"

#include <QPainter>
#include <QLinearGradient>

ProtocolWidget::ProtocolWidget(QWidget *parent) : QListWidget(parent) {
    file = qnullptr;
    setSelectionMode(QAbstractItemView::NoSelection);
    protocolHasChanged = false;
    nextChangeFromList = false;
    setStyleSheet( "QListWidget::item { border-bottom: 1px solid lightGray; }" );
    setIconSize(QSize(15, 15));
    connect(this, &ProtocolWidget::itemClicked, this, &ProtocolWidget::stepClicked);
}

void ProtocolWidget::setFile(MidiFile *f) {
    file = f;
    protocolHasChanged = true;
    nextChangeFromList = false;
    connect(file->protocol(), &Protocol::actionFinished, this, &ProtocolWidget::protocolChanged);
    update();
}

void ProtocolWidget::protocolChanged() {
    protocolHasChanged = true;
    update();
}

void ProtocolWidget::update() {

    if (protocolHasChanged) {

        clear();

        if (!file) {
            QListWidget::update();
            return;
        }

        // construct list
        int stepsBack = file->protocol()->stepsBack();
        int stepsForward = file->protocol()->stepsForward();

        QFont undoFont;
        QFont redoFont;
        redoFont.setItalic(true);
        QFont currentFont;
        currentFont.setBold(true);

        QListWidgetItem *firstToRedo = qnullptr;

        for (int i = 0; i < stepsBack + stepsForward; i++) {
            ProtocolStep *step;
            QColor bg = Qt::black;
            QFont f = undoFont;
            if (i<stepsBack) {
                step = file->protocol()->undoStep(i);
                if (i == stepsBack-1) {
                    f = currentFont;
                }
            } else {
                step = file->protocol()->redoStep(stepsForward-i+stepsBack-1);
                bg = Qt::lightGray;
                f = redoFont;
            }

            // construct item
            QListWidgetItem *item = new QListWidgetItem(step->description());
            item->setSizeHint(QSize(0, 30));
            item->setFont(f);
            if (step->image()) {
                QImage img = step->image()->scaled(20, 20, Qt::KeepAspectRatio);
                item->setIcon(QIcon(QPixmap::fromImage(img)));
            } else {
                item->setIcon(QIcon(":/run_environment/graphics/tool/noicon.png"));
            }
            QVariant v;
            v.setValue(i);
            item->setData(Qt::UserRole, v);
            item->setForeground(bg);
            addItem(item);

            if (i>=stepsBack && !firstToRedo) {
                firstToRedo = item;
            }
        }


        protocolHasChanged = false;

        if (!nextChangeFromList) {
            if (!firstToRedo) {
                scrollToBottom();
            } else {
                scrollToItem(firstToRedo, QAbstractItemView::PositionAtCenter);
            }
        }
        nextChangeFromList = false;
    }

    QListWidget::update();
}

void ProtocolWidget::stepClicked(QListWidgetItem *item) {

    if (!file) {
        return;
    }

    nextChangeFromList = true;

    int num = item->data(Qt::UserRole).toInt();

    int stepsBack = file->protocol()->stepsBack();
    int stepsForward = file->protocol()->stepsForward();

    ProtocolStep *step;
    if (num<stepsBack) {
        step = file->protocol()->undoStep(num);
    } else {
        step = file->protocol()->redoStep(stepsForward - num + stepsBack - 1);
    }

    file->protocol()->goTo(step);
}
