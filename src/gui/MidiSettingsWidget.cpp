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

#include "MidiSettingsWidget.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QCheckBox>
#include <QTextEdit>
#include <QSpinBox>
#include "../midi/MidiOutput.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiFile.h"
#include "../Terminal.h"


MidiSettingsWidget::MidiSettingsWidget(QWidget *parent) : SettingsWidget("Midi I/O", parent) {

    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);

    int row = 0;

    QWidget *playerModeInfo = createInfoBox("Choose the Midi ports on your machine to which "
                                            "MidiEditor connects in order to play and record Midi data.");
    layout->addWidget(playerModeInfo, row++, 0, 1, 6);

    // output
    layout->addWidget(new QLabel("Midi output: ", this), row, 0, 1, 2);
    // input
    layout->addWidget(new QLabel("Midi input: ", this), row,3,1,2);

    _outList = new QListWidget(this);
    connect(_outList, &QListWidget::itemChanged, this, &MidiSettingsWidget::outputChanged);

    _inList = new QListWidget(this);
    connect(_inList, &QListWidget::itemChanged, this, &MidiSettingsWidget::inputChanged);

    QPushButton *reloadOutputList = new QPushButton(this);
    reloadOutputList->setToolTip("Refresh port list");
    reloadOutputList->setFlat(true);
    reloadOutputList->setIcon(QIcon(":/refresh.png"));
    reloadOutputList->setFixedSize(30, 30);
    layout->addWidget(reloadOutputList, row, 2, 1, 1);
    connect(reloadOutputList, &QPushButton::clicked, this, &MidiSettingsWidget::reloadOutputPorts);
    reloadOutputPorts();

    QPushButton *reloadInputList = new QPushButton(this);
    reloadInputList->setFlat(true);
    layout->addWidget(reloadInputList, row++, 5, 1, 1);
    reloadInputList->setToolTip("Refresh port list");
    reloadInputList->setIcon(QIcon(":/refresh.png"));
    reloadInputList->setFixedSize(30, 30);
    connect(reloadInputList, &QPushButton::clicked, this, &MidiSettingsWidget::reloadInputPorts);
    reloadInputPorts();

    layout->addWidget(_outList, row, 0, 1, 3);
    layout->addWidget(_inList, row++, 3, 1, 3);
}

void MidiSettingsWidget::reloadInputPorts() {

    disconnect(_inList, &QListWidget::itemChanged, this, &MidiSettingsWidget::inputChanged);

    // clear the list
    _inList->clear();

    for (const QString &name : MidiInput::instance()->inputPorts()) {

        QListWidgetItem *item = new QListWidgetItem(name, _inList,
                QListWidgetItem::UserType);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                Qt::ItemIsUserCheckable);

        if (name == MidiInput::instance()->inputPort()) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
        _inList->addItem(item);
    }
    connect(_inList, &QListWidget::itemChanged, this, &MidiSettingsWidget::inputChanged);
}

void MidiSettingsWidget::reloadOutputPorts() {

    disconnect(_outList, &QListWidget::itemChanged, this, &MidiSettingsWidget::outputChanged);

    // clear the list
    _outList->clear();

    for (const QString &name : MidiOutput::instance()->outputPorts()) {

        QListWidgetItem *item = new QListWidgetItem(name, _outList,
                QListWidgetItem::UserType);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled |
                Qt::ItemIsUserCheckable);

        if (name == MidiOutput::instance()->outputPort()) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
        _outList->addItem(item);
    }
    connect(_outList, &QListWidget::itemChanged, this, &MidiSettingsWidget::outputChanged);
}

void MidiSettingsWidget::inputChanged(QListWidgetItem *item) {

    if (item->checkState() == Qt::Checked) {

        MidiInput::instance()->setInputPort(item->text());
        _settings.in_port = item->text();
        reloadInputPorts();
    }
}

void MidiSettingsWidget::outputChanged(QListWidgetItem *item) {

    if (item->checkState() == Qt::Checked) {

        MidiOutput::instance()->setOutputPort(item->text());
        _settings.out_port = item->text();
        reloadOutputPorts();
    }
}
