#ifndef GUISETTINGSWIDGET_H
#define GUISETTINGSWIDGET_H

#include <QObject>
#include "SettingsWidget.h"

class QWidget;
class QCheckBox;

class GuiSettingsWidget : public SettingsWidget {
    Q_OBJECT

public:
    GuiSettingsWidget(QWidget *parent = qnullptr);

private:
    QCheckBox *_selectAndMove;
    QCheckBox *_antiAliasingBox;
    QCheckBox *_velocityDraggingBox;
};

#endif // GUISETTINGSWIDGET_H
