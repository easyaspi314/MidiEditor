#ifndef MACGLUE_H
#define MACGLUE_H
#include <QObject>
#ifdef Q_OS_MAC
class QMacToolBar;
class MainWindow;

QMacToolBar *setupMacActions(MainWindow *mainWindow);

#endif
#endif // MACGLUE_H
