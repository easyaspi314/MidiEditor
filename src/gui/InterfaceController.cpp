// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/*
 * MidiEditor
 * Copyright (C) 2010  Markus Schwenk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.+
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "MainWindow.h"

#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QMenu>
#include <QComboBox>
#include <QProcess>
#include <QDir>
#include <QMimeData>
#include <QScrollArea>

#include "../midi/MidiFile.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiTrack.h"
#include "../midi/Metronome.h"
#include "../midi/PlayerThread.h"

#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/NoteOnEvent.h"

#include "../Terminal.h"


#include "ChannelListWidget.h"
#include "NToleQuantizationDialog.h"

#include "MiscWidget.h"
#include "SettingsDialog.h"
#include "EventWidget.h"
#include "UpdateDialog.h"
#include "MatrixWidget.h"
#include "AboutDialog.h"
#include "DonateDialog.h"
#include "FileLengthDialog.h"
#include "ProtocolWidget.h"
#include "RecordDialog.h"
#include "TransposeDialog.h"
#include "TrackListWidget.h"
#include "InstrumentChooser.h"

#include "../gba/MidFix4AgbDialog.h"


#include "../midi/MidiPlayer.h"
#include "../midi/MidiOutput.h"
#include "../midi/MidiInput.h"
#include "../tool/Selection.h"
#include "../tool/Tool.h"
#include "../tool/NewNoteTool.h"
#include "../tool/EventTool.h"
#include "../protocol/Protocol.h"

/*
 * This file simply contains the huge mass of slots from MainWindow.
 *
 * I would like to reduce these soon.
 */


void MainWindow::ioReady(bool isInput) {
    if (isInput) {
        inputIsReady = true;
    } else {
        outputIsReady = true;
    }
    if (inputIsReady && outputIsReady) {
        // terminal
        Terminal::initTerminal(_settings.start_cmd,
                _settings.in_port,
                _settings.out_port);
        //upperTabWidget->addTab(Terminal::terminal()->console(), "Terminal");
    }
}

void MainWindow::dropEvent(QDropEvent *ev)
{
    const QList<QUrl> urls = ev->mimeData()->urls();
    for (const QUrl &url : urls) {
        QString newFile = url.toLocalFile();
        if (!newFile.isEmpty()) {
            loadFile(newFile);
            break;
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->accept();
}

void MainWindow::scrollPositionsChanged(int x,int y)
{
    matrixArea->ensureVisible(x, y, 50, 0);
}

void MainWindow::setFile(MidiFile *file) {

    if (!file) {
        qWarning("Tried to set a null file.");
    }
    EventTool::clearSelection();
    Selection::instance()->setFile(file);
    Metronome::instance()->setFile(file);
    protocolWidget->setFile(file);
    channelWidget->setFile(file);
    _trackWidget->setFile(file);
#ifdef ENABLE_REMOTE
    _remoteServer->setFile(file);
#endif
    eventWidget()->setFile(file);

    Tool::setFile(file);
    this->_file = file;
    connect(file, &MidiFile::trackChanged, this, &MainWindow::updateTrackMenu);
    //setWindowTitle(QApplication::applicationName());
    setWindowFilePath(file->path());
    markEdited(false);
    connect(file, &MidiFile::cursorPositionChanged, channelWidget, &ChannelListWidget::update);
    connect(file, &MidiFile::recalcWidgetSize, mw_matrixWidget, &MatrixWidget::calcSizes);
    connect(file->protocol(), &Protocol::fileModified, this, &MainWindow::markEdited);
    connect(file->protocol(), &Protocol::actionFinished, eventWidget(), &EventWidget::reload);
    connect(file->protocol(), &Protocol::actionFinished, this, &MainWindow::checkEnableActionsForSelection);
    mw_matrixWidget->setFile(file);
    mw_timelineWidget->setFile(file);
    mw_pianoWidget->setFile(file);
    _miscWidget->setFile(file);
    updateChannelMenu();
    updateTrackMenu();
    mw_matrixWidget->update();
    mw_timelineWidget->update();
    mw_pianoWidget->update();
    _miscWidget->redraw();
    checkEnableActionsForSelection();
}

/*void MainWindow::matrixSizeChanged(int maxScrollTime, double maxScrollLine,
        int vX, double vY)
{
    vert->setMaximum(qRound(maxScrollLine));
    hori->setMaximum(maxScrollTime);
    vert->setValue(qRound(vY));
    hori->setValue(vX);
    //mw_matrixWidget->repaint();
}*/

void MainWindow::playStop() {
    if (MidiPlayer::instance()->isPlaying()) {
        stop();
    } else {
        play();
    }
}

void MainWindow::play() {
    // warn if there is no output port
    if (_settings.out_port.isEmpty()
            && MidiOutput::instance()->outputPort().isEmpty() &&
            !_settings.ignore_empty_port) {
        QMessageBox *emptyOutputWarning = new QMessageBox(this);
        emptyOutputWarning->setWindowModality(Qt::WindowModal);
        emptyOutputWarning->setText("There is no MIDI output selected. Would you like to open the settings to set one?");
        emptyOutputWarning->setInformativeText( _("Without an output port, playback will not work. "
                                                "To select an output port, select \"Yes\" and check an output device in the left column.\n\n"
                                                "If the left column is empty, make sure you have installed an output port and it is active."));
#ifdef Q_OS_OSX
        emptyOutputWarning->setDetailedText(_("On macOS, MIDI output may not be available by default.\n\n"
                                            "To enable playback to an instrument, check the \"Audio MIDI Setup\" app in /Applications/Utilities. "
                                            "Then, configure your device in Window→Show MIDI Studio.\n\n"
                                            "If you would like to enable playback through the headphones or speakers, check "
                                            "https://github.com/wbsoft/frescobaldi/wiki/MIDI-playback-on-Mac-OS-X "
                                            "for instructions."));
#else
#ifdef Q_OS_UNIX
        // more info in the future
        emptyOutputWarning->setDetailedText("On Unix-based systems, check your package manager for ALSA, Jack, or Fluidsynth.");
#endif
#ifdef Q_OS_WIN
        emptyOutputWarning->setDetailedText(_("On Windows, the default \"Microsoft GS Wavetable Synth\" has some compatibility issues with some MIDI effects.\n\n"
                                            "If you would like more accurate MIDI playback driver, a recommended synthesizer is \"Coolsoft VirtualMIDISynth\""
                                            " at http://coolsoft.altervista.org/en/virtualmidisynth combined with a SF2 soundfont such as the ones found"
                                            "on the website."));
#endif
#endif
        emptyOutputWarning->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Ignore);
        emptyOutputWarning->setDefaultButton(QMessageBox::Yes);
        int ret = emptyOutputWarning->exec();
        switch(ret) {
            case QMessageBox::No:
                break;
            case QMessageBox::Yes:
                openConfig();
                return;
            case QMessageBox::Ignore:
                _settings.ignore_empty_port = true;
                break;
            default:
                break;
        }
    }
    if (_file && !MidiInput::instance()->recording() && !MidiPlayer::instance()->isPlaying()) {
        mw_matrixWidget->timeMsChanged(_file->msOfTick(_file->cursorTick()), true);

        _miscWidget->setEnabled(false);
        channelWidget->setEnabled(false);
        protocolWidget->setEnabled(false);
        mw_matrixWidget->setEnabled(false);
        _trackWidget->setEnabled(false);
        eventWidget()->setEnabled(false);

        MidiPlayer::instance()->play(_file);

        connect(MidiPlayer::player(), &PlayerThread::playerStopped, this, SIGNAL_OL(MainWindow, stop));

#ifdef Q_OS_WIN32
        connect(MidiPlayer::player(), &PlayerThread::timeMsChanged, mw_matrixWidget, &MatrixWidget::timeMsChanged);
#endif
#ifdef ENABLE_REMOTE
        _remoteServer->play();
#endif
    }

}


void MainWindow::record() {
    // warn if there is no input port selected
    if (_settings.in_port.isEmpty() &&
            MidiInput::instance()->inputPort().isEmpty() &&
            !_settings.ignore_empty_port) {
        QMessageBox *emptyOutputWarning = new QMessageBox(this);
        emptyOutputWarning->setModal(true);
        emptyOutputWarning->setText("There is no MIDI input selected. Would you like to open the settings to set one?");
        emptyOutputWarning->setInformativeText(_("Without an input port, recording will not work. "
                                                 "To select an input port, select \"Yes\" and check an output port in the right column.\n\n"
                                                 "If the right column is empty, make sure you have installed an input device and it is active."));
        emptyOutputWarning->setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Ignore);
        emptyOutputWarning->setDefaultButton(QMessageBox::Yes);
        int ret = emptyOutputWarning->exec();
        switch(ret) {
            case QMessageBox::No:
                break;
            case QMessageBox::Yes:
                openConfig();
                return;
            case QMessageBox::Ignore:
                _settings.ignore_empty_port = true;
                break;
            default:
                break;
        }
    }
    if (!_file) {
        newFile();
    }

    if (!MidiInput::instance()->recording() && !MidiPlayer::instance()->isPlaying()) {
        // play current file
        if (_file) {

            if (_file->pauseTick() > 0) {
                _file->setCursorTick(_file->pauseTick());
                _file->setPauseTick(0);
            }

            mw_matrixWidget->timeMsChanged(_file->msOfTick(_file->cursorTick()), true);

            _miscWidget->setEnabled(false);
            channelWidget->setEnabled(false);
            protocolWidget->setEnabled(false);
            mw_matrixWidget->setEnabled(false);
            _trackWidget->setEnabled(false);
            eventWidget()->setEnabled(false);
#ifdef ENABLE_REMOTE
            _remoteServer->record();
#endif
            MidiPlayer::instance()->play(_file);
            MidiInput::instance()->startInput();
            connect(MidiPlayer::player(), &PlayerThread::playerStopped, this, SIGNAL_OL(MainWindow, stop));
            #ifdef Q_OS_WIN32
            connect(MidiPlayer::player(),
                    &PlayerThread::timeMsChanged, mw_matrixWidget, &MatrixWidget::timeMsChanged);
            #endif
        }
    }
}


