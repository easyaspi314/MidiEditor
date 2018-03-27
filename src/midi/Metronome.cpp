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

#include "Metronome.h"

#include "MidiFile.h"
#include "MidiPlayer.h"
#include "PlayerThread.h"

#include "../Singleton.h"

#include <QtCore/qmath.h>
#include <QFile>
#include <QFileInfo>
#include <QSoundEffect>
#include <QTime>

QSoundEffect *Metronome::clickSound = 0;

Metronome::Metronome(QObject *parent) : QObject(parent) {
    if (!clickSound) {
        clickSound = new QSoundEffect();
        clickSound->setSource(QUrl::fromLocalFile(":/metronome.wav"));
    }
    _file = 0;
    num = 4;
    denom = 2;
    lastMeasure = -1;
    lastPos = -1;
}

void Metronome::setFile(MidiFile *file) {
    _file = file;
}

void Metronome::measureUpdate(int measure, int tickInMeasure) {

    // compute pos
    if (!_file) {
        return;
    }

    int ticksPerClick = int((_file->ticksPerQuarter() * 4) / qPow(2, denom));
    int pos = tickInMeasure / ticksPerClick;

    if (lastMeasure < measure) {
        QTimer::singleShot(_settings.playbackDelay, this, &Metronome::doClick);
        lastMeasure = measure;
        lastPos = 0;
        return;
    } else {
        if (pos > lastPos) {
            QTimer::singleShot(_settings.playbackDelay, this, &Metronome::doClick);
            lastPos = pos;
            return;
        }
    }
}

void Metronome::meterChanged(ubyte n, ubyte d) {
    num = n;
    denom = d;
}

void Metronome::doClick() {
    emit click();
}

void Metronome::playbackStarted() {
    reset();
}

void Metronome::playbackStopped() {

}
Metronome *Metronome::createInstance() {
    return new Metronome();
}
Metronome *Metronome::instance() {
    return Singleton<Metronome>::instance(Metronome::createInstance);
}

void Metronome::reset() {
    lastPos = 0;
    lastMeasure = -1;
}

void Metronome::setEnabled(bool b) {
    if (b) {
        // metronome
        connect(MidiPlayer::player(), &PlayerThread::measureChanged,
                Metronome::instance(), &Metronome::measureUpdate);
        connect(MidiPlayer::player(), &PlayerThread::measureUpdate,
                Metronome::instance(), &Metronome::measureUpdate);
        connect(MidiPlayer::player(), &PlayerThread::meterChanged,
                Metronome::instance(), &Metronome::meterChanged);
        connect(MidiPlayer::player(), &PlayerThread::playerStopped,
                Metronome::instance(), &Metronome::playbackStopped);
        connect(MidiPlayer::player(), &PlayerThread::playerStarted,
                Metronome::instance(), &Metronome::playbackStarted);
        connect(Metronome::instance(), &Metronome::click,
                clickSound, &QSoundEffect::play, Qt::DirectConnection);
    } else {
        // metronome
        disconnect(MidiPlayer::player(), &PlayerThread::measureChanged,
                   Metronome::instance(), &Metronome::measureUpdate);
        disconnect(MidiPlayer::player(), &PlayerThread::measureUpdate,
                   Metronome::instance(), &Metronome::measureUpdate);
        disconnect(MidiPlayer::player(), &PlayerThread::meterChanged,
                   Metronome::instance(), &Metronome::meterChanged);
        disconnect(MidiPlayer::player(), &PlayerThread::playerStopped,
                   Metronome::instance(), &Metronome::playbackStopped);
        disconnect(MidiPlayer::player(), &PlayerThread::playerStarted,
                   Metronome::instance(), &Metronome::playbackStarted);
        disconnect(Metronome::instance(), &Metronome::click,
                   clickSound, &QSoundEffect::play);
    }
    _settings.metronome = b;
}
