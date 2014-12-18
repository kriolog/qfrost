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

#ifndef QFGUI_GROWINGPOLYGON_H
#define QFGUI_GROWINGPOLYGON_H

#include <graphicsviews/nonscalableitem.h>
#include <qfrost.h>

namespace qfgui
{

/**
 * Полигон, который можно "наращивать" по одной точке.
 */
class GrowingPolygon: public NonScalableItem
{
    Q_OBJECT
public:
    GrowingPolygon(QGraphicsItem *parent):
        NonScalableItem(parent),
        mPolygon(),
        mIsSimple(false) { }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void addPoint(const QPointF &point);

    QPolygonF polygon() {
        return mPolygon;
    }

    void clear() {
        prepareGeometryChange();
        mPolygon.clear();
        mIsSimple = false;
    }

    bool isSimple() {
        return mIsSimple;
    }

    void deleteLastPoint();

protected:
    QRectF baseRect() const {
        return mPolygon.boundingRect();
    }

private:
    void updateSimplicity();

    QPolygonF mPolygon;
    bool mIsSimple;
};

}

#endif // QFGUI_GROWINGPOLYGON_H
