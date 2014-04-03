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

#include <tools/growingpolygon.h>

#include <QtGui/QPainter>

#include <boundarypolygoncalc.h>

using namespace qfgui;

void GrowingPolygon::paint(QPainter *painter,
                           const QStyleOptionGraphicsItem *option,
                           QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (mPolygon.size() == 0) {
        return;
    }

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor color;
    color = isSimple() ? Qt::blue : Qt::red;

    if (mPolygon.size() > 1) {
        // рисуем заполнение
        painter->setBrush(color);
        painter->setPen(Qt::NoPen);
        painter->setOpacity(0.2);
        painter->drawPolygon(mPolygon);

        QPen pen;

        /* HACK: Можно бы было использовать border() и сделать setCosmetic(true),
         * но в Qt 4.8 оно работает как-то не так */
        pen.setWidthF(scaledBorder() * 0.6); // чуть тоньше кружков
        pen.setJoinStyle(Qt::MiterJoin);
        pen.setColor(color);
        pen.setStyle(Qt::DashLine);
        pen.setCapStyle(Qt::RoundCap);

        // рисуем полилинию (незамкнутую)
        painter->setPen(pen);
        painter->setOpacity(1);
        painter->drawPolyline(mPolygon);

        // соединяем первую и последнюю точку полупрозрачным отрезком
        painter->setOpacity(0.25);
        painter->drawLine(mPolygon.first(), mPolygon.last());
    }

    // ставим кружочек в последней точке
    QRectF circleRect(-scaledBorder(), -scaledBorder(),
                      scaledBorder() * 2, scaledBorder() * 2);
    painter->setOpacity(1);
    painter->setBrush(color);
    painter->setPen(QPen(Qt::green, 0));
    painter->drawEllipse(circleRect.translated(mPolygon.last()));

    painter->restore();
}

void GrowingPolygon::addPoint(const QPointF &point)
{
    // Если новая точка равна последней, делать ничего не надо.
    if (!mPolygon.isEmpty() && qFuzzyCompare(point, polygon().last())) {
        return;
    }
    /* если вектора, построенные на двух последних отрезках полигона,
     * сонаправлены, эти отрезки нужно объединить путём удаления промежуточной
     * точки. */
    if (mPolygon.size() >= 2 &&
            BoundaryPolygonCalc::areCollinearLines(QLineF(mPolygon.at(mPolygon.size() - 2),
                    mPolygon.at(mPolygon.size() - 1)),
                    QLineF(mPolygon.at(mPolygon.size() - 1),
                           point))) {
        prepareGeometryChange();
        mPolygon.remove(mPolygon.size() - 1);
    }

    if (mPolygon.isEmpty()
            || !qFuzzyCompare(point, mPolygon.last())) {
        // Добавляем точку, только если она не равна последней точке полигона.
        prepareGeometryChange();
        mPolygon << point;
    }
    // Отсутствие противонаправленных смежных сторон учтено выше
    updateSimplicity();
}

void GrowingPolygon::deleteLastPoint()
{
    if (!mPolygon.isEmpty()) {
        prepareGeometryChange();
        mPolygon.remove(mPolygon.size() - 1);
        updateSimplicity();
    }
}

void GrowingPolygon::updateSimplicity()
{
    // Подразумевается, что отсутствие противонаправленных смежных сторон
    // предусмотрено заранее, так что преверяем только самопересекаемость
    mIsSimple = BoundaryPolygonCalc::polygonIsNotSelfIntersecting(mPolygon);
}
