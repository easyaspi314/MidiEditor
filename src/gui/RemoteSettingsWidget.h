/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.+
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef REMOTEDIALOG_H_
#define REMOTEDIALOG_H_
#ifdef ENABLE_REMOTE


#include "SettingsWidget.h"

class RemoteServer;
class QLabel;
class QLineEdit;

class RemoteSettingsWidget : public SettingsWidget {

	Q_OBJECT

	public:

		RemoteSettingsWidget(RemoteServer *server, QWidget *parent = Q_NULLPTR);

		bool accept() Q_DECL_OVERRIDE;

	private:
		RemoteServer *_server;

		QLineEdit *_ipField, *_portField;
};
#else
class RemoteSettingsWidget {
	private:
		RemoteSettingsWidget();
};
#endif
#endif

