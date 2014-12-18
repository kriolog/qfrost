/*
 * Copyright (C) 2011-2014  Denis Pesotsky, Maxim Torgonsky
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

#include <tools/boundaryellipsecreator.h>

#include <QtGui/QPainter>
#include <QtWidgets/QUndoStack>

#include <graphicsviews/scene.h>
#include <boundarypolygoncalc.h>

using namespace qfgui;

void Ellipse::paint(QPainter *painter,
                    const QStyleOptionGraphicsItem *option,
                    QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    QPen pen;
    /* HACK: Можно бы было использовать border() и сделать setCosmetic(true),
     * но в Qt 4.8 оно работает как-то не так */
    pen.setWidthF(scaledBorder());
    pen.setColor(Qt::blue);
    pen.setStyle(Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    painter->setBrush(QColor(0, 0, 255, 51));
    painter->setPen(pen);
    painter->drawEllipse(baseRect());
    painter->restore();
}

BoundaryEllipseCreator::BoundaryEllipseCreator(ToolSettings *settings):
    RectangularTool(settings, true),
    mNumberOfAngles(120),
    mEllipse(new Ellipse(this))
{
    //mEllipse->scaleToParent(); понадобится, только если изн. размер != (0,0)
}

void BoundaryEllipseCreator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::magenta, 0, Qt::DashLine));
    painter->drawRect(boundingRect());
    painter->restore();
}

void BoundaryEllipseCreator::apply(bool alt)
{
    Scene *qfscene = qobject_cast<Scene *>(scene());
    Q_ASSERT(qfscene != NULL);
    
    const QPolygonF poly = BoundaryPolygonCalc::ellipseShapedPolygon(sceneBoundingRect());
    if (!alt) {
        qfscene->uniteBoundaryPolygon(poly, QUndoStack::tr("append ellipse"));
    } else {
        qfscene->subtractBoundaryPolygon(poly, QUndoStack::tr("subtract ellipse"));
    }
    qfscene->setTool(QFrost::boundaryEllipseCreator);
}
