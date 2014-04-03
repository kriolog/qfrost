/*
 * Copyright (C) 2011-2012  Denis Pesotsky
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

#ifndef QFGUI_BLOCK_WITHIN_POLYGON_H
#define QFGUI_BLOCK_WITHIN_POLYGON_H

#include <geometry/boost_geometry_adaptation.h>

#include <block.h>

namespace qfgui
{

class BlockWithinPolygon
{
public:
    /**
     * Any of block's corners is inside outer ring of @p p (excluding borders)
     * and there is no inner ring of @p, that contains all block's corners
     * (including borders).
     */
    inline static bool blockIntersectsPolygon(const Block *block,
            const BoundaryPolygon *p) {
        Q_ASSERT(p->isOuter());
        if (!rectIntersectsPolygon(block->rect(), p->polygon())) {
            /* Временный фикс. boost::geometry::intersects возвращает true,
             * если блок хотя бы одним углом лежит на границе полигона, при этом
             * вся его площадь может находится вне полигона. Минус того, что
             * эта функция не используется - блоки в острых граничных условиях
             * (когда все точки лежат вне полигона, но площади пересекаются)
             * считаются лежащими вне полигона, но так как такие полигоны не
             * нужны для нормальных задач, то это нормально */
            //if (!boost::geometry::intersects(p->polygon(), block->rect())) {
            return false;
            //}
        }
        foreach(BoundaryPolygon * innerPolygon, p->childBoundaryPolygonItems()) {
            if (rectIsWithinPolygon(block->rect(), innerPolygon->polygon())) {
                return false;
            }
        }
        return true;
    }

    /// Each corner of rect is within polygon (or on polygon's border)
    inline static bool rectIsWithinPolygon(const QRectF &rect,
                                           const QPolygonF &polygon) {
        return  pointIsWithinPolygon(rect.topLeft(), polygon)
                && pointIsWithinPolygon(rect.topRight(), polygon)
                && pointIsWithinPolygon(rect.bottomLeft(), polygon)
                && pointIsWithinPolygon(rect.bottomRight(), polygon);
    }

private:
    /**
     * Any corner of @p rect is inside polygon (excluding borders) or at least
     * 3 corners of it are on polygon border.
     */
    inline static bool rectIntersectsPolygon(const QRectF &rect,
            const QPolygonF &polygon) {
        if (boost::geometry::within(rect.topLeft(), polygon)
                || boost::geometry::within(rect.topRight(), polygon)
                || boost::geometry::within(rect.bottomLeft(), polygon)
                || boost::geometry::within(rect.bottomRight(), polygon)) {
            return true;
        }

        return pointIsOnPolygonBorder(rect.topLeft(), polygon)
               && pointIsOnPolygonBorder(rect.topRight(), polygon)
               && pointIsOnPolygonBorder(rect.bottomLeft(), polygon)
               && pointIsOnPolygonBorder(rect.bottomRight(), polygon);
    }

    inline static bool pointIsOnSegment(const QPointF &p, const QLineF &l) {
        if (p.x() < qMin(l.p1().x(), l.p2().x())
                || p.x() > qMax(l.p1().x(), l.p2().x())
                || p.y() < qMin(l.p1().y(), l.p2().y())
                || p.y() > qMax(l.p1().y(), l.p2().y())) {
            // point is outside segment's bounding rect, it can't be on segment
            return false;
        }

        // test for collinearity
        return qFuzzyIsNull(p.x() * (l.p1().y() - l.p2().y())
                            + l.p1().x() * (l.p2().y() - p.y())
                            + l.p2().x() * (p.y() - l.p1().y()));
    }

    inline static bool pointIsOnPolygonBorder(const QPointF &point,
            const QPolygonF &polygon) {
        QPolygonF::ConstIterator it;
        for (it = polygon.constBegin(); it != polygon.constEnd() - 1; ++it) {
            if (pointIsOnSegment(point, QLineF(*it, *(it + 1)))) {
                return true;
            }
        }
        return false;
    }

    /// Point is within polygon (or on polygon's border)
    inline static bool pointIsWithinPolygon(const QPointF &point,
                                            const QPolygonF &polygon) {
        return boost::geometry::within(point, polygon)
               || pointIsOnPolygonBorder(point, polygon);
    }
};

}

#endif // QFGUI_BLOCK_WITHIN_POLYGON_H