void MainWindow::pause() {
    if (_file) {
        if (MidiPlayer::instance()->isPlaying()) {
            _file->setPauseTick(_file->tick(MidiPlayer::instance()->timeMs()));
            stop(false, false, false);
        }
    }
}
void MainWindow::stop() {
    stop(false, true, true);
}
void MainWindow::stop(bool autoConfirmRecord, bool addEvents, bool resetPause) {

    if (!_file) {
        return;
    }

    disconnect(MidiPlayer::player(), &PlayerThread::playerStopped, this, SIGNAL_OL(MainWindow, stop));


    if (resetPause) {
        _file->setPauseTick(-1);
        mw_matrixWidget->update();
    }
    if (!MidiInput::instance()->recording() && MidiPlayer::instance()->isPlaying()) {
        MidiPlayer::instance()->stop();
        _miscWidget->setEnabled(true);
        channelWidget->setEnabled(true);
        _trackWidget->setEnabled(true);
        protocolWidget->setEnabled(true);
        mw_matrixWidget->setEnabled(true);
        eventWidget()->setEnabled(true);
        mw_matrixWidget->timeMsChanged(MidiPlayer::instance()->timeMs(), true);
        _trackWidget->setEnabled(true);
#ifdef ENABLE_REMOTE
        _remoteServer->stop();
#endif
        panic();
    }

    MidiTrack *track = _file->track(NewNoteTool::editTrack());
    if (!track) {
        return;
    }

    if (MidiInput::instance()->recording()) {
        MidiPlayer::instance()->stop();
        panic();
        _miscWidget->setEnabled(true);
        channelWidget->setEnabled(true);
        protocolWidget->setEnabled(true);
        mw_matrixWidget->setEnabled(true);
        _trackWidget->setEnabled(true);
        eventWidget()->setEnabled(true);
#ifdef ENABLE_REMOTE
        _remoteServer->stop();
#endif
        QMultiMap<int, MidiEvent*> events = MidiInput::instance()->endInput(track);

        if (events.isEmpty() && !autoConfirmRecord) {
            QMessageBox::information(this, "Information", "No events recorded.");
        } else {
            RecordDialog *dialog = new RecordDialog(_file, events, this);
            dialog->setModal(true);
            if (!autoConfirmRecord) {
                dialog->show();
            } else {
                if (addEvents) {
                    dialog->enter();
                }
            }
        }
    }
}

void MainWindow::forward() {
    if (!_file) return;

    QList<TimeSignatureEvent*> *eventlist = new QList<TimeSignatureEvent*>;
    int ticksleft;
    int oldTick = _file->cursorTick();
    if (_file->pauseTick() > 0) {
        oldTick = _file->pauseTick();
    }
    if (MidiPlayer::instance()->isPlaying() && !MidiInput::instance()->recording()) {
        oldTick = _file->tick(MidiPlayer::instance()->timeMs());
        stop(true);
    }
    _file->measure(oldTick, oldTick, &eventlist, &ticksleft);

    int newTick = oldTick - ticksleft + eventlist->last()->ticksPerMeasure();
    _file->setPauseTick(0);
    if (newTick <= _file->endTick()) {
        _file->setCursorTick(newTick);
        mw_matrixWidget->timeMsChanged(_file->msOfTick(newTick), true);
    }
    mw_matrixWidget->update();
    mw_timelineWidget->update();
}

