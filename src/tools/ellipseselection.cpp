/*
 * Copyright (C) 2011-2014  Denis Pesotsky
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

#include <tools/ellipseselection.h>

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QWidget>
#include <QtMath>

#include <qfrost.h>
#include <graphicsviews/scene.h>
#include <boundarypolygoncalc.h>
#include <graphicsviews/block.h>

using namespace qfgui;

void EllipseSelection::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::magenta, 0, Qt::DashLine));
    painter->drawRect(boundingRect());

    painter->setBrush(QBrush(QColor(80, 80, 80, 80), Qt::SolidPattern));
    painter->drawEllipse(boundingRect());
}

void EllipseSelection::onStartChange()
{
    scene()->clearSelection();
}

void EllipseSelection::onStopChange()
{
    selectBlocks();
}

void EllipseSelection::selectBlocks()
{
    scene()->clearSelection();

    const double rx2 = qPow(rectInScene().width()/2.0, 2);
    const double ry2 = qPow(rectInScene().height()/2.0, 2);

    const double h = rectInScene().center().x();
    const double k = rectInScene().center().y();

    foreach(Block * block, static_cast<Scene *>(scene())->blocks(rectInScene())) {
        const QPointF c = block->sceneBoundingRect().center();
        if (qPow(c.x() - h, 2)/rx2 + qPow(c.y() - k, 2)/ry2 <= 1.0) {
            // Центр лежит внутри эллипса, выделяем (не суперточно, зато быстро)
            block->setSelected(true);
        }
    }
}

void EllipseSelection::onSceneHasChanged()
{
    RectangularTool::onSceneHasChanged();
    if (scene() != NULL) {
        selectBlocks();
        connect(this, SIGNAL(destroyed()), scene(), SLOT(clearSelection()));
    }
}

void EllipseSelection::beforeSceneChange()
{
    if (scene() != NULL) {
        scene()->clearSelection();
        disconnect(this, SIGNAL(destroyed()), scene(), SLOT(clearSelection()));
    }
}
