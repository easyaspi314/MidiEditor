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

#include <QNetworkAccessManager>
#include <QObject>
#include <QStringList>
#include "Utils.h"

class QNetworkReply;

class Update {

    public:
        Update() {}
        void setVersionID(int newUpdate) { _versionID = newUpdate; }
        void setChangelog(const QString &changelog) {_changelog = changelog; }
        void setDownloadPath(const QString &path) {_path = path; }
        void setVersionString(const QString &newVersionString) { _versionString = newVersionString; }

        int versionID() const { return _versionID; }
        const QString &path() const { return _path; }
        const QString &changelog() const { return _changelog; }
        const QString &versionString() const { return _versionString; }

    private:
        QString _path;
        QString _changelog;
        QString _versionString;
        int _versionID;
};

class UpdateManager : public QObject {

    Q_OBJECT

    public:
        void init();
        static UpdateManager *instance();
        QString versionString();
        QString date();

    public slots:
        void checkForUpdates();
        void fileDownloaded(QNetworkReply*);

    signals:
        void updateDetected(Update *update);

    private:
        UpdateManager(QObject *parent = qnullptr);
        static UpdateManager *createInstance();
        void tryNextMirror();

        QNetworkAccessManager _webCtrl;
        QStringList _mirrors;
        QString _versionString, _date, _system;

        int _updateID;
        int listIndex;

        bool _inited;
};

#endif
