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

#include <QApplication>

#include "gui/MainWindow.h"
#include "midi/MidiOutput.h"
#include "midi/MidiInput.h"
#include "Utils.h"
#include "main.h"

#include "version.h"

#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>

#include <QMultiMap>
#include "UpdateManager.h"
#include <QResource>
#include "qlogging.h"
#include <execinfo.h>
#include <QDesktopServices>

#ifdef Q_OS_UNIX
    #include <signal.h>
    #include <unistd.h>
#endif

Settings _settings;

// TODO: Windows stack handling
#ifdef Q_OS_UNIX
void handler(int sig) {
#ifdef Q_OS_MAC
    // macOS's stack traces are very noisy with a bunch of Cocoa stuff,
    // so we make its stacks longer.
    int length = 20;
    void *array[20];
#else
    int length = 10;
    void *array[10];
#endif
    int size = backtrace(array, length); // NOLINT
    char **symbols = backtrace_symbols(array, size); // NOLINT
    QMessageBox *msgBox = new QMessageBox();
    QStringList list = QStringList();
    list.reserve(size + 1);
    list.append(_("Error: signal %1:").arg(sig));
    for (int i = 1; i < size; i++) {
        qWarning("%s", *(symbols + i)); // NOLINT
        list.append(QString::fromUtf8(*(symbols + i))); // NOLINT
    }
    QString message = list.join('\n');
    QUrl url = QUrl(_("https://github.com/abreheret/MidiEditor/issues/new"
                      "?body=Please enter your issue and system info below:\n"
                      "\n"
                      "<details><summary>System info:</summary>\n"
                      "\n"
                      "%1 version %2 %3 (%4)\n"
                      "%5 (%6)\n"
                      "</details>"
                      "<details>"
                      "<summary>Stack trace:</summary>\n"
                      "\n"
                      "```\n"
                      "%7\n"
                      "```\n"
                      "\n"
                      "</details>")
                    .arg(QApplication::instance()->applicationName(),
                         UpdateManager::instance()->versionString(),
                         _(META_INFO),
                         QSysInfo::buildCpuArchitecture(),
                         QSysInfo::prettyProductName(),
                         QSysInfo::currentCpuArchitecture(),
                         message)
                    // so the pluses in the stack traces aren't replaced with spaces
                    .replace("+", "%2B")
                    );
    if (sig == SIGABRT) {
        // we don't have time for a fancy dialog.
        QDesktopServices::openUrl(url);
    } else {
        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setWindowTitle("Error!");
        msgBox->setInformativeText("Press Report to open a bug report on GitHub.");
        msgBox->setText(_("%1 has crashed!").arg(QApplication::instance()->applicationName()));
        msgBox->setDetailedText(message);
        msgBox->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox->setButtonText(QMessageBox::Ok, "Report");
        if (msgBox->exec() == QMessageBox::Ok) {
            QDesktopServices::openUrl(url);
        }
    }
    // Stop the signal capture for recursion's sake
    signal(sig, SIG_DFL);
    exit(sig);
}
#endif

int main(int argc, char *argv[]) {

#ifdef Q_OS_UNIX
    // Install crash handlers
    signal(SIGSEGV, handler);
    signal(SIGFPE, handler);
    signal(SIGBUS, handler);
    signal(SIGPIPE, handler);
  //  signal(SIGABRT, handler);
#endif
    MyApplication a(argc, argv);

#ifdef Q_OS_MAC
    // macOS registers resources in a separate location because of .app packaging.
    bool ok = QResource::registerResource(a.applicationDirPath() %
                                          "/../Resources/ressources.rcc");

    if (!ok) {
        QResource::registerResource("ressources.rcc");
    }
#else
    bool ok = QResource::registerResource(a.applicationDirPath() %
                                          "/ressources.rcc");
    if (!ok) {
        QResource::registerResource("ressources.rcc");
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
    // Load styles for macOS to style buttons.
    QFile styleFile(":/run_environment/macos.qss");
    styleFile.open(QFile::ReadOnly);

    // Apply the loaded stylesheet
    QString style(L1(styleFile.readAll()));
    a.setStyleSheet(style);
#endif

#ifdef Q_PROCESSOR_X86_64
    a.setProperty("arch", "64");
#else
    a.setProperty("arch", "32");
#endif

    MainWindow w(a.arguments().size() >= 2 ? a.arguments().at(1) : QString(), qnullptr, Qt::Window);
#ifdef Q_OS_MAC
    w.setUnifiedTitleAndToolBarOnMac(true);
#endif
    w.resize(QSize(1280, 800));
    w.show();
    QTimer::singleShot(0, MidiOutput::instance(), &MidiOutput::init);
    QTimer::singleShot(0, MidiInput::instance(), &MidiInput::init);

    return a.exec();
}


