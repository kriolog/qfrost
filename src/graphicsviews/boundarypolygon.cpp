/*
 * Copyright (C) 2010-2012  Denis Pesotsky, Maxim Torgonsky
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

#include "boundarypolygon.h"

#include <QtCore/qmath.h>
#include <QtGui/QPainter>

#include <boundarypolygoncalc.h>
#include <pointonboundarypolygon.h>
#include <graphicsviews/scene.h>
#include <block.h>

using namespace qfgui;

BoundaryPolygon::BoundaryPolygon(const QPolygonF &polygon,
                                 const BoundaryCondition *condition)
    : NonScalableItem()
    , mCorners(BoundaryPolygonCalc::setCondition(polygon, condition))
{
    setZValue(QFrost::BoundaryPolygonZValue);
}

BoundaryPolygon::BoundaryPolygon(const QList<Vertex> &corners)
    : NonScalableItem()
    , mCorners(corners)
{
    Q_ASSERT(mCorners.size() >= 3);

    setZValue(QFrost::BoundaryPolygonZValue);

    int conditionPointsCount;
    conditionPointsCount = 0;
    int onlyConditionPointCornerNum;
    onlyConditionPointCornerNum = 0;

    /* Запоминаем номер угла, где была единственная точка смены условия.
     * Если точка смены условия не единственная, выходим из конструктора. */
    for (QList<Vertex>::ConstIterator i = mCorners.constBegin();
            i != mCorners.constEnd(); ++i) {
        if (!(*i).conditionPoints.isEmpty()) {
            onlyConditionPointCornerNum = i - mCorners.constBegin();
            conditionPointsCount += (*i).conditionPoints.size();
        }
        if (conditionPointsCount > 1) {
            return;
        }
    }

    Q_ASSERT(conditionPointsCount > 0);

    // Если точка смены условия единственная, переносим её в начало.
    if (conditionPointsCount == 1 && onlyConditionPointCornerNum != 0) {
        Q_ASSERT(mCorners.at(onlyConditionPointCornerNum).conditionPoints.size() == 1);
        ConditionPoint onlyConditionPoint;
        onlyConditionPoint = mCorners.at(onlyConditionPointCornerNum).conditionPoints.first();
        Q_ASSERT(onlyConditionPoint.distance == 0);
        mCorners[onlyConditionPointCornerNum].conditionPoints.clear();
        mCorners.first().conditionPoints.append(onlyConditionPoint);
    }

    setZValue(QFrost::BoundaryPolygonZValue);
}

BoundaryPolygon::BoundaryPolygon(const BoundaryPolygon *other)
    : NonScalableItem()
    , mCorners(other->mCorners)
{
    setZValue(QFrost::BoundaryPolygonZValue);
}

void BoundaryPolygon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    // рисуем заполнение
    if (parentItem() == NULL) {
        painter->setBrush(Qt::lightGray);
        painter->setPen(Qt::NoPen);
        painter->setOpacity(0.1);

        QPainterPath path;
        path.addPolygon(polygon());
        foreach(QGraphicsItem * item, childItems()) {
            BoundaryPolygon *p = qgraphicsitem_cast<BoundaryPolygon *>(item);
            Q_ASSERT(p != NULL);
            path.addPolygon(p->polygon());
        };

        painter->drawPath(path);
    }

    painter->setOpacity(1);

    QPen pen;
    /* HACK: Можно бы было использовать border() и сделать setCosmetic(true),
     * но в Qt 4.8 оно работает как-то не так */
    pen.setWidthF(scaledBorder());
    //pen.setCosmetic(true);
    pen.setJoinStyle(Qt::RoundJoin);

    //Отрисовка полилиний
    foreach(BoundaryPolyline polyline, boundaryPolylines()) {
        Q_ASSERT(polyline.condition != NULL);
        pen.setColor(polyline.condition->color());
        painter->setPen(pen);
        painter->drawPolyline(polyline.polygon);
    }

    //отрисовка кружочков на точках смены условия
    if (conditionPoints().size() > 1) {
        /* если точка смены условия одна, то это нулевая точка, задающая условие
         * так что рисуем точки смены условия только в остальных случаях */

        QRectF circleRect(-scaledBorder(), -scaledBorder(),
                          scaledBorder() * 2, scaledBorder() * 2);
        painter->setBrush(QBrush(Qt::black, Qt::SolidPattern));
        QPen circlePen(Qt::white);
        circlePen.setCosmetic(true);
        painter->setPen(circlePen);

        foreach(QPointF conditionPoint, conditionPoints()) {
            painter->drawEllipse(circleRect.translated(conditionPoint));
        }
    }

    painter->restore();
}

