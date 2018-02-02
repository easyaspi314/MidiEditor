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

#include "AdditionalMidiSettingsWidget.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QCheckBox>
#include <QSettings>
#include <QTextEdit>
#include <QSpinBox>
#include "../midi/MidiOutput.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiPlayer.h"
#include "../midi/MidiFile.h"
#include "../Terminal.h"

AdditionalMidiSettingsWidget::AdditionalMidiSettingsWidget(QSettings *settings, QWidget *parent) : SettingsWidget("Additional Midi Settings", parent) {

	_settings = settings;

	QGridLayout *layout = new QGridLayout(this);
	setLayout(layout);

	int row = 0;

	layout->addWidget(new QLabel(tr("Default ticks per quarter note:"), this), row, 0, 1, 2);
	_tpqBox = new QSpinBox(this);
	_tpqBox->setMinimum(1);
	_tpqBox->setMaximum(1024);
	_tpqBox->setValue(MidiFile::defaultTimePerQuarter);
	connect(_tpqBox, SIGNAL(valueChanged(int)), this, SLOT(setDefaultTimePerQuarter(int)));
	layout->addWidget(_tpqBox, row++, 2, 1, 4);

	QWidget *tpqInfo = createInfoBox(tr("Note: There aren't many reasons to change this. "
						"MIDI files have a resolution for how many ticks can fit in a quarter note. "
						"Higher values = more detail. Lower values may be required for compatibility. "
						"Only affects new files."));
	layout->addWidget(tpqInfo, row++, 0, 1, 6);

	layout->addWidget(separator(), row++, 0, 1, 6);

	_alternativePlayerModeBox = new QCheckBox(tr("Manually stop notes"), this);
	_alternativePlayerModeBox->setChecked(MidiOutput::isAlternativePlayer());
	connect(_alternativePlayerModeBox, SIGNAL(toggled(bool)), this, SLOT(manualModeToggled(bool)));
	layout->addWidget(_alternativePlayerModeBox, row++, 0, 1, 6);

	QWidget *playerModeInfo = createInfoBox(tr("Note: the above option should not be enabled in general. "
						   "It is only required if the stop button does not stop playback as expected "
						   "(e.g. when some notes are not stopped correctly)."));
	layout->addWidget(playerModeInfo, row++, 0, 1, 6);

	layout->addWidget(separator(), row++, 0, 1, 6);

	layout->addWidget(new QLabel(tr("Start command:"), this), row, 0, 1, 2);
	startCmd = new QLineEdit(this);
	layout->addWidget(startCmd, row++, 2, 1, 4);

	QWidget *startCmdInfo = createInfoBox("The start command can be used to start additional software components (e.g. Midi synthesizers) each time, MidiEditor is started. You can see the output of the started software / script in the field below.");
	layout->addWidget(startCmdInfo, row++, 0, 1, 6);

	layout->addWidget(Terminal::terminal()->console(), row, 0, 1, 6);
	startCmd->setText(_settings->value("start_cmd", "").toString());
	layout->setRowStretch(3, 1);

	row += 3;

	_gbaMode = new QCheckBox("Game Boy Advance mode (non-linear scaling)", this);
	_gbaMode->setChecked(MidiOutput::isGBAMode());
	connect(_gbaMode, SIGNAL(toggled(bool)), this, SLOT(gbaModeToggled(bool)));
	layout->addWidget(_gbaMode, row++, 0, 1, 6);

	layout->addWidget(new QLabel("Playback delay (for delayed MIDI drivers), ms:", this), row, 0, 1, 2);
	_playbackDelay = new QSpinBox(this);
	_playbackDelay->setMinimum(0);
	_playbackDelay->setMaximum(100000);
	_playbackDelay->setValue(MidiPlayer::playbackDelay());
	connect(_playbackDelay, SIGNAL(valueChanged(int)), this, SLOT(playbackDelayChanged(int)));
	layout->addWidget(_playbackDelay, row++, 2, 1, 4);

}

void AdditionalMidiSettingsWidget::manualModeToggled(bool enable){
	MidiOutput::setAlternativePlayer(enable);
}

void AdditionalMidiSettingsWidget::setDefaultTimePerQuarter(int value) {
	MidiFile::defaultTimePerQuarter = value;
}

void AdditionalMidiSettingsWidget::gbaModeToggled(bool enable) {
	MidiOutput::setGBAMode(enable);
}

void AdditionalMidiSettingsWidget::playbackDelayChanged(int value) {
	MidiPlayer::setPlaybackDelay(value);
}

bool AdditionalMidiSettingsWidget::accept(){
	QString text = startCmd->text();
	if(!text.isEmpty()){
		_settings->setValue("start_cmd", text);
	}
	return true;
}
