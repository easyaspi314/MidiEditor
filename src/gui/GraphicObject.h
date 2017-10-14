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

#ifndef GRAPHICOBJECT_H_
#define GRAPHICOBJECT_H_

#include <QPainter>
#include <QColor>
#include <QGraphicsRectItem>
class QGraphicsItem;

class GraphicObject : public QGraphicsRectItem {


	public:
		GraphicObject(QGraphicsItem *parent = Q_NULLPTR);

/*		qreal x();
		qreal y();


		void setX(qreal x);
		void setY(qreal y);*/
		void setWidth(qreal w);
		void setHeight(qreal h);
		qreal width() const;
		qreal height() const;
		QRectF boundingRect() const Q_DECL_OVERRIDE;

		virtual void paint(QPainter *painter,
			   const QStyleOptionGraphicsItem *option,
			   QWidget *widget = Q_NULLPTR) Q_DECL_OVERRIDE;

		bool shown();
		void setShown(bool b);


	private:
		qreal _x, _y, _width, _height;
		bool shownInWidget;
};

#endif
