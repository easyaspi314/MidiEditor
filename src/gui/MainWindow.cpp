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

#include <QGridLayout>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QSplitter>
#include <QMessageBox>
#include <QTabWidget>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QIcon>
#include <QStringList>
#include <QComboBox>
#include <QToolButton>
#include <QLabel>
#include <QTextStream>
#include <QMultiMap>
#include <QSettings>
#include <QInputDialog>
#include <QTextEdit>
#include <QToolBar>
#include <QScrollArea>
#include <QMimeData>

#include "DonateDialog.h"
#include "AboutDialog.h"
#include "FileLengthDialog.h"
#include "RecordDialog.h"
#include "MiscWidget.h"
#include "TransposeDialog.h"
#include "MatrixWidget.h"
#include "ChannelListWidget.h"
#include "TrackListWidget.h"
#include "EventWidget.h"
#include "ProtocolWidget.h"
#include "ClickButton.h"
#include "InstrumentChooser.h"
#include "SettingsDialog.h"
#include "NToleQuantizationDialog.h"
#include "TimelineWidget.h"
#include "PianoWidget.h"

#include "../tool/Tool.h"
#include "../tool/SelectTool.h"
#include "../tool/SizeChangeTool.h"
#include "../tool/EraserTool.h"
#include "../tool/StandardTool.h"
#include "../tool/NewNoteTool.h"
#include "../tool/EventMoveTool.h"
#include "../tool/EventTool.h"
#include "../tool/ToolButton.h"
#include "../tool/Selection.h"

#include "../protocol/Protocol.h"
#include "../Terminal.h"

#include "../gba/MidFix4AgbDialog.h"

#ifdef ENABLE_REMOTE
#include "../remote/RemoteServer.h"
#endif

#include "../midi/MidiFile.h"
#include "../midi/MidiTrack.h"
#include "../midi/MidiPlayer.h"
#include "../midi/PlayerThread.h"
#include "../midi/MidiChannel.h"
#include "../midi/MidiOutput.h"
#include "../midi/MidiInput.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../MidiEvent/OnEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../midi/Metronome.h"

#include "../UpdateManager.h"
#include "UpdateDialog.h"

#include "../gba/SappyHelper.h"
#include <QtCore/qmath.h>
#include "../Utils.h"
#include <QTime>

// Static instance
MainWindow * MainWindow::_mainWindow;

/*
 * Note: This class is split into two cpp files, MainWindow.cpp and InterfaceController.cpp.
 *
 * MainWindow.cpp controls the basic setup, while InterfaceController.cpp has all the masses of slots
 * and signals.
 */
MainWindow::MainWindow(const QString &initFile, QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
    _mainWindow = this;
    QTime time;

    _initFile = initFile;

    inputIsReady = false;
    outputIsReady = false;
    // file may be set already.
    if (file)
        file = qnullptr;
    QSettings settings("MidiEditor", "NONE");

    _moveSelectedEventsToChannelMenu = qnullptr;
    _moveSelectedEventsToTrackMenu = qnullptr;

#ifdef ENABLE_REMOTE
    _settings.client_port = settings.value("udp_client_port", -1).toInt();
    _settings.client_ip = settings.value("udp_client_ip", QString()).toString();
#endif
    _settings.alt_stop = settings.value("alt_stop", false).toBool();
    _settings.screen_locked = settings.value("screen_locked", false).toBool();
    _settings.div = settings.value("div", 2).toUInt() & 0x7;
    _settings.antialiasing = settings.value("antialiasing", true).toBool();
    if (!QApplication::arguments().contains("--no-antialiasing")) {
        _settings.antialiasing = false;
    }
    _settings.velocityDragging = settings.value("velocity_dragging", false).toBool();

    _settings.gba_mode = settings.value("gba_mode", false).toBool();
    _settings.playbackDelay = settings.value("playback_delay", 0).toUInt() & 0x7FF;
    _settings.select_and_move = settings.value("select_and_move", true).toBool();
    _settings.ticks_per_quarter = settings.value("ticks_per_quarter", 192).toUInt() & 0x7FFF;
    _settings.magnet = settings.value("magnet", false).toBool();
    _settings.metronome = settings.value("metronome", false).toBool();
    _settings.thru = settings.value("thru", false).toBool();
    uint numStarts = settings.value("numStart", 0).toUInt();
    // Reset this to 15.
    if (numStarts > 15) {
        settings.setValue("numStart", 15);
        numStarts = 15;
    }
    _settings.numStarts = uint(numStarts) & 0xF;
    Metronome::setEnabled(_settings.metronome);

#ifdef ENABLE_REMOTE
     _remoteServer = new RemoteServer();
     _remoteServer->setIp(_settings.client_ip);
     _remoteServer->setPort(_settings.client_port);
     _remoteServer->tryConnect();

    connect(_remoteServer, &RemoteServer::playRequest, this, &MainWindow::play);
    connect(_remoteServer, &RemoteServer::stopRequest, this, &MainWindow::stop);
    connect(_remoteServer, &RemoteServer::recordRequest, this, &MainWindow::record);
    connect(_remoteServer, &RemoteServer::backRequest, this, &MainWindow::back);
    connect(_remoteServer, &RemoteServer::forwardRequest, this, &MainWindow::forward);
    connect(_remoteServer, &RemoteServer::pauseRequest, this, &MainWindow::pause);

    connect(MidiPlayer::player(), &PlayerThread::timeMsChanged, _remoteServer, &RemoteServer::setTime);
    connect(MidiPlayer::player(), &PlayerThread::meterChanged, _remoteServer, &RemoteServer::setMeter);
    connect(MidiPlayer::player(), &PlayerThread::tonalityChanged, _remoteServer, &RemoteServer::setTonality);
    connect(MidiPlayer::player(), &PlayerThread::measureChanged, _remoteServer, &RemoteServer::setMeasure);

#endif

    _settings.channel_colors = settings.value("colors_from_channel", false).toBool();
    _settings.auto_update = settings.value("auto_update", true).toBool();
    connect(UpdateManager::instance(), &UpdateManager::updateDetected, this, &MainWindow::updateDetected);
    _settings.quantization = settings.value("quantization", 3).toUInt() & 0x7;


/*
    startDirectory = QDir::homePath();

    if (settings.value("open_path", QDir::homePath()).toString()!="") {
        startDirectory = settings.value("open_path").toString();
    } else {
        _qsettings->setValue("open_path", startDirectory);
    }
*/
    startDirectory = settings.value("open_path", QDir::homePath()).toString();
    // read recent paths
    _recentFilePaths = settings.value("recent_file_list").toStringList();
    //qWarning() << _settings;
    EditorTool::setMainWindow(this);

    setWindowTitle(QApplication::applicationName());
    setWindowIcon(QIcon(":/run_environment/graphics/icon.png"));



    setAcceptDrops(true);
    QTimer::singleShot(0, this, &MainWindow::initUI);
    QTimer::singleShot(250, this, &MainWindow::loadInitFile);

    if (_settings.auto_update) {
        QTimer::singleShot(500, UpdateManager::instance(), &UpdateManager::checkForUpdates);
    }
}

