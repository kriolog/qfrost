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

#ifndef QFGUI_BOUNDARYELLIPSECREATOR_H
#define QFGUI_BOUNDARYELLIPSECREATOR_H

#include <nonscalableitem.h>
#include <tools/rectangulartool.h>

namespace qfgui
{
/// Эллипс с неизменяемой толщиной пера.
class Ellipse: public NonScalableItem
{
public:
    Ellipse(QGraphicsItem *parent): NonScalableItem(parent, 3), mRect() {
        setFlag(ItemStacksBehindParent);
    }
    void scaleToParent() {
        setRect(parentItem()->boundingRect());
    }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
protected:
    QRectF baseRect() const {
        return mRect;
    }

private:
    void setRect(const QRectF &rect) {
        if (mRect == rect) {
            return;
        }
        prepareGeometryChange();
        mRect = rect;
    }
    QRectF mRect;
};

/// Создавалка эллиптических граничных условий
class BoundaryEllipseCreator: public RectangularTool
{
    Q_OBJECT
public:
    BoundaryEllipseCreator(ToolSettings *settings);

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void apply(bool alt);

protected:
    void onStopChange() {
        // Действия не требуется
    }

    void onSizeChange() {
        mEllipse->scaleToParent();
    }

private:
    const uint mNumberOfAngles;

    Ellipse *mEllipse;

    /**
     * Полигон, вписанный в единичную окружность с центром в (0,0), имеющий
     * @a mNumberOfAngles углов.
     */
    QPolygonF unitRoundPolygon() const;

    QPolygonF ellipseShapedPolygon() const;
};
}

#endif // QFGUI_BOUNDARYELLIPSECREATOR_H
