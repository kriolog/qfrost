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

#include <tools/boundaryellipsecreator.h>

#include <QtGui/QPainter>
#include <QtWidgets/QUndoStack>
#include <QtCore/qmath.h>

#include <boost/math/constants/constants.hpp>

#include <cmath>

#include <scene.h>

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
    if (!alt) {
        qfscene->uniteBoundaryPolygon(ellipseShapedPolygon(),
                                      QUndoStack::tr("append ellipse"));
    } else {
        qfscene->subtractBoundaryPolygon(ellipseShapedPolygon(),
                                         QUndoStack::tr("subtract ellipse"));
    }
    qfscene->setTool(QFrost::boundaryEllipseCreator);
}

QPolygonF BoundaryEllipseCreator::unitRoundPolygon() const
{
    Q_ASSERT(360 % mNumberOfAngles == 0);
    // Укладывается ли шаг целое число раз в 90 градусов.
    Q_ASSERT(mNumberOfAngles % 4 == 0);

    QPolygonF result;

    //Заполняем превую четверть окружности.
    //для сохранения точности.
    result << QPointF(1, 0);
    for (uint i = 1; i < mNumberOfAngles / 4; ++i) {
        const double angle = double(i) * (2.0 * boost::math::constants::pi<double>() / double(mNumberOfAngles));
        result << QPointF(qCos(angle), qSin(angle));
    }

    //Заполняем вторую четверть окружности.
    //для сохранения точности.
    result << QPointF(0, 1);
    for (uint i = 1; i < mNumberOfAngles / 4 + 1; ++i) {
        QPointF currentPoint;
        currentPoint = result.at(mNumberOfAngles / 4 - i);
        result << QPointF(-currentPoint.x(),
                          currentPoint.y());
    }

    //Заполняем вторую половину окружности.
    for (uint i = 1; i < mNumberOfAngles / 2; ++i) {
        QPointF currentPoint;
        currentPoint = result.at(mNumberOfAngles / 2 - i);
        result << QPointF(currentPoint.x(),
                          -currentPoint.y());
    }

    return result;
}

QPolygonF BoundaryEllipseCreator::ellipseShapedPolygon() const
{
    /// Полуширина эллипса
    qreal a;
    /// Полувысота эллипса
    qreal b;
    /// Координаты середины эллипса
    QPointF shiftPoint;

    a = boundingRect().width() / 2;
    b = boundingRect().height() / 2;
    shiftPoint = mapToScene(boundingRect().center());

    QPolygonF result;
    foreach(QPointF point, unitRoundPolygon()) {
        QPointF currentPoint;

        currentPoint = QPointF(point.x() * a, point.y() * b)
                       + shiftPoint;
        result.append(currentPoint);
    }
    //Замыкаем полигон
    result.append(result.first());
    return result;
}