void MainWindow::initUI() {
    QWidget *central = new QWidget(this);
    QGridLayout *centralLayout = new QGridLayout(central);
    centralLayout->setContentsMargins(3,3,3,5);

    // there is a vertical split
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, central);
    //mainSplitter->setHandleWidth(0);

    // The left side
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, mainSplitter);
    leftSplitter->setHandleWidth(0);
    mainSplitter->addWidget(leftSplitter);
    leftSplitter->setContentsMargins(0,0,0,0);

    // The right side
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, mainSplitter);
    //rightSplitter->setHandleWidth(0);
    mainSplitter->addWidget(rightSplitter);

    // Set the sizes of mainSplitter
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 0);
    mainSplitter->setContentsMargins(0,0,0,0);

    // the channelWidget and the trackWidget are tabbed
    QTabWidget *upperTabWidget = new QTabWidget(rightSplitter);
    rightSplitter->addWidget(upperTabWidget);
    rightSplitter->setContentsMargins(0,0,0,0);

    // protocolList and EventWidget are tabbed
    lowerTabWidget = new QTabWidget(rightSplitter);
    rightSplitter->addWidget(lowerTabWidget);

    // MatrixArea
    QWidget *matrixAreaContainer = new QWidget(leftSplitter);
    QGridLayout *matrixAreaLayout = new QGridLayout(matrixAreaContainer);
    matrixAreaLayout->setSpacing(0);
    matrixArea = new QScrollArea(matrixAreaContainer);

    matrixArea->setContentsMargins(0,0,0,0);
    matrixArea->setWidgetResizable(true);

    timelineArea = new QScrollArea(matrixAreaContainer);
    timelineArea->setFixedHeight(50);
    timelineArea->setWidgetResizable(true);
    timelineArea->setContentsMargins(0, 0, 0, 0);
    // insist that the timeline scrollbar begone
    QScrollBar *timelineScrollBar = timelineArea->horizontalScrollBar();
    timelineArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    timelineArea->verticalScrollBar()->hide();
    timelineScrollBar->hide();
    timelineScrollBar->setStyleSheet("QScrollBar{height:0px}");

    pianoArea = new QScrollArea(matrixAreaContainer);
    pianoArea->setContentsMargins(0, 0, 0, 0);
    pianoArea->setWidgetResizable(true);
    pianoArea->setFixedWidth(110);
    pianoArea->setMinimumHeight(1);
    pianoArea->setBackgroundRole(QPalette::Dark);
    // scrollbar begone
    pianoArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *pianoScrollBar = pianoArea->verticalScrollBar();
    pianoScrollBar->hide();
    pianoArea->horizontalScrollBar()->hide();
    pianoScrollBar->setStyleSheet("QScrollBar{width:0px}");

    mw_matrixWidget = new MatrixWidget(matrixArea);
    mw_matrixWidget->setMinimumSize(QSize(150,150));

    mw_timelineWidget = new TimelineWidget(timelineArea);
    mw_timelineWidget->setFixedHeight(50);
    mw_timelineWidget->setMinimumWidth(50);

    mw_pianoWidget = new PianoWidget(pianoArea);
    mw_pianoWidget->setFixedWidth(110);
    mw_pianoWidget->setMinimumHeight(1);

    matrixAreaLayout->setContentsMargins(0,0,0,0);
    matrixAreaLayout->addWidget(matrixArea, 1, 1, 2, 2);
    matrixAreaLayout->addWidget(timelineArea, 0, 1, 1, 2);
    matrixAreaLayout->addWidget(pianoArea, 1, 0, 2, 1);

    matrixAreaLayout->setColumnStretch(0, 1);
    matrixAreaContainer->setLayout(matrixAreaLayout);
    matrixArea->setWidget(mw_matrixWidget);


    timelineArea->setWidget(mw_timelineWidget);
    pianoArea->setWidget(mw_pianoWidget);

    matrixArea->setFrameShape(QFrame::NoFrame);
    pianoArea->setFrameShape(QFrame::NoFrame);
    timelineArea->setFrameShape(QFrame::NoFrame);

    vert = matrixArea->verticalScrollBar();
    hori = matrixArea->horizontalScrollBar();

    leftSplitter->addWidget(matrixAreaContainer);

    mw_pianoWidget->setMatrixWidget(mw_matrixWidget);
    mw_timelineWidget->setMatrixWidget(mw_matrixWidget);

    mw_matrixWidget->setScreenLocked(_settings.screen_locked);
    mw_matrixWidget->setDiv(_settings.div);

    // VelocityArea
    QWidget *velocityArea = new QWidget(leftSplitter);
    velocityArea->setContentsMargins(0,0,0,0);
    leftSplitter->addWidget(velocityArea);

    QGridLayout *velocityAreaLayout = new QGridLayout(velocityArea);
    velocityAreaLayout->setContentsMargins(0,0,0,0);
    velocityAreaLayout->setHorizontalSpacing(6);
    _miscWidgetControl = new QWidget(velocityArea);
    _miscWidgetControl->setFixedWidth(110-velocityAreaLayout->horizontalSpacing());

    velocityAreaLayout->addWidget(_miscWidgetControl, 0,0,1,1);
    // there is a Scrollbar on the right side of the velocityWidget doing
    // nothing but making the VelocityWidget as big as the matrixWidget

    velocityAreaLayout->setRowStretch(0, 1);
    velocityArea->setLayout(velocityAreaLayout);
    miscArea = new QScrollArea(velocityArea);
    miscArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    miscArea->setWidgetResizable(true);
    miscArea->setContentsMargins(0, 0, 0, 0);
    miscArea->setMinimumWidth(100);
    miscArea->setFrameShape(QFrame::NoFrame);
    velocityAreaLayout->addWidget(miscArea, 0, 1, 1, 1);

    _miscWidget = new MiscWidget(mw_matrixWidget, miscArea);
    _miscWidget->setContentsMargins(0,0,0,0);
    miscArea->setWidget(_miscWidget);

    QScrollBar *miscScrollBar = miscArea->horizontalScrollBar();
    miscScrollBar->hide();
    miscScrollBar->setStyleSheet("QScrollBar{height:0px}");

    // Sync the scroll bars.
    // Until we can make a magical QScrollArea that will show what we want,
    // this is what we are stuck with.
    connect(hori, &QScrollBar::valueChanged, timelineScrollBar, &QScrollBar::setValue);
    connect(timelineScrollBar, &QScrollBar::valueChanged, hori, &QScrollBar::setValue);
    connect(vert, &QScrollBar::valueChanged, pianoScrollBar, &QScrollBar::setValue);
    connect(pianoScrollBar, &QScrollBar::valueChanged, vert, &QScrollBar::setValue);
    connect(hori, &QScrollBar::valueChanged, miscScrollBar, &QScrollBar::setValue);
    connect(miscScrollBar, &QScrollBar::valueChanged, hori, &QScrollBar::setValue);


    // controls for velocity widget
    _miscControlLayout = new QGridLayout(_miscWidgetControl);
    _miscControlLayout->setHorizontalSpacing(0);

    _miscWidgetControl->setLayout(_miscControlLayout);
    _miscMode = new QComboBox(_miscWidgetControl);

    for (ubyte i = 0; i <= 5; i++) {
        _miscMode->addItem(MiscWidget::modeToString(i));
    }
    _miscControlLayout->addWidget(_miscMode, 1, 0, 1, 3);
    connect(_miscMode, SIGNAL_OL(QComboBox, currentIndexChanged, int), this, &MainWindow::changeMiscMode);

    //_miscControlLayout->addWidget(new QLabel("Control:", _miscWidgetControl), 2, 0, 1, 3);
    _miscController = new QComboBox(_miscWidgetControl);
    for (ubyte i = 0; i < 128; i++) {
        _miscController->addItem(MidiFile::controlChangeName(i));
    }
    _miscControlLayout->addWidget(_miscController, 3, 0, 1, 3);
    connect(_miscController, SIGNAL_OL(QComboBox, currentIndexChanged, int), _miscWidget, &MiscWidget::setControl);

    _miscChannel = new QComboBox(_miscWidgetControl);
    for (ubyte i = 0; i < 15; i++) {
        _miscChannel->addItem(_("Channel %1").arg(i));
    }
    _miscControlLayout->addWidget(_miscChannel, 5, 0, 1, 3);
    connect(_miscChannel, SIGNAL_OL(QComboBox, currentIndexChanged, int), _miscWidget, &MiscWidget::setChannel);
    _miscControlLayout->setRowStretch(6, 1);
    _miscMode->setCurrentIndex(0);
    _miscChannel->setEnabled(false);
    _miscController->setEnabled(false);

    setSingleMode = new QAction(QIcon(":/run_environment/graphics/tool/misc_single.png"), "Single mode", this);
    setSingleMode->setCheckable(true);
    setFreehandMode = new QAction(QIcon(":/run_environment/graphics/tool/misc_freehand.png"), "Free-hand mode", this);
    setFreehandMode->setCheckable(true);
    setLineMode = new QAction(QIcon(":/run_environment/graphics/tool/misc_line.png"), "Line mode", this);
    setLineMode->setCheckable(true);

    QActionGroup *group = new QActionGroup(this);
    group->setExclusive(true);
    group->addAction(setSingleMode);
    group->addAction(setFreehandMode);
    group->addAction(setLineMode);
    setSingleMode->setChecked(true);
    connect(group, &QActionGroup::triggered, this, &MainWindow::selectModeChanged);

    QToolButton *btnSingle = new QToolButton(_miscWidgetControl);
    btnSingle->setDefaultAction(setSingleMode);
    QToolButton *btnHand = new QToolButton(_miscWidgetControl);
    btnHand->setDefaultAction(setFreehandMode);
    QToolButton *btnLine = new QToolButton(_miscWidgetControl);
    btnLine->setDefaultAction(setLineMode);

    _miscControlLayout->addWidget(btnSingle, 9, 0, 1, 1);
    _miscControlLayout->addWidget(btnHand, 9, 1, 1, 1);
    _miscControlLayout->addWidget(btnLine, 9, 2, 1, 1);

    // Set the sizes of leftSplitter
    leftSplitter->setStretchFactor(0, 8);
    leftSplitter->setStretchFactor(1, 1);

    // Track
    QWidget *tracks = new QWidget(upperTabWidget);
    QGridLayout *tracksLayout = new QGridLayout(tracks);
    tracks->setLayout(tracksLayout);
    QToolBar *tracksTB = new QToolBar(tracks);
    tracksTB->setIconSize(QSize(20, 20));
    // macOS hack to remove gradient
    tracksTB->setStyleSheet("QToolBar{border:none}");

    tracksLayout->addWidget(tracksTB, 0, 0, 1, 1);

    QAction *newTrack = new QAction("Add track", this);
    newTrack->setIcon(QIcon(":/run_environment/graphics/tool/add.png"));
    connect(newTrack, &QAction::triggered, this, &MainWindow::addTrack);
    tracksTB->addAction(newTrack);

    tracksTB->addSeparator();

    _allTracksAudible = new QAction("All tracks audible", this);
    _allTracksAudible->setIcon(QIcon(":/run_environment/graphics/tool/all_audible.png"));
    connect(_allTracksAudible, &QAction::triggered, this, &MainWindow::unmuteAllTracks);
    tracksTB->addAction(_allTracksAudible);

    _allTracksMute = new QAction("Mute all tracks", this);
    _allTracksMute->setIcon(QIcon(":/run_environment/graphics/tool/all_mute.png"));
    connect(_allTracksMute, &QAction::triggered, this, &MainWindow::muteAllTracks);
    tracksTB->addAction(_allTracksMute);

    tracksTB->addSeparator();

    _allTracksVisible = new QAction("Show all tracks", this);
    _allTracksVisible->setIcon(QIcon(":/run_environment/graphics/tool/all_visible.png"));
    connect(_allTracksVisible, &QAction::triggered, this, &MainWindow::allTracksVisible);
    tracksTB->addAction(_allTracksVisible);

    _allTracksInvisible = new QAction("Hide all tracks", this);
    _allTracksInvisible->setIcon(QIcon(":/run_environment/graphics/tool/all_invisible.png"));
    connect(_allTracksInvisible, &QAction::triggered, this, &MainWindow::allTracksInvisible);
    tracksTB->addAction(_allTracksInvisible);

    _trackWidget = new TrackListWidget(tracks);
    connect(_trackWidget, &TrackListWidget::trackRenameClicked, this, &MainWindow::renameTrack,
            Qt::QueuedConnection);
    connect(_trackWidget, &TrackListWidget::trackRemoveClicked, this, &MainWindow::removeTrack,
            Qt::QueuedConnection);
    connect(_trackWidget, &TrackListWidget::trackClicked, this, &MainWindow::editTrackAndChannel,
            Qt::QueuedConnection);

    tracksLayout->addWidget(_trackWidget, 1, 0, 1, 1);
    upperTabWidget->addTab(tracks, "Tracks");

    // Channels
    QWidget *channels = new QWidget(upperTabWidget);
    QGridLayout *channelsLayout = new QGridLayout(channels);
    channels->setLayout(channelsLayout);
    QToolBar *channelsTB = new QToolBar(channels);
    channelsTB->setIconSize(QSize(20, 20));
    channelsTB->setStyleSheet("QToolBar { border: none; }");
    channelsLayout->addWidget(channelsTB, 0, 0, 1, 1);

    _allChannelsAudible = new QAction("All channels audible", this);
    _allChannelsAudible->setIcon(QIcon(":/run_environment/graphics/tool/all_audible.png"));
    connect(_allChannelsAudible, &QAction::triggered, this, &MainWindow::unmuteAllChannels);
    channelsTB->addAction(_allChannelsAudible);

    _allChannelsMute = new QAction("Mute all channels", this);
    _allChannelsMute->setIcon(QIcon(":/run_environment/graphics/tool/all_mute.png"));
    connect(_allChannelsMute, &QAction::triggered, this, &MainWindow::muteAllChannels);
    channelsTB->addAction(_allChannelsMute);

    channelsTB->addSeparator();

    _allChannelsVisible = new QAction("Show all channels", this);
    _allChannelsVisible->setIcon(QIcon(":/run_environment/graphics/tool/all_visible.png"));
    connect(_allChannelsVisible, &QAction::triggered, this, &MainWindow::allChannelsVisible);
    channelsTB->addAction(_allChannelsVisible);

    _allChannelsInvisible = new QAction("Hide all channels", this);
    _allChannelsInvisible->setIcon(QIcon(":/run_environment/graphics/tool/all_invisible.png"));
    connect(_allChannelsInvisible, &QAction::triggered, this, &MainWindow::allChannelsInvisible);
    channelsTB->addAction(_allChannelsInvisible);

    channelWidget = new ChannelListWidget(channels);
    connect(channelWidget, &ChannelListWidget::channelStateChanged, this,
            &MainWindow::updateChannelMenu, Qt::QueuedConnection);
    connect(channelWidget, &ChannelListWidget::selectInstrumentClicked, this,
            &MainWindow::setInstrumentForChannel, Qt::QueuedConnection);
    channelsLayout->addWidget(channelWidget, 1, 0, 1, 1);
    upperTabWidget->addTab(channels, "Channels");

    // Protocollist
    protocolWidget = new ProtocolWidget(lowerTabWidget);
    lowerTabWidget->addTab(protocolWidget, "Protocol");

    // EventWidget
    _eventWidget = new EventWidget(lowerTabWidget);
    Selection::_eventWidget = _eventWidget;
    lowerTabWidget->addTab(_eventWidget, "Event");
    MidiEvent::setEventWidget(_eventWidget);
    connect(_eventWidget, &EventWidget::selectionChangedByTool, this, &MainWindow::showEventWidget);

    // below add two rows for choosing track/channel new events shall be assigned to
    QWidget *chooser = new QWidget(rightSplitter);
    chooser->setMinimumWidth(350);
    rightSplitter->addWidget(chooser);
    QGridLayout *chooserLayout = new QGridLayout(chooser);
    QLabel *trackchannelLabel = new QLabel("Add new events to…");
    chooserLayout->addWidget(trackchannelLabel, 0,0,1, 2);
    QLabel *channelLabel = new QLabel("Channel: ", chooser);
    chooserLayout->addWidget(channelLabel, 2, 0, 1, 1);
    _chooseEditChannel = new QComboBox(chooser);
    for (ubyte i = 0; i < 16;i++) {
        _chooseEditChannel->addItem(_("Channel %1").arg(i));
    }
    connect(_chooseEditChannel, SIGNAL_OL(QComboBox, activated, int),
            this, SIGNAL_OL(MainWindow,editChannel,int));

    chooserLayout->addWidget(_chooseEditChannel, 2, 1, 1, 1);
    QLabel *trackLabel = new QLabel("Track:", chooser);
    chooserLayout->addWidget(trackLabel, 1, 0, 1, 1);
    _chooseEditTrack = new QComboBox(chooser);
    chooserLayout->addWidget(_chooseEditTrack, 1, 1, 1, 1);
    connect(_chooseEditTrack, SIGNAL_OL(QComboBox, activated, int), this, SIGNAL_OL(MainWindow,editTrack,int));
    chooserLayout->setColumnStretch(1, 1);

    connect(channelWidget, &ChannelListWidget::channelStateChanged,
            mw_matrixWidget, SIGNAL_OL(MatrixWidget, update, void) /* totally a signal :P */);
   // connect(mw_matrixWidget, &MatrixWidget::sizeChanged, this, &MainWindow::matrixSizeChanged);

    connect(mw_matrixWidget, &MatrixWidget::scrollChanged, this, &MainWindow::scrollPositionsChanged);

    setCentralWidget(central);

    QToolBar *buttons = setupActions(this);
    addToolBar(buttons);

    rightSplitter->setStretchFactor(0, 5);
    rightSplitter->setStretchFactor(1, 5);

    // Add the Widgets to the central Layout
    centralLayout->setSpacing(0);

    centralLayout->addWidget(mainSplitter,1,0);
    centralLayout->setRowStretch(1, 1);
    central->setLayout(centralLayout);

    if (_settings.channel_colors) {
        colorsByChannel();
    } else {
        colorsByTrack();
    }
    copiedEventsChanged();

}

