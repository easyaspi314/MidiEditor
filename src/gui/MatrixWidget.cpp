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

#include "MatrixWidget.h"
#include <QSizePolicy>
#include "../midi/MidiFile.h"
#include "../MidiEvent/MidiEvent.h"
#include "../MidiEvent/TempoChangeEvent.h"
#include "../MidiEvent/TimeSignatureEvent.h"
#include "../MidiEvent/NoteOnEvent.h"
#include "../MidiEvent/OffEvent.h"
#include "../midi/MidiChannel.h"
#include "../tool/Tool.h"
#include "../tool/EditorTool.h"
#include "../tool/EventTool.h"
#include "../protocol/Protocol.h"
#include "../midi/MidiPlayer.h"
#include "../midi/PlayerThread.h"
#include "../midi/MidiInput.h"
#include "../midi/MidiTrack.h"
#include "../tool/Selection.h"

#include <QList>
#include <QPixmapCache>
#include <QtCore/qmath.h>
#include <QBuffer>
#include <QTime>
#include <QGLWidget>
#include <QOpenGLWidget>

#include <iostream>

/*
 * TODO:
 * - Remove old variables that we don't need
 * - Make painting even more efficient:
 *     > Further decrease CPU usage.
 *     > Hardware acceleration?
 *     > Decrease update frequency
 *     > Only repaint areas that are updated.
 *     > Render in chunks?
 *     > Set backgrounds when appropriate
 *     > Maybe get rid of QPixmap entirely?
 * - Remove unnecessary checks
 * - Make sure everything that should be
 *   changed...changes, and everything that
 *   shouldn't...shouldn't.
 *     > Divs don't update properly
 *     > MiscWidget doesn't update properly.
 * - PianoWidget interaction
 */

bool MatrixWidget::antiAliasingEnabled = true;
MatrixWidget::MatrixWidget(QWidget *parent) : QGraphicsView(parent) {
	pianoKeys = QMap<int, QRectF>();
	scrollTimer = new QTimer(this);
	scrollTimer->setSingleShot(true);
	connect(scrollTimer, SIGNAL(timeout()), this, SLOT(enableUpdates()));

	// TODO: Maybe reimplement this so QMacCGContext will shut up

	screen_locked = false;
	startTimeX = 0;

	endLineY = NUM_LINES;
	endTimeX = 0;
	startLineY = 0;
	scrollDir = NONE;
	// Double the cache space as we use it a lot.
	//QPixmapCache::setCacheLimit(20480);
	file = Q_NULLPTR;
	scaleX = 1;
	pianoEvent = new NoteOnEvent(0, 100, 0, Q_NULLPTR);
	scaleY = 1;
	timeHeight = 0;
	currentTempoEvents = new QList<MidiEvent *>;
	currentTimeSignatureEvents = new QList<TimeSignatureEvent *>;
	msOfFirstEventInList = 0;
	velocityObjects = new QList<MidiEvent *>;

	scrolling = false;
	setViewportMargins(110, 50, 0, 0);
	_div = 2;

}

void MatrixWidget::init() {
	_mouseInWidget = underMouse();
	setDragMode(RubberBandDrag);
	setViewportUpdateMode(MinimalViewportUpdate);
	setOptimizationFlags(DontAdjustForAntialiasing | DontSavePainterState);
	setViewport(new QOpenGLWidget);
	// TODO: Antialiasing + OpenGL
	//setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

	//setRenderHints(QPainter::HighQualityAntialiasing | QPainter::Antialiasing);
	setInteractive(true);
	matrixScene = new QGraphicsScene(
			      qRectF(QRectF(0, 0,
					    // don't make it too small
					    qMax((40 / 120 / 1000) * (PIXEL_PER_S), viewport()->width()),
					    qMax(NUM_LINES * PIXEL_PER_LINE, viewport()->height()))),
			      this);
	EditorTool::setMatrixWidget(this);
	setMouseTracking(true);
	setFocusPolicy(Qt::ClickFocus);
	setScene(matrixScene);
	setupLines();

	connect(MidiPlayer::player(), SIGNAL(timeMsChanged(int)),
			  this, SLOT(timeMsChanged(int)));
}

