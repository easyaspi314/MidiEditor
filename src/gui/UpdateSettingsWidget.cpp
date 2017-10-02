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

#include "UpdateSettingsWidget.h"

#include <QSettings>
#include <QGridLayout>
#include <QCheckBox>
#include <QPushButton>

#include "../UpdateManager.h"

UpdateSettingsWidget::UpdateSettingsWidget(QSettings *settings, QWidget *parent) : SettingsWidget("Updates", parent) {

	_settings = settings;

	QGridLayout *layout = new QGridLayout(this);
	setLayout(layout);

	_auto = new QCheckBox("Automatically check for Updates", this);
	_auto->setChecked(UpdateManager::autoCheckForUpdates());

	connect(_auto, SIGNAL(toggled(bool)), this, SLOT(enableAutoUpdates(bool)));
	layout->addWidget(_auto, 0, 0, 1, 6);

	layout->setRowStretch(5, 1);
}

void UpdateSettingsWidget::enableAutoUpdates(bool enable){
	UpdateManager::setAutoCheckUpdatesEnabled(enable);
}