void MainWindow::loadInitFile() {
    // macOS does things differently.
    if (file) {
        return;
    }
    if (!_initFile.isEmpty()) {
        loadFile(_initFile);
    } else {
        newFile();
    }
}

MainWindow * MainWindow::getMainWindow() {
    return _mainWindow;
}

void MainWindow::closeEvent(QCloseEvent *event) {

    if (!file || !file->modified()) {
        event->accept();
    } else {
        if (saveDialog()) {
            event->accept();
        } else {
            event->ignore();
            return;
        }

    }

    // stop any playback
    stop();

    QSettings settings("MidiEditor", "NONE");
    if (!MidiOutput::instance()->outputPort().isEmpty()) {
        settings.setValue("out_port", _settings.out_port);
    }
    if (!MidiInput::instance()->inputPort().isEmpty()) {
        settings.setValue("in_port", _settings.in_port);
    }
#ifdef ENABLE_REMOTE
    if (!_remoteServer->clientIp().isEmpty()) {
        settings.setValue("udp_client_ip", _settings.client_ip);
    }
    if (_remoteServer->clientPort() > 0) {
        settings.setValue("udp_client_port", _settings.client_port);
    }
    _remoteServer->stopServer();
#endif
    if (_settings.numStarts < 15) {
        settings.setValue("numStart", _settings.numStarts + 1);
    }

    // save the current Path
    settings.setValue("open_path", startDirectory);
    settings.setValue("alt_stop", _settings.alt_stop);
    settings.setValue("ticks_per_quarter", _settings.ticks_per_quarter);
    settings.setValue("screen_locked", _settings.screen_locked);
    settings.setValue("magnet", _settings.magnet);
    settings.setValue("gba_mode", _settings.gba_mode);
    settings.setValue("playback_delay", _settings.playbackDelay);
    // don't change the setting if the user manually overrode it
    if (!QApplication::arguments().contains("--no-antialiasing")) {
        settings.setValue("antialiasing", _settings.antialiasing);
    }
    settings.setValue("velocity_dragging", _settings.velocityDragging);
    settings.setValue("select_and_move", _settings.select_and_move);
    settings.setValue("div", _settings.div);
    settings.setValue("colors_from_channel", _settings.channel_colors);

    settings.setValue("metronome", _settings.metronome);
    settings.setValue("thru", _settings.thru);
    settings.setValue("quantization", _settings.quantization);

    settings.setValue("auto_update", _settings.auto_update);

}