void MatrixWidget::scrollContentsBy(int dx, int dy) {
	QGraphicsView::scrollContentsBy(dx, dy);
	//scrolling = true;
	//setViewportUpdateMode(SmartViewportUpdate);
	//scrollTimer->start(250);
	if (dx && timelineWidget){
		timelineWidget->move(timelineWidget->x() + dx, timelineWidget->y());
	}
	if (dy && pianoWidget) {
		pianoWidget->move(pianoWidget->x(), pianoWidget->y() + dy);
	}

}

void MatrixWidget::setScreenLocked(bool b) {
	screen_locked = b;
}
void MatrixWidget::enableUpdates() {
	//setViewportUpdateMode(MinimalViewportUpdate);
	scrolling = false;
}
bool MatrixWidget::screenLocked() {
	return screen_locked;
}
QRectF MatrixWidget::relativeRect() {
	return mapToScene(viewport()->geometry()).boundingRect();
}
void MatrixWidget::setupLines() {
	QPixmap linesTexture;
		linesTexture = QPixmap(1, NUM_LINES);

		QPainter linesPainter(&linesTexture);

		for (int i = 0; i <= NUM_LINES; i++) {
			int startLine = i;
			QColor c(194, 230, 255);
			if (i % 2 == 0) {
				c = QColor(234, 246, 255);
			}
			if (i > 127) {
				c = QColor(194, 194, 194);
				if (i % 2 == 1) {
					c = QColor(234, 246, 255);
				}
			}
			linesPainter.fillRect(qRectF(0, startLine, 1, 1), c);

		}
		linesPainter.end();
		setBackgroundBrush(QBrush(linesTexture.scaledToHeight(qCeil(sceneRect().height()))));
		setCacheMode(CacheBackground);
}
void MatrixWidget::timeMsChanged(int ms, bool ignoreLocked) {
	if (!file) {
		return;
	}
	int x = xPosOfMs(ms);
	int startTimeXTemp = msOfXPos(relativeRect().left());
	int endTimeXTemp = msOfXPos(relativeRect().right());

	if ((!screen_locked || ignoreLocked) && (x < 0 || ms < startTimeXTemp
			  || ms > endTimeXTemp
			  || x > (relativeRect().left() + relativeRect().width() + 100))) {

		// return if the last tick is already shown
		if (file->maxTime() <= endTimeXTemp && ms >= startTimeXTemp) {
			return;
		}

		// sets the new position and repaints
		emit scrollChanged(qRound(relativeRect().right() + (relativeRect().width() *
								    0.9)), relativeRect().top());
}

}
#ifdef OLD
///*
// * FIXME: Divs don't update after undo/redo
// */
//void MatrixWidget::paintEvent(QPaintEvent *event) {
//	if (!file || !file->protocol() || height() <= 0 || paintingActive()) {
//		return;
//	}
//	QPainter painter(this);
//	QFont font = painter.font();
//	font.setPixelSize(12);
//	painter.setFont(font);
//	QRect viewport = relativeRect();

//	/*
//	 * Check if we are calling a flat-out repaint
//	 * or just updating part of the widget.
//	 *
//	 * TODO: Make further use of this, especially
//	 * when scrolling.
//	 */
//	int updatemode = 0;
//	if (pixmap && !event->region().isNull() && !event->region().isEmpty()) {
//		updatemode = 1;
//		painter.setClipping(true);
//		painter.setClipRegion(event->region());
//	} else if (pixmap && !event->rect().isNull() && !event->rect().isEmpty()) {
//		updatemode = 2;
//		painter.setClipping(true);
//		painter.setClipRect(event->rect());
//	} else {
//		painter.setClipping(true);
//		painter.setClipRect(viewport);
//	}
//	QPixmap linesTexture;
//	if (!QPixmapCache::find("MatrixWidget_" + QString::number(scaleY, 'f', 2),
//									linesTexture)) {
//		linesTexture = QPixmap(1, height());
//		//linesTexture.fill(Qt::transparent);

//		QPainter linesPainter(&linesTexture);
//		if (antiAliasingEnabled) {
//			linesPainter.setRenderHint(QPainter::Antialiasing);
//		}
//		for (int i = qFloor(startLineY); i <= qFloor(endLineY); i++) {
//			int startLine = yPosOfLine(i);
//			QColor c(194, 230, 255);
//			if (i % 2 == 0) {
//				c = QColor(234, 246, 255);
//			}
//			if (i > 127) {
//				c = QColor(194, 194, 194);
//				if (i % 2 == 1) {
//					c = QColor(234, 246, 255);
//				}
//			}
//			linesPainter.fillRect(qRectF(0, startLine, 1, startLine + lineHeight()), c);

