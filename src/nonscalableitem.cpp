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

#include <nonscalableitem.h>

#include <QtWidgets/QGraphicsScene>

#include <view.h>

using namespace qfgui;

NonScalableItem::NonScalableItem(QGraphicsItem *parent, qreal border):
    QGraphicsObject(parent),
    mBorder(border)
{

}

QRectF NonScalableItem::boundingRect() const
{
    return baseRect().adjusted(-mScaledBorder, -mScaledBorder,
                               mScaledBorder, mScaledBorder);
}

QVariant NonScalableItem::itemChange(QGraphicsItem::GraphicsItemChange change,
                                     const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
        if (scene() != NULL) {
            connect(scene(), SIGNAL(mainViewScaleChanged(qreal)),
                    this, SLOT(onViewScaleChanged(qreal)));
            Q_ASSERT(!scene()->views().isEmpty());
            View *qfview = qobject_cast<View *>(scene()->views().first());
            Q_ASSERT(qfview != NULL);
            onViewScaleChanged(qfview->transform().m11());
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void NonScalableItem::onViewScaleChanged(qreal viewScale)
{
    mScaledBorder = mBorder / viewScale;
}
