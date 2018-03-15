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

#ifndef DATAEDITOR_H
#define DATAEDITOR_H

#include <QScrollArea>

#include "../Utils.h"

class QPushButton;
class QLineEdit;

class DataLineEditor : public QObject {

    Q_OBJECT

    public:
        DataLineEditor(int line, QPushButton *plus, QPushButton *minus = qnullptr,
                       QLineEdit *edit = qnullptr, QObject *parent = qnullptr);

    public slots:
        void plus();
        void minus();
        void changed(const QString &text);

    signals:
        void dataChanged(int line, ubyte data);
        void plusClicked(int line);
        void minusClicked(int line);

    private:
        int _line;
};

class DataEditor : public QScrollArea
{
    Q_OBJECT

    public:
        DataEditor(QWidget *parent = qnullptr);
        void setData(const QByteArray &data);
        const QByteArray &data();

        void rebuild();

    public slots:
        void dataChanged(int line, ubyte data);
        void plusClicked(int line);
        void minusClicked(int line);

    private:
        QByteArray _data;
        QWidget *_central;

};

#endif