//		}
//		linesPainter.end();
//		QPixmapCache::insert("MatrixWidget_" + QString::number(scaleY, 'f', 2),
//									linesTexture);
//		QPalette palette;
//		palette.setBrush(backgroundRole(), QBrush(linesTexture));
//		setPalette(palette);
//	}
//	// This complex QString serves as the ID of the events pixmap.
//	// It stores the zoom level, measure division, and the UUID of the current ProtocolStep.
//	QString pixmapId = "ProtocolStep_"
//							 + QString::number(scaleX, 'f', 2) + "_"
//							 + QString::number(scaleY, 'f', 2) + "_"
//							 + QString::number(div()) + "_"
//							 + file->protocol()->currentStepId();
//	bool totalRepaint = !pixmap && !QPixmapCache::find(pixmapId, pixmap);

//	if (totalRepaint) {
//		this->pianoKeys.clear();
//		pixmap = new QPixmap(width(), height());

//		if (pixmap->paintingActive()) {
//			return;
//		}
//		pixmap->fill(Qt::transparent);
//		QPainter pixpainter(pixmap);

//		pixpainter.setBrush(Qt::transparent);
//		if (antiAliasingEnabled) {
//			pixpainter.setRenderHint(QPainter::Antialiasing);
//		}

//		QFont f = pixpainter.font();
//		f.setPixelSize(12);
//		pixpainter.setFont(f);
//		pixpainter.setClipping(false);

//		for (int i = 0; i < objects->length(); i++) {
//			objects->at(i)->setShown(false);
//			OnEvent *onev = qobject_cast<OnEvent *>(objects->at(i));
//			if (onev && onev->offEvent()) {
//				onev->offEvent()->setShown(false);
//			}
//		}
//		objects->clear();
//		velocityObjects->clear();
//		currentTempoEvents->clear();
//		currentTimeSignatureEvents->clear();
//		currentDivs.clear();

//		startTick = file->tick(startTimeX, endTimeX, &currentTempoEvents,
//									  &endTick, &msOfFirstEventInList);

//		TempoChangeEvent *ev = qobject_cast<TempoChangeEvent *>(currentTempoEvents->at(
//											0));
//		if (!ev) {
//			return;
//		}

//		// paint measures and timeline
//		currentTimeSignatureEvents = new QList<TimeSignatureEvent *>;
//		int measure = file->measure(0, file->endTick(), &currentTimeSignatureEvents);

//		TimeSignatureEvent *currentEvent = currentTimeSignatureEvents->at(0);
//		int i = 0;
//		if (!currentEvent) {
//			return;
//		}
//		int tick = currentEvent->midiTime();
//		while (tick + currentEvent->ticksPerMeasure() <= 0) {
//			tick += currentEvent->ticksPerMeasure();
//		}
//		qreal xfrom, xDiv, metronomeDiv;
//		int measureStartTick, ticksPerDiv, startTickDiv, divTick;
//		QPen oldPen, dashPen;
//		while (tick < file->endTick()) {
//			TimeSignatureEvent *measureEvent = currentTimeSignatureEvents->at(i);
//			xfrom = xPosOfMs(msOfTick(tick));
//			currentDivs.append(QPair<qreal, int>(xfrom, tick));
//			measure++;
//			measureStartTick = tick;
//			tick += currentEvent->ticksPerMeasure();
//			if (i < currentTimeSignatureEvents->length() - 1) {
//				if (currentTimeSignatureEvents->at(i + 1)->midiTime() <= tick) {
//					currentEvent = currentTimeSignatureEvents->at(i + 1);
//					tick = currentEvent->midiTime();
//					i++;
//				}
//			}
//			// draw measures
//			if (_div >= 0) {
//				metronomeDiv = 4 / qPow(2, _div);
//				ticksPerDiv = metronomeDiv * file->ticksPerQuarter();
//				startTickDiv = ticksPerDiv;
//				oldPen = pixpainter.pen();
//				dashPen = QPen(Qt::lightGray, 1, Qt::DashLine);
//				pixpainter.setPen(dashPen);
//				while (startTickDiv <= measureEvent->ticksPerMeasure()) {
//					divTick = startTickDiv + measureStartTick;
//					xDiv = xPosOfMs(msOfTick(divTick));
//					currentDivs.append(QPair<qreal, int>(xDiv, divTick));
//					pixpainter.drawLine(QLineF(xDiv, 0, xDiv, height()));
//					startTickDiv += ticksPerDiv;
//				}
//				pixpainter.setPen(oldPen);
//			}
//		}

