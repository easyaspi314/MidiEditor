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

#include "GuiSettingsWidget.h"

#include <QLabel>
#include <QGridLayout>
#include <QCheckBox>

#include "MatrixWidget.h"
#include "../tool/StandardTool.h"
#include "../tool/NewNoteTool.h"

GuiSettingsWidget::GuiSettingsWidget( QWidget *parent) : SettingsWidget("Interface Settings", parent) {

        QGridLayout *layout = new QGridLayout(this);
        setLayout(layout);

        int row = 0;

        layout->addWidget(new QLabel(QStringLiteral("Editor settings"), this), row++, 0, 1, 2);

        _selectAndMove = new QCheckBox(QStringLiteral("Select and move simultaneously in the Standard Tool"), this);
        _selectAndMove->setChecked(_settings.select_and_move);
        connect(_selectAndMove, &QCheckBox::toggled, this, [=](bool enable){ _settings.select_and_move = enable; });
        layout->addWidget(_selectAndMove, row++, 0, 1, 2);

        layout->addWidget(separator(), row++, 0, 1, 2);

        layout->addWidget(new QLabel(QStringLiteral("Visual settings"), this), row++, 0, 1, 2);

        _antiAliasingBox = new QCheckBox(QStringLiteral("Use antialiasing in the editor"), this);
        _antiAliasingBox->setChecked(_settings.antialiasing);

        connect(_antiAliasingBox, &QCheckBox::toggled, this,
                [=](bool enable) {
                    _settings.antialiasing = enable;
                    QPixmapCache::clear();
                });
        layout->addWidget(_antiAliasingBox, row++, 0, 1, 2);

        _velocityDraggingBox = new QCheckBox(QStringLiteral("Set velocity of new notes by dragging up and down (beta)"), this);
        _velocityDraggingBox->setChecked(_settings.velocityDragging);

        connect(_velocityDraggingBox, &QCheckBox::toggled, this, [=](bool enable) { _settings.velocityDragging = enable;});
        layout->addWidget(_velocityDraggingBox, row++, 0, 1, 2);

        layout->setRowStretch(row++, 4);

}
