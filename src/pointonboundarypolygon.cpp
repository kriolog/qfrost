/*
 * Copyright (C) 2011-2012  Denis Pesotsky, Maxim Torgonsky
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

#include <pointonboundarypolygon.h>

#include <boundarypolygon.h>
#include <boundarypolygoncalc.h>
#include <qfrost.h>

using namespace qfgui;

PointOnBoundaryPolygon::PointOnBoundaryPolygon(const BoundaryPolygon *polygon,
        int cornerNumber,
        qreal distance) :
    mPolygon(polygon),
    mCornerNumber(cornerNumber),
    mDistance(distance)
{
    Q_ASSERT(mCornerNumber < mPolygon->corners().size() - 1);
    Q_ASSERT(mDistance >= 0);
    Q_ASSERT(mPolygon != NULL);
    // Точка должна лежать внутри отрезка, но не на его конце
    Q_ASSERT(mPolygon->segment(cornerNumber).length() > distance);
}

PointOnBoundaryPolygon::PointOnBoundaryPolygon():
    mPolygon(NULL),
    mCornerNumber(0),
    mDistance(0)
{

}

QPointF PointOnBoundaryPolygon::toPoint() const
{
    if (mPolygon != NULL) {
        /// сторона полигона, которой пренадлежит данная точка
        QLineF side;
        side.setP1(mPolygon->corners().at(mCornerNumber).point);
        side.setP2(mPolygon->corners().at(mCornerNumber + 1).point);
        Q_ASSERT(side.length() > mDistance);
        return BoundaryPolygonCalc::pointOnSegment(side, mDistance);
    } else {
        return QFrost::noPoint;
    }
}

QLineF PointOnBoundaryPolygon::segment() const
{
    Q_ASSERT(mPolygon != NULL);
    Q_ASSERT(mCornerNumber < mPolygon->polygon().size());
    return mPolygon->segment(mCornerNumber);
}
