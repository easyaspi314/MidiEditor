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

#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QStringList>
#include <QStringBuilder>

#include <QRectF>
#include <QPointF>
#include <QLineF>
#include <QPolygonF>
#include <QtMath>

/*
 * Because I hate "char" when it has nothing to do with letters,
 * and MinGW hates qint8(0).
 */
using byte = signed char;
using ubyte = unsigned char;

/*
 * long long is too long long.
 */
typedef long long int64;
typedef unsigned long long uint64;

/*
 * A cleaner version of some of the ugly Q_WHATEVER names.
 */
#define qoverride Q_DECL_OVERRIDE
#define qnullptr Q_NULLPTR

/*
 * Clean up the new signal/slot syntax when there is an overload.
 *
 * x = class name
 * y = method name
 * z = parameter type
 *
 * Example:
 *     SIGNAL_OL(QSpinBox, valueChanged, int)
 * would expand to
 *     static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged)
 * or the equivalent to
 *     SIGNAL(valueChanged(int))
 * with the slower string syntax.
 */
#define SIGNAL_OL(x, y, ...) static_cast<void (x::*)(__VA_ARGS__)>(&x::y)

/*
 * Wrap literal strings in _() for short for QStringLiteral.
 *
 * Use it when passing C strings to places where the argument is a
 * QString, as otherwise, QString allocates memory for a wrapper.
 *
 * So for example, instead of
 *     timelineScrollBar->setStyleSheet("QScrollBar{height:0px}");
 * which would transform to
 *     timelineScrollBar->setStyleSheet(QString("QScrollBar{height:0px}"));
 * which allocates extra memory, use
 *     timelineScrollBar->setStyleSheet("QScrollBar{height:0px}");
 * which expands to
 *     timelineScrollBar->setStyleSheet(QStringLiteral("QScrollBar{height:0px}"));
 * and there is no extra memory allocation.
 *
 * There are some times when using QLatin1String is better, mainly in stuff like
 *     if (someString == "this")
 * or
 *     someString.contains("something");
 * so use the next macro for that.
 */
#define _(x) QStringLiteral(x)

/*
 * Sometimes, it is better to use QLatin1String, as QStringLiteral requires
 * you to store the text in UTF-32.
 *
 * For that, we can wrap it in L1().
 */
#define L1(x) QLatin1String(x)

/*
 * The same for QLatin1Char.
 */
#define L1C(x) QLatin1Char(x)

inline ushort qRoundUShort(qreal val, bool fullRange = false) {
    return val >= 0.0 ?                        // if (val >= 0.0) {
                fullRange ?                    //     if (fullRange) {
                    ushort(val + 0.5)          //         return ushort(val + 0.5);
                :                              //     } else {
                    ushort(val + 0.5) & 0x3FFF //         return ushort(val + 0.5) & 0x7F;
                                               //     }
               :                               // }
            0;                                 // return 0;
}

inline ubyte qRoundUByte(qreal val, bool fullRange = false) {
    return val >= 0.0 ?                       // if (val >= 0.0) {
                fullRange ?                   //     if (fullRange) {
                    ubyte(val + 0.5)          //         return ubyte(val + 0.5);
                    :                         //     } else {
                    ubyte(val + 0.5) & 0x7F   //         return ubyte(val + 0.5) & 0x7F;
                                              //     }
               :                              // }
            0;                                // return 0;
}

inline ubyte qRoundUByte(float val, bool fullRange = false) {
    return val >= 0.0f ?                      // if (val >= 0.0) {
                fullRange ?                   //     if (fullRange) {
                    ubyte(val + 0.5f)         //         return ubyte(val + 0.5f);
                    :                         //     } else {
                    ubyte(val + 0.5f) & 0x7F  //         return ubyte(val + 0.5f) & 0x7F;
                                              //     }
               :                              // }
            0;                                // return 0;
}
/*
 * A class representing a standard one-byte single value.
 * Midi values go from 0-127, but everything is stored
 * as unsigned.
 */