void MainWindow::back() {
    if (!_file) return;

    QList<TimeSignatureEvent*> *eventlist = new QList<TimeSignatureEvent*>;
    int ticksleft;
    int oldTick = _file->cursorTick();
    if (_file->pauseTick() > 0) {
        oldTick = _file->pauseTick();
    }
    if (MidiPlayer::instance()->isPlaying() && !MidiInput::instance()->recording()) {
        oldTick = _file->tick(MidiPlayer::instance()->timeMs());
        stop(true);
    }
    _file->measure(oldTick, oldTick, &eventlist, &ticksleft);
    int newTick = oldTick;
    if (ticksleft > 0) {
        newTick -= ticksleft;
    } else {
        newTick -= eventlist->last()->ticksPerMeasure();
    }
    _file->measure(newTick, newTick, &eventlist, &ticksleft);
    if (ticksleft > 0) {
        newTick -= ticksleft;
    }
    _file->setPauseTick(0);
    if (newTick > 0) {
        _file->setCursorTick(newTick);
        mw_matrixWidget->timeMsChanged(_file->msOfTick(newTick), true);
    }
    mw_matrixWidget->update();
    mw_timelineWidget->update();
}

void MainWindow::backToBegin() {
    if (!_file) return;

    _file->setPauseTick(0);
    _file->setCursorTick(0);

    mw_matrixWidget->update();
    mw_timelineWidget->update();
}

void MainWindow::save() {

    if (!_file) return;

    if (QFile(_file->path()).exists()) {

        bool printMuteWarning = false;

        for (ubyte i = 0; i < 16; i++) {
            MidiChannel *ch = _file->channel(i);
            if (ch->mute()) {
                printMuteWarning = true;
            }
        }
        const QList<MidiTrack *> *tracks = _file->tracks();
        for (MidiTrack *track : *tracks) {
            if (track->muted()) {
                printMuteWarning = true;
            }
        }

        if (printMuteWarning) {
            QMessageBox::information(this, "Channels/Tracks mute",
                    "One or more channels/tracks are not audible. They will be audible in the saved file.",
                    "Save file");
        }

        if (!_file->save(_file->path())) {
            QMessageBox::warning(this,
                    "Error",
                    "The file could not be saved. Please make sure that the destination directory exists and that you have the correct access rights to write into this directory.");
        } else {
            setWindowModified(false);
        }
    } else {
        saveas();
    }
}

void MainWindow::saveas() {

    if (!_file) return;

    QString oldPath = _file->path();
    QFile *f = new QFile(oldPath);
    QString dir = startDirectory;
    if (f->exists()) {
        QFileInfo(*f).dir().path();
    }
    QString newPath = QFileDialog::getSaveFileName(this, "Save file as…",
            dir);

    if (newPath.isEmpty()) {
        return;
    }

    // automatically add '.mid' extension
    if (!newPath.endsWith(L1(".mid"), Qt::CaseInsensitive) && !newPath.endsWith(L1(".midi"), Qt::CaseInsensitive))
    {
        newPath.append(".mid");
    }

    if (_file->save(newPath)) {

        bool printMuteWarning = false;

        for (ubyte i = 0; i < 16; i++) {
            MidiChannel *ch = _file->channel(i);
            if (ch->mute() || !ch->visible()) {
                printMuteWarning = true;
            }
        }
        for (MidiTrack *track : *(_file->tracks())) {
            if (track->muted() || track->hidden()) {
                printMuteWarning = true;
            }
        }

        if (printMuteWarning) {
            QMessageBox::information(this, "Channels/Tracks mute",
                    "One or more channels/tracks are not audible. They will be audible in the saved file.",
                    "Save file");
        }

        _file->setPath(newPath);
        //setWindowTitle(QApplication::applicationName()+" - " +file->path()+"[*]");
        setWindowFilePath(_file->path());
        updateRecentPathsList();
        setWindowModified(false);
    } else {
        QMessageBox::warning(this, "Error",
                             "The file could not be saved. Please make sure that the destination directory exists and that you have the correct access rights to write into this directory.");
    }
}

bool MainWindow::saveDialog() {
    QMessageBox box(QMessageBox::Question,
                         "Save file?",
                         "Save changes to " + (_file->path().isEmpty() ? "Untitled Document" : _file->path()) + " before closing?",
                         (QMessageBox::Save | QMessageBox::Cancel | QMessageBox::Discard),
                         this);
    // fancy slide-down dialogs on macOS
    box.setWindowModality(Qt::WindowModal);
    int result = box.exec();
    switch(result)
    {
        case QMessageBox::Save: {
            // save
            if (QFile(_file->path()).exists()) {
                _file->save(_file->path());
            } else {
                saveas();
            }
            return true;
        }
        case QMessageBox::Discard: {
            // close
            return true;
        }
        case QMessageBox::Cancel: {
            // break
            return false;
        }
    }
    return false;
}
void MainWindow::load() {
    QString oldPath = startDirectory;
    if (_file) {
        oldPath = _file->path();
        if (_file->modified()) {
            if (!saveDialog()) {
                return;
            }
        }
    }

    QFile *f = new QFile(oldPath);
    QString dir = startDirectory;
    if (f->exists()) {
        QFileInfo(*f).dir().path();
    }
    QString newPath = QFileDialog::getOpenFileName(this, "Open file",
            dir, "MIDI Files(*.mid *.midi);;All Files(*)");

    if (!newPath.isEmpty()) {
        openFile(newPath);
    }
}

void MainWindow::loadFile(const QString &nfile) {
    QString oldPath = startDirectory;
    if (_file) {
        oldPath = _file->path();
        if (_file->modified()) {
            switch (QMessageBox::question(this, "Save file?", "Save file " %
                _file->path() %
                " before closing?", "Save", "Close without saving", "Cancel", 0, 2))
            {
            case 0: {
                // save
                if (QFile(_file->path()).exists()) {
                    _file->save(_file->path());
                }
                else {
                    saveas();
                }
                break;
            }
            case 1: {
                // close
                break;
            }
            case 2: {
                // break
                return;
            }
            }
        }
    }
    if (!nfile.isEmpty()) {
        openFile(nfile);
    }
}

void MainWindow::openFile(const QString &filePath) {

    bool ok = true;

    QFile nf(filePath);

    if (!nf.exists()) {

        QMessageBox::warning(this, "Error", _("The file [%1] does not exist!").arg(filePath));
        return;
    }

    startDirectory = QFileInfo(nf).absoluteDir().path();
    QStringList list;
    MidiFile *mf = new MidiFile(filePath, &ok, &list, this);

    if (ok) {
        stop();
        setFile(mf);
        updateRecentPathsList();
    } else {
        QMessageBox::warning(this, "Error", _("The file is damaged and cannot be opened.").arg(list.join('\n')));
        delete mf;
    }
}

void MainWindow::redo() {
    if (_file) _file->protocol()->redo(true);
    updateTrackMenu();
}

