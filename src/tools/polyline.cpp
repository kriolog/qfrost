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

#include <tools/polyline.h>

#include <QtGui/QPainter>

using namespace qfgui;

void Polyline::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (mPolyline.size() == 0) {
        return;
    }

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    if (mPolyline.size() > 1) {
        QPen pen;
        Q_ASSERT(border() > 2);
        /* HACK: Можно бы было использовать border() и сделать setCosmetic(true),
         * но в Qt 4.8 оно работает как-то не так */
        pen.setWidthF(scaledBorder() * 0.6); // чуть тоньше кружков
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setColor(Qt::black);
        pen.setStyle(Qt::DashLine);
        pen.setCapStyle(Qt::RoundCap);

        // рисуем полилинию (незамкнутую)
        painter->setPen(pen);
        painter->setOpacity(1);
        painter->drawPolyline(mPolyline);

    }

    // ставим кружочек в первой точке
    if (!mPolyline.isClosed() || mPolyline.size() == 1) {
        QRectF circleRect(-scaledBorder(), -scaledBorder(),
                          scaledBorder() * 2, scaledBorder() * 2);
        painter->setOpacity(1);
        painter->setBrush(Qt::blue);
        painter->setPen(QPen(Qt::green, 0));
        painter->drawEllipse(circleRect.translated(mPolyline.first()));
    }

    painter->restore();
}

void Polyline::updatePolyline(const QPolygonF &polyline)
{
    prepareGeometryChange();
    mPolyline = polyline;
    show();
}

void Polyline::updatePolyline(const QPointF &onlyPointPolyline)
{
    updatePolyline(QPolygonF() << onlyPointPolyline);
}