//		// line between time texts and matrixarea
//		pixpainter.setPen(Qt::gray);
//		pixpainter.drawLine(QLineF(0, 0, width(), 0));
//		pixpainter.drawLine(qLineF(0, 0, 0,
//											height()));

//		pixpainter.setPen(Qt::black);

//		// paint the events
//		//pixpainter.setClipping(true);
//		//pixpainter.setClipRect(qRectF(0, 0, width(), height()));
//		for (int i = 0; i < 19; i++) {
//			paintChannel(&pixpainter, i);
//		}
//		pixpainter.end();

//		QPixmapCache::insert(pixmapId, *pixmap);

//		// Set the background of the widget to prevent further
//		// repaints.
//		/*QPalette palette = this->palette();
//		palette.setBrush(backgroundRole(), QBrush(notesPixmap));
//		setPalette(palette);*/

//		// TODO: I don't think we need this.

//	}
//	switch (updatemode) {
//		case 1: {
//			QPainter::PixmapFragment *frags = new
//			QPainter::PixmapFragment[event->region().rectCount()];
//			QVector<QRect> rects = event->region().rects();
//			for (int i = 0; i < event->region().rectCount(); i++) {
//				QRect rect = rects.at(i);
//				frags[i] = QPainter::PixmapFragment::create(rect.center(), rect, 1, 1, 0, 1);
//			}
//			painter.drawPixmapFragments(frags, event->region().rectCount(), *pixmap);
//			delete[] frags;
//			break;
//		}
//		case 2: {
//			QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
//															event->rect().center(), event->rect());
//			painter.drawPixmapFragments(&frag, 1, *pixmap);
//			break;
//		}
//		default:
//			QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(viewport.center(), viewport);

//			painter.drawPixmapFragments(&frag, 1, *pixmap);
//			//painter.drawPixmap(0, 0, *pixmap);
//	}
//	if (antiAliasingEnabled) {
//		painter.setRenderHint(QPainter::Antialiasing);
//	}
//	// draw the piano / linenames
//	if (Tool::currentTool()) {
//		painter.save();
//		painter.setClipping(true);
//		painter.setClipRect(viewport);
//		Tool::currentTool()->draw(&painter);
//		painter.restore();
//	}

//	int timelinePos = timelineWidget->mousePosition();
//	if (enabled && timelinePos >= 0 && !MidiPlayer::instance()->isPlaying()) {
//		painter.setPen(Qt::red);
//		painter.drawLine(qLineF(timelinePos, 0, timelinePos, height()));
//	}

//	if (MidiPlayer::instance()->isPlaying()) {
//		painter.setPen(Qt::red);
//		int x = xPosOfMs(MidiPlayer::instance()->timeMs());
//		if (x >= 0 && viewport.contains(QPoint(x, viewport.center().y()))) {
//			painter.drawLine(qLineF(x, 0, x, height()));
//		}

//	}

//	// border
//	//	painter.setPen(Qt::gray);
//	//	painter.drawLine(qLineF(width() - 1, height() - 1, 0,
//	//							 height() - 1));
//	//	painter.drawLine(qLineF(width() - 1, height() - 1, width() - 1, 2));

//	// if the recorder is recording, show red circle
//	if (MidiInput::instance()->recording()) {
//		painter.setPen(Qt::black);
//		painter.setBrush(Qt::red);
//		painter.drawEllipse(qPointF(viewport.right() - 20, 5), 15, 15);
//	}

//	// if MouseRelease was not used, delete it
//	mouseReleased = false;

//	if (totalRepaint) {
//		emit objectListChanged();
//	}
//}