void MainWindow::undo() {
    if (_file) _file->protocol()->undo(true);
    updateTrackMenu();
}

EventWidget *MainWindow::eventWidget() {
    return _eventWidget;
}

void MainWindow::showEventWidget(bool show) {
    if (show) {
        lowerTabWidget->setCurrentIndex(1);
    } else {
        lowerTabWidget->setCurrentIndex(0);
    }
}

void MainWindow::renameTrackMenuClicked(QAction *action) {
    ushort track = ushort(action->data().toUInt());
    renameTrack(track);
}

void MainWindow::renameTrack(ushort tracknumber) {

    if (!_file) {
        return;
    }

    _file->protocol()->startNewAction("Edit Track Name");

    bool ok;
    QString text = QInputDialog::getText(this, "Set Track Name",
         _("Track name (Track %1)").arg(tracknumber),
         QLineEdit::Normal,
         _file->tracks()->at(tracknumber)->name(), &ok);
    if (ok && !text.isEmpty()) {
        _file->tracks()->at(tracknumber)->setName(text);
    }

    _file->protocol()->endAction();
    updateTrackMenu();
}

void MainWindow::removeTrackMenuClicked(QAction *action) {
    ushort track = ushort(action->data().toUInt());
    removeTrack(track);
}

void MainWindow::removeTrack(ushort tracknumber) {

    if (!_file) {
        return;
    }
    MidiTrack *track = _file->track(tracknumber);
    _file->protocol()->startNewAction("Remove track");
    for (MidiEvent *event : Selection::instance()->selectedEvents()) {
        if (event->track() == track) {
            EventTool::deselectEvent(event);
        }
    }
    if (!_file->removeTrack(track)) {
        QMessageBox::warning(this, "Error", "The selected track can\'t be removed!\n It\'s the last track of the file.");
    }
    _file->protocol()->endAction();
    updateTrackMenu();
}

void MainWindow::addTrack() {

    if (_file) {

        bool ok;
        QString text = QInputDialog::getText(this, "Set Track Name",
             "Track name (New Track)", QLineEdit::Normal,
             "New Track", &ok);
        if (ok && !text.isEmpty()) {
            _file->protocol()->startNewAction("Add track");
            _file->addTrack();
            _file->tracks()->at(_file->numTracks()-1)->setName(text);
            _file->protocol()->endAction();

            updateTrackMenu();
        }
    }
}

void MainWindow::muteAllTracks() {
    if (!_file) return;
    _file->protocol()->startNewAction("Mute all tracks", qnullptr, false);
    for (MidiTrack *track : *(_file->tracks())) {
        track->setMuted(true);
    }
    _file->protocol()->endAction();
    _trackWidget->update();
}

void MainWindow::unmuteAllTracks() {
    if (!_file) return;
    _file->protocol()->startNewAction("All tracks audible", qnullptr, false);
    for (MidiTrack *track : *(_file->tracks())) {
        track->setMuted(false);
    }
    _file->protocol()->endAction();
    _trackWidget->update();
}

void MainWindow::allTracksVisible() {
    if (!_file) return;
    _file->protocol()->startNewAction("Show all tracks", qnullptr, false);
    for (MidiTrack *track : *(_file->tracks())) {
        track->setHidden(false);
    }
    _file->protocol()->endAction();
    _trackWidget->update();
}

void MainWindow::allTracksInvisible() {
    if (!_file) return;
    _file->protocol()->startNewAction("Hide all tracks", qnullptr, false);
    for (MidiTrack *track : *(_file->tracks())) {
        track->setHidden(true);
    }
    _file->protocol()->endAction();
    _trackWidget->update();
}

void MainWindow::showTrackMenuClicked(QAction *action) {
    ushort track = ushort(action->data().toUInt());
    if (_file) {
        _file->protocol()->startNewAction("Show track", qnullptr, false);
        _file->track(track)->setHidden(!(action->isChecked()));
        updateTrackMenu();
        _trackWidget->update();
        _file->protocol()->endAction();
    }
}

void MainWindow::muteTrackMenuClicked(QAction *action) {
    ushort track = ushort(action->data().toUInt());
    if (_file) {
        _file->protocol()->startNewAction("Mute track", qnullptr, false);
        _file->track(track)->setMuted(action->isChecked());
        updateTrackMenu();
        _trackWidget->update();
        _file->protocol()->endAction();
    }
}


void MainWindow::selectAllFromChannel(QAction *action) {

    if (!_file) {
        return;
    }
    ubyte channel = ubyte(action->data().toInt());
    _file->protocol()->startNewAction(_("Select all events from channel %1").arg(channel), qnullptr, false);
    EventTool::clearSelection();
    _file->channel(channel)->setVisible(true);
    for (MidiEvent *e : _file->channel(channel)->eventMap()->values()) {
        if (e->track()->hidden()) {
            e->track()->setHidden(false);
        }
        EventTool::selectEvent(e, false);
    }

    _file->protocol()->endAction();
}

void MainWindow::selectAllFromTrack(QAction *action) {

    if (!_file) {
        return;
    }

    ushort track = ushort(action->data().toUInt());
    _file->protocol()->startNewAction("Select all events from track "+QString::number(track), qnullptr, false);
    EventTool::clearSelection();
    _file->track(track)->setHidden(false);
    for (ubyte channel = 0; channel < 16; channel++) {
        for (MidiEvent *e : _file->channel(channel)->eventMap()->values()) {
            if (e->track()->number() == track) {
                _file->channel(e->channel())->setVisible(true);
                EventTool::selectEvent(e, false);
            }
        }
    }
    _file->protocol()->endAction();
}

void MainWindow::selectAll() {

    if (!_file) {
        return;
    }

    _file->protocol()->startNewAction("Select all", qnullptr, false);

    for (ubyte i = 0; i < 16; i++) {
        for (MidiEvent *event : _file->channel(i)->eventMap()->values()) {
            EventTool::selectEvent(event, false, true);
        }
    }

    _file->protocol()->endAction();
}

void MainWindow::transposeNSemitones() {

    if (!_file) {
        return;
    }

    QList<NoteOnEvent*> events;
    for (MidiEvent *event : Selection::instance()->selectedEvents()) {
        if (NoteOnEvent *on = protocol_cast<NoteOnEvent*>(event)) {
            events.append(on);
        }
    }

    if (events.isEmpty()) {
        return;
    }

    TransposeDialog *d = new TransposeDialog(events, _file, this);
    d->setModal(true);
    d->show();
}

void MainWindow::copy() {
    EventTool::copyAction();
}

void MainWindow::paste() {
    EventTool::pasteAction();
}

void MainWindow::markEdited(bool modified) {
    setWindowModified(modified);
}

