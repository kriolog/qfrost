/*
 * Copyright (C) 2014-2016  Denis Pesotsky
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
 * 
 */

#ifndef QFGUI_CROSS_H
#define QFGUI_CROSS_H

#include <QGraphicsItem>

namespace qfgui {

class Cross : public QGraphicsItem
{
public:
    Cross(const QColor &color,
          QGraphicsItem *parent = NULL,
          uint halfSize = 16);

    QRectF boundingRect() const;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

private:
    const QColor mColor;
    const int mHalfSize;
    QVector<QPointF> mCrossPointPairs;
};
}

#endif // QFGUI_CROSS_H
