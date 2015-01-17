/*
 * Copyright (C) 2010-2015  Denis Pesotsky, Maxim Torgonsky
 *
 * This file is part of QFrost.
 *
 * QFrost is free software: you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "colorgenerator.h"

#include <QtCore/QtGlobal>
#include <QtCore/QPointF>
#include <QtCore/QLineF>
#include <QtCore/qmath.h>

#include <QtGui/QColor>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>
#include <QtGui/QStaticText>

#include <core/soilblock.h>

using namespace qfgui;

ColorGenerator::ColorGenerator(QObject *parent)
    : QObject(parent)
    , mDiscretizeColors(true)
    , mT1(2)
    , mT2_1(5)
    , mT2(10)
    , mT3(20)
{
}

QColor ColorGenerator::colorFromTemperature(const qfcore::SoilBlock &soilBlock) const
{
    return colorFromTemperature(soilBlock.temperature());
}

QColor ColorGenerator::colorFromTemperatureDiff(const qfcore::SoilBlock &soilBlock) const
{
    return colorFromTemperature(soilBlock.temperature() - soilBlock.transitionTemperature());
}

QColor ColorGenerator::colorFromTemperature(double t) const
{
    /*
     * Для положительных:
     *      0       ->      mT1    ->     mT2   ->    mT3+
     * 255,255,255  ->  255,255,0  ->   255,0,0 ->  127,0,0
     * То есть:
     *  <=mT1:    255      255   255..0
     *  <=mT2:    255    255..0     0
     *  <=mT3:  255..127    0       0
     * Для отрицательных меняются местами красная и синяя составляющие.
     */

    if (t == 0.0) {
        return QColor(255, 255, 255);
    }

    const bool temperatureIsPositive = (t > 0);
    if (!temperatureIsPositive) {
        t = -t;
    }

    int a = 255;
    int b = 255;
    int c = 255;

    if (t <= mT1) {
        if (!mDiscretizeColors) {
            c = 255 - qRound(255.0 * t / mT1);
        } else {
            c = 0;
        }
    } else {
        c = 0;
        if (t <= mT2) {
            if (!mDiscretizeColors) {
                b = 255 - qRound(255.0 * (t - mT1) / (mT2 - mT1));
            } else {
                b = (t <= mT2_1) ? 127 : 0;
            }
        } else {
            b = 0;
            if (t <= mT3) {
                if (!mDiscretizeColors) {
                    a = 255 - qRound(128.0 * (t - mT2) / (mT3 - mT2));
                } else {
                    a = 192;
                }
            } else {
                a = 127;
            }
        }
    }

    Q_ASSERT(a >= 0 && a <= 255);
    Q_ASSERT(b >= 0 && b <= 255);
    Q_ASSERT(c >= 0 && c <= 255);

    return QColor(temperatureIsPositive ? a : c,
                  b,
                  temperatureIsPositive ? c : a);
}

QColor ColorGenerator::colorFromThawedPart(const qfcore::SoilBlock &soilBlock) const
{
    int green;
    double thawed_part = soilBlock.thawedPart();
    if (mDiscretizeColors) {
        if (thawed_part != 0.0 && thawed_part != 1.0) {
            /// Количество делений, помимо v=1 и v=0
            static const double n = 2;
            thawed_part = (static_cast<double>(qFloor(thawed_part * n)) + 1.0) / (n + 1);
            // Получается число в диапазоне 1/(n+1)..n/(n+1) с целым числителем
        }
    }
    green = 100 + qRound(thawed_part * 155.0);
    return QColor(0, green, 0);
}

void ColorGenerator::setDiscretizeColors(bool discretizeColors)
{
    if (mDiscretizeColors != discretizeColors) {
        mDiscretizeColors = discretizeColors;
        emit changed(true);
    }
}