void MainWindow::colorsByChannel() {
    mw_matrixWidget->setColorsByChannel();
    _colorsByChannel->setChecked(true);
    _colorsByTracks->setChecked(false);
    mw_matrixWidget->update();
    _miscWidget->update();
}
void MainWindow::colorsByTrack() {
    mw_matrixWidget->setColorsByTracks();
    _colorsByChannel->setChecked(false);
    _colorsByTracks->setChecked(true);
    mw_matrixWidget->update();
    _miscWidget->update();
}

void MainWindow::editChannel(int i) {
    editChannel(ubyte(i), true);
}

void MainWindow::editChannel(ubyte i, bool assign) {
    if (!_file) {
        return;
    }
    NewNoteTool::setEditChannel(i);

    // assign channel to track
    if (assign && _file && _file->track(NewNoteTool::editTrack())) {
        _file->track(NewNoteTool::editTrack())->assignChannel(i);
    }

    MidiOutput::instance()->setStandardChannel(i);

    ubyte prog = _file->channel(i)->progAtTick(_file->cursorTick());
    MidiOutput::instance()->sendProgram(i, prog);

    updateChannelMenu();
}

void MainWindow::editTrack(int i) {
    editTrack(ushort(i), true);
}

void MainWindow::editTrack(ushort i, bool assign) {
    NewNoteTool::setEditTrack(i);

    // assign channel to track
    if (assign && _file && _file->track(i)) {
        _file->track(i)->assignChannel(NewNoteTool::editChannel());
    }
    updateTrackMenu();
}

void MainWindow::editTrackAndChannel(MidiTrack *track) {
    editTrack(track->number(), false);
    if (track->assignedChannel() < 31) {
        editChannel(track->assignedChannel(), false);
    }
}

void MainWindow::setInstrumentForChannel(ubyte i) {
    if (i > 16) {
        return;
    }
    InstrumentChooser *d = new InstrumentChooser(_file, i, this);
    d->setModal(true);
    d->exec();

    if (i == NewNoteTool::editChannel()) {
        editChannel(i);
    }
    updateChannelMenu();
}

void MainWindow::instrumentChannel(QAction *action) {
    if (_file) {
        setInstrumentForChannel(ubyte(action->data().toUInt()));
    }
}
#ifdef ENABLE_GBA
void MainWindow::showMidFixDialog() {
    MidFix4AgbDialog *midFixDialog = new MidFix4AgbDialog(file, this);
    midFixDialog->show();
}
#endif
void MainWindow::changeMiscMode(int mode) {
    MiscWidgetMode mw_mode = MiscWidgetMode(mode);
    _miscWidget->setMode(mw_mode);
    if (mw_mode == MiscWidgetMode::VelocityEditor) {
        _miscChannel->setEnabled(false);
    } else {
        _miscChannel->setEnabled(true);
    }
    if (mw_mode == MiscWidgetMode::ControlEditor || mw_mode == MiscWidgetMode::KeyPressureEditor) {
        _miscController->setEnabled(true);
        _miscController->clear();

        for (ubyte i = 0; i < 128; i++) {
             if (mw_mode == MiscWidgetMode::ControlEditor) {
                 _miscController->addItem(_("%1: %2").arg(QString::number(i), MidiFile::controlChangeName(i)));
             } else {
                 _miscController->addItem(_("Note: %1").arg(QString::number(i)));
             }
        }
    } else {
        _miscController->setEnabled(false);
    }
}


void MainWindow::muteAllChannels() {
    if (!_file) return;
    _file->protocol()->startNewAction("Mute all channels", qnullptr, false);
    for (ubyte i = 0; i < 19; i++) {
        _file->channel(i)->setMute(true);
    }
    _file->protocol()->endAction();
    channelWidget->update();
}

void MainWindow::unmuteAllChannels() {
    if (!_file) return;
    _file->protocol()->startNewAction("All channels audible", qnullptr, false);
    for (ubyte i = 0; i < 19; i++) {
        _file->channel(i)->setMute(false);
    }
    _file->protocol()->endAction();
    channelWidget->update();
}

void MainWindow::allChannelsVisible() {
    if (!_file) return;
    _file->protocol()->startNewAction("All channels visible", qnullptr, false);
    for (ubyte i = 0; i < 19; i++) {
        _file->channel(i)->setVisible(true);
    }
    _file->protocol()->endAction();
    channelWidget->update();
}

void MainWindow::allChannelsInvisible() {
    if (!_file) return;
    _file->protocol()->startNewAction("Hide all channels", qnullptr, false);
    for (ubyte i = 0; i < 19; i++) {
        _file->channel(i)->setVisible(false);
    }
    _file->protocol()->endAction();
    channelWidget->update();
}

void MainWindow::donate() {
    DonateDialog *d = new DonateDialog(this);
    d->setModal(true);
    d->show();
}

void MainWindow::about() {
    AboutDialog *d = new AboutDialog(this);
    d->setModal(true);
    d->show();
}

void MainWindow::setFileLengthMs() {
    if (!_file) return;

    FileLengthDialog *d = new FileLengthDialog(_file, this);
    d->setModal(true);
    d->show();
}

void MainWindow::setStartDir(const QString &dir) {
    startDirectory = dir;
}

void MainWindow::newFile() {
    if (_file && _file->modified()) {
        if (!saveDialog()) {
            return;
        }
    }

    // create new File
    MidiFile *f = new MidiFile(this);

    setFile(f);

    editTrack(1);
    setWindowTitle(QApplication::applicationName()+ " - Untitled Document[*]");

    if (_settings.numStarts == 14) {
        donate();
    }
}

void MainWindow::panic() {
    MidiPlayer::instance()->panic();
}

void MainWindow::screenLockPressed(bool enable) {
    mw_matrixWidget->setScreenLocked(enable);
}

void MainWindow::scaleSelection() {
    bool ok;
    double scale = QInputDialog::getDouble(this, "Scale factor",
            "Scale factor:", 1.0, 0, INT_MAX, 17, &ok);
    if (ok && scale > 0 && !Selection::instance()->selectedEvents().isEmpty() && _file) {
        // find minimum
        int minTime = INT_MAX;
        for (MidiEvent *e : Selection::instance()->selectedEvents()) {
            if (e->midiTime() < minTime) {
                minTime = e->midiTime();
            }
        }

        _file->protocol()->startNewAction("Scale events", qnullptr);
        for (MidiEvent *e : Selection::instance()->selectedEvents()) {
            // Use qFloor because we don't want to have rounding issues.
            e->setMidiTime(qFloor((e->midiTime() - minTime) * scale + minTime));
            if (OnEvent *on = protocol_cast<OnEvent*>(e)) {
                MidiEvent *off = on->offEvent();
                off->setMidiTime(qFloor((off->midiTime() - minTime) * scale + minTime));
            }
        }
        _file->protocol()->endAction();
    }
}

