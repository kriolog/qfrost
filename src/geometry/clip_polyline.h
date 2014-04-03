/*
 * Copyright (C) 2011-2013  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_CLIP_POLYLINE_H
#define QFGUI_CLIP_POLYLINE_H

#include <cmath>

#include <boost/math/constants/constants.hpp>

#include <geometry/boost_geometry_adaptation.h>
#include <qfrost.h>

namespace qfgui
{

class ClipPolyline
{
private:
    /// Distance between points (x1, y1) and (x2, y2)
    inline static qreal length(qreal x1, qreal y1, qreal x2, qreal y2) {
        return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    }

    inline static QLineF clippedSegment(qreal x1, qreal y1,
                                        qreal x2, qreal y2,
                                        const QRectF &rect);

    inline static QLineF clippedSegment(const QRectF &rect, const QLineF &segment) {
        return clippedSegment(segment.x1(), segment.y1(),
                              segment.x2(), segment.y2(),
                              rect);
    }

public:
    /**
     * Длина отклипенного блоком @p block участка полилинии @p polyline
     * и "средневзвешенный" радиус.
     * Этот радиус позволяет заменить несколько площадок теплопотка (по одной
     * на каждый отклипенный отрезок) на одну эквивалентную.
     * Для горизонтальных и вертикальных отрезков величины, складывающие
     * его, берутся из соответствующих измерений блока, а для наклонных --
     * из "среднеквадратичного" размера
     * @param isAxiallySymmetric следует ли считать площади контактов для
     * осесимметричной задачи
     */
    inline static QPair<qreal, qreal> clippedParams(const Block *block,
            const QPolygonF &polyline,
            bool isAxiallySymmetric) {
        const QRectF rect = block->rect();
        // Находим отрезки полилинии, которые попали в прямоугольник
        QList<QLineF> segments;
        {
            QPolygonF::ConstIterator it;
            for (it = polyline.constBegin(); it != polyline.constEnd() - 1; ++it) {
                QLineF s = clippedSegment(rect, QLineF(*it, *(it + 1)));
                if (!s.isNull()) {
                    Q_ASSERT((s.p1() - s.p2()).manhattanLength() > 1.1 * QFrost::accuracy);
                    segments.append(s);
                }
            }
        }

        if (segments.isEmpty()) {
            return qMakePair(0.0, 0.0);
        }

        // Считаем числитель обратного "средневзвешенного" радиуса
        qreal weightedRNumerator = 0.0;
        // ... длину отклипенной полилинии
        qreal totalLength = 0.0;
        // ... и площадь отклипенного полигона (которая равна длине для обычной
        // задачи и отличается от неё для осесимметричной)
        qreal totalSquare = 0.0;

        QList<QLineF>::ConstIterator it;
        for (it = segments.constBegin(); it != segments.constEnd(); ++it) {
            const qreal length = it->length();

            const qreal a = rect.width();
            const qreal b = rect.height();
            const qreal sinus = it->dy() / length;
            const qreal cosinus = it->dx() / length;
            // Радиус вписанного в блок эллипса. Это тот радиус, который
            // упирался бы в касательную, проведённую параллельно сегменту
            const qreal radius = a * b / sqrt(a * a * cosinus * cosinus + b * b * sinus * sinus) / 2.0;

            weightedRNumerator += length / radius;
            totalLength += length;
            totalSquare += isAxiallySymmetric
                           // l*2*pi*|x цетра| = l*2*pi*|x1+x2)/2| = l*pi*|x1+x2|
                           ? length * boost::math::constants::pi<qreal>() * QFrost::meters(qAbs(it->x1() + it->x2()))
                           : length;
        }

        return qMakePair(QFrost::meters(totalSquare),
                         QFrost::meters(totalLength / weightedRNumerator));
    }

};

QLineF ClipPolyline::clippedSegment(qreal x1, qreal y1,
                                    qreal x2, qreal y2, const QRectF &rect)
{
    qreal left = rect.left();
    qreal right = rect.right();
    qreal top = rect.top();
    qreal bottom = rect.bottom();

    enum { Left, Right, Top, Bottom };
    int p1 = ((x1 < left) << Left)
             | ((x1 > right) << Right)
             | ((y1 < top) << Top)
             | ((y1 > bottom) << Bottom);
    int p2 = ((x2 < left) << Left)
             | ((x2 > right) << Right)
             | ((y2 < top) << Top)
             | ((y2 > bottom) << Bottom);

    if (!(p1 || p2)) {
        // segment is fully included
        return QLineF(x1, y1, x2, y2);
    }

    if (p1 & p2) {
        // both points are on same side
        return QLineF();
    }

    if (p1 | p2) {
        qreal dx = x2 - x1;
        qreal dy = y2 - y1;

        // clip x coordinates
        if (x1 < left) {
            y1 += dy / dx * (left - x1);
            x1 = left;
        } else if (x1 > right) {
            y1 -= dy / dx * (x1 - right);
            x1 = right;
        }
        if (x2 < left) {
            y2 += dy / dx * (left - x2);
            x2 = left;
        } else if (x2 > right) {
            y2 -= dy / dx * (x2 - right);
            x2 = right;
        }

        p1 = ((y1 < top) << Top)
             | ((y1 > bottom) << Bottom);
        p2 = ((y2 < top) << Top)
             | ((y2 > bottom) << Bottom);

        if (p1 & p2) {
            return QLineF();
        }

        // clip y coordinates
        if (y1 < top) {
            x1 += dx / dy * (top - y1);
            y1 = top;
        } else if (y1 > bottom) {
            x1 -= dx / dy * (y1 - bottom);
            y1 = bottom;
        }
        if (y2 < top) {
            x2 += dx / dy * (top - y2);
            y2 = top;
        } else if (y2 > bottom) {
            x2 -= dx / dy * (y2 - bottom);
            y2 = bottom;
        }

        p1 = ((x1 < left) << Left) | ((x1 > right) << Right);
        p2 = ((x2 < left) << Left) | ((x2 > right) << Right);

        if (p1 & p2) {
            return QLineF();
        }
        if (qAbs(x1 - x2) + qAbs(y1 - y2) > QFrost::accuracy) {
            return QLineF(x1, y1, x2, y2);
        }
    }
    return QLineF();
}

}

#endif // QFGUI_CLIP_POLYLINE_H
