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

#ifndef SETTINGSDIALOG_H_
#define SETTINGSDIALOG_H_

#include <QDialog>

class QListWidget;
class QWidget;
class QString;
class QStackedWidget;
class SettingsWidget;
#ifdef ENABLE_REMOTE
class RemoteServer;
#endif

class SettingsDialog : public QDialog {

    Q_OBJECT

    public:
        SettingsDialog(const QString &title,
               #ifdef ENABLE_REMOTE
                       RemoteServer *server,
               #endif
                       QWidget *parent);
        void addSetting(SettingsWidget *settingsWidget);

    public slots:
        void rowChanged(int row);
        void submit();

    protected:
        QListWidget *_listWidget;
        QList<SettingsWidget*> *_settingsWidgets;
        QStackedWidget *_container;
};

#endif
