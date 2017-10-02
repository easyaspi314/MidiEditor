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

#ifndef UPDATESETTINGSWIDGET_H
#define UPDATESETTINGSWIDGET_H

#include "SettingsWidget.h"

class QSettings;
class QCheckBox;

class UpdateSettingsWidget : public SettingsWidget {

	Q_OBJECT

	public:
		UpdateSettingsWidget(QSettings *settings, QWidget *parent = Q_NULLPTR);

	public slots:
		void enableAutoUpdates(bool enable);

	private:
		QCheckBox *_auto;
		QSettings *_settings;
};

#endif