class MidiValue {
    private:
        ubyte _value : 7;
    public:
        inline MidiValue(int val) { _value = ubyte(val & 0x7F); }
        inline MidiValue(uint val) { _value = ubyte(val & 0x7F); }
        inline MidiValue(qreal val) { _value = qRoundUByte(val); }
        inline MidiValue &operator=(int val) {
            _value = val & 0x7F;
            return *this;
        }
        inline MidiValue &operator=(uint val) {
            _value = val & 0x7F;
            return *this;
        }
        inline MidiValue &operator=(qreal val) {
            _value = qRoundUByte(val);
            return *this;
        }
        inline operator ubyte() const { return _value; }
        inline ubyte value() const { return _value; }

};

/*
 * A template for a value split in one or two bytes.
 */
template<typename T>
class BaseValuePair {
    protected:
        constexpr static ubyte valueSize()  {
            return (sizeof(T) == 2) ? 14 : 8;
        }
        constexpr static ubyte splitValueSize()  {
            return valueSize() / 2;
        }
        constexpr static T bitMask()  {
            return (1 << valueSize()) - 1;
        }
        constexpr static ubyte splitBitMask()  {
            return (1 << splitValueSize()) - 1;
        }
        struct Data {
            T lsb : splitValueSize();
            T msb : splitValueSize();
        };
        union {
            T _value : valueSize();
            Data _data;
        };
    public:
        /*
         * Convenience stuff
         */
        BaseValuePair &operator=(T val) {
            _value = val & bitMask();
            return *this;
        }

        BaseValuePair(ubyte msb, ubyte lsb) {
            _data.msb = msb & splitBitMask();
            _data.lsb = lsb & splitBitMask();

        }
        BaseValuePair(T val) {
            _value = val & bitMask();
        }
        inline operator T() const { return _value; }
        inline bool isSingleValue() const { return _data.lsb == 0; }
        inline T value() const { return _value; }
        inline void setValue(T val) { _value = val & bitMask(); }
        inline ubyte LSB() const { return _data.lsb; }
        inline void setLSB(ubyte val) { _data.lsb = val & splitBitMask(); }
        inline ubyte MSB() const { return _data.msb; }
        inline void setMSB(ubyte val) { _data.msb = val & splitBitMask(); }
};

/*
 * Represents a value pair held in one byte,
 * 4 bits for the top, 4 bits for the bottom.
 *
 * This is mainly used for the event type.
 */
using ByteValuePair = BaseValuePair<ubyte>;

/*
 * Represents a value pair held in two bytes.
 * 7 bits for the top, 7 bits for the bottom.
 *
 * This is your standard LSB/MSB pair.
 */
using ShortValuePair = BaseValuePair<ushort>;

/*
 * Reduce the ugly casting when adding to QByteArray.
 */
inline void append(QByteArray array, ubyte data) {
    array.append(char(data));
}

/*
 * Store settings in a very compact struct. Compared to QSettings,
 * this uses very little RAM, No more than 40 bytes, excluding string
 * data.
 *
 * Note that if NO_BIT_PACK is defined, the order is different to reduce
 * padding.
 */
