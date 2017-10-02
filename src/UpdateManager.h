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

#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QObject>
#include <QStringList>

#include <QByteArray>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class Update {

	public:
		Update() {}
		void setVersionID(int newUpdate) { _versionID = newUpdate; }
		void setChangelog(QString changelog) {_changelog = changelog; }
		void setDownloadPath(QString path) {_path = path; }
		void setVersionString(QString newVersionString) { _versionString = newVersionString; }

		int versionID() {return _versionID; }
		QString path() {return _path; }
		QString changelog() { return _changelog; }
		QString versionString() { return _versionString; }

	private:
		int _versionID;
		QString _path;
		QString _changelog;
		QString _versionString;
};

class UpdateManager : public QObject {

	Q_OBJECT

	public:
		void init();
		static UpdateManager *instance();
		static bool autoCheckForUpdates();
		static void setAutoCheckUpdatesEnabled(bool b);
		QString versionString();
		QString date();

	public slots:
		void checkForUpdates();
		void fileDownloaded(QNetworkReply*);

	signals:
		void updateDetected(Update *update);

	private:
		UpdateManager(QObject *parent = Q_NULLPTR);
		static UpdateManager *createInstance();
		QStringList _mirrors;
		static bool _autoMode;
		bool _inited;
		QString _versionString, _date, _system;
		int _updateID;

		int listIndex;
		void tryNextMirror();

		QNetworkAccessManager _webCtrl;
};

#endif
