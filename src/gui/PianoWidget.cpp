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

#include "PianoWidget.h"

#include <QPainter>
#include "../tool/Selection.h"
#include <QStyle>
#include <QStyleOption>

/*
 * FIXME:
 * - This is literally a static image. There is no
 *   interaction whatsoever.
 */
PianoWidget::PianoWidget(QWidget *parent) : PaintWidget(parent) {
    matrixWidget = qnullptr;
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFixedWidth(110);
    setMinimumHeight(1);
    setRepaintOnMouseMove(false);
    setRepaintOnMouseMove(false);
    setRepaintOnMouseRelease(false);
    setMouseTracking(true);
}
void PianoWidget::setMatrixWidget(MatrixWidget *widget) {
    matrixWidget = widget;

    matrixWidget->setPianoWidget(this);
    update();
}
void PianoWidget::calcSizes() {
    if (!matrixWidget) {
        return;
    }
    // Detect fading scrollbars. If we don't adjust the height for this,
    // the piano keys misalign.
    QStyleOptionSlider option = QStyleOptionSlider();
    option.initFrom(parentWidget());
    if (style()->styleHint(QStyle::SH_ScrollBar_Transient, &option)) {
        // Do nothing for fading scrollbars
        setFixedHeight(matrixWidget->height());
    } else {
        // Add the height of the scrollbar to the PianoWidget for persistet scrollbars.
        // This extra height counteracts the problem.
        setFixedHeight(matrixWidget->height() + style()->pixelMetric(QStyle::PM_ScrollBarExtent, &option));
    }
}
void PianoWidget::setFile(MidiFile *file) {
    Q_UNUSED(file)
    update();
}
QSize PianoWidget::sizeHint() const {
    return QSize(110, 150);
}
void PianoWidget::paintEvent(QPaintEvent *event) {

    if (paintingActive() || !matrixWidget || !matrixWidget->midiFile()) {
        return;
    }
    QPainter painter(this);
    QFont font = painter.font();
    font.setPixelSize(12);
    byte updatemode = 0;
    if (!event->region().isNull() && !event->region().isEmpty()) {
        updatemode = 1;
        painter.setClipping(true);
        painter.setClipRegion(event->region());
    } else if (!event->rect().isNull() && !event->rect().isEmpty()) {
        updatemode = 2;
        painter.setClipping(true);
        painter.setClipRect(event->rect());
    } else {
        painter.setClipping(true);
        painter.setClipRect(relativeRect());
    }
    QPixmap pianoPixmap;
    if (!QPixmapCache::find("PianoWidget_" % QString::number(ubyte(matrixWidget->scaleY * 10), 36), pianoPixmap)) {
        calcSizes();

        pianoPixmap = QPixmap(110, height());

        QPainter pixpainter(&pianoPixmap);
        if (_settings.antialiasing) {
            pixpainter.setRenderHint(QPainter::Antialiasing);
        }
        pixpainter.fillRect(qRectF(0, 0, 110, height()), QApplication::palette().window());
        ubyte pianoKeys = NUM_LINES;
//        if (matrixWidget->endLineY > 127) {
//            pianoKeys -= (matrixWidget->startLineY - 127);
//        }
        if (pianoKeys > 0) {
            pixpainter.fillRect(qRectF(0, 0, 100,
                                        pianoKeys * -1 * matrixWidget->lineHeight()), Qt::white);
        }
        for (ubyte i = 0; i <= NUM_LINES; i++) {
            int startLine = matrixWidget->yPosOfLine(i);
            if (i >= 0 && i <= 127) {
                paintPianoKey(&pixpainter, 127 - i, 0, startLine,
                              110, matrixWidget->lineHeight());
            } else {
                QString text = QString();
                switch (MidiEventLine(i)) {
                    case MidiEventLine::ControlChangeEventLine: {
                        text = "Control Change";
                        break;
                    }
                    case MidiEventLine::TempoChangeEventLine: {
                        text = "Tempo Change";
                        break;
                    }
                    case MidiEventLine::TimeSignatureEventLine: {
                        text = "Time Signature";
                        break;
                    }
                    case MidiEventLine::KeySignatureEventLine: {
                        text = "Key Signature.";
                        break;
                    }
                    case MidiEventLine::ProgramChangeEventLine: {
                        text = "Program Change";
                        break;
                    }
                    case MidiEventLine::KeyPressureEventLine: {
                        text = "Key Pressure";
                        break;
                    }
                    case MidiEventLine::ChannelPressureEventLine: {
                        text = "Channel Pressure";
                        break;
                    }
                    case MidiEventLine::TextEventLine: {
                        text = "Text";
                        break;
                    }
                    case MidiEventLine::PitchBendEventLine: {
                        text = "Pitch Bend";
                        break;
                    }
                    case MidiEventLine::SysExEventLine: {
                        text = "System Exclusive";
                        break;
                    }
                    case MidiEventLine::UnknownEventLine: {
                        text = "(Unknown)";
                        break;
                    }
                }
                pixpainter.setPen(Qt::darkGray);
                QFont font = pixpainter.font();
                font.setPixelSize(9);
                pixpainter.setFont(font);
                qreal textlength = QFontMetricsF(font).width(text);
                qreal textHeight = QFontMetricsF(font).height();
                // Don't draw text if zoomed too far out.
                if (textHeight < (matrixWidget->lineHeight() + 1)) {
                    pixpainter.drawText(qPointF(95 - textlength, startLine +
                                            matrixWidget->lineHeight() - 1), text);
                }
            }
        }
        pixpainter.end();
        QPixmapCache::insert("PianoWidget_" % QString::number(ubyte(matrixWidget->scaleY * 10), 36), pianoPixmap);
        QPalette palette = this->palette();
        palette.setBrush(backgroundRole(), pianoPixmap);
        setPalette(palette);
    }
    /*switch (updatemode) {
        case 1: {
            QPainter::PixmapFragment frags[event->region().rectCount()];
            QVector<QRect> rects = event->region().rects();
            for (int i = 0; i < event->region().rectCount(); i++) {
                QRect rect = rects.at(i);
                frags[i] = QPainter::PixmapFragment::create(rect.center(), rect, 1, 1, 0, 1);
            }
            painter.drawPixmapFragments(frags, event->region().rectCount(), pianoPixmap);
            break;
        }
        case 2: {
            QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
                                                event->rect().center(), event->rect());
            painter.drawPixmapFragments(&frag, 1, pianoPixmap);
            break;
        }
        default:
            QPainter::PixmapFragment frag = QPainter::PixmapFragment::create(
                                                relativeRect().center(), relativeRect());
            painter.drawPixmapFragments(&frag, 1, pianoPixmap);
    }*/
}