//void MatrixWidget::paintChannel(QPainter *painter, int channel) {
//	if (!file->channel(channel)->visible()) {
//		return;
//	}
//	QColor cC = file->channel(channel)->color();

//	// filter events
//	QMultiMap<int, MidiEvent *> *map = file->channelEvents(channel);

//	QMap<int, MidiEvent *>::iterator it = map->lowerBound(startTick);
//	while (it != map->end() && it.key() <= endTick) {
//		MidiEvent *event = it.value();
//		if (eventInWidget(event)) {
//			// insert all Events in objects, set their coordinates
//			// Only onEvents are inserted. When there is an On
//			// and an OffEvent, the OnEvent will hold the coordinates
//			int line = event->line();

//			OffEvent *offEvent = qobject_cast<OffEvent *>(event);
//			OnEvent *onEvent = qobject_cast<OnEvent *>(event);

//			qreal x, width;
//			qreal y = yPosOfLine(line);
//			double height = lineHeight();

//			if (onEvent || offEvent) {
//				if (onEvent) {
//					offEvent = onEvent->offEvent();
//				} else if (offEvent) {
//					onEvent = qobject_cast<OnEvent *>(offEvent->onEvent());
//				}

//				width = xPosOfMs(msOfTick(offEvent->midiTime())) -
//						  xPosOfMs(msOfTick(onEvent->midiTime()));
//				x = xPosOfMs(msOfTick(onEvent->midiTime()));
//				event = onEvent;
//				if (objects->contains(event)) {
//					it++;
//					continue;
//				}
//			} else {
//				width = PIXEL_PER_EVENT;
//				x = xPosOfMs(msOfTick(event->midiTime()));
//			}
//			event->setX(x);
//			event->setY(y);
//			event->setWidth(width);
//			event->setHeight(height);

//			if (!(event->track()->hidden())) {
//				if (!_colorsByChannels) {
//					cC = *event->track()->color();
//				}
//				event->draw(painter, cC);


//				if (Selection::instance()->selectedEvents().contains(event)) {
//					painter->setPen(Qt::gray);
//					painter->drawLine(qLineF(0, y, this->width(), y));
//					painter->drawLine(qLineF(0, y + height, this->width(), y + height));
//					painter->setPen(Qt::black);

//				}
//				objects->prepend(event);
//			}
//		}

//		if (!(event->track()->hidden())) {
//			// append event to velocityObjects if its not a offEvent and if it
//			// is in the x-Area
//			OffEvent *offEvent = qobject_cast<OffEvent *>(event);
//			if (!offEvent && event->midiTime() >= startTick &&
//					  event->midiTime() <= endTick &&
//					  !velocityObjects->contains(event)) {
//				qreal mX = xPosOfMs(msOfTick(event->midiTime()));
//				event->setX(mX);

//				velocityObjects->prepend(event);
//			}
//		}
//		it++;
//	}
//}
#endif
// TODO: Figure out why the size is doubled.
void MatrixWidget::addChannel(int channel) {
	if (!file->channel(channel)->visible()) {
		return;
	}
	QColor cC = file->channel(channel)->color();
	QPen mPen = QPen(Qt::gray);
	mPen.setCosmetic(true);

	// filter events
	QMultiMap<int, MidiEvent *> *map = file->channelEvents(channel);
	QMap<int, MidiEvent *>::iterator it = map->lowerBound(startTick);
	while (it != map->end() && it.key() <= endTick) {

		MidiEvent *event = it.value();

		qreal width = PIXEL_PER_EVENT;
		qreal x = xPosOfMs(file->msOfTick(event->midiTime()));
		event->setPos(x, yPosOfLine(event->line()));
		event->setHeight(lineHeight());
		event->setWidth(width);
		if (event->type() == MidiEvent::OnEventType || event->type() == MidiEvent::OffEventType || event->type() == MidiEvent::NoteOnEventType) {
			OnEvent *onEvent;
			OffEvent *offEvent;
			if (event->type() == MidiEvent::OnEventType || event->type() == MidiEvent::NoteOnEventType) {
				onEvent = qobject_cast<OnEvent *>(event);
				offEvent = onEvent->offEvent();
			} else {
				offEvent = qobject_cast<OffEvent *>(event);
				onEvent = offEvent->onEvent();
			}
			if (!onEvent || !offEvent) {
				qWarning("wut!");
			} else {
				width = xPosOfMs(msOfTick(offEvent->midiTime())) -
						  xPosOfMs(msOfTick(onEvent->midiTime()));
				x = xPosOfMs(msOfTick(onEvent->midiTime()));
				offEvent->setWidth(width);
				onEvent->setWidth(width);
				event = onEvent;
				if (onEvent->hasBeenAdded) {
					it++;
					continue;
				}
				onEvent->hasBeenAdded = true;
			}
		}
		//event->setX(x);

		//event->setPos(qPointF(x, yPosOfLine(event->line())));

		matrixScene->addItem(event);
		it++;
	}
	//updateSceneRect(relativeRect());
}

