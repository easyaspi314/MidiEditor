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

#include "DataEditor.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

#include "../Utils.h"

DataLineEditor::DataLineEditor(int line, QPushButton *plus, QPushButton *minus, QLineEdit *edit, QObject *parent) : QObject(parent) {
    _line = line;
    connect(plus, &QPushButton::clicked, this, &DataLineEditor::plus);
    if (minus) {
        connect(minus, &QPushButton::clicked, this, &DataLineEditor::minus);
    }
    if (edit) {
        connect(edit, &QLineEdit::textChanged, this, &DataLineEditor::changed);
    }
}

void DataLineEditor::plus() {
    emit plusClicked(_line);
}

void DataLineEditor::minus() {
    emit minusClicked(_line);
}

void DataLineEditor::changed(const QString &text) {
    bool ok;
    int i = text.toInt(&ok, 16);
    if (ok) {
        emit dataChanged(_line, ubyte(i));
    }
}

DataEditor::DataEditor(QWidget *parent) :
    QScrollArea(parent)
{
    _central = new QWidget(this);
    setWidget(_central);
    setWidgetResizable(true);
    QGridLayout *layout = new QGridLayout(_central);
    _central->setLayout(layout);
    layout->setColumnStretch(4, 1);
}

void DataEditor::setData(const QByteArray &data) {
    this->_data = data;
    rebuild();
}

const QByteArray &DataEditor::data() {
    return _data;
}

void DataEditor::rebuild() {

    QGridLayout *layout = qobject_cast<QGridLayout*>(_central->layout());
    if (layout) {
        QList<QWidget*> widgets = _central->findChildren<QWidget *>();
        foreach(QWidget * child, widgets) {
            layout->removeWidget(child);
            delete child;
        }

        int row = 1;
        QPushButton *plus = new QPushButton("+", _central);
        layout->addWidget(plus, 0, 3, 1, 1);
        plus->setMaximumWidth(50);
        DataLineEditor *dle0 = new DataLineEditor(0, plus);
        connect(dle0, &DataLineEditor::plusClicked, this, &DataEditor::plusClicked);

        for (byte c : _data) {

            QLabel *label = new QLabel("0x", _central);
            layout->addWidget(label, row, 0, 1, 1);

            QLineEdit *edit = new QLineEdit(_central);
            edit->setInputMask("HH");
            QString text;
            text.sprintf("%02X", byte(c));
            edit->setText(text);
            layout->addWidget(edit, row, 1, 1, 1);

            QPushButton *minus = new QPushButton("-", _central);
            minus->setMaximumWidth(50);
            layout->addWidget(minus, row, 2, 1, 1);

            QPushButton *plus = new QPushButton("+", _central);
            layout->addWidget(plus, row, 3, 1, 1);
            plus->setMaximumWidth(50);

            DataLineEditor *dle = new DataLineEditor(row, plus, minus, edit);
            connect(dle, &DataLineEditor::minusClicked, this, &DataEditor::minusClicked);
            connect(dle, &DataLineEditor::plusClicked, this, &DataEditor::plusClicked);
            connect(dle, &DataLineEditor::dataChanged, this, &DataEditor::dataChanged);
            row++;
        }
    }
}

void DataEditor::dataChanged(int line, ubyte data) {
    _data[line-1] = byte(data);
}

void DataEditor::plusClicked(int line) {
    _data = _data.insert(line, char(0));
    rebuild();
}

void DataEditor::minusClicked(int line) {
    _data = _data.remove(line - 1, 1);
    rebuild();
}
