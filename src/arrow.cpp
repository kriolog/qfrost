/*
 * Copyright (C) 2011-2013  Denis Pesotsky
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

#include <arrow.h>

#include <QtGui/QPainter>
#include <QtCore/QTimer>

#include <boost/math/constants/constants.hpp>

#include <qfrost.h>

using namespace qfgui;

Arrow::Arrow(const QGraphicsItem *startItem, const QGraphicsItem *endItem,
             const QColor &color)
    : QGraphicsObject()
    , mColor(color)
    , mLine(QLineF(mapFromItem(startItem, startItem->boundingRect().center()),
                   mapFromItem(endItem, randomPointIn(endItem->boundingRect()))))
    , mArrowHead()
{
    Q_ASSERT(startItem->scene() != NULL);
    Q_ASSERT(endItem->scene() != NULL);
    setZValue(QFrost::AnchorZValue - 1);

    double angle = acos(mLine.dx() / mLine.length());
    if (mLine.dy() >= 0) {
        angle = (boost::math::constants::pi<double>() * 2.0) - angle;
    }

    static const qreal halfAngle = boost::math::constants::pi<double>() / 2.5;
    QPointF p1 = mLine.p2() - QPointF(sin(angle + halfAngle) * kmArrowSize,
                                      cos(angle + halfAngle) * kmArrowSize);
    QPointF p2 = mLine.p2() - QPointF(sin(angle + boost::math::constants::pi<double>() - halfAngle) * kmArrowSize,
                                      cos(angle + boost::math::constants::pi<double>() - halfAngle) * kmArrowSize);

    mArrowHead << mLine.p2() << p1 << p2;

    QTimer *deathTimer = new QTimer(this);
    deathTimer->singleShot(3000, this, SLOT(deleteLater()));
}

QPointF Arrow::randomPointIn(const QRectF &rect)
{
    // получается точка в прямоугольнике, втрое меньшем rect, ...
    int dx = qrand() % int(rect.width() / 3);
    int dy = qrand() % int(rect.height() / 3);
    // ... который своим центром лежит в центре изначального
    return QPointF(rect.left() + dx + rect.width() / 3.0,
                   rect.top() + dy + rect.height() / 3.0);
}

QRectF Arrow::boundingRect() const
{
    return QRectF(mLine.p1(), QSizeF(mLine.p2().x() - mLine.p1().x(),
                                     mLine.p2().y() - mLine.p1().y()))
           .normalized()
           .adjusted(-kmArrowSize, -kmArrowSize,
                     kmArrowSize, kmArrowSize);
}

void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(mColor, 0));
    painter->setBrush(mColor);

    painter->drawLine(mLine);
    painter->drawPolygon(mArrowHead);
    painter->restore();
}
