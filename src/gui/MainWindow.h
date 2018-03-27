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

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include <QScrollBar>
#include <QCloseEvent>
#include <QSettings>
#include <QToolBar>
#include "../Utils.h"

class MatrixWidget;
class TimelineWidget;
class PianoWidget;
class MidiEvent;
class MidiFile;
class ChannelListWidget;
class ProtocolWidget;
class EventWidget;
class ClickButton;
class QStringList;
class QTabWidget;
class QMenu;
class TrackListWidget;
class QComboBox;
#ifdef ENABLE_REMOTE
class RemoteServer;
#endif
class MiscWidget;
class QGridLayout;
class MidiTrack;
class QScrollArea;
class QShowEvent;
class Update;

class MainWindow : public QMainWindow {

    Q_OBJECT

    public:

        MainWindow(const QString &initFile = QString(), QWidget *parent = qnullptr,
                   Qt::WindowFlags flags = Qt::WindowFlags());
        void setFile(MidiFile *f);
        EventWidget *eventWidget();
        void setStartDir(const QString &dir);
        void setInitFile(const char * _file);
        static MainWindow *_mainWindow;
        static MainWindow *getMainWindow();
        MidiFile *file();

        /**
         * Make setupActions a little less scary by reducing code duplication.
         *
         * Usage:
         * QAction *action = createAction("Name" &MainWindow::someSlot, ":/icon.png", QKeySequence::Something);
         *
         * would equal:
         *
         * QAction *action = new QAction("Name", this);
         * action->setIcon(QIcon(":/icon.png"));
         * action->setKeySequence(QKeySequence::Something);
         * connect(action, &QAction::triggered, this, &MainWindow::someSlot);
         */
        template<typename FuncPointer>
        QAction *createAction(const QString &name, FuncPointer func, const QString &iconPath = "",
                             const QKeySequence &shortcut = QKeySequence());

    protected:
        void dropEvent(QDropEvent *ev) qoverride;
        void dragEnterEvent(QDragEnterEvent *ev) qoverride;

    public slots:
        void initUI();
        void loadInitFile();
       // void matrixSizeChanged(int maxScrollTime, double maxScrollLine, int vX, double vY);
        void play();
        void playStop();
        void stop();
        void stop(bool autoConfirmRecord, bool addEvents = true, bool resetPause = true);
        void pause();
        void forward();
        void back();
        // Alert that the input and output ports are ready.
        // This lets us load the UI without having to wait as long.
        void ioReady(bool isInput);
        void backToBegin();
        void load();
        void loadFile(const QString &_file);
        void openFile(const QString &filePath);
        void save();
        void saveas();
        void undo();
        void redo();
        void muteAllChannels();
        void unmuteAllChannels();
        void allChannelsVisible();
        void allChannelsInvisible();
        void muteAllTracks();
        void unmuteAllTracks();
        void allTracksVisible();
        void allTracksInvisible();
        void donate();
        void about();
        void setFileLengthMs();
        void scrollPositionsChanged(int x, int y);
        void record();
        void newFile();
        void panic();
        void screenLockPressed(bool enable);
        void scaleSelection();
        void alignLeft();
        void alignRight();
        void equalize();
        void deleteSelectedEvents();
        void deleteChannel(QAction *action);
        void moveSelectedEventsToChannel(QAction *action);
        void moveSelectedEventsToTrack(QAction *action);
        void updateRecentPathsList();
        void openRecent(QAction *action);
        void updateChannelMenu();
        void updateTrackMenu();
        void muteChannel(QAction *action);
        void soloChannel(QAction *action);
        void viewChannel(QAction *action);
        void instrumentChannel(QAction *action);

        void renameTrackMenuClicked(QAction *action);
        void removeTrackMenuClicked(QAction *action);
        void showEventWidget(bool show);
        void showTrackMenuClicked(QAction *action);
        void muteTrackMenuClicked(QAction *action);

        void renameTrack(ushort tracknumber);
        void removeTrack(ushort tracknumber);

        void setInstrumentForChannel(ubyte i);
        void spreadSelection();
        void copy();
        void paste();

        void addTrack();

        void selectAll();

        void transposeNSemitones();

        void markEdited(bool modified);

        void colorsByChannel();
        void colorsByTrack();

        void editChannel(int i);
        void editTrack(int i);
        void editChannel(ubyte i, bool assign);
        void editTrack(ushort i, bool assign);
        void editTrackAndChannel(MidiTrack *track);

        void manual();

        void changeMiscMode(int mode);
        void selectModeChanged(QAction *action);

        void pasteToChannel(QAction *action);
        void pasteToTrack(QAction *action);

        void selectAllFromChannel(QAction *action);
        void selectAllFromTrack(QAction *action);

        void divChanged(QAction* action);
        void quantizationChanged(QAction*);

        void enableMagnet(bool enable);

        void openConfig();

        void enableMetronome(bool enable);
        void enableThru(bool enable);

        void quantizeSelection();
        void quantizeNtoleDialog();
        void quantizeNtole();

        void setSpeed(QAction*);

        void checkEnableActionsForSelection();
        void toolChanged();
        void copiedEventsChanged();

        void updateDetected(Update *update);
#ifdef ENABLE_GBA
        void showMidFixDialog();
#endif
        void crash();
    protected:
        void closeEvent(QCloseEvent *event) qoverride;
        void keyPressEvent(QKeyEvent* e) qoverride;
        void keyReleaseEvent(QKeyEvent *event) qoverride;

    private:
        QToolBar *setupActions(QWidget *parent);
        int quantize(int t, QList<int> ticks);
        bool saveDialog();

        QList<QAction*> _activateWithSelections;

        MatrixWidget *mw_matrixWidget;
        TimelineWidget *mw_timelineWidget;
        PianoWidget *mw_pianoWidget;
        QScrollBar *vert, *hori;
        QScrollArea *matrixArea, *timelineArea, *pianoArea, *miscArea;
        ChannelListWidget *channelWidget;
        ProtocolWidget *protocolWidget;
        TrackListWidget *_trackWidget;
        MidiFile *_file;
        QString startDirectory, _initFile;
        EventWidget *_eventWidget;
        QStringList _recentFilePaths;
        QMenu *_recentPathsMenu, *_deleteChannelMenu,
            *_moveSelectedEventsToTrackMenu, *_moveSelectedEventsToChannelMenu,
            *_pasteToTrackMenu, *_pasteToChannelMenu, *_selectAllFromTrackMenu, *_selectAllFromChannelMenu;

        QTabWidget *lowerTabWidget;
        QAction *_colorsByChannel, *_colorsByTracks;

        QComboBox *_chooseEditTrack, *_chooseEditChannel;

#ifdef ENABLE_REMOTE
        RemoteServer *_remoteServer;
#endif

        QWidget *_miscWidgetControl;
        QGridLayout *_miscControlLayout;

        QComboBox *_miscMode, *_miscController, *_miscChannel;
        QAction *setSingleMode, *setLineMode, *setFreehandMode, *_allChannelsVisible, *_allChannelsInvisible, *_allTracksAudible, *_allTracksMute,
            *_allChannelsAudible, *_allChannelsMute, *_allTracksVisible, *_allTracksInvisible, *stdToolAction, *undoAction, *redoAction, *_pasteAction, *pasteActionTB;
        MiscWidget *_miscWidget;

        #ifdef NO_BIT_PACK
            bool inputIsReady;
            bool outputIsReady;
        #else
            bool inputIsReady : 1;
            bool outputIsReady : 1;
        #endif


};

#endif