void ColorGenerator::drawTemperatureLegend(QPainter *painter, const QRect &rect, bool useDarkPen, const QString &zeroLabel) const
{
    static const int penWidth = 3;
    QRectF scaleRect = rect;
    scaleRect.adjust(qFloor(double(penWidth) / 2.0), qFloor(double(penWidth) / 2.0),
                     -qCeil(double(penWidth) / 2.0), -qCeil(double(penWidth) / 2.0));

    QRectF barRect = scaleRect;
    barRect.setWidth(16);
    /// Длина стрелок в пикселах
    const double cap = qRound(barRect.width() / qSqrt(2));
    const double k = 1.0 / (2.0 * mT3);

    QGradientStops stops;
    if (!mDiscretizeColors) {
        stops << qMakePair(0.0,               QColor(0, 0, 127))
              << qMakePair(k * (mT3 - mT2),   QColor(0, 0, 255))
              << qMakePair(k * (mT3 - mT1),   QColor(0, 255, 255))
              << qMakePair(0.5,               QColor(255, 255, 255));
    } else {
        stops << qMakePair(0.0,               QColor(0, 0, 127))
              << qMakePair(k * (mT3 - mT2),   QColor(0, 0, 192))
              << qMakePair(k * (mT3 - mT2_1), QColor(0, 0, 255))
              << qMakePair(k * (mT3 - mT1),   QColor(0, 127, 255))
              << qMakePair(0.5,               QColor(0, 255, 255));
    }

    QGradientStops positiveStops;
    QGradientStops::ConstIterator i = stops.constEnd() - (mDiscretizeColors ? 1 : 2);
    for (; i >= stops.constBegin(); --i) {
        Q_ASSERT(i->first <= 0.5);
        const QColor &c = i->second;
        positiveStops << qMakePair(1.0 - i->first,
                                   QColor(c.blue(), c.green(), c.red()));
    }
    stops << positiveStops;

    QPolygonF p;
    p << QPointF(barRect.left(), barRect.top() + cap)
      << QPointF(barRect.center().x(), barRect.top())
      << QPointF(barRect.right(), barRect.top() + cap)
      << QPointF(barRect.right(), barRect.bottom() - cap)
      << QPointF(barRect.center().x(), barRect.bottom())
      << QPointF(barRect.left(), barRect.bottom() - cap);

    painter->save();
    Q_ASSERT(painter->testRenderHint(QPainter::Antialiasing) == false);
    if (mDiscretizeColors) {
        QPainterPath clipPath;
        clipPath.addPolygon(p);
        painter->setClipPath(clipPath);

        QRectF tRect = barRect;
        bool isInFirstHalf = true;
        for (i = stops.constBegin(); i != stops.constEnd(); ++i) {
            if (isInFirstHalf) {
                painter->fillRect(tRect, i->second);
                if ((i + 1)->first > 0.5) {
                    isInFirstHalf = false;
                }
            }
            tRect.setBottom(qRound(barRect.bottom() - cap -
                                   i->first * (barRect.height() - 2.0 * cap)));
            if (!isInFirstHalf) {
                painter->fillRect(tRect, i->second);
            }
        }

        painter->setPen(QPen(Qt::white, penWidth, Qt::SolidLine, Qt::FlatCap));
        painter->drawLine(QLineF(barRect.left(), barRect.center().y(),
                                 barRect.right(), barRect.center().y()));
        painter->setClipPath(QPainterPath(), Qt::NoClip);
        painter->setBrush(Qt::NoBrush);
    } else {
        QLinearGradient gradient(barRect.left(), barRect.bottom() - cap,
                                 barRect.left(), barRect.top() + cap);
        gradient.setStops(stops);
        painter->setBrush(gradient);
    }
    const QColor bordersAndTextColor = useDarkPen ? Qt::black : Qt::lightGray;
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(bordersAndTextColor, 2));
    painter->drawPolygon(p);
    // FIXME: для светлой обводки надо делать и тёмную (как у текста)

    painter->setRenderHint(QPainter::Antialiasing, false);
    bool zeroIsDrawn = false;

    const int textX = barRect.right() + penWidth + 1;
    foreach(const QGradientStop & s, stops) {
        qreal y;
        QString t;
        bool mustUseStaticText = false;
        if (s.first == 0.5) {
            if (zeroIsDrawn) {
                continue;
            } else {
                zeroIsDrawn = true;
            }
            y = barRect.center().y();
            t = zeroLabel;
            mustUseStaticText = (zeroLabel != "0");
        } else {
            y = qRound(barRect.bottom() - cap - s.first * (barRect.height() - 2.0 * cap));
            double temp = s.first / k - mT3;
            t = QString::number(temp);
            if (!zeroIsDrawn) {
                y -= 1;
            }
        }

        const QStaticText staticText(t);

        const double textYDelta = double(mustUseStaticText
                                         ? staticText.size().height()
                                         : painter->fontMetrics().tightBoundingRect(t).height())
                                  / 2.0;

        const int textY = qRound(y + (mustUseStaticText ? -textYDelta : textYDelta));
        if (!useDarkPen) {
            painter->setPen(Qt::black);
            QList<QPoint> shadowPoints;
            shadowPoints << QPoint(textX, textY)
                         << QPoint(textX + 1, textY)
                         << QPoint(textX - 1, textY)
                         << QPoint(textX, textY - 1)
                         << QPoint(textX, textY + 1);
            if (mustUseStaticText) {
                foreach(const QPoint &p, shadowPoints) {
                    painter->drawStaticText(p, staticText);
                }
            } else {
                foreach(const QPoint &p, shadowPoints) {
                    painter->drawText(p, t);
                }
            }
        }
        painter->setPen(bordersAndTextColor);
        painter->drawLine(barRect.right() - 7, y,
                          barRect.right(), y);
        if (mustUseStaticText) {
            painter->drawStaticText(QPoint(textX, textY), staticText);
        } else {
            painter->drawText(QPoint(textX, textY), t);
        }
    }

    painter->restore();
}

void ColorGenerator::drawThawedPartLegend(QPainter *painter,
        const QRectF &rect,
        bool useDarkPen) const
{
    if (rect.isNull()) {
        return;
    }
    static const int penWidth = 3;
    QRectF scaleRect = rect;
    scaleRect.adjust(qFloor(double(penWidth) / 2.0), qFloor(double(penWidth) / 2.0),
                     -qCeil(double(penWidth) / 2.0), -qCeil(double(penWidth) / 2.0));
    /// Длина стрелок в пикселах
    const double cap = qRound(scaleRect.width() / qSqrt(2));

    QLinearGradient gradient(scaleRect.left(), scaleRect.bottom() - cap,
                             scaleRect.left(), scaleRect.top() + cap);

    QGradientStops stops;
    stops << qMakePair(0.0, QColor(0, 100, 0))
          << qMakePair(1.0, QColor(0, 255, 0));
    gradient.setStops(stops);
    painter->save();
    painter->setBrush(gradient);
    painter->setPen(QPen(useDarkPen ? Qt::black : Qt::lightGray, 2));
    painter->drawRect(scaleRect);
    painter->restore();
}
