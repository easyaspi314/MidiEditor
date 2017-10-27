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

#include <QApplication>

#include "gui/MainWindow.h"
#include "midi/MidiOutput.h"
#include "midi/MidiInput.h"
#include "Utils.h"
#include "main.h"

#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>

#include <QMultiMap>
#include "UpdateManager.h"
#include <QResource>
#include <QSurfaceFormat>

/*
 * TODO: Serious hack. Try to find out how to make QMacCGContext happy do we don't have to shut it up.
 * QTBUG-32639
 * Basically, Qt's documentation itself says "To render using OpenGL, simply call setViewport(new QGLWidget)."
 * It "works" but dragging a selection with the QOpenGLWidget (which works just the same as QGLWidget but isn't
 * deprecated) on macOS causes "QMacCGContext:: Unsupported painter devtype type 1" to be spammed in the log because
 * Qt is trying to draw a widget on OpenGL, which isn't supported yet.
 */
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	if (msg.contains("QMacCGContext:: Unsupported painter devtype type 1")) {
		// Shut up!
		return;
	}
    QByteArray localMsg = msg.toLocal8Bit();

    switch (type) {
    case QtInfoMsg:
	fprintf(stdout, "%s\n", localMsg.constData());
	break;
    case QtFatalMsg:
	fprintf(stderr, "%s\n", localMsg.constData());
	abort();
    default:
	fprintf(stderr, "%s\n", localMsg.constData());
	break;
    }
}
int main(int argc, char *argv[]) {
	MyApplication a(argc, argv);
   qInstallMessageHandler(myMessageOutput);
#ifdef Q_OS_MAC
   // macOS registers resources in a separate location because of .app packaging.
   bool ok = QResource::registerResource(a.applicationDirPath() +
				"/../Resources/ressources.rcc");

   if (!ok) {
      ok = QResource::registerResource("ressources.rcc");
   }
#else
   bool ok = QResource::registerResource(a.applicationDirPath() +
				"/ressources.rcc");
   if (!ok) {
      ok = QResource::registerResource("ressources.rcc");
   }
#endif

   UpdateManager::instance()->init();
   a.setApplicationVersion(UpdateManager::instance()->versionString());
   a.setApplicationName("MidiEditor");
   a.setQuitOnLastWindowClosed(true);
   a.setProperty("date_published", UpdateManager::instance()->date());
#ifdef Q_OS_MAC
   // Don't show menu icons on macOS.
   a.setAttribute(Qt::AA_DontShowIconsInMenus, true);
   a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
   // Load styles for macOS to style buttons.
   //QFile styleFile(":/run_environment/macos.qss");
   //styleFile.open(QFile::ReadOnly);

   // Apply the loaded stylesheet
  // QString style(styleFile.readAll());
   //a.setStyleSheet(style);
#endif

#ifdef Q_PROCESSOR_X86_64
   a.setProperty("arch", "64");
#else
   a.setProperty("arch", "32");
#endif

   MainWindow w(a.arguments().size() >= 2 ? a.arguments().at(1) : Q_NULLPTR, Q_NULLPTR, Qt::Window);
#ifdef Q_OS_MAC
   //  TODO: Native Toolbar + OpenGL
 //  w.setUnifiedTitleAndToolBarOnMac(true);
#endif
  // w.show();

   QTimer::singleShot(100, MidiOutput::instance(), SLOT(init()));
   QTimer::singleShot(100, MidiInput::instance(), SLOT(init()));

   return a.exec();
}


