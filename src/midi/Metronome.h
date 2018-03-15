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

#ifndef METRONOME_H
#define METRONOME_H

#include <QObject>
#include <QSoundEffect>
#include "../Utils.h"

class MidiFile;

class Metronome : public QObject {

    Q_OBJECT

    public:
        void setFile(MidiFile *file);
        static Metronome *instance();
        static void setEnabled(bool b);

    public slots:
        void measureUpdate(int measure, int tickInMeasure);
        void meterChanged(ubyte n, ubyte d);
        void playbackStarted();
        void playbackStopped();
        void doClick();

    signals:
        void click();


    private:
        Metronome(QObject *parent = qnullptr);
        static Metronome *createInstance();
        void reset();
        MidiFile *_file;
        static QSoundEffect *clickSound;

        int lastPos, lastMeasure;
        ubyte num, denom;
};

#endif