void MatrixWidget::setFile(MidiFile *f) {

	matrixScene->clear();
	file = f;

	scaleX = 1;
	scaleY = 1;

	startTimeX = 0;
	// Roughly vertically center on Middle C.
	startLineY = 0;
	endLineY = NUM_LINES;
	endTimeX = file->maxTime();
	calcSizes();

	// scroll down to see events
	int maxNote = -1;
	velocityObjects->clear();
	currentTempoEvents->clear();
	currentTimeSignatureEvents->clear();
	currentDivs.clear();

	startTick = file->tick(startTimeX, endTimeX, &currentTempoEvents,
								  &endTick, &msOfFirstEventInList);

	TempoChangeEvent *ev = qobject_cast<TempoChangeEvent *>(currentTempoEvents->at(
										0));
	if (!ev) {
		return;
	}

	// paint measures and timeline
	currentTimeSignatureEvents = new QList<TimeSignatureEvent *>;
	int measure = file->measure(0, file->endTick(), &currentTimeSignatureEvents);

	TimeSignatureEvent *currentEvent = currentTimeSignatureEvents->at(0);
	if (!currentEvent)
		return;
	int i = 0;
	int tick = currentEvent->midiTime();
	while (tick + currentEvent->ticksPerMeasure() <= 0) {
		tick += currentEvent->ticksPerMeasure();
	}
	qreal xfrom, xDiv, metronomeDiv;
	int measureStartTick, ticksPerDiv, startTickDiv, divTick;
	QPen oldPen, dashPen;
	dashPen = QPen(Qt::lightGray, 1, Qt::DashLine);
	dashPen.setCosmetic(true);
	setOptimizationFlag(DontSavePainterState, false);
	while (tick < file->endTick()) {
		TimeSignatureEvent *measureEvent = currentTimeSignatureEvents->at(i);
		xfrom = xPosOfMs(msOfTick(tick));
		currentDivs.append(QPair<qreal, int>(xfrom, tick));
		measure++;
		measureStartTick = tick;
		tick += currentEvent->ticksPerMeasure();
		if (i < currentTimeSignatureEvents->length() - 1) {
			if (currentTimeSignatureEvents->at(i + 1)->midiTime() <= tick) {
				currentEvent = currentTimeSignatureEvents->at(i + 1);
				tick = currentEvent->midiTime();
				i++;
			}
		}
		// draw measures
		if (_div >= 0) {
			metronomeDiv = 4 / qPow(2, _div);
			ticksPerDiv = metronomeDiv * file->ticksPerQuarter();
			startTickDiv = ticksPerDiv;

			while (startTickDiv <= measureEvent->ticksPerMeasure()) {
				divTick = startTickDiv + measureStartTick;
				xDiv = xPosOfMs(msOfTick(divTick));
				currentDivs.append(QPair<qreal, int>(xDiv, divTick));
				matrixScene->addLine(QLineF(xDiv, 0, xDiv, sceneRect().height()), dashPen);
				startTickDiv += ticksPerDiv;
			}
		}
	}
	setOptimizationFlag(DontSavePainterState, true);

	for (int channel = 0; channel < 18; channel++) {

		QMultiMap<int, MidiEvent *> *map = file->channelEvents(channel);

		QMap<int, MidiEvent *>::iterator it = map->lowerBound(0);
		while (it != map->end()) {
			NoteOnEvent *onev = qobject_cast<NoteOnEvent *>(it.value());
			if (onev && eventInWidget(onev)) {
				if (onev->line() < maxNote || maxNote < 0) {
					maxNote = onev->line();
				}
			}
			it++;
		}
		addChannel(channel);
	}

	if (maxNote - 5 > 0) {
		// startLineY = maxNote - 5.0;
	}

}

