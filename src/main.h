#ifndef MAIN_H
#define MAIN_H
#include <QApplication>
#ifdef Q_OS_MAC
#include <QFileOpenEvent>
#include "gui/MainWindow.h"
#endif
class MyApplication : public QApplication
{
		Q_OBJECT
public:
	 MyApplication(int &argc, char **argv)
		  : QApplication(argc, argv)
	 {
	 }

	 bool event(QEvent *event)
	 {
#ifdef Q_OS_MAC
		 // macOS handles files differently.
		  if (event->type() == QEvent::FileOpen) {
				QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
				if (MainWindow::getMainWindow()) {
					MainWindow::getMainWindow()->openFile(openEvent->file());
				}
		  }
#endif
		  return QApplication::event(event);
	 }
};
#endif // MAIN_H
