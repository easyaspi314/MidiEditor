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

#include "NToleQuantizationDialog.h"

#include <QComboBox>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>

#include <QtCore/qmath.h>

int NToleQuantizationDialog::ntoleNNum = 3;
int NToleQuantizationDialog::ntoleBeatNum = 3;
int NToleQuantizationDialog::replaceNumNum = 1;
int NToleQuantizationDialog::replaceDenomNum = 2;

NToleQuantizationDialog::NToleQuantizationDialog(QWidget *parent) : QDialog(parent) {

	connect(this, SIGNAL(accepted()), this, SLOT(takeResults()));

	setWindowTitle("Tuplet Quantization");

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(new QLabel("tuplet: ", this), 0,0,1,1);
	layout->addWidget(new QLabel("instead of: ", this), 1,0,1,1);

	ntoleBeat = new QComboBox(this);
	ntoleN = new QComboBox(this);
	replaceDenom = new QComboBox(this);
	replaceNum = new QComboBox(this);

	layout->addWidget(ntoleN, 0, 1, 1, 2);
	layout->addWidget(ntoleBeat, 0, 3, 1, 1);
	layout->addWidget(replaceNum, 1, 1, 1, 2);
	layout->addWidget(replaceDenom, 1, 3, 1, 1);

	for(int i = 1; i<21; i++){
		ntoleN->addItem(QString::number(i));
		replaceNum->addItem(QString::number(i));
	}

	for(int i = 0; i<=5; i++){
		QString text = "";

		if(i == 0){
			text = "Whole note";
		} else if(i == 1){
			text = "Half note";
		} else if(i == 2){
			text = "Quarter note";
		} else {
			text = QString::number((int)qPow(2, i))+"th note";
		}

		ntoleBeat->addItem(text);
		replaceDenom->addItem(text);
	}

	ntoleBeat->setCurrentIndex(ntoleBeatNum);
	ntoleN->setCurrentIndex(ntoleNNum-1);
	replaceDenom->setCurrentIndex(replaceDenomNum);
	replaceNum->setCurrentIndex(replaceNumNum-1);

	QPushButton *ok = new QPushButton("Ok", this);
	connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
	layout->addWidget(ok, 2, 0, 1, 2);


	QPushButton *close = new QPushButton("Cancel", this);
	connect(close, SIGNAL(clicked()), this, SLOT(reject()));
	layout->addWidget(close, 2, 2, 1, 2);
}

void NToleQuantizationDialog::takeResults(){
	ntoleNNum = ntoleN->currentIndex()+1;
	ntoleBeatNum = ntoleBeat->currentIndex();
	replaceNumNum = replaceNum->currentIndex()+1;
	replaceDenomNum = replaceDenom->currentIndex();
}
