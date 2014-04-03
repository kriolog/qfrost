/*
 * Copyright (C) 2011-2012  Maxim Torgonsky
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

#ifndef QFGUI_POLYLINE_H
#define QFGUI_POLYLINE_H

#include <nonscalableitem.h>

namespace qfgui
{

/// Полилиния с немасштабируемой шириной
class Polyline:  public NonScalableItem
{
    Q_OBJECT
public:
    Polyline(QGraphicsItem *parent): NonScalableItem(parent),
        mPolyline() {
    }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void updatePolyline(const QPolygonF &polyline);

    void updatePolyline(const QPointF &onlyPointPolyline);

protected:
    QRectF baseRect() const {
        return mPolyline.boundingRect();
    }

private:
    QPolygonF mPolyline;
};

}

#endif // QFGUI_POLYLINE_H