QPainterPath BoundaryPolygon::shape() const
{
    //TODO: Сделать тонкие повёрнутые полигоны, ввести в
    QPainterPath result;

    QPolygonF polygon;
    polygon = BoundaryPolygon::polygon();
    for (QPolygonF::ConstIterator i = polygon.constBegin(); i != polygon.constEnd() - 1; ++i) {
        QLineF segment(*i, *(i + 1));
        QLineF normalVector;
        normalVector = segment.normalVector();
        QPointF offset;
        offset = (normalVector.p2() - normalVector.p1()) / segment.length();
        QLineF left(segment.translated(-offset));
        QLineF right(segment.translated(offset));

        QPolygonF thinPolygon;
        thinPolygon << left.p1() << left.p2()
                    << right.p2() << right.p1()
                    << left.p1();
        result.addPolygon(thinPolygon);
    }

    return result;
}

QPolygonF BoundaryPolygon::polygon() const
{
    QPolygonF result;
    foreach(Vertex vertex, mCorners) {
        result.append(vertex.point);
    }

    Q_ASSERT(result.isClosed());

    return result;
}

QList<QPointF> BoundaryPolygon::conditionPoints() const
{
    QList<QPointF> result;
    for (QList<Vertex>::ConstIterator i = mCorners.constBegin();
            i != mCorners.constEnd() - 1; ++i) {
        foreach(ConditionPoint conditionPoint, (*i).conditionPoints) {
            PointOnBoundaryPolygon p(this, i - mCorners.constBegin(),
                                     conditionPoint.distance);
            result.append(p.toPoint());
        }
    }

    Q_ASSERT(!result.isEmpty());

    return result;
}

QList<BoundaryPolyline> BoundaryPolygon::boundaryPolylines(const QList<Vertex> &corners)
{
    QList<BoundaryPolyline> result;
    /**
     * Хвост полилинии, образованной последней точкой смены условия.
     * Он находится в начале полигона (до первой точки смены условия).
     */
    QPolygonF lastPolylineTag;

    for (QList<Vertex>::ConstIterator i = corners.constBegin();
            i != corners.constEnd() - 1; ++i) {
        /* если в текущем углу нет точки смены условия с нулевой дистанцией,
         * добавляем её в полилинию, если есть - она добавится дальше сама */
        if ((*i).conditionPoints.isEmpty() || (*i).conditionPoints.first().distance != 0) {
            if (result.isEmpty()) {
                //заполняем хвост, пока не появилась первая точка смены условия
                lastPolylineTag.append((*i).point);
            } else {
                //добавляем угол к последней полилинии
                result.last().polygon.append((*i).point);
            }
        }

        /// точка следующего угла полигона
        QPointF nextCornerPoint;
        nextCornerPoint = (*(i + 1)).point;
        foreach(ConditionPoint conditionPoint, (*i).conditionPoints) {
            QPointF nextPoint = BoundaryPolygonCalc::pointOnSegment((*i).point,
                                nextCornerPoint,
                                conditionPoint.distance);
            if (result.isEmpty()) {
                //добавляем последнюю точку в хвост и начинаем
                //новую полилинию
                lastPolylineTag.append(nextPoint);
            } else {
                //добавляем последнюю точку к последней полилинии и начинаем
                //новую полилинию
                result.last().polygon.append(nextPoint);
            }
            //начинаем новую полилинию
            BoundaryPolyline newPolyline;
            newPolyline.polygon.append(nextPoint);
            newPolyline.condition = conditionPoint.condition;
            //добавляем новую полилинию в список
            result.append(newPolyline);
        }
    }

    Q_ASSERT(!result.isEmpty());
    //добавляем хвост к последней полилинии
    result.last().polygon += lastPolylineTag;

    return result;
}

