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

#include <tools/boundarypolygoncreator.h>

#include <QtGui/QPainter>
#include <QtWidgets/QUndoStack>

#include <graphicsviews/scene.h>
#include <tools/growingpolygon.h>
#include <boundarypolygoncalc.h>
#include <pointonboundarypolygon.h>
#include <graphicsviews/boundarypolygon.h>

using namespace qfgui;

BoundaryPolygonCreator::BoundaryPolygonCreator(): Tool(),
    mGrowingPolygon(new GrowingPolygon(this))
{
    setFlag(ItemHasNoContents);
}

void BoundaryPolygonCreator::onSceneHasChanged()
{
    if (scene() != NULL) {
        connect(scene(), SIGNAL(mousePressed(QPointF)), SLOT(addPoint(QPointF)));
        connect(scene(), SIGNAL(mousePressed(PointOnBoundaryPolygon)),
                SLOT(addPoint(PointOnBoundaryPolygon)));
    }
}

void BoundaryPolygonCreator::apply(bool alt)
{
    if (mGrowingPolygon->isSimple()) {
        QPolygonF polygon;
        polygon = mGrowingPolygon->polygon();
        /* если полигон неcамопересекается и не содержит смежные стороны,
            * принадлежащие одной прямой. */
        if (!polygon.isClosed()) {
            polygon.append(polygon.first());
        }

        polygon = BoundaryPolygonCalc::removeZeroAndFlatAngles(polygon);
        Scene *qfscene = qobject_cast<Scene *>(scene());
        Q_ASSERT(qfscene != NULL);
        if (!alt) {
            qfscene->uniteBoundaryPolygon(polygon,
                                          QUndoStack::tr("append polygon"));
        } else {
            qfscene->subtractBoundaryPolygon(polygon,
                                             QUndoStack::tr("subtract polygon"));
        }
        mGrowingPolygon->clear();
    }
}

void BoundaryPolygonCreator::addPoint(const QPointF &point)
{
    mGrowingPolygon->addPoint(point);
}

void BoundaryPolygonCreator::addPoint(const PointOnBoundaryPolygon &point)
{
    addPoint(point.toPoint());
}

void BoundaryPolygonCreator::cancelLastChange()
{
    mGrowingPolygon->deleteLastPoint();
}
