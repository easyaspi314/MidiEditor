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
#include <QSettings>

#include "MatrixWidget.h"
#include "../tool/StandardTool.h"
#include "../tool/NewNoteTool.h"

GuiSettingsWidget::GuiSettingsWidget(QSettings *settings, QWidget *parent) : SettingsWidget("Interface Settings", parent) {

		_settings = settings;

		QGridLayout *layout = new QGridLayout(this);
		setLayout(layout);

		int row = 0;

		layout->addWidget(new QLabel("Editor settings", this), row++, 0, 1, 2);

		_selectAndMove = new QCheckBox("Select and move simultaneously in the Standard Tool", this);
		_selectAndMove->setChecked(StandardTool::selectAndMoveEnabled);
		connect(_selectAndMove, SIGNAL(toggled(bool)), this, SLOT(setSelectAndMove(bool)));
		layout->addWidget(_selectAndMove, row++, 0, 1, 2);

		layout->addWidget(separator(), row++, 0, 1, 2);

		layout->addWidget(new QLabel("Visual settings", this), row++, 0, 1, 2);

		_antiAliasingBox = new QCheckBox("Use antialiasing in the editor", this);
		_antiAliasingBox->setChecked(MatrixWidget::antiAliasingEnabled);

		connect(_antiAliasingBox, SIGNAL(toggled(bool)), this, SLOT(setAntiAliasing(bool)));
		layout->addWidget(_antiAliasingBox, row++, 0, 1, 2);

		_velocityDraggingBox = new QCheckBox("Set velocity of new notes by dragging up and down (beta)", this);
		_velocityDraggingBox->setChecked(NewNoteTool::enableVelocityDragging);

		connect(_velocityDraggingBox, SIGNAL(toggled(bool)), this, SLOT(setVelocityDragging(bool)));
		layout->addWidget(_velocityDraggingBox, row++, 0, 1, 2);

		layout->setRowStretch(row++, 4);

}

void GuiSettingsWidget::setAntiAliasing(bool enable) {
	MatrixWidget::antiAliasingEnabled = enable;
	QPixmapCache::clear();
}

void GuiSettingsWidget::setSelectAndMove(bool enable) {
	StandardTool::selectAndMoveEnabled = enable;
}

void GuiSettingsWidget::setVelocityDragging(bool enable) {
	NewNoteTool::enableVelocityDragging = enable;
}
