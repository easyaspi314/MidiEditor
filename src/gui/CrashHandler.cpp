#include "MainWindow.h"

#ifdef Q_OS_UNIX
#include <signal.h>
#include <unistd.h>
#include <cxxabi.h>
#include <QStringList>
#include <QUrl>
#include <QMessageBox>
#include <QDesktopServices>
#include <QApplication>
#include "../UpdateManager.h"
#include "version.h"
#include <execinfo.h>
#include "../midi/MidiFile.h"
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>

void handler(int sig) {
    int length = 20;
    void *array[20];

    // Generate the backtrace
    int size = backtrace(array, length);
    char **symbols = backtrace_symbols(array, size);
    QStringList list = QStringList();
    QStringList tmpList;
    list.reserve(size + 1);
    list.append(_("Error: Signal %1:").arg(sig));

    for (int i = 1; i < size; i++) {
        QString str = QString::fromUtf8(*(symbols + i));

        // Demangle the symbols.
        tmpList = str.split(' ');
        for (int j = 0; j < tmpList.size(); j++) {
            const QString &toDemangle = tmpList.at(j).trimmed();
            if (toDemangle.startsWith("_Z")) {
                int status = 0;
                const char *demangled_cstr = abi::__cxa_demangle(toDemangle.toUtf8().constData(),
                                                                 qnullptr, qnullptr, &status);
                if (status == 0) {
                    str.replace(toDemangle, QString::fromUtf8(demangled_cstr));
                }
                delete demangled_cstr;
                break;
            };
        }
        list.append(str); // NOLINT
    }
    const QString message = list.join('\n');
    qWarning("%s", message.toUtf8().constData());
    QUrl url = QUrl(_("https://github.com/abreheret/MidiEditor/issues/new"  // Create an issue
                      "?body="                                              // Automatically create body
                      "Please enter your issue and system info below:\n\n"  // Header
                      "<details>"                                           // fancy HTML5 collapsing
                      "<summary>System info:</summary>\n\n"                 // collapse summary
                      "%1 version %2 %3 (%4)\n"                             // Version name
                      "%5 (%6)\n"                                           // OS name and arch
                      "</details>"                                          // end collapse
                      "<details>"                                           // new collapse
                      "<summary>Stack trace:</summary>\n\n"                 // collapse
                      "```\n"                                               // begin code block
                      "%7\n"                                                // stack trace
                      "```\n\n"                                             // end code block
                      "</details>"                                          // end collapse
                      )
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
    // Stop playback and save to a backup file
    bool saveSuccessful = false;
    bool fileModified = false;
    if (MainWindow::_mainWindow) {
        MainWindow::_mainWindow->panic();
        MidiFile *file = MainWindow::_mainWindow->file();
        if (file) {
            if (!file->modified()) {
                fileModified = true;
                QString path = file->path();
                if (path.isEmpty()) {
                    path = "UntitledDocument.mid.bak";
                } else {
                    path = QFileInfo(path).fileName() % ".bak";
                }
                QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
                if (dataDir.isEmpty()) {
                    dataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
                }
                if (QDir().mkpath(dataDir)) {
                    QFile out;
                    int i = 0;
                    QString savePath = dataDir % "/" % path;
                    while (QFile(savePath).exists() && i < 10) {
                        savePath = dataDir % "/" % path % QString::number(++i);
                    }
                    saveSuccessful = file->save(savePath);
                    if (saveSuccessful) {
                        QSettings settings("MidiEditor", "None");
                        settings.setValue("backup_path", savePath);
                        settings.sync();
                    }
                }
            }
        }
    }
    if (sig == SIGABRT) {
        // SIGABRT means that we are closing and we can't do much about it.
        QDesktopServices::openUrl(url);
    } else {
        QMessageBox *msgBox = new QMessageBox();

        msgBox->setIcon(QMessageBox::Critical);
        msgBox->setWindowTitle("Error!");
        msgBox->setInformativeText(
                    _("Press Report to open a bug report on GitHub.$1").arg(
                        (fileModified) ? (
                                             (saveSuccessful) ?
                                                 _("\n\nYour changes will be restored when you relaunch.")
                                                 :
                                                 _("\n\nUnfortunately, your changes have been lost.")
                                         ) : QString()
                        ));
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
    _exit(sig);
}
#endif
