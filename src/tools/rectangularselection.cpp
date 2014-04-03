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

#include <tools/rectangularselection.h>

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QWidget>

#include <qfrost.h>

using namespace qfgui;

void RectangularSelection::paint(QPainter *painter,
                                 const QStyleOptionGraphicsItem *option,
                                 QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->setPen(QPen(Qt::magenta, 0, Qt::DashLine));
    painter->setBrush(QBrush(QColor(80, 80, 80, 80), Qt::SolidPattern));
    painter->drawRect(boundingRect());
}

void RectangularSelection::onStartChange()
{
    scene()->clearSelection();
}

void RectangularSelection::onStopChange()
{
    selectBlocks();
}

void RectangularSelection::selectBlocks()
{
    QPainterPath path;
    /* Уменьшаем прямоугольник с каждой стороны, дабы не включать (некоторые)
     * блоки, которые лишь соприкасаются с нами. */
    static const qreal margin = QFrost::unitsInGridStep / 2;
    path.addRect(rectInScene().adjusted(margin, margin, -margin, -margin));
    scene()->setSelectionArea(path, Qt::IntersectsItemBoundingRect, transform());
}

void RectangularSelection::onSceneHasChanged()
{
    RectangularTool::onSceneHasChanged();
    if (scene() != NULL) {
        selectBlocks();
        connect(this, SIGNAL(destroyed()), scene(), SLOT(clearSelection()));
    }
}

void RectangularSelection::beforeSceneChange()
{
    if (scene() != NULL) {
        scene()->clearSelection();
        disconnect(this, SIGNAL(destroyed()), scene(), SLOT(clearSelection()));
    }
}
