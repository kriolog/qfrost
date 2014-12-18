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

#ifndef QFGUI_ARROW_H
#define QFGUI_ARROW_H

#include <QtWidgets/QGraphicsObject>

namespace qfgui
{

class Block;

class Arrow : public QGraphicsObject
{
    Q_OBJECT
public:
    Arrow(const QGraphicsItem *startItem, const QGraphicsItem *endItem,
          const QColor &color = Qt::red);

    QRectF boundingRect() const;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

private:
    QColor mColor;
    QLineF mLine;
    static QPointF randomPointIn(const QRectF &rect);
    QPolygonF mArrowHead;
    static const int kmArrowSize = 20;
};

}

#endif // QFGUI_ARROW_H
