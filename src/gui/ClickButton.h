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

#ifndef CLICKBUTTON_H
#define CLICKBUTTON_H

#include <QPushButton>
#include <QFont>
#include <QPainter>
#include <QImage>

#include "../Utils.h"

class ClickButton  : public QPushButton {

    Q_OBJECT

    public:
        ClickButton(const QString &imageName, QWidget *parent = qnullptr);
        void setImageName(const QString &imageName);

    public slots:
        void buttonClick();

    protected:
        void paintEvent(QPaintEvent *event) qoverride;
        void enterEvent(QEvent *event) qoverride;
        void leaveEvent(QEvent *event) qoverride;

    private:
        QImage *image;
        #ifdef NO_BIT_PACK
            bool button_mouseInButton;
            bool button_mouseClicked;
        #else
            bool button_mouseInButton : 1;
            bool button_mouseClicked : 1;
        #endif

};
#endif
