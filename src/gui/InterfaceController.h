#ifndef INTERFACECONTROLLER_H
#define INTERFACECONTROLLER_H

#include <QObject>

#include "../Utils.h"

class InterfaceController : public QObject
{
        Q_OBJECT
    public:
        explicit InterfaceController(QObject *parent = nullptr);

    signals:

    public slots:

};

#endif // INTERFACECONTROLLER_H