void MatrixWidget::setPianoWidget(PianoWidget *widget) {
	pianoWidget = widget;
	pianoWidget->setGeometry(0, 50, 110, qRound(sceneRect().height() - 50));

}

void MatrixWidget::setTimelineWidget(TimelineWidget *widget) {
	timelineWidget = widget;
	timelineWidget->setGeometry(110, 0, qRound(sceneRect().width() - 110), 50);

}

void MatrixWidget::calcSizes() {
	if (!file) {
		return;
	}


	qreal widthNew = (endTimeX / 1000) * (PIXEL_PER_S);
	QRectF rect = sceneRect();

	rect.setWidth(qMax(widthNew, qreal(viewport()->width())));
	setSceneRect(rect);
	resetMatrix();
	scale(scaleX, scaleY);

	ToolArea = QRectF(parentWidget()->geometry());
	PianoArea = qRectF(0, timeHeight, lineNameWidth, height() - timeHeight);
	TimeLineArea = qRectF(lineNameWidth, 0, width() - lineNameWidth, timeHeight);
	//timelineWidget->setGeometry(qMax(qRound(relativeRect().x()), 110), 0, qRound(sceneRect().width() - 110), 50);
	//pianoWidget->setGeometry(0, qMax(qRound(relativeRect().y()), 50), 110, qRound(sceneRect().height() - 50));

}


MidiFile *MatrixWidget::midiFile() {
	return file;
}

void MatrixWidget::mouseMoveEvent(QMouseEvent *event) {
	QGraphicsView::mouseMoveEvent(event);
	//if (scrolling) {
	//	return;
	//}
//	if (!enabled) {
//		return;
//	}

	if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {
		Tool::currentTool()->move(event->localPos().x(), event->localPos().y());
//		if (mousePressed) {
//			update(relativeRect());
//		}
	}
}

void MatrixWidget::resizeEvent(QResizeEvent *event) {
	QGraphicsView::resizeEvent(event);
	if (pianoWidget) {
		pianoWidget->setGeometry(0, 50, 110, sceneRect().height() - 50);
	}
	if (timelineWidget) {
		timelineWidget->setGeometry(110, 0, sceneRect().width() - 110, 50);
	}
	calcSizes();
}

qreal MatrixWidget::xPosOfMs(qreal ms) {
	return ms * sceneRect().width() / qAbs(endTimeX - startTimeX);
}

qreal MatrixWidget::yPosOfLine(int line) {
	return (line - startLineY) * lineHeight();
}

qreal MatrixWidget::lineHeight() {
	if (qFuzzyIsNull(endLineY - startLineY)) {
		return 0;
	}
	return (sceneRect().height() - timeHeight) / (endLineY - startLineY);
}

void MatrixWidget::enterEvent(QEvent *event) {
	QGraphicsView::enterEvent(event);
	_mouseInWidget = false;
	if (Tool::currentTool()) {
		Tool::currentTool()->enter();
//		if (enabled) {
//			update(relativeRect());
//		}
	}
}
void MatrixWidget::leaveEvent(QEvent *event) {
	QGraphicsView::leaveEvent(event);
	_mouseInWidget = false;
	if (Tool::currentTool()) {
		Tool::currentTool()->exit();
//		if (enabled) {
//			update(relativeRect());
//		}
	}
}
bool MatrixWidget::mouseInWidget() {
	return _mouseInWidget;
}
void MatrixWidget::mousePressEvent(QMouseEvent *event) {
	QGraphicsView::mousePressEvent(event);

	if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {
		if (Tool::currentTool()->press(event->buttons() == Qt::LeftButton)) {
//			if (enabled) {
//				update(relativeRect());
//			}
		}
	}
}
void MatrixWidget::mouseReleaseEvent(QMouseEvent *event) {
	QGraphicsView::mouseReleaseEvent(event);

	if (!MidiPlayer::instance()->isPlaying() && Tool::currentTool()) {
		if (Tool::currentTool()->release()) {
//				if (enabled) {
//					update();
//				}
		}
	} else if (Tool::currentTool()) {
		if (Tool::currentTool()->releaseOnly()) {
//				if (enabled) {
//					update();
//				}
		}
	}
	//emit objectListChanged();

}

