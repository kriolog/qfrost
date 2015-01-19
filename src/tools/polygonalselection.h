/*
 * Copyright (C) 2011-2015  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_POLYGONALSELECTION_H
#define QFGUI_POLYGONALSELECTION_H

#include <tools/tool.h>
#include <graphicsviews/nonscalableitem.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(PointOnBoundaryPolygon)

class SmallCircle: public NonScalableItem
{
public:
    SmallCircle(QGraphicsItem *parent): NonScalableItem(parent) { }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

};

class PolygonalSelection: public Tool
{
    Q_OBJECT
public:
    PolygonalSelection();

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void clear() {
        prepareGeometryChange();
        mPolygon.clear();
    }


    void apply(bool alt) {
        Q_UNUSED(alt)
        /* Для выделения нет понятия "применить".
         * Точнее, оно применяется сразу после остановки изменений. */
    }

    QRectF boundingRect() const {
        return mPolygon.boundingRect();
    }

    const QPolygonF &polygon() const {
        return mPolygon;
    }

    void cancelLastChange();

    QPointF visualCenter() const {
        return mPolygon.isEmpty() ? QFrost::noPoint : Tool::visualCenter();
    }

public slots:
    void addPoint(const QPointF &point);
    void addPoint(const PointOnBoundaryPolygon &point);

protected:
    void onSceneHasChanged();
    void beforeSceneChange();

private:
    /**
     * Выделяет блоки в сцене. Блок считается выбраным, когда хотя бы три из его
     * пяти контрольных точек (четыре вершины и центр) принадлежат полигону.
     */
    void selectBlocks();
    QPolygonF mPolygon;
    SmallCircle *mCircle;
};

}

#endif // QFGUI_POLYGONALSELECTION_H
