#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include "limits.h"
#include <QRectF>
#include <QLineF>
#include <QPolygonF>
#include <QPointF>
/*
 * Because I hate "char" when it has nothing to do with letters,
 * and MinGW hates qint8(0).
 */
typedef signed char byte;
typedef unsigned char ubyte;

static bool antiAliasingEnabled = true;
static bool colorByChannel = false;

inline QRectF qRectF(QRectF other) {
	if (antiAliasingEnabled) {
		return other;
	} else {
		return QRectF(other.toRect());
	}
}
inline QRectF qRectF(qreal x, qreal y, qreal w, qreal h) {
	return qRectF(QRectF(x, y, w, h));
}
inline QLineF qLineF(QLineF other) {
	if (antiAliasingEnabled) {
		return other;
	} else {
		return QLineF(other.toLine());
	}
}
inline QLineF qLineF(qreal x1, qreal y1, qreal x2, qreal y2) {
	return qLineF(QLineF(x1, y1, x2, y2));
}
inline QPolygonF qPolygonF(QPolygonF other) {
	if (antiAliasingEnabled) {
		return other;
	} else {
		return QPolygonF(other.toPolygon());
	}
}
inline QPointF qPointF(QPointF  other) {
	if (antiAliasingEnabled) {
		return other;
	} else {
		return QPointF(other.toPoint());
	}
}
inline QPointF qPointF(qreal x, qreal y) {
	return qPointF(QPointF(x, y));
}

#endif // UTILS_H