void PianoWidget::paintPianoKey(QPainter *painter, ubyte number, qreal x,
                                qreal y,
                                qreal width, qreal height) {
    width -= 10;
    if (number >= 0 && number <= 127) {

        qreal scaleHeightBlack = 0.5;
        qreal scaleWidthBlack = 0.6;

        PianoKeyShape shape = WhiteOnly;
        QString name;

        switch (number % 12) {
            case C: {
                shape = WhiteBelowBlack;
                short i = number / 12;
                //if (i<4) {
                //	name="C";{
                //		for (int j = 0; j<3-i; j++) {
                //			name+="'";
                //		}
                //	}
                //} else {
                //	name = "c";
                //	for (int j = 0; j<i-4; j++) {
                //		name+="'";
                //	}
                //}
                name = "C" % QString::number(i - 1);
                break;
            }
            case C_Sharp: {
                shape = Black;
                break;
            }
            case D: {
                shape = WhiteBetweenBlack;
                break;
            }
            case D_Sharp: {
                shape = Black;
                break;
            }
            case E: {
                shape = WhiteAboveBlack;
                break;
            }
            case F: {
                shape = WhiteBelowBlack;
                break;
            }
            case F_Sharp: {
                shape = Black;
                break;
            }
            // G
            case G: {
                shape = WhiteBetweenBlack;
                break;
            }
            // g#
            case G_Sharp: {
                shape = Black;
                break;
            }
            // A
            case A: {
                shape = WhiteBetweenBlack;
                break;
            }
            // A#
            case A_Sharp: {
                shape = Black;
                break;
            }
            // B
            case B: {
                shape = WhiteAboveBlack;
                break;
            }
        }

        bool selected = mouseY >= y && mouseY <= y + height && mouseX > 0
                        && mouseOver;
        for (MidiEvent *event : Selection::instance()->selectedEvents()) {
            if (event->line() == qFloor(127 - number)) {
                selected = true;
                break;
            }
        }

        QPolygonF keyPolygon;

        bool inRect = false;

        if (shape == Black) {
            painter->drawLine(qLineF(x, y + height / 2, x + width, y + height / 2));
            y += (height - height * scaleHeightBlack) / 2;
            QRectF playerRect;
            playerRect.setX(x);
            playerRect.setY(y);
            playerRect.setWidth(width * scaleWidthBlack);
            playerRect.setHeight(height * scaleHeightBlack + 0.5);
            QColor c = Qt::black;
            /*if (mouseInWidget(matrixWidget)) {
                c = QColor(200, 200, 200);
                inRect = true;
            }*/
            painter->fillRect(qRectF(playerRect), c);

            keyPolygon.append(qPointF(x, y));
            keyPolygon.append(qPointF(x, y + height * scaleHeightBlack));
            keyPolygon.append(qPointF(x + width * scaleWidthBlack,
                                      y + height * scaleHeightBlack));
            keyPolygon.append(qPointF(x + width * scaleWidthBlack, y));

        } else {

            if (shape == WhiteAboveBlack) {
                keyPolygon.append(qPointF(x, y));
                keyPolygon.append(qPointF(x + width, y));
            } else {
                keyPolygon.append(qPointF(x, y - height * scaleHeightBlack / 2));
                keyPolygon.append(qPointF(x + width * scaleWidthBlack,
                                          y - height * scaleHeightBlack / 2));
                keyPolygon.append(qPointF(x + width * scaleWidthBlack,
                                          y - height * scaleHeightBlack));
                keyPolygon.append(qPointF(x + width, y - height * scaleHeightBlack));
            }
            if (shape == WhiteBelowBlack) {
                painter->drawLine(qLineF(x, y + height, x + width, y + height));
                keyPolygon.append(qPointF(x + width, y + height));
                keyPolygon.append(qPointF(x, y + height));
            } else {
                keyPolygon.append(qPointF(x + width, y + height + height * scaleHeightBlack));
                keyPolygon.append(qPointF(x + width * scaleWidthBlack,
                                          y + height + height * scaleHeightBlack));
                keyPolygon.append(qPointF(x + width * scaleWidthBlack,
                                          y + height + height * scaleHeightBlack / 2));
                keyPolygon.append(qPointF(x, y + height + height * scaleHeightBlack / 2));
            }
            inRect = mouseInRect(x, y, width, height);
        }

        if (shape == Black) {
            if (inRect) {
                painter->setBrush(Qt::lightGray);
            } else if (selected) {
                painter->setBrush(Qt::darkGray);
            } else {
                painter->setBrush(Qt::black);
            }
        } else {
            if (inRect) {
                painter->setBrush(Qt::darkGray);
            } else if (selected) {
                painter->setBrush(Qt::lightGray);
            } else {
                painter->setBrush(Qt::white);
            }
        }
        painter->setPen(Qt::darkGray);
        painter->drawPolygon(qPolygonF(keyPolygon), Qt::OddEvenFill);


        if (!name.isEmpty()) {
            painter->setPen(Qt::gray);
            int textlength = QFontMetrics(painter->font()).width(name);
            painter->drawText(qPointF(x + width - textlength - 2, y + height - 1), name);
            painter->setPen(Qt::black);
        }
        if (inRect && enabled) {
            // mark the current Line
            QColor lineColor = QColor(0, 0, 100, 40);
            painter->fillRect(qRectF(x + width + 10, matrixWidget->yPosOfLine(127 - number),
                                     this->width() - x - width - 10, height), lineColor);
        }
    }
}