extern struct Settings {
    // Memory Usage
    // No remote | Remote
    // x86 | x64 | x86 | x64
    //   4 |   8 |   4 |   8
    QString start_cmd;
    //   8 |  16 |   8 |  16
    QString out_port;
    //  12 |  24 |  12 |  24
    QString in_port;
#ifdef ENABLE_REMOTE
    //  12 |  24 |  16 |  32
    QString client_ip;
    //  12 |  24 |  18 |  34
    ushort client_port;
#endif
#ifdef NO_BIT_PACK
    //  14 |  26 |  20 |  36
    ushort ticks_per_quarter;
    //  16 |  28 |  22 |  38
    ushort playbackDelay;
    //  17 |  29 |  23 |  39
    ubyte numStarts;

    //  18 |  30 |  24 |  40
    ubyte quantization;
    //  19 |  31 |  25 |  41
    ubyte div;
    //  20 |  32 |  26 |  42
    bool screen_locked;
    //  21 |  33 |  27 |  43
    bool channel_colors;
    //  22 |  34 |  28 |  44
    bool ignore_empty_port;
    //  23 |  35 |  29 |  45
    bool auto_update;

    //  24 |  36 |  30 |  46
    bool alt_stop;
    //  25 |  37 |  31 |  47
    bool antialiasing;
    //  26 |  38 |  32 |  48
    bool velocityDragging;
    //  27 |  39 |  33 |  49
    bool select_and_move;
    //  28 |  40 |  34 |  50
    bool magnet;
    //  29 |  41 |  35 |  51
    bool metronome;
    //  30 |  42 |  36 |  52
    bool thru;
    //  31 |  43 |  37 |  53
    bool gba_mode;
    // after padding:
    //  32 |  48 |  40 |  56
    // after padding:
    //  32 |  48 |  36 |  56
#else
    //  14 |  26 |  20 |  36
    ushort ticks_per_quarter : 15; // 0-7FFF    15
    bool ignore_empty_port   :  1; // bool      16

    //  16 |  28 |  22 |  38
    ushort playbackDelay     : 11; // 0-7FF     11
    ubyte numStarts          :  4; // 0-F       15
    bool auto_update         :  1; // bool      16

    //  17 |  29 |  23 |  39
    ubyte quantization       :  3; // 0-7        3
    ubyte div                :  3; // 0-7        6
    bool screen_locked       :  1; // bool       7
    bool channel_colors      :  1; // bool       8

    //  18 |  30 |  24 |  40
    bool alt_stop            :  1; // bool       1
    bool antialiasing        :  1; // bool       2
    bool velocityDragging    :  1; // bool       3
    bool select_and_move     :  1; // bool       4
    bool magnet              :  1; // bool       5
    bool metronome           :  1; // bool       6
    bool thru                :  1; // bool       7
    bool gba_mode            :  1; // bool       8
    //  after padding:
    //  20 |  32 |  24 |  40
#endif
} _settings;

#ifdef ENABLE_GBA
/*
 * Converts a linear General MIDI value to an exponential value like on the
 * GBA if gba_mode is enabled.
 */
inline ubyte toExpVal(ubyte v) {
    return (_settings.gba_mode) ? (ushort(v) * ushort(v)) / 127) : v;
}
/*
 * Converts an exponential value like on the GBA to a linear General MIDI value
 * if gba_mode is enabled.
 */
#define fromExpVal(v) ubyte((_settings.gba_mode) ? qRoundUByte(sqrt(127.0 * qreal(v))) : v)
#else
/**
 * Noop the above GBA functions.
 */
#define toExpVal(v) v
#define fromExpVal(v) v
#endif
inline QRectF qRectF(const QRectF &other) {
    if (_settings.antialiasing) {
        return other;
    } else {
        return QRectF(other.toRect());
    }
}
inline QRectF qRectF(qreal x, qreal y, qreal w, qreal h) {
    return qRectF(QRectF(x, y, w, h));
}
inline QLineF qLineF(const QLineF &other) {
    if (_settings.antialiasing) {
        return other;
    } else {
        return QLineF(other.toLine());
    }
}
inline QLineF qLineF(qreal x1, qreal y1, qreal x2, qreal y2) {
    return qLineF(QLineF(x1, y1, x2, y2));
}
inline QPolygonF qPolygonF(const QPolygonF &other) {
    if (_settings.antialiasing) {
        return other;
    } else {
        return QPolygonF(other.toPolygon());
    }
}
inline QPointF qPointF(QPointF  other) {
    if (_settings.antialiasing) {
        return other;
    } else {
        return QPointF(other.toPoint());
    }
}
inline QPointF qPointF(qreal x, qreal y) {
    return qPointF(QPointF(x, y));
}

#endif // UTILS_H
