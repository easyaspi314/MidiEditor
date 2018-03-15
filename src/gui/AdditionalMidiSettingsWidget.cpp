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

#include "AdditionalMidiSettingsWidget.h"

#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QCheckBox>
#include <QTextEdit>
#include <QSpinBox>

#include "../midi/MidiOutput.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiPlayer.h"
#include "../midi/MidiFile.h"
#include "../Terminal.h"

AdditionalMidiSettingsWidget::AdditionalMidiSettingsWidget(QWidget *parent) : SettingsWidget("Additional Midi Settings", parent) {
    QGridLayout *layout = new QGridLayout(this);
    setLayout(layout);

    int row = 0;

    layout->addWidget(new QLabel(tr("Default ticks per quarter note:"), this), row, 0, 1, 2);
    _tpqBox = new QSpinBox(this);
    _tpqBox->setMinimum(1);
    _tpqBox->setMaximum(16383);
    _tpqBox->setValue(_settings.ticks_per_quarter);
    connect(_tpqBox, SIGNAL_OL(QSpinBox, valueChanged, int),
                    this, [=](int value) { _settings.ticks_per_quarter = ushort(value); } );
    layout->addWidget(_tpqBox, row++, 2, 1, 4);

    QWidget *tpqInfo = createInfoBox(tr("Note: There aren't many reasons to change this. "
                        "MIDI files have a resolution for how many ticks can fit in a quarter note. "
                        "Higher values = more detail. Lower values may be required for compatibility. "
                        "Only affects new files."));
    layout->addWidget(tpqInfo, row++, 0, 1, 6);

    layout->addWidget(separator(), row++, 0, 1, 6);

    _alternativePlayerModeBox = new QCheckBox(tr("Manually stop notes"), this);
    _alternativePlayerModeBox->setChecked(_settings.alt_stop);
    connect(_alternativePlayerModeBox, &QCheckBox::toggled, this, [=](bool enable) { _settings.alt_stop = enable; });
    layout->addWidget(_alternativePlayerModeBox, row++, 0, 1, 6);

    QWidget *playerModeInfo = createInfoBox("Note: the above option should not be enabled in general. "
                           "It is only required if the stop button does not stop playback as expected "
                           "(e.g. when some notes are not stopped correctly).");
    layout->addWidget(playerModeInfo, row++, 0, 1, 6);

    layout->addWidget(separator(), row++, 0, 1, 6);

    layout->addWidget(new QLabel("Start command:", this), row, 0, 1, 2);
    startCmd = new QLineEdit(this);
    layout->addWidget(startCmd, row++, 2, 1, 4);

    QWidget *startCmdInfo = createInfoBox("The start command can be used to start additional software components (e.g. Midi synthesizers) each time, MidiEditor is started. You can see the output of the started software / script in the field below.");
    layout->addWidget(startCmdInfo, row++, 0, 1, 6);

    layout->addWidget(Terminal::terminal()->console(), row, 0, 1, 6);
    startCmd->setText(_settings.start_cmd);
    layout->setRowStretch(3, 1);

    row += 3;
#ifdef ENABLE_GBA
    _gbaMode = new QCheckBox("Game Boy Advance mode (non-linear scaling)", this);
    _gbaMode->setChecked(_settings.gba_mode);
    connect(_gbaMode, &QCheckBox::toggled, this, [=](bool enable) { _settings.gba_mode = enable; });
    layout->addWidget(_gbaMode, row++, 0, 1, 6);
#endif
    layout->addWidget(new QLabel("Playback delay (for delayed MIDI drivers), ms:", this), row, 0, 1, 2);
    _playbackDelay = new QSpinBox(this);
    _playbackDelay->setMinimum(0);
    _playbackDelay->setMaximum(2047);
    _playbackDelay->setValue(_settings.playbackDelay);
    connect(_playbackDelay, SIGNAL_OL(QSpinBox, valueChanged, int),
                    this, [=](int value) { _settings.playbackDelay = ushort(value); });
    layout->addWidget(_playbackDelay, row++, 2, 1, 4);

}


bool AdditionalMidiSettingsWidget::accept() {
    QString text = startCmd->text();
    if (!text.isEmpty()) {
        _settings.start_cmd = text;
    }
    return true;
}
