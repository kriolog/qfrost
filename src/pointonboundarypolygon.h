/*
 * Copyright (C) 2011-2014  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_POINTONBOUNDARYPOLYGON_H
#define QFGUI_POINTONBOUNDARYPOLYGON_H

#include <QtCore/QLineF>

#include "qfrost.h"

namespace qfgui
{

/**
 * Точка на полигоне граничных условий,представленная в терминах самого
 * полигона (номер угла - расстояние до следующего угла).
 */
class PointOnBoundaryPolygon
{
public:
    PointOnBoundaryPolygon(const BoundaryPolygon *polygon,
                           int cornerNumber,
                           qreal distance);

    PointOnBoundaryPolygon();

    const BoundaryPolygon *polygon() const {
        return mPolygon;
    }

    QPointF toPoint() const;

    QLineF segment() const;

    int index() const {
        return mCornerNumber;
    }

    qreal distance() const {
        return mDistance;
    }

    bool isNull() const {
        return mPolygon == NULL;
    }

    /// Находится ли этот сегмент на эллипсе (или куске эллипса)
    bool isInEllipse() const;

private:
    const BoundaryPolygon *mPolygon;
    int mCornerNumber;
    qreal mDistance;
};

}

#endif // QFGUI_POINTONBOUNDARYPOLYGON_H