void MatrixWidget::takeKeyPressEvent(QKeyEvent *event) {
	if (Tool::currentTool()) {
		if (Tool::currentTool()->pressKey(event->key())) {
//			update();
		}
	}
}

void MatrixWidget::takeKeyReleaseEvent(QKeyEvent *event) {
	if (Tool::currentTool()) {
		if (Tool::currentTool()->releaseKey(event->key())) {
//			update();
		}
	}
	//emit objectListChanged();

}

QList<MidiEvent *> *MatrixWidget::velocityEvents() {
	return velocityObjects;
}

int MatrixWidget::msOfXPos(qreal x) {
	return qRound(startTimeX + (x * (endTimeX - startTimeX)) / sceneRect().width());
}

int MatrixWidget::msOfTick(int tick) {
	return file->msOfTick(tick, currentTempoEvents, msOfFirstEventInList);
}

int MatrixWidget::timeMsOfWidth(int w) {
	return (w * (endTimeX - startTimeX)) / sceneRect().width() ;
}

bool MatrixWidget::eventInWidget(MidiEvent *event) {
	NoteOnEvent *on = qobject_cast<NoteOnEvent *>(event);
	OffEvent *off = qobject_cast<OffEvent *>(event);
	if (on) {
		off = on->offEvent();
	} else if (off) {
		on = qobject_cast<NoteOnEvent *>(off->onEvent());
	}
	if (on && off) {
		int line = off->line();
		int tick = off->midiTime();
		bool offIn =  line >= startLineY && line <= endLineY && tick >= startTick &&
						  tick <= endTick;
		line = on->line();
		tick = on->midiTime();
		bool onIn = line >= startLineY && line <= endLineY && tick >= startTick &&
						tick <= endTick;

		off->setShown(offIn);
		on->setShown(onIn);

		return offIn || onIn;

	} else {
		int line = event->line();
		int tick = event->midiTime();
		bool shown = line >= startLineY && line <= endLineY && tick >= startTick &&
						 tick <= endTick;
		event->setShown(shown);

		return shown;
	}
}

int MatrixWidget::lineAtY(qreal y) {
	return qRound(y / lineHeight() + startLineY);
}

void MatrixWidget::zoomStd() {
	scaleX = 1;
	scaleY = 1;
	calcSizes();
}

void MatrixWidget::zoomHorIn() {
	if (scaleX < MAX_HORIZ_ZOOM) {
		scaleX += 0.1;
		calcSizes();
	}
}

void MatrixWidget::zoomHorOut() {
	if (sceneRect().width() * (scaleX - 0.1) >= relativeRect().width() && scaleX >= 0.2) {
		scaleX -= 0.1;
		calcSizes();
	}
}

void MatrixWidget::zoomVerIn() {
	if (scaleY < MAX_VERT_ZOOM) {
		scaleY += 0.1;
		calcSizes();
	}
}

void MatrixWidget::zoomVerOut() {
	if (sceneRect().height() * (scaleY - 0.1) >= relativeRect().width() && scaleY >= 0.2) {
		scaleY -= 0.1;
		calcSizes();

	}
}

int MatrixWidget::minVisibleMidiTime() {
	return startTick;
}

int MatrixWidget::maxVisibleMidiTime() {
	return endTick;
}
void MatrixWidget::keyPressEvent(QKeyEvent *event) {
	QGraphicsView::keyPressEvent(event);
	takeKeyPressEvent(event);
}

void MatrixWidget::keyReleaseEvent(QKeyEvent *event) {
	QGraphicsView::keyReleaseEvent(event);
	takeKeyReleaseEvent(event);
}

void MatrixWidget::setColorsByChannel() {
	_colorsByChannels = true;
}
void MatrixWidget::setColorsByTracks() {
	_colorsByChannels = false;
}

bool MatrixWidget::colorsByChannel() {
	return _colorsByChannels;
}

void MatrixWidget::setDiv(int div) {
	_div = div;
	redraw();
}

QList<QPair<qreal, int> > MatrixWidget::divs() {
	return currentDivs;
}

int MatrixWidget::div() {
	return _div;
}
void MatrixWidget::redraw() {
}
