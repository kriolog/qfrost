/*
 * Copyright (C) 2014  Denis Pesotsky
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

#include "cross.h"

#include <QPainter>

using namespace qfgui;

Cross::Cross(QGraphicsItem *parent, uint halfSize) :
    QGraphicsItem(parent),
    mHalfSize(halfSize)
{
    setFlag(QGraphicsItem::ItemIgnoresTransformations);

    mCrossPointPairs << QPointF(-mHalfSize, 0) << QPointF(mHalfSize, 0)
                     << QPointF(0, -mHalfSize) << QPointF(0, mHalfSize);
}

QRectF Cross::boundingRect() const
{
    return QRectF(-mHalfSize, -mHalfSize, 2 * mHalfSize, 2 * mHalfSize);
}

void Cross::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, 
                  QWidget *widget)
{
    painter->setPen(QPen(Qt::white, 3));
    painter->drawLines(mCrossPointPairs);

    painter->setPen(Qt::black);
    painter->drawLines(mCrossPointPairs);
}