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

#include <tools/polygonalselection.h>

#include <QtGui/QPainter>

#include <boundarypolygoncalc.h>
#include <graphicsviews/scene.h>
#include <pointonboundarypolygon.h>
#include <block.h>

using namespace qfgui;

PolygonalSelection::PolygonalSelection()
    : Tool()
    , mPolygon()
    , mCircle(new SmallCircle(this))
{
    mCircle->hide();
}

void PolygonalSelection::paint(QPainter *painter,
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

    if (mPolygon.size() > 1) {
        // рисуем заполнение
        painter->setBrush(QBrush(QColor(80, 80, 80, 80), Qt::SolidPattern));
        painter->setPen(Qt::NoPen);
        painter->drawPolygon(mPolygon);

        // рисуем полилинию (незамкнутую)
        painter->setPen(QPen(Qt::magenta, 0, Qt::DashLine));
        painter->drawPolyline(mPolygon);

        // соединяем первую и последнюю точку полупрозрачным отрезком
        painter->setOpacity(0.25);
        painter->drawLine(mPolygon.first(), mPolygon.last());
    }

    painter->restore();
}

void PolygonalSelection::addPoint(const QPointF &point)
{
    // Если новая точка равна последней, делать ничего не надо.
    if (!mPolygon.isEmpty() && qFuzzyCompare(point, mPolygon.last())) {
        return;
    }

    if (mPolygon.isEmpty()) {
        mCircle->setPos(point);
        mCircle->show();
    }
    /* если вектора, построенные на двух последних отрезках полигона,
     * сонаправлены, эти отрезки нужно объединить путём удаления промежуточной
     * точки. */
    if (mPolygon.size() >= 2
            && BoundaryPolygonCalc::areCollinearLines(QLineF(mPolygon.at(mPolygon.size() - 2),
                    mPolygon.at(mPolygon.size() - 1)),
                    QLineF(mPolygon.at(mPolygon.size() - 1), point))) {
        prepareGeometryChange();
        mPolygon.remove(mPolygon.size() - 1);
    }

    if (mPolygon.isEmpty() || !qFuzzyCompare(point, mPolygon.last())) {
        // Добавляем точку, только если она не равна последней точке полигона.
        prepareGeometryChange();
        mPolygon << point;
    }
    selectBlocks();
}

void PolygonalSelection::addPoint(const PointOnBoundaryPolygon &point)
{
    addPoint(point.toPoint());
}

void PolygonalSelection::selectBlocks()
{
    Q_ASSERT(scene() != NULL);

    scene()->clearSelection();

    if (mPolygon.size() < 3) {
        return;
    }

    QList<QPointF>::ConstIterator it;

    QPainterPath path;
    path.addPolygon(mPolygon);
    path.closeSubpath();

    // Перебираем все блоки, задевающие нашь полигон на соответствие критерию...
    foreach(Block * block, static_cast<Scene *>(scene())->blocks(path)) {
        QRectF blockRect;
        /* Немного обрезаем прямоугольник ячейки, чтобы обработать
         * некоторые исключения. Например, если блок примыкает внешним
         * образом к углу полигона, равному 270 градусам, то теперь она
         * не будет выделяться. */
        blockRect = block->rect().adjusted(QFrost::accuracy, QFrost::accuracy,
                                           -QFrost::accuracy, -QFrost::accuracy);
        QList<QPointF> pointsToCheck;
        pointsToCheck << blockRect.center()
                      << blockRect.topLeft()
                      << blockRect.topRight()
                      << blockRect.bottomLeft()
                      << blockRect.bottomRight();
        uint counter = 0;
        for (it = pointsToCheck.constBegin(); it != pointsToCheck.constEnd(); ++it) {
            counter += mPolygon.containsPoint(*it, Qt::OddEvenFill);
            if (counter >= 3) {
                block->setSelected(true);
                break;
            }
        }
    }
}

void PolygonalSelection::onSceneHasChanged()
{
    if (scene() != NULL) {
        selectBlocks();
        connect(this, SIGNAL(destroyed()), scene(), SLOT(clearSelection()));
        connect(scene(), SIGNAL(mousePressed(QPointF)),
                SLOT(addPoint(QPointF)));
        connect(scene(), SIGNAL(mousePressed(PointOnBoundaryPolygon)),
                SLOT(addPoint(PointOnBoundaryPolygon)));
    }
}

void PolygonalSelection::beforeSceneChange()
{
    if (scene() != NULL) {
        scene()->clearSelection();
        disconnect(this, SIGNAL(destroyed()), scene(), SLOT(clearSelection()));
    }
}

void PolygonalSelection::cancelLastChange()
{
    if (!mPolygon.isEmpty()) {
        prepareGeometryChange();
        mPolygon.remove(mPolygon.size() - 1);
        if (mPolygon.isEmpty()) {
            mCircle->hide();
        }
        selectBlocks();
    }
}

void SmallCircle::paint(QPainter *painter,
                        const QStyleOptionGraphicsItem *option,
                        QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);

    QRectF circleRect(-scaledBorder(), -scaledBorder(),
                      scaledBorder() * 2, scaledBorder() * 2);
    //painter->setOpacity(0.3);
    painter->setBrush(QColor(80, 80, 80, 80));
    painter->setPen(QPen(Qt::magenta, 0));
    painter->drawEllipse(circleRect);

    painter->restore();
}
