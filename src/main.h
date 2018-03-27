#ifndef MAIN_H
#define MAIN_H

#include <QtGlobal>
#include <QApplication>

#include "Utils.h"

#include <QEvent>
#include "gui/MainWindow.h"



class MyApplication : public QApplication
{
        Q_OBJECT
public:
     MyApplication(int &argc, char **argv)
          : QApplication(argc, argv)
     {
     }
#ifdef Q_OS_MAC

     bool event(QEvent *event) qoverride
     {
         // macOS handles files differently.
          if (event->type() == QEvent::FileOpen) {
                QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
                if (MainWindow::getMainWindow()) {
                    MainWindow::getMainWindow()->openFile(openEvent->file());
                }
          }
          return QApplication::event(event);
     }
     #endif
};
#endif // MAIN_H