void MainWindow::alignLeft() {
    if (Selection::instance()->selectedEvents().size() > 1 && _file) {
        // find minimum
        int minTime = INT_MAX;
        for (MidiEvent *e : Selection::instance()->selectedEvents()) {
            if (e->midiTime() < minTime) {
                minTime = e->midiTime();
            }
        }

        _file->protocol()->startNewAction("Align left", new QImage(":/align_left.png"));
        for (MidiEvent *e : Selection::instance()->selectedEvents()) {
            int onTime = e->midiTime();
            e->setMidiTime(minTime);
            OnEvent *on = protocol_cast<OnEvent*>(e);
            if (on) {
                MidiEvent *off = on->offEvent();
                off->setMidiTime(minTime + (off->midiTime()-onTime));
            }
        }
        _file->protocol()->endAction();
    }
}

void MainWindow::alignRight() {
    if (Selection::instance()->selectedEvents().size() > 1 && _file) {
        // find maximum
        int maxTime = 0;
        for (MidiEvent *e : Selection::instance()->selectedEvents()) {
            OnEvent *on = protocol_cast<OnEvent*>(e);
            if (on) {
                MidiEvent *off = on->offEvent();
                if (off->midiTime() > maxTime) {
                        maxTime = off->midiTime();
                }
            }
        }

        _file->protocol()->startNewAction("Align right", new QImage(":/align_right.png"));
        for (MidiEvent *e : Selection::instance()->selectedEvents()) {
                int onTime = e->midiTime();
                OnEvent *on = protocol_cast<OnEvent*>(e);
                if (on) {
                        MidiEvent *off = on->offEvent();
                        e->setMidiTime(maxTime - (off->midiTime()-onTime));
                        off->setMidiTime(maxTime);
                }
        }
        _file->protocol()->endAction();
    }
}

void MainWindow::equalize()
{
    if (Selection::instance()->selectedEvents().size() > 1 && _file) {
        // find average
        int avgStart = 0;
        int avgTime = 0;
        int count = 0;
        for (MidiEvent *e : Selection::instance()->selectedEvents()) {
            OnEvent *on = protocol_cast<OnEvent*>(e);
            if (on) {
                MidiEvent *off = on->offEvent();
                avgStart += e->midiTime();
                avgTime += (off->midiTime() - e->midiTime());
                count++;
            }
        }
        if (count > 1) {
            avgStart /= count;
            avgTime /= count;

            _file->protocol()->startNewAction("Equalize", new QImage(":/equalize.png"));
            for (MidiEvent *e : Selection::instance()->selectedEvents()) {
                OnEvent *on = protocol_cast<OnEvent*>(e);
                if (on) {
                    MidiEvent *off = on->offEvent();
                    e->setMidiTime(avgStart);
                    off->setMidiTime(avgStart + avgTime);
                }
            }
        }
        _file->protocol()->endAction();
    }
}

void MainWindow::deleteSelectedEvents() {
    bool showsSelected = false;
    if (Tool::currentTool()) {
        EventTool *eventTool = protocol_cast<EventTool*>(Tool::currentTool());
        if (eventTool) {
            showsSelected = eventTool->showsSelection();
        }
    }
    if (showsSelected && Selection::instance()->selectedEvents().size()>0 && _file) {

        _file->protocol()->startNewAction("Remove event(s)");
        for (MidiEvent *ev : Selection::instance()->selectedEvents()) {
            _file->channel(ev->channel())->removeEvent(ev);
        }
        Selection::instance()->clearSelection();
        eventWidget()->reportSelectionChangedByTool();
        _file->protocol()->endAction();
    }
}

void MainWindow::deleteChannel(QAction *action) {

    if (!_file) {
        return;
    }

    ubyte num = ubyte(action->data().toUInt());
    _file->protocol()->startNewAction("Remove all events from channel "+QString::number(num));
    for (MidiEvent *event : _file->channel(num)->eventMap()->values()) {
        if (Selection::instance()->selectedEvents().contains(event)) {
            EventTool::deselectEvent(event);
        }
    }

    _file->channel(num)->deleteAllEvents();
    _file->protocol()->endAction();
}


void MainWindow::moveSelectedEventsToChannel(QAction *action) {

    if (!_file) {
        return;
    }

    ubyte num = ubyte(action->data().toUInt());
    MidiChannel *channel = _file->channel(num);

    if (Selection::instance()->selectedEvents().size() > 0) {
        _file->protocol()->startNewAction("Move selected events to channel " + QString::number(num));
        for (MidiEvent *ev : Selection::instance()->selectedEvents()) {
            _file->channel(ev->channel())->removeEvent(ev);
            ev->setChannel(num, true);
            OnEvent *onevent = protocol_cast<OnEvent*>(ev);
            if (onevent) {
                channel->insertEvent(onevent->offEvent(), onevent->offEvent()->midiTime());
                onevent->offEvent()->setChannel(num);
            }
            channel->insertEvent(ev, ev->midiTime());
        }

        _file->protocol()->endAction();
    }
}

void MainWindow::moveSelectedEventsToTrack(QAction *action) {

    if (!_file) {
        return;
    }

    ubyte num = ubyte(action->data().toUInt());
    MidiTrack *track = _file->track(num);

    if (Selection::instance()->selectedEvents().size()>0) {
        _file->protocol()->startNewAction("Move selected events to track "+QString::number(num));
        for (MidiEvent *ev : Selection::instance()->selectedEvents()) {
            ev->setTrack(track, true);
            OnEvent *onevent = protocol_cast<OnEvent*>(ev);
            if (onevent) {
                onevent->offEvent()->setTrack(track);
            }
        }

        _file->protocol()->endAction();
    }
}

void MainWindow::updateRecentPathsList() {

    // if file opened put it at the top of the list
    if (_file) {

        QString currentPath = _file->path();
        QStringList newList;
        newList.append(currentPath);

        for (const QString &str : qAsConst(_recentFilePaths)) {
            if (str != currentPath && newList.size() < 10) {
                newList.append(str);
            }
        }

        _recentFilePaths = newList;
    }
    // update menu
    _recentPathsMenu->clear();
    for (const QString &path : qAsConst(_recentFilePaths)) {
        QFile f(path);
        QString name = QFileInfo(f).fileName();

        QVariant variant(path);
        QAction *openRecentFileAction = new QAction(name, this);
        openRecentFileAction->setData(variant);
        _recentPathsMenu->addAction(openRecentFileAction);
    }

}

void MainWindow::openRecent(QAction *action) {

    QString path = action->data().toString();

    if (_file) {
        if (_file->modified()) {
            if (!saveDialog()) {
                return;
            }
        }
    }

    openFile(path);
}


