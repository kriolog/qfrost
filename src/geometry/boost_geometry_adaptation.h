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

#ifndef QFGUI_BOOST_GEOMETRY_ADAPTATION_H
#define QFGUI_BOOST_GEOMETRY_ADAPTATION_H

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>

#include <graphicsviews/boundarypolygon.h>

BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(QPointF, qreal, cs::cartesian, x, y, setX, setY)
BOOST_GEOMETRY_REGISTER_RING(QPolygonF)

/*struct QFPolygon {
    QFPolygon(const qfgui::BoundaryPolygon *p) {
        Q_ASSERT(p->isOuter());
        boundary = p->polygon();
        foreach(qfgui::BoundaryPolygon * innerPolygon, p->childBoundaryPolygonItems()) {
            holes.pushBack(innerPolygon->polygon());
        }
        boost::geometry::correct(*this);
    }
    QPolygonF boundary;
    std::vector<QPolygonF> holes;
};

namespace boost
{
namespace geometry
{
namespace traits
{
template<> struct tag<QFPolygon> {
    typedef polygonTag type;
};
template<> struct ringType<QFPolygon> {
    typedef QPolygonF type;
};
template<> struct interiorType<QFPolygon> {
    typedef std::vector<QPolygonF> type;
};
template<> struct exteriorRing<QFPolygon> {
    static QPolygonF &get(QFPolygon &p) {
        return p.boundary;
    }
    static QPolygonF const &get(QFPolygon const &p) {
        return p.boundary;
    }
};
template<> struct interiorRings<QFPolygon> {
    typedef std::vector<QPolygonF> holesType;
    static holesType &get(QFPolygon &p) {
        return p.holes;
    }
    static holesType const &get(QFPolygon const &p) {
        return p.holes;
    }
};
}
}
}*/

////////////////////////////////////////////////////////////////////////////////

#include <boost/geometry/geometries/register/box.hpp>

namespace boost
{
namespace geometry
{
namespace traits
{
template <> struct tag<QRectF> {
    typedef box_tag type;
};
template <> struct point_type<QRectF> {
    typedef QPointF type;
};
template <std::size_t C, std::size_t D>
struct indexed_access<QRectF, C, D> {
    static inline qreal get(const QRectF &qr) {
        return   C == min_corner && D == 0 ? qr.left()
                 : C == min_corner && D == 1 ? qr.top()
                 : C == max_corner && D == 0 ? qr.right()
                 : C == max_corner && D == 1 ? qr.bottom()
                 : 0;
    }
    static inline void set(QRectF &qr, const qreal &value) {
        if (C == min_corner && D == 0) {
            qr.setX(value);
        } else if (C == min_corner && D == 1) {
            qr.setY(value);
        } else if (C == max_corner && D == 0) {
            qr.setWidth(value);
        } else if (C == max_corner && D == 1) {
            qr.setHeight(value);
        }
    }
};
}
}
}

#endif // QFGUI_BOOST_GEOMETRY_ADAPTATION_H