QList< BoundaryPolyline > BoundaryPolygon::boundaryPolylines() const
{
    return boundaryPolylines(mCorners);
}

bool BoundaryPolygon::intersects(const QPolygonF &polygon) const
{
    QPainterPath thisPath;
    QPainterPath polygonPath;
    thisPath.addPolygon(this->polygon());
    polygonPath.addPolygon(polygon);
    if (thisPath.intersects(polygonPath)) {
        return true;
    }
    return false;
}

bool BoundaryPolygon::intersectsExcludingOnlyPoints(const QPolygonF &polygon) const
{
    QPainterPath thisPath;
    QPainterPath polygonPath;
    thisPath.addPolygon(this->polygon());
    polygonPath.addPolygon(polygon);
    QList<QPolygonF> resultedPolygons;
    /* Список всех полигонов, получившихся после объединения исходных. Если в
     * нём будет содержаться только 1 элемент, значит полигоны имеют общие
     * отрезки/фигуры и могут иметь общие (изолированные) граничные точки. Если
     * элементов 2 или более, то исходные полигоны либо не пересекаются вообще,
     * либо делают это только в конечном количестве точек. */
    resultedPolygons = BoundaryPolygonCalc::united(thisPath, polygonPath);
    //теперь избавимся от всех полигонов, которые содержатся в других.
    if (resultedPolygons.size() > 1) {
        for (int i = 0; i < resultedPolygons.size(); ++i) {
            for (int j = 0; j < resultedPolygons.size(); ++j) {
                if (i != j) {
                    if (BoundaryPolygonCalc::isContained(resultedPolygons.at(i), resultedPolygons.at(j))) {
                        resultedPolygons.removeAt(j);
                        if (i > j) {
                            --i;
                        }
                        --j;
                    }
                }
            }
        }
    }
    if (resultedPolygons.size() == 1) {
        return true;
    }
    return false;
}

bool BoundaryPolygon::intersectsExcludingBorder(const QPolygonF &polygon) const
{
    QPainterPath thisPath;
    QPainterPath polygonPath;
    thisPath.addPolygon(this->polygon());
    polygonPath.addPolygon(polygon);
    if (!thisPath.intersected(polygonPath).isEmpty()) {
        return true;
    }
    return false;
}

bool BoundaryPolygon::contains(const QPolygonF &polygon) const
{
    QPolygonF thisPolygon;
    thisPolygon = this->polygon();
    return BoundaryPolygonCalc::isContained(this->polygon(), polygon);
}

QLineF BoundaryPolygon::segment(int i, bool safe) const
{
    if (safe) {
        while (i > mCorners.size()) {
            i -= mCorners.size();
        }
        while (i < 0) {
            i += mCorners.size();
        }
        if (i == mCorners.size() - 1) {
            i = 0;
        }
    }
    Q_ASSERT(i >= 0);
    Q_ASSERT(i < mCorners.size() - 1);
    return QLineF(mCorners.at(i).point, mCorners.at(i + 1).point);
}

bool qfgui::operator==(const ConditionPoint &c1, const ConditionPoint &c2)
{
    return (c1.distance == c2.distance) && (c1.condition == c2.condition);
}

bool qfgui::operator== (const Vertex &v1, const Vertex &v2)
{
    return (v1.point == v2.point) && (v1.conditionPoints == v2.conditionPoints);
}

QVariant BoundaryPolygon::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    return NonScalableItem::itemChange(change, value);
    if (change == ItemSceneHasChanged) {
        Q_ASSERT(pos() == QPointF(0, 0));
        if (scene() != NULL && parentItem() == NULL) {
        }
    }
    return NonScalableItem::itemChange(change, value);
}