void MainWindow::updateChannelMenu() {

    // delete channel events menu
    for (QAction *action : _deleteChannelMenu->actions()) {
        ubyte channel = ubyte(action->data().toUInt());
        if (_file && channel <= 0xF) {
            action->setText(QString::number(channel) % " " % MidiFile::instrumentName(_file->channel(channel)->progAtTick(0)));
        }
    }

    // move events to channel...
    for (QAction *action : _moveSelectedEventsToChannelMenu->actions()) {
        ubyte channel = ubyte(action->data().toUInt());
        if (_file && channel <= 0xF) {
            action->setText(QString::number(channel) % " " % MidiFile::instrumentName(_file->channel(channel)->progAtTick(0)));
        }
    }

    // paste events to channel...
    for (QAction *action : _pasteToChannelMenu->actions()) {
        ubyte channel = ubyte(action->data().toUInt());
        if (_file && channel <= 0xF) {
            action->setText(QString::number(channel) % " " % MidiFile::instrumentName(_file->channel(channel)->progAtTick(0)));
        }
    }

    // select all events from channel...
    for (QAction *action : _selectAllFromChannelMenu->actions()) {
        ubyte channel = ubyte(action->data().toUInt());
        if (_file && channel < 0xFF) {
            action->setText(QString::number(channel) % " " % MidiFile::instrumentName(_file->channel(channel)->progAtTick(0)));
        }
    }

    _chooseEditChannel->setCurrentIndex(NewNoteTool::editChannel());
}

void MainWindow::updateTrackMenu() {

    _moveSelectedEventsToTrackMenu->clear();
    _chooseEditTrack->clear();
    _selectAllFromTrackMenu->clear();

    if (!_file) {
        return;
    }

    for (ushort i = 0; i < _file->numTracks(); i++) {
        QVariant variant(i);
        QAction *moveToTrackAction = new QAction(QString::number(i) % " " % _file->tracks()->at(i)->name(), this);
        moveToTrackAction->setData(variant);
        moveToTrackAction->setShortcut(QKeySequence(Qt::Key_0 + ushort(i) + Qt::ALT));
        _moveSelectedEventsToTrackMenu->addAction(moveToTrackAction);
        QAction *select = new QAction(QString::number(i) % " " % _file->tracks()->at(i)->name(), this);
        select->setData(variant);
        _selectAllFromTrackMenu->addAction(select);
        _chooseEditTrack->addItem("Track " % QString::number(i) % ": " % _file->tracks()->at(i)->name());
    }
    if (NewNoteTool::editTrack() >= _file->numTracks()) {
        NewNoteTool::setEditTrack(0);
    }
    _chooseEditTrack->setCurrentIndex(NewNoteTool::editTrack());

    _pasteToTrackMenu->clear();
    QActionGroup *pasteTrackGroup = new QActionGroup(this);
    pasteTrackGroup->setExclusive(true);

    bool checked = false;
    for (int i = -2; i < _file->numTracks(); i++) {
        QVariant variant(i);
        QString text = QString::number(i);
        if (i == -2) {
            text = "Same as selected for new events";
        } else if (i == -1) {
            text = "Keep track";
        } else {
            text = _("Track %1: %2").arg(QString::number(i), _file->tracks()->at(i)->name());
        }
        QAction *pasteToTrackAction = new QAction(text, this);
        pasteToTrackAction->setData(variant);
        pasteToTrackAction->setCheckable(true);
        _pasteToTrackMenu->addAction(pasteToTrackAction);
        pasteTrackGroup->addAction(pasteToTrackAction);
        if (i == EventTool::pasteTrack()) {
            pasteToTrackAction->setChecked(true);
            checked = true;
        }
    }
    if (!checked) {
        _pasteToTrackMenu->actions().constFirst()->setChecked(true);
        EventTool::setPasteTrack(0);
    }
}

void MainWindow::muteChannel(QAction *action) {
    ubyte channel = ubyte(action->data().toUInt());
    if (_file) {
        _file->protocol()->startNewAction("Mute channel", qnullptr, false);
        _file->channel(channel)->setMute(action->isChecked());
        updateChannelMenu();
        channelWidget->update();
        _file->protocol()->endAction();
    }
}
void MainWindow::soloChannel(QAction *action) {
    ubyte channel = ubyte(action->data().toUInt());
    if (_file) {
        _file->protocol()->startNewAction("Select solo channel", qnullptr, false);
        for (ubyte i = 0; i < 16; i++) {
            _file->channel(i)->setSolo(i == channel && action->isChecked());
        }
        _file->protocol()->endAction();
    }
    channelWidget->update();
    updateChannelMenu();
}

void MainWindow::viewChannel(QAction *action) {
    ubyte channel = action->data().toUInt() & 0xF;
    if (_file) {
        _file->protocol()->startNewAction("Channel visibility changed", qnullptr, false);
        _file->channel(channel)->setVisible(action->isChecked());
        updateChannelMenu();
        channelWidget->update();
        _file->protocol()->endAction();
    }
}

void MainWindow::selectModeChanged(QAction *action) {
    if (action == setSingleMode) {
        _miscWidget->setEditMode(MiscWidgetEditMode::SingleMode);
    }
    if (action == setLineMode) {
        _miscWidget->setEditMode(MiscWidgetEditMode::LineMode);
    }
    if (action == setFreehandMode) {
        _miscWidget->setEditMode(MiscWidgetEditMode::FreehandMode);
    }
}

void MainWindow::pasteToChannel(QAction *action) {
    EventTool::setPasteChannel(ubyte(action->data().toUInt()));
}

void MainWindow::pasteToTrack(QAction *action) {
    EventTool::setPasteTrack(ubyte(action->data().toUInt()));
}

void MainWindow::divChanged(QAction* action) {
    int data = action->data().toInt();
    if (data > 0) {
        mw_matrixWidget->setDiv(ubyte(data));
    }
}

void MainWindow::enableMagnet(bool enable) {
    _settings.magnet = enable;
}

void MainWindow::openConfig() {
#ifdef ENABLE_REMOTE
    SettingsDialog *d = new SettingsDialog("Settings", _remoteServer, this);
#else
    SettingsDialog *d = new SettingsDialog("Settings", this);
#endif
    d->show();
}

void MainWindow::enableMetronome(bool enable) {
    Metronome::setEnabled(enable);
}

void MainWindow::enableThru(bool enable) {
    _settings.thru = enable;
}

void MainWindow::quantizationChanged(QAction *action) {
    _settings.quantization = ubyte(action->data().toUInt());
}

void MainWindow::quantizeSelection() {

    if (!_file) {
        return;
    }

    // get list with all quantization ticks
    QList<int> ticks = _file->quantization(_settings.quantization);

    _file->protocol()->startNewAction("Quantify events", new QImage(":/quantize.png"));
    for (MidiEvent *e : Selection::instance()->selectedEvents()) {
        int onTime = e->midiTime();
        e->setMidiTime(quantize(onTime, ticks));
        OnEvent *on = protocol_cast<OnEvent*>(e);
        if (on) {
            MidiEvent *off = on->offEvent();
            off->setMidiTime(quantize(off->midiTime(), ticks));
            if (off->midiTime() == on->midiTime()) {
                int idx = ticks.indexOf(off->midiTime());
                if ((idx >= 0) && (ticks.size()>idx+1)) {
                    off->setMidiTime(ticks.at(idx+1));
                }
            }
        }
    }
    _file->protocol()->endAction();
}

