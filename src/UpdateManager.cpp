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

#include "UpdateManager.h"

#include <QSettings>

#include <QtXml/QDomElement>
#include <QtXml/QDomDocument>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

#include "Singleton.h"

bool UpdateManager::_autoMode = false;

UpdateManager::UpdateManager(QObject *parent) : QObject(parent) {
	_versionString = "Failed to determine Version";
	_updateID = -1;
#ifdef Q_OS_WIN32
	_system = "win32";
#else
	#ifdef Q_OS_MAC
		_system = "macOS";
	#else
		#ifdef Q_PROCESSOR_X86_64
			_system = "linux64";
		#else
			_system = "linux32";
		#endif
	#endif
#endif
	connect(&_webCtrl, SIGNAL(finished(QNetworkReply *)), this,
			SLOT(fileDownloaded(QNetworkReply *)));
}

void UpdateManager::init() {
	_mirrors.append("https://greric.de/midieditor");
	_mirrors.append("http://midieditor.sourceforge.net/update");

	// read own configuration
	QDomDocument doc("version_info");
	QFile file("version_info.xml");
	if (file.open(QIODevice::ReadOnly)) {
		QString error;
		if (doc.setContent(&file, &error)) {
			QDomElement element = doc.documentElement();
			if (element.tagName() != "version_info") {
				_inited = false;
				qWarning("Error: UpdateManager failed to parse version_info.xml (unexpected root element)");
			} else {
				QDomElement version = element.firstChildElement("version");
				_versionString = version.attribute("string");
				_updateID = version.attribute("id").toInt();
				_date = version.attribute("date_published");
				_inited = true;
			}
		} else {
			_inited = false;
			qWarning("Error: UpdateManager failed to parse version_info.xml");
			qWarning("%s", error.toUtf8().constData());
		}
		file.close();
	} else {
		qWarning("Error: UpdateManager failed to open version_info.xml");
	}
}

QString UpdateManager::versionString() {
	return _versionString;
}

QString UpdateManager::date() {
	return _date;
}

UpdateManager *UpdateManager::createInstance() {
	return new UpdateManager();
}

UpdateManager *UpdateManager::instance() {
	return Singleton<UpdateManager>::instance(UpdateManager::createInstance);
}

void UpdateManager::checkForUpdates() {

	if (!_inited) {
		return;
	}

	listIndex = 0;
	tryNextMirror();
}

bool UpdateManager::autoCheckForUpdates() {
	return _autoMode;
}

void UpdateManager::setAutoCheckUpdatesEnabled(bool b) {
	_autoMode = b;
}

void UpdateManager::tryNextMirror() {

	if (listIndex >= _mirrors.size()) {
		return;
	}

	QString mirror = _mirrors.at(listIndex);
	listIndex++;

	QNetworkRequest request(QUrl(mirror + "/update.php?version=" + QString::number(
									 _updateID) + "&system=" + _system));
	request.setRawHeader("User-Agent", "MidiEditor UpdateManager");
	_webCtrl.get(request);
}

void UpdateManager::fileDownloaded(QNetworkReply *reply) {
	if (reply->error() != QNetworkReply::NoError) {
		//QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
		//if(!possibleRedirectUrl.isEmpty()){
		//	QNetworkRequest request(possibleRedirectUrl);
		//	_webCtrl.get(request);
		//	reply->deleteLater();
		//	return;
		//}
		qWarning("Network error: %s", reply->errorString().toUtf8().constData());
		tryNextMirror();
		return;
	}
	QByteArray data = reply->readAll();
	reply->deleteLater();

	QDomDocument doc("update");
	QString error;
	if (!doc.setContent(data, &error)) {
		qWarning("Error: UpdateManager failed to parse downloaded xml");
		qWarning("%s", error.toUtf8().constData());
		tryNextMirror();
		return;
	} else {
		QDomElement element = doc.documentElement();
		if (element.tagName() != "update") {
			_inited = false;
			qWarning("Error: UpdateManager failed to parse downloaded xml (unexpected root element)");
			tryNextMirror();
			return;
		} else {
			int thisVersion = element.attribute("your_version").toInt();
			if (thisVersion != _updateID) {
				qWarning("Error: UpdateManager failed to parse downloaded xml (unexpected own version ID)");
				tryNextMirror();
				return;
			}
			int newUpdate = element.attribute("latest_version").toInt();

			if (newUpdate > _updateID) {
				QString path = element.attribute("download_path");
				QString changelog = element.firstChildElement("changelog").text();
				QString newVersionString = element.attribute("latest_version_string");
				Update *update = new Update();
				update->setVersionID(newUpdate);
				update->setChangelog(changelog);
				update->setDownloadPath(path);
				update->setVersionString(newVersionString);

				emit updateDetected(update);
			}
		}
	}
}