void MainWindow::keyPressEvent(QKeyEvent *event) {
    mw_matrixWidget->takeKeyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    mw_matrixWidget->takeKeyReleaseEvent(event);
}

template<typename FuncPointer>
QAction *MainWindow::createAction(const QString &name, FuncPointer func, const QString &iconPath,
                                  const QKeySequence &shortcut) {
    QAction *action = new QAction(name, this);
    if (!iconPath.isEmpty())
        action->setIcon(QIcon(iconPath));
    if (!shortcut.isEmpty())
        action->setShortcut(shortcut);
    connect(action, &QAction::triggered, this, func);
    return action;
}

QToolBar *MainWindow::setupActions(QWidget *parent) {

    // Menubar
    QMenu *fileMB = menuBar()->addMenu("File");
    QMenu *editMB = menuBar()->addMenu("Edit");
    QMenu *toolsMB = menuBar()->addMenu("Tools");
    QMenu *viewMB = menuBar()->addMenu("View");
    QMenu *playbackMB = menuBar()->addMenu("Playback");
    QMenu *midiMB = menuBar()->addMenu("Midi");
    QMenu *helpMB = menuBar()->addMenu("Help");

    // File
    /*QAction *newAction = new QAction("New", this);
    newAction->setShortcut(QKeySequence::New);
    newAction->setIcon(QIcon(":/run_environment/graphics/tool/new.png"));
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);*/
    QAction *newAction = createAction("New", &MainWindow::newFile,
                                      ":/run_environment/graphics/tool/new.png",
                                      QKeySequence::New);
    fileMB->addAction(newAction);

    QAction *loadAction = new QAction("Open…", this);
    loadAction->setShortcut(QKeySequence::Open);
    loadAction->setIcon(QIcon(":/run_environment/graphics/tool/load.png"));
    connect(loadAction, &QAction::triggered, this, &MainWindow::load);
    fileMB->addAction(loadAction);

    _recentPathsMenu = new QMenu("Open recent…", this);
    _recentPathsMenu->setIcon(QIcon(":/run_environment/graphics/tool/noicon.png"));
    fileMB->addMenu(_recentPathsMenu);
    connect(_recentPathsMenu, &QMenu::triggered, this, &MainWindow::openRecent);

    updateRecentPathsList();

    fileMB->addSeparator();

    QAction *saveAction = new QAction("Save", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setIcon(QIcon(":/run_environment/graphics/tool/save.png"));
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);
    fileMB->addAction(saveAction);

    QAction *saveAsAction = new QAction("Save as…", this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    saveAsAction->setIcon(QIcon(":/run_environment/graphics/tool/saveas.png"));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveas);
    fileMB->addAction(saveAsAction);

    fileMB->addSeparator();

    QAction *quitAction = new QAction("Quit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setIcon(QIcon(":/run_environment/graphics/tool/noicon.png"));
    connect(quitAction, &QAction::triggered, this, &MainWindow::close);
    fileMB->addAction(quitAction);

    // Edit
    undoAction = new QAction("Undo", this);
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/run_environment/graphics/tool/undo.png"));
    connect(undoAction, &QAction::triggered, this, &MainWindow::undo);
    editMB->addAction(undoAction);

    redoAction = new QAction("Redo", this);
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/run_environment/graphics/tool/redo.png"));
    connect(redoAction, &QAction::triggered, this, &MainWindow::redo);
    editMB->addAction(redoAction);

    editMB->addSeparator();

    QAction *selectAllAction = new QAction("Select all", this);
    selectAllAction->setToolTip("Select all visible events");
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, &MainWindow::selectAll);
    editMB->addAction(selectAllAction);

    _selectAllFromChannelMenu = new QMenu("Select all events from channel…", editMB);
    editMB->addMenu(_selectAllFromChannelMenu);
    connect(_selectAllFromChannelMenu, &QMenu::triggered, this, &MainWindow::selectAllFromChannel);

    _selectAllFromTrackMenu = new QMenu("Select all events from track…", editMB);
    editMB->addMenu(_selectAllFromTrackMenu);
    connect(_selectAllFromTrackMenu, &QMenu::triggered, this, &MainWindow::selectAllFromTrack);

    editMB->addSeparator();

    QAction *copyAction = new QAction("Copy events", this);
    _activateWithSelections.append(copyAction);
    copyAction->setIcon(QIcon(":/run_environment/graphics/tool/copy.png"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MainWindow::copy);
    editMB->addAction(copyAction);

    _pasteAction = new QAction("Paste events", this);
    _pasteAction->setToolTip("Paste events at cursor position");
    _pasteAction->setShortcut(QKeySequence::Paste);
    _pasteAction->setIcon(QIcon(":/run_environment/graphics/tool/paste.png"));
    connect(_pasteAction, &QAction::triggered, this, &MainWindow::paste);

    _pasteToTrackMenu = new QMenu("Paste to track…");
    _pasteToChannelMenu = new QMenu("Paste to channel…");
    QMenu *pasteOptionsMenu = new QMenu("Paste options…");
    pasteOptionsMenu->addMenu(_pasteToChannelMenu);
    QActionGroup *pasteChannelGroup = new QActionGroup(this);
    pasteChannelGroup->setExclusive(true);
    connect(_pasteToChannelMenu, &QMenu::triggered, this, &MainWindow::pasteToChannel);
    connect(_pasteToTrackMenu, &QMenu::triggered, this, &MainWindow::pasteToTrack);

    for (int i = -2; i < 16; i++) {
        QVariant variant(i);
        QString text = QString::number(i);
        if (i == -2) {
            text = "Same as selected for new events";
        }
        if (i == -1) {
            text = "Keep channel";
        }
        QAction *pasteToChannelAction = new QAction(text, this);
        pasteToChannelAction->setData(variant);
        pasteToChannelAction->setCheckable(true);
        _pasteToChannelMenu->addAction(pasteToChannelAction);
        pasteChannelGroup->addAction(pasteToChannelAction);
        pasteToChannelAction->setChecked(i < 0);
    }
    pasteOptionsMenu->addMenu(_pasteToTrackMenu);
    editMB->addAction(_pasteAction);
    editMB->addMenu(pasteOptionsMenu);

    editMB->addSeparator();

    QAction *configAction = new QAction("Settings…", this);
    configAction->setIcon(QIcon(":/run_environment/graphics/tool/config.png"));
    connect(configAction, &QAction::triggered, this, &MainWindow::openConfig);
    editMB->addAction(configAction);

    // Tools
    QMenu *toolsToolsMenu = new QMenu("Current tool…", toolsMB);

    StandardTool *tool = new StandardTool();
    Tool::setCurrentTool(tool);
    stdToolAction = new ToolButton(tool, QKeySequence(Qt::Key_F1), toolsToolsMenu);
    toolsToolsMenu->addAction(stdToolAction);
    tool->buttonClick();

    QAction *selectSingleAction = new ToolButton(new SelectTool(SelectionType::Single),
                                                 QKeySequence(Qt::Key_F2), toolsToolsMenu);
    toolsToolsMenu->addAction(selectSingleAction);
    QAction *selectBoxAction = new ToolButton(new SelectTool(SelectionType::Box),
                                              QKeySequence(Qt::Key_F3), toolsToolsMenu);
    toolsToolsMenu->addAction(selectBoxAction);
    QAction *selectLeftAction = new ToolButton(new SelectTool(SelectionType::Left),
                                               QKeySequence(Qt::Key_F4), toolsToolsMenu);
    toolsToolsMenu->addAction(selectLeftAction);
    QAction *selectRightAction = new ToolButton(new SelectTool(SelectionType::Right),
                                                QKeySequence(Qt::Key_F5), toolsToolsMenu);
    toolsToolsMenu->addAction(selectRightAction);

    toolsToolsMenu->addSeparator();
    QAction *moveAllAction = new ToolButton(new EventMoveTool(true, true),
                                            QKeySequence(Qt::Key_F6), toolsToolsMenu);
    _activateWithSelections.append(moveAllAction);
    toolsToolsMenu->addAction(moveAllAction);
    QAction *moveLRAction = new ToolButton(new EventMoveTool(false, true),
                                           QKeySequence(Qt::Key_F7), toolsToolsMenu);
    _activateWithSelections.append(moveLRAction);
    toolsToolsMenu->addAction(moveLRAction);
    QAction *moveUDAction = new ToolButton(new EventMoveTool(true, false),
                                           QKeySequence(Qt::Key_F8), toolsToolsMenu);
    _activateWithSelections.append(moveUDAction);
    toolsToolsMenu->addAction(moveUDAction);
    QAction *sizeChangeAction = new ToolButton(new SizeChangeTool(),
                                               QKeySequence(Qt::Key_F9), toolsToolsMenu);
    _activateWithSelections.append(sizeChangeAction);
    toolsToolsMenu->addAction(sizeChangeAction);

    toolsToolsMenu->addSeparator();

    QAction *newNoteAction = new ToolButton(new NewNoteTool(),
                                            QKeySequence(Qt::Key_F10), toolsToolsMenu);
    toolsToolsMenu->addAction(newNoteAction);
    QAction *removeNotesAction = new ToolButton(new EraserTool(),
                                                QKeySequence(Qt::Key_F11), toolsToolsMenu);
    toolsToolsMenu->addAction(removeNotesAction);

    toolsMB->addMenu(toolsToolsMenu);

    QAction *deleteAction = new QAction("Remove events", this);
    _activateWithSelections.append(deleteAction);
    deleteAction->setToolTip("Remove selected events");
    deleteAction->setShortcut(QKeySequence::Delete);
    deleteAction->setIcon(QIcon(":/run_environment/graphics/tool/eraser.png"));
    connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelectedEvents);
    toolsMB->addAction(deleteAction);

    toolsMB->addSeparator();

    QAction *alignLeftAction = new QAction("Align left", this);
    _activateWithSelections.append(alignLeftAction);
    alignLeftAction->setShortcut(QKeySequence(Qt::Key_Left + Qt::CTRL));
    alignLeftAction->setIcon(QIcon(":/run_environment/graphics/tool/align_left.png"));
    connect(alignLeftAction, &QAction::triggered, this, &MainWindow::alignLeft);
    toolsMB->addAction(alignLeftAction);

    QAction *alignRightAction = new QAction("Align right", this);
    _activateWithSelections.append(alignRightAction);
    alignRightAction->setIcon(QIcon(":/run_environment/graphics/tool/align_right.png"));
    alignRightAction->setShortcut(QKeySequence(Qt::Key_Right + Qt::CTRL));
    connect(alignRightAction, &QAction::triggered, this, &MainWindow::alignRight);
    toolsMB->addAction(alignRightAction);

    QAction *equalizeAction = new QAction("Equalize selection", this);
    _activateWithSelections.append(equalizeAction);
    equalizeAction->setIcon(QIcon(":/run_environment/graphics/tool/equalize.png"));
    equalizeAction->setShortcut(QKeySequence(Qt::Key_Up + Qt::CTRL));
    connect(equalizeAction, &QAction::triggered, this, &MainWindow::equalize);
    toolsMB->addAction(equalizeAction);

    toolsMB->addSeparator();

    QAction *quantizeAction = new QAction("Quantify selection", this);
    _activateWithSelections.append(quantizeAction);
    quantizeAction->setIcon(QIcon(":/run_environment/graphics/tool/quantize.png"));
    quantizeAction->setShortcut(QKeySequence(Qt::Key_G + Qt::CTRL));
    connect(quantizeAction, &QAction::triggered, this, &MainWindow::quantizeSelection);
    toolsMB->addAction(quantizeAction);


    QMenu *quantMenu = new QMenu("Quantization fractions", viewMB);
    QActionGroup *quantGroup = new QActionGroup(viewMB);
    quantGroup->setExclusive(true);

    for (ubyte i = 0; i <= 5; i++) {
        QVariant variant(i);
        QString text;

        switch (i) {
            case 0:
                text = "Whole note";
                break;
            case 1:
                text = "Half note";
                break;
            case 2:
                text = "Quarter note";
                break;
            default:
                text = _("%1th note").arg(qRound(qPow(2, i)));
        }

        QAction *a = new QAction(text, this);
        a->setData(variant);
        quantGroup->addAction(a);
        quantMenu->addAction(a);
        a->setCheckable(true);
        a->setChecked(i == _settings.quantization);
    }
    connect(quantMenu, &QMenu::triggered, this, &MainWindow::quantizationChanged);
    toolsMB->addMenu(quantMenu);

    QAction *quantizeNToleAction = new QAction("Quantify tuplet…", this);
    _activateWithSelections.append(quantizeNToleAction);
    quantizeNToleAction->setShortcut(QKeySequence(Qt::Key_H + Qt::CTRL + Qt::SHIFT));
    connect(quantizeNToleAction, &QAction::triggered, this, &MainWindow::quantizeNtoleDialog);
    toolsMB->addAction(quantizeNToleAction);

    QAction *quantizeNToleActionRepeat = new QAction("Repeat tuplet quantization", this);
    _activateWithSelections.append(quantizeNToleActionRepeat);
    quantizeNToleActionRepeat->setShortcut(QKeySequence(Qt::Key_H + Qt::CTRL));
    connect(quantizeNToleActionRepeat, &QAction::triggered, this, &MainWindow::quantizeNtole);
    toolsMB->addAction(quantizeNToleActionRepeat);

    //toolsMB->addSeparator();

    QAction *spreadAction = new QAction("Spread selection", this);
    _activateWithSelections.append(spreadAction);
    connect(spreadAction, &QAction::triggered, this, &MainWindow::spreadSelection);
    //toolsMB->addAction(spreadAction);

    toolsMB->addSeparator();

    QAction *addTrackAction = new QAction("Add track…", toolsMB);
    toolsMB->addAction(addTrackAction);
    connect(addTrackAction, &QAction::triggered, this, &MainWindow::addTrack);

    toolsMB->addSeparator();

    _deleteChannelMenu = new QMenu("Remove events from channel…", toolsMB);
    toolsMB->addMenu(_deleteChannelMenu);
    connect(_deleteChannelMenu, &QMenu::triggered, this, &MainWindow::deleteChannel);

    _moveSelectedEventsToChannelMenu = new QMenu("Move events to channel…", editMB);
    toolsMB->addMenu(_moveSelectedEventsToChannelMenu);
    connect(_moveSelectedEventsToChannelMenu, &QMenu::triggered, this,
            &MainWindow::moveSelectedEventsToChannel);

    for (ubyte i = 0; i < 16; i++) {
        QVariant variant(i);
        QAction *delChannelAction = new QAction(QString::number(i), this);
        delChannelAction->setData(variant);
        _deleteChannelMenu->addAction(delChannelAction);
        QAction *moveToChannelAction = new QAction(QString::number(i), this);
        moveToChannelAction->setData(variant);
        _moveSelectedEventsToChannelMenu->addAction(moveToChannelAction);
        QAction *selAllTrackAction = new QAction(QString::number(i), this);
        selAllTrackAction->setData(variant);
        _selectAllFromTrackMenu->addAction(selAllTrackAction);
        QAction *selAllChannelAction = new QAction(QString::number(i), this);
        selAllChannelAction->setData(variant);
        _selectAllFromChannelMenu->addAction(selAllChannelAction);
    }

    _moveSelectedEventsToTrackMenu = new QMenu("Move events to track…", editMB);
    toolsMB->addMenu(_moveSelectedEventsToTrackMenu);
    connect(_moveSelectedEventsToTrackMenu, &QMenu::triggered, this,
            &MainWindow::moveSelectedEventsToTrack);

    toolsMB->addSeparator();

    QAction *transposeAction = new QAction("Transpose selection…", this);
    _activateWithSelections.append(transposeAction);
    transposeAction->setShortcut(QKeySequence(Qt::Key_T + Qt::CTRL));
    connect(transposeAction, &QAction::triggered, this, &MainWindow::transposeNSemitones);
    toolsMB->addAction(transposeAction);

    toolsMB->addSeparator();
#ifdef ENABLE_GBA
    QAction *midFixAction = new QAction("Convert normal MIDI for GBA (beta)");
    connect(midFixAction, &QAction::triggered, this, &MainWindow::howMidFixDialog);
    toolsMB->addAction(midFixAction);
#endif

    QAction *setFileLengthMs = new QAction("Set file duration", this);
    connect(setFileLengthMs, &QAction::triggered, this, &MainWindow::setFileLengthMs);
    toolsMB->addAction(setFileLengthMs);

    QAction *scaleSelection = new QAction("Scale events", this);
    _activateWithSelections.append(scaleSelection);
    connect(scaleSelection, &QAction::triggered, this, &MainWindow::scaleSelection);
    toolsMB->addAction(scaleSelection);

    toolsMB->addSeparator();

    QAction *magnetAction = new QAction("Magnet", editMB);
    toolsMB->addAction(magnetAction);
    magnetAction->setShortcut(QKeySequence(Qt::Key_M + Qt::CTRL));
    magnetAction->setIcon(QIcon(":/run_environment/graphics/tool/magnet.png"));
    magnetAction->setCheckable(true);
    magnetAction->setChecked(_settings.magnet);
    connect(magnetAction, &QAction::triggered, this, &MainWindow::enableMagnet);

    // View
    QMenu *zoomMenu = new QMenu("Zoom…", viewMB);
    QAction *zoomHorOutAction = new QAction("Horizontal out", this);
    zoomHorOutAction->setShortcut(QKeySequence(Qt::Key_Minus+Qt::CTRL));
    zoomHorOutAction->setIcon(QIcon(":/run_environment/graphics/tool/zoom_hor_out.png"));
    connect(zoomHorOutAction, &QAction::triggered, mw_matrixWidget, &MatrixWidget::zoomHorOut);
    zoomMenu->addAction(zoomHorOutAction);

    QAction *zoomHorInAction = new QAction("Horizontal in", this);
    zoomHorInAction->setIcon(QIcon(":/run_environment/graphics/tool/zoom_hor_in.png"));
    zoomHorInAction->setShortcut(QKeySequence(Qt::Key_Plus + Qt::CTRL));
    connect(zoomHorInAction, &QAction::triggered, mw_matrixWidget, &MatrixWidget::zoomHorIn);
    zoomMenu->addAction(zoomHorInAction);

    QAction *zoomVerOutAction = new QAction("Vertical out", this);
    zoomVerOutAction->setIcon(QIcon(":/run_environment/graphics/tool/zoom_ver_out.png"));
    zoomVerOutAction->setShortcut(QKeySequence(Qt::Key_Minus + Qt::CTRL + Qt::ALT));
    connect(zoomVerOutAction, &QAction::triggered, mw_matrixWidget, &MatrixWidget::zoomVerOut);
    zoomMenu->addAction(zoomVerOutAction);

    QAction *zoomVerInAction = new QAction("Vertical in", this);
    zoomVerInAction->setIcon(QIcon(":/run_environment/graphics/tool/zoom_ver_in.png"));
    zoomVerInAction->setShortcut(QKeySequence(Qt::Key_Plus + Qt::CTRL+Qt::ALT));
    connect(zoomVerInAction, &QAction::triggered, mw_matrixWidget, &MatrixWidget::zoomVerIn);
    zoomMenu->addAction(zoomVerInAction);

    zoomMenu->addSeparator();

    QAction *zoomStdAction = new QAction("Restore default", this);
    zoomStdAction->setShortcut(QKeySequence(Qt::Key_0+Qt::CTRL));
    connect(zoomStdAction, &QAction::triggered, mw_matrixWidget, &MatrixWidget::zoomStd);
    zoomMenu->addAction(zoomStdAction);

    viewMB->addMenu(zoomMenu);


    viewMB->addSeparator();

    viewMB->addAction(_allChannelsVisible);
    viewMB->addAction(_allChannelsInvisible);
    viewMB->addAction(_allTracksVisible);
    viewMB->addAction(_allTracksInvisible);

    viewMB->addSeparator();

    QMenu *colorMenu = new QMenu("Colors…", viewMB);
    _colorsByChannel = new QAction("From channels", this);
    _colorsByChannel->setCheckable(true);
    connect(_colorsByChannel, &QAction::triggered, this, &MainWindow::colorsByChannel);
    colorMenu->addAction(_colorsByChannel);

    _colorsByTracks = new QAction("From tracks", this);
    _colorsByTracks->setCheckable(true);
    connect(_colorsByTracks, &QAction::triggered, this, &MainWindow::colorsByTrack);
    colorMenu->addAction(_colorsByTracks);

    viewMB->addMenu(colorMenu);

    viewMB->addSeparator();

    // TODO: Better name
    QMenu *divMenu = new QMenu("Raster", viewMB);
    QActionGroup *divGroup = new QActionGroup(viewMB);
    divGroup->setExclusive(true);

    for (int i = -1; i <= 5; i++) {
        QVariant variant(i);
        QString text = "Off";
        if (i == 0) {
            text = "Whole note";
        } else if (i == 1) {
            text = "Half note";
        } else if (i == 2) {
            text = "Quarter note";
        } else if (i > 0) {
            text = _("%1th note").arg(qRound(qPow(2, i)));
        }
        QAction *a = new QAction(text, this);
        a->setData(variant);
        divGroup->addAction(a);
        divMenu->addAction(a);
        a->setCheckable(true);
        a->setChecked(i == _settings.div);
    }
    connect(divMenu, &QMenu::triggered, this, &MainWindow::divChanged);
    viewMB->addMenu(divMenu);

    // Playback
    QAction *playStopAction = new QAction("PlayStop");
    QList<QKeySequence> playStopActionShortcuts;
    playStopActionShortcuts << QKeySequence(Qt::Key_Space)
                            << QKeySequence(Qt::Key_K)
                            << QKeySequence(Qt::Key_P + Qt::CTRL);
    playStopAction->setShortcuts(playStopActionShortcuts);
    connect(playStopAction, &QAction::triggered, this, &MainWindow::playStop);
    playbackMB->addAction(playStopAction);

    QAction *playAction = new QAction("Play", this);
    playAction->setIcon(QIcon(":/run_environment/graphics/tool/play.png"));
    connect(playAction, &QAction::triggered, this, &MainWindow::play);
    playbackMB->addAction(playAction);

    QAction *pauseAction = new QAction("Pause", this);
    pauseAction->setIcon(QIcon(":/run_environment/graphics/tool/pause.png"));
    pauseAction->setShortcut(QKeySequence(Qt::Key_Space + Qt::CTRL));
    connect(pauseAction, &QAction::triggered, this, &MainWindow::pause);
    playbackMB->addAction(pauseAction);

    QAction *recAction = new QAction("Record", this);
    recAction->setIcon(QIcon(":/run_environment/graphics/tool/record.png"));
    recAction->setShortcut(QKeySequence(Qt::Key_R + Qt::CTRL));
    connect(recAction, &QAction::triggered, this, &MainWindow::record);
    playbackMB->addAction(recAction);

    QAction *stopAction = new QAction("Stop", this);
    stopAction->setIcon(QIcon(":/run_environment/graphics/tool/stop.png"));
    connect(stopAction, &QAction::triggered, this, SIGNAL_OL(MainWindow,stop));
    playbackMB->addAction(stopAction);

    playbackMB->addSeparator();

    QAction *backToBeginAction = new QAction("Back to begin", this);
    backToBeginAction->setIcon(QIcon(":/run_environment/graphics/tool/back_to_begin.png"));
    QList<QKeySequence> backToBeginActionShortcuts;
    backToBeginActionShortcuts << QKeySequence(Qt::Key_Home)
                                << QKeySequence(Qt::Key_J + Qt::SHIFT)
                                << QKeySequence(Qt::Key_Left + Qt:: SHIFT);
    backToBeginAction->setShortcuts(backToBeginActionShortcuts);
    connect(backToBeginAction, &QAction::triggered, this, &MainWindow::backToBegin);
    playbackMB->addAction(backToBeginAction);

    QAction *backAction = new QAction("Previous measure", this);
    backAction->setIcon(QIcon(":/run_environment/graphics/tool/back.png"));
    QList<QKeySequence> backActionShortcuts;
    backActionShortcuts << QKeySequence(Qt::Key_J)
                        << QKeySequence(Qt::Key_Left);
    backAction->setShortcuts(backActionShortcuts);
    connect(backAction, &QAction::triggered, this, &MainWindow::back);
    playbackMB->addAction(backAction);

    QAction *forwAction = new QAction("Next measure", this);
    forwAction->setIcon(QIcon(":/run_environment/graphics/tool/forward.png"));
    QList<QKeySequence> forwActionShortcuts;
    forwActionShortcuts << QKeySequence(Qt::Key_L)
                        << QKeySequence(Qt::Key_Right);
    forwAction->setShortcuts(forwActionShortcuts);
    connect(forwAction, &QAction::triggered, this, &MainWindow::forward);
    playbackMB->addAction(forwAction);

    playbackMB->addSeparator();

    QMenu *speedMenu = new QMenu("Playback speed…");
    connect(speedMenu, &QMenu::triggered, this, &MainWindow::setSpeed);

    QList<double> speeds;
    speeds.append(0.25);
    speeds.append(0.5);
    speeds.append(0.75);
    speeds.append(1);
    speeds.append(1.25);
    speeds.append(1.5);
    speeds.append(1.75);
    speeds.append(2);
    QActionGroup *speedGroup = new QActionGroup(this);
    speedGroup->setExclusive(true);

    for (const double &s : speeds) {
        QAction *speedAction = new QAction(QString::number(s, 'g', 2), this);
        speedAction->setData(QVariant::fromValue(s));
        speedMenu->addAction(speedAction);
        speedGroup->addAction(speedAction);
        speedAction->setCheckable(true);
        speedAction->setChecked(qFuzzyCompare(s, 1));
    }

    playbackMB->addMenu(speedMenu);

    playbackMB->addSeparator();

    playbackMB->addAction(_allChannelsAudible);
    playbackMB->addAction(_allChannelsMute);
    playbackMB->addAction(_allTracksAudible);
    playbackMB->addAction(_allTracksMute);

    playbackMB->addSeparator();

    QAction *lockAction = new QAction("Lock screen while playing", this);
    lockAction->setIcon(QIcon(":/run_environment/graphics/tool/screen_unlocked.png"));
    lockAction->setCheckable(true);
    connect(lockAction, &QAction::triggered, this, &MainWindow::screenLockPressed);
    playbackMB->addAction(lockAction);
    lockAction->setChecked(mw_matrixWidget->screenLocked());

    QAction *metronomeAction = new QAction("Metronome", this);
    metronomeAction->setIcon(QIcon(":/run_environment/graphics/tool/metronome.png"));
    metronomeAction->setCheckable(true);
    metronomeAction->setChecked(_settings.metronome);
    connect(metronomeAction, &QAction::triggered, this, &MainWindow::enableMetronome);
    playbackMB->addAction(metronomeAction);

    // Midi
    QAction *configAction2 = new QAction("Settings…", this);
    configAction2->setIcon(QIcon(":/run_environment/graphics/tool/config.png"));
    connect(configAction2, &QAction::triggered, this, &MainWindow::openConfig);
    midiMB->addAction(configAction2);

    QAction *thruAction = new QAction("Connect Midi In/Out", this);
    thruAction->setIcon(QIcon(":/run_environment/graphics/tool/connection.png"));
    thruAction->setCheckable(true);
    thruAction->setChecked(_settings.thru);
    connect(thruAction, &QAction::triggered, this, &MainWindow::enableThru);
    midiMB->addAction(thruAction);

    midiMB->addSeparator();

    QAction *panicAction = new QAction("Midi panic", this);
    connect(panicAction, &QAction::triggered, this, &MainWindow::panic);
    midiMB->addAction(panicAction);

    // Help
    QAction *manualAction = new QAction("Manual", this);
    connect(manualAction, &QAction::triggered, this, &MainWindow::manual);
    helpMB->addAction(manualAction);

    QAction *aboutAction = new QAction("About MidiEditor", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);
    helpMB->addAction(aboutAction);

    QAction *donateAction = new QAction("Donate", this);
    connect(donateAction, &QAction::triggered, this, &MainWindow::donate);
    helpMB->addAction(donateAction);

    QToolBar *out = new QToolBar("Toolbar", parent);
    out->setFloatable(false);
    out->setContentsMargins(0,0,0,0);
    out->layout()->setSpacing(0);
    out->setMovable(false);

    QWidget *buttonBar = new QWidget(out);
    QGridLayout *btnLayout = new QGridLayout(buttonBar);
    buttonBar->setLayout(btnLayout);
    btnLayout->setSpacing(0);
    buttonBar->setContentsMargins(0,0,0,0);
    QToolBar *fileTB = new QToolBar("File", buttonBar);


    fileTB->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    fileTB->setFloatable(false);
    fileTB->setContentsMargins(0,0,0,0);
    fileTB->layout()->setSpacing(0);
    fileTB->setIconSize(QSize(35,35));
    fileTB->addAction(newAction);
    fileTB->setStyleSheet("QToolBar{border:0px}");
    QAction *loadAction2 = new QAction("Open…", this);
    loadAction2->setIcon(QIcon(":/run_environment/graphics/tool/load.png"));
    connect(loadAction2, &QAction::triggered, this, &MainWindow::load);
    loadAction2->setMenu(_recentPathsMenu);
    fileTB->addAction(loadAction2);

    fileTB->addAction(saveAction);
    fileTB->addSeparator();

    fileTB->addAction(undoAction);
    fileTB->addAction(redoAction);
    fileTB->addSeparator();

    btnLayout->addWidget(fileTB, 0, 0, 2, 1);



    QToolBar *upperTB = new QToolBar(buttonBar);
    QToolBar *lowerTB = new QToolBar(buttonBar);
    btnLayout->addWidget(upperTB, 0, 2, 1, 1);
    btnLayout->addWidget(lowerTB, 1, 2, 1, 1);
    upperTB->setFloatable(false);
    upperTB->setContentsMargins(0,0,0,0);
    upperTB->layout()->setSpacing(0);
    upperTB->setIconSize(QSize(20,20));
    lowerTB->setFloatable(false);
    lowerTB->setContentsMargins(0,0,0,0);
    lowerTB->layout()->setSpacing(0);
    lowerTB->setIconSize(QSize(20,20));
    lowerTB->setStyleSheet("QToolBar{border:0px}");
    upperTB->setStyleSheet("QToolBar{border:0px}");

    lowerTB->addAction(copyAction);

    pasteActionTB = new QAction("Paste events", this);
    pasteActionTB->setToolTip("Paste events at cursor position");
    pasteActionTB->setIcon(QIcon(":/run_environment/graphics/tool/paste.png"));
    connect(pasteActionTB, &QAction::triggered, this, &MainWindow::paste);
    pasteActionTB->setMenu(pasteOptionsMenu);

    lowerTB->addAction(pasteActionTB);

    lowerTB->addSeparator();

    lowerTB->addAction(zoomHorInAction);
    lowerTB->addAction(zoomHorOutAction);
    lowerTB->addAction(zoomVerInAction);
    lowerTB->addAction(zoomVerOutAction);

    lowerTB->addSeparator();

    if (QApplication::arguments().contains(L1("--large-playback-toolbar"))) {

        QToolBar *playTB = new QToolBar("Playback", buttonBar);

        playTB->setFloatable(false);
        playTB->setContentsMargins(0,0,0,0);
        playTB->layout()->setSpacing(0);
        playTB->setIconSize(QSize(35,35));

        playTB->addAction(backToBeginAction);
        playTB->addAction(backAction);
        playTB->addAction(playAction);
        playTB->addAction(pauseAction);
        playTB->addAction(stopAction);
        playTB->addAction(recAction);
        playTB->addAction(forwAction);
        playTB->addSeparator();

        btnLayout->addWidget(playTB, 0, 1, 2, 1);
    } else {

        lowerTB->addAction(backToBeginAction);
        lowerTB->addAction(backAction);
        lowerTB->addAction(playAction);
        lowerTB->addAction(pauseAction);
        lowerTB->addAction(stopAction);
        lowerTB->addAction(recAction);
        lowerTB->addAction(forwAction);
        lowerTB->addSeparator();
    }

    lowerTB->addAction(lockAction);

    upperTB->addAction(stdToolAction);
    upperTB->addAction(selectSingleAction);
    upperTB->addAction(selectBoxAction);
    upperTB->addAction(selectLeftAction);
    upperTB->addAction(selectRightAction);

    upperTB->addSeparator();
    upperTB->addAction(moveAllAction);
    upperTB->addAction(moveLRAction);
    upperTB->addAction(moveUDAction);
    upperTB->addAction(sizeChangeAction);

    upperTB->addSeparator();

    upperTB->addAction(alignLeftAction);
    upperTB->addAction(alignRightAction);
    upperTB->addAction(equalizeAction);

    upperTB->addSeparator();

    upperTB->addAction(quantizeAction);

    upperTB->addSeparator();

    upperTB->addAction(newNoteAction);
    upperTB->addAction(removeNotesAction);

    upperTB->addSeparator();
    upperTB->addAction(magnetAction);

    lowerTB->addSeparator();
    lowerTB->addAction(metronomeAction);
    lowerTB->addSeparator();
    lowerTB->addAction(thruAction);

    btnLayout->setColumnStretch(4, 1);


    out->addWidget(buttonBar);
    return out;
}



void MainWindow::checkEnableActionsForSelection() {
    bool enabled = !Selection::instance()->selectedEvents().isEmpty();
    for (QAction *action : qAsConst(_activateWithSelections)) {
        action->setEnabled(enabled);
    }
    if (_moveSelectedEventsToChannelMenu) {
        _moveSelectedEventsToChannelMenu->setEnabled(enabled);
    }
    if (_moveSelectedEventsToTrackMenu) {
        _moveSelectedEventsToTrackMenu->setEnabled(enabled);
    }
    if (Tool::currentTool() && Tool::currentTool()->button() && !Tool::currentTool()->button()->isEnabled()) {
        stdToolAction->trigger();
    }
    if (file) {
        undoAction->setEnabled(file->protocol()->stepsBack()>1);
        redoAction->setEnabled(file->protocol()->stepsForward()>0);
    }
}