int MainWindow::quantize(int t, QList<int> ticks) {

    int min = -1;

    for (int j = 0; j < ticks.size(); j++) {

        if (min < 0) {
            min = j;
            continue;
        }

        int i = ticks.at(j);

        int dist = t - i;
        bool distIsPositive = false;
        if (t > i) {
            distIsPositive = true;
            dist = t - i;
        } else {
            dist = i - t;
        }

        int b = qAbs(t - ticks.at(min));

        if (dist  < b) {
            min = j;
        }

        if (!distIsPositive) {
            return ticks.at(min);
        }
    }
    return ticks.last();
}

void MainWindow::quantizeNtoleDialog() {

    if (!_file || Selection::instance()->selectedEvents().isEmpty()) {
        return;
    }


    NToleQuantizationDialog *d = new NToleQuantizationDialog(this);
    d->setModal(true);
    if (d->exec()) {
        quantizeNtole();
    }
}

void MainWindow::quantizeNtole() {

    if (!_file || Selection::instance()->selectedEvents().isEmpty()) {
        return;
    }

    // get list with all quantization ticks
    QList<int> ticks = _file->quantization(_settings.quantization);

    _file->protocol()->startNewAction("Quantify tuplet", new QImage(":/quantize.png"));

    // find minimum starting time
    bool startTickFound = false;
    int startTick = 0;
    for (MidiEvent *e : Selection::instance()->selectedEvents()) {
        int onTime = e->midiTime();
        if ((!startTickFound) || (onTime < startTick)) {
            startTickFound = true;
            startTick = onTime;
        }
    }

    // quantize start tick
    startTick = quantize(startTick, ticks);

    // compute new quantization grid
    QList<int> ntoleTicks;
    int ticksDuration = (NToleQuantizationDialog::replaceNumNum * _file->ticksPerQuarter() * 4)/(qPow(2, NToleQuantizationDialog::replaceDenomNum));
    int fractionSize = ticksDuration/NToleQuantizationDialog::ntoleNNum;

    for (ubyte i = 0; i <= NToleQuantizationDialog::ntoleNNum; i++) {
        ntoleTicks.append(startTick+i*fractionSize);
    }

    // quantize
    for (MidiEvent *e : Selection::instance()->selectedEvents()) {
        int onTime = e->midiTime();
        e->setMidiTime(quantize(onTime, ntoleTicks));
        OnEvent *on = protocol_cast<OnEvent*>(e);
        if (on) {
            MidiEvent *off = on->offEvent();
            off->setMidiTime(quantize(off->midiTime(), ntoleTicks));
            if (off->midiTime() == on->midiTime()) {
                int idx = ntoleTicks.indexOf(off->midiTime());
                if ((idx >= 0) && (ntoleTicks.size() > idx + 1)) {
                    off->setMidiTime(ntoleTicks.at(idx+1));
                } else if ((ntoleTicks.size() == idx + 1)) {
                    on->setMidiTime(ntoleTicks.at(idx - 1));
                }
            }
        }
    }
    _file->protocol()->endAction();
}

void MainWindow::setSpeed(QAction *action) {
    qreal d = action->data().toReal();
    MidiPlayer::instance()->setSpeedScale(d);
}

void MainWindow::manual() {

    QProcess *process = new QProcess;
    process->setWorkingDirectory("assistant");
    QStringList args;
    args << "-collectionFile"
    << "midieditor-collection.qhc";

#ifdef Q_OS_WIN32
    process->start("assistant/assistant", args);
#else
    process->start("assistant", args);
#endif

    if (!process->waitForStarted())
    return;

}

void MainWindow::spreadSelection() {

    if (!_file) {
        return;
    }

    bool ok;
    qreal numMs = QInputDialog::getDouble(this, "Set spread-time",
        "Spread time [ms]", 10, 5,500, 2, &ok);

    if (!ok) {
        numMs = 1;
    }

    QMultiMap<ubyte, int> spreadChannel[19];
    QMultiMap<ubyte, int>::const_iterator it;
    for (MidiEvent *event : Selection::instance()->selectedEvents()) {
        ubyte c = event->channel();
        if (std::find(spreadChannel[c].cbegin(), spreadChannel[c].cend(), event->midiTime()) != spreadChannel[c].cend()) {
            spreadChannel[c].insert(event->line(), event->midiTime());
        }
    }

    _file->protocol()->startNewAction("Spread events");
    int numSpreads = 0;
    QList<int> seenBefore;
    QList<MidiEvent*> events;

    for (ubyte i = 0; i < 19; i++) {

        MidiChannel *channel = _file->channel(i);
        seenBefore.clear();
        auto *eventsWithAllLines = channel->eventMap();

        it = spreadChannel[i].constBegin();

        for (; it != spreadChannel[i].constEnd(); ++it) {
            int line = it.key();
            if (seenBefore.contains(line)) {
                continue;
            }

            seenBefore.append(line);

            //foreach(int position, spreadChannel[i].values(line)) {

                QMultiMap<int, MidiEvent*>::const_iterator it2 = eventsWithAllLines->lowerBound(it.value());
                QMultiMap<int, MidiEvent*>::const_iterator upperBound = eventsWithAllLines->upperBound(it.value());

                for (; it2 !=upperBound; ++it2) {
                    if (it2.value()->line() == line) {
                        events.append(it2.value());
                    }
                }

                //spread events for the channel at the given position
                int num = events.count();
                if (num > 1) {

                    qreal timeToInsert = _file->msOfTick(it.value()) + numMs * num / 2.0;


                    for (int y = 0; y < num; y++) {

                        MidiEvent *toMove = events.at(y);

                        toMove->setMidiTime(_file->tick(qFloor(timeToInsert)), true);
                        numSpreads++;

                        timeToInsert -= numMs;
                    }
                }
            //}
        }
    }
    _file->protocol()->endAction();

    QMessageBox::information(this, "Spreading done", _("Spreaded %1 events").arg(numSpreads));
}

void MainWindow::toolChanged() {
    checkEnableActionsForSelection();
    _miscWidget->update();
    mw_matrixWidget->update();
}

void MainWindow::copiedEventsChanged() {
    bool enable = !EventTool::copiedEvents->isEmpty();
    _pasteAction->setEnabled(enable);
    pasteActionTB->setEnabled(enable);
}

void MainWindow::updateDetected(Update *update) {
    UpdateDialog *d = new UpdateDialog(update, this);
    d->setModal(true);
    d->exec();
}
