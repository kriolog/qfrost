/*
 * Copyright (C) 2010-2014  Denis Pesotsky, Maxim Torgonsky
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

#include <tools/anchor.h>

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <graphicsviews/block.h>
#include <graphicsviews/scene.h>
#include <graphicsviews/view.h>
#include <qfrost.h>
#include <graphicsviews/boundarypolygon.h>
#include <tools/boundaryconditionsapplicator.h>
#include <boundarypolygoncalc.h>

using namespace qfgui;

Anchor::Anchor(): QGraphicsObject(),
    mInnerPart(0.5),
    mPosOnPolygon(),
    mAnchorAngle(),
    mStickRadius(),
    mLogic(),
    mFoundAnchorType(Nowhere),
    mIsNeeded(true),
    mViewScale()
{
    setZValue(QFrost::AnchorZValue);
    setFlag(ItemIgnoresTransformations);
}

QRectF Anchor::boundingRect() const
{
    QRectF result;
    result.setSize(QSize(mBoundingSize, mBoundingSize));
    result.moveCenter(QPointF(0, 0));
    return result;
}

void Anchor::paint(QPainter *painter,
                   const QStyleOptionGraphicsItem *option,
                   QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    static const int blockAnchorHalfSize = 5;
    static const int polygonAnchorHalfSize = 7;
    static const int circleAnchorHalfSize = 8;
    // 2^0.5 < 1.5;
    Q_ASSERT(qMax(blockAnchorHalfSize,
                  qMax(polygonAnchorHalfSize,
                       blockAnchorHalfSize))
             <= mBoundingSize * 1.5);

    if (mFoundAnchorType == Nowhere) {
        return;
    }

    painter->save();
    painter->setPen(QPen(Qt::magenta, 0));
    painter->setBrush(Qt::NoBrush);

    painter->setRenderHint(QPainter::Antialiasing, false);
    switch (mFoundAnchorType) {
    case OnVisibleGrid: {
        // привязка на сетке
        painter->drawLine(-blockAnchorHalfSize + 1, -blockAnchorHalfSize,
                          blockAnchorHalfSize, blockAnchorHalfSize - 1);
        painter->drawLine(-blockAnchorHalfSize, -blockAnchorHalfSize + 1,
                          blockAnchorHalfSize - 1, blockAnchorHalfSize);
        painter->drawLine(-blockAnchorHalfSize, blockAnchorHalfSize - 1,
                          blockAnchorHalfSize - 1, -blockAnchorHalfSize);
        painter->drawLine(-blockAnchorHalfSize + 1, blockAnchorHalfSize,
                          blockAnchorHalfSize, -blockAnchorHalfSize + 1);
        break;
    } case OnPolygonSide: {
        // привязка на стороне полигона
        painter->setRenderHint(QPainter::Antialiasing);
        painter->rotate(mAnchorAngle);
        QPolygonF sandclock;
        sandclock.append(QPointF(-polygonAnchorHalfSize, -polygonAnchorHalfSize));
        sandclock.append(QPointF(polygonAnchorHalfSize, -polygonAnchorHalfSize));
        sandclock.append(QPointF(-polygonAnchorHalfSize, polygonAnchorHalfSize));
        sandclock.append(QPointF(polygonAnchorHalfSize, polygonAnchorHalfSize));
        sandclock.append(QPointF(-polygonAnchorHalfSize, -polygonAnchorHalfSize));
        painter->drawPolygon(sandclock);
        break;
    } case OnPolygonCorner: {
        // привязка на угле полигона
        painter->drawRect(-polygonAnchorHalfSize,
                          -polygonAnchorHalfSize,
                          2 * polygonAnchorHalfSize,
                          2 * polygonAnchorHalfSize);
        break;
    } case OnPolygonConditionPoint: {
        // привязка на точке смены условия полигона
        painter->setRenderHint(QPainter::Antialiasing);
        painter->drawEllipse(-circleAnchorHalfSize,
                             -circleAnchorHalfSize,
                             2 * circleAnchorHalfSize,
                             2 * circleAnchorHalfSize);
        painter->drawEllipse(-1, -1, 2, 2);
        break;
    } case OnBlockCorner: {
        // привязка на углу блока
        QList<QRectF> rects;
        // Внешний прямоугольник...
        painter->drawRect(QRectF(-blockAnchorHalfSize,
                                 -blockAnchorHalfSize,
                                 2 * blockAnchorHalfSize,
                                 2 * blockAnchorHalfSize));
        // ... и внутренний
        painter->drawRect(QRectF(-blockAnchorHalfSize + 2,
                                 -blockAnchorHalfSize + 2,
                                 2 * blockAnchorHalfSize - 4,
                                 2 * blockAnchorHalfSize - 4));
        break;

    } case OnBlockSide: {
        // привязка на стороне блока
        QPolygonF sandclock_1;
        sandclock_1.append(QPointF(-polygonAnchorHalfSize,
                                   polygonAnchorHalfSize / 2));
        sandclock_1.append(QPointF(-polygonAnchorHalfSize / 2,
                                   polygonAnchorHalfSize));
        sandclock_1.append(QPointF(polygonAnchorHalfSize / 2,
                                   -polygonAnchorHalfSize));
        sandclock_1.append(QPointF(polygonAnchorHalfSize,
                                   -polygonAnchorHalfSize / 2));
        sandclock_1.append(QPointF(-polygonAnchorHalfSize,
                                   polygonAnchorHalfSize / 2));

        QPolygonF sandclock_2;
        sandclock_2.append(QPointF(polygonAnchorHalfSize,
                                   polygonAnchorHalfSize / 2));
        sandclock_2.append(QPointF(polygonAnchorHalfSize / 2,
                                   polygonAnchorHalfSize));
        sandclock_2.append(QPointF(-polygonAnchorHalfSize / 2,
                                   -polygonAnchorHalfSize));
        sandclock_2.append(QPointF(-polygonAnchorHalfSize,
                                   -polygonAnchorHalfSize / 2));
        sandclock_2.append(QPointF(polygonAnchorHalfSize,
                                   polygonAnchorHalfSize / 2));

        painter->drawPolygon(sandclock_1);
        painter->drawPolygon(sandclock_2);
        break;
    } case OnBlockCenter: {
        /// Привязка к центру блока
        static const int diagHalfSize = 3;
        QVector<QLine> lines;
        lines << QLine(-circleAnchorHalfSize, 0, circleAnchorHalfSize, 0)
              << QLine(0, -circleAnchorHalfSize, 0, circleAnchorHalfSize)
              << QLine(diagHalfSize, diagHalfSize, -diagHalfSize, -diagHalfSize)
              << QLine(diagHalfSize, -diagHalfSize, -diagHalfSize, diagHalfSize);
        painter->drawLines(lines);
        break;
    } case Nowhere:
        break;
    }
    painter->restore();
}

void Anchor::findAnchorFreezed(const QPointF &point)
{
    if (mIsNeeded) {
        Q_ASSERT(false);
        return;
    }
    mIsNeeded = true;
    findAnchor(point);
    mIsNeeded = false;
}

void Anchor::findAnchor(const QPointF &point)
{
    Q_ASSERT(scene() != NULL);
    if (point == QFrost::noPoint) {
        return;
    }
    if (!mIsNeeded) {
        return;
    }
    if (!QFrost::boundRectF.contains(point)) {
        QPointF boundedPoint;
        QRectF rect = QFrost::boundRect;
        boundedPoint.setX(qBound(rect.left(), point.x(), rect.right()));
        boundedPoint.setY(qBound(rect.top(), point.y(), rect.bottom()));
        findAnchor(boundedPoint);
        return;
    }

    if (mLogic.testFlag(NeedPolygons)) {
        APointOnBoundaryPolygon result;
        result = anchorOnBoundaryPolygon(point);
        if (!result.first.isNull()) {
            updateFoundAnchor(result);
            return;
        }
    }

    if (mLogic.testFlag(NeedBlockBorders) || mLogic.testFlag(NeedBlockCenters)) {
        APoint result;
        result = anchorOnBlock(point);
        if (result.first != QFrost::noPoint) {
            updateFoundAnchor(result);
            return;
        }
    }

    if (mLogic.testFlag(NeedVisibleGrid)) {
        updateFoundAnchor(nearestGridNode(point));
        return;
    }

    updateFoundAnchor(APoint(QFrost::noPoint, Nowhere));
}

Anchor::APointOnBoundaryPolygon Anchor::anchorOnBoundaryPolygon(const QPointF &point)
{
    QList<BoundaryPolygon *> polygons;

    BoundaryConditionsApplicator *bcCreator;
    bcCreator = qobject_cast<BoundaryConditionsApplicator *>(qfScene()->activeTool());
    if (bcCreator != NULL && bcCreator->isStickedToPolygon()) {
        // Существует создавалка полигонов и она уже выбрала целевой полигон
        polygons.append(const_cast<BoundaryPolygon *>(bcCreator->polygonToRepaint()));
    } else {
        if (mLogic == NeedPolygons) {
            polygons = qfScene()->boundaryPolygons();
        } else {
            polygons = qfScene()->boundaryPolygons(stickRect(point));
        }
    }
    if (!polygons.isEmpty()) {
        APointOnBoundaryPolygon corner;
        corner = nearestCornerOnBoundaryPolygon(point, polygons);
        if (pointsAreCloseEnought(point, corner.first.toPoint())) {
            return corner;
        }

        APointOnBoundaryPolygon conditionPoint;
        conditionPoint = nearestConditionPointOnBoundaryPolygon(point, polygons);
        if (pointsAreCloseEnought(point, conditionPoint.first.toPoint())) {
            return conditionPoint;
        }

        APointOnBoundaryPolygon discretSide;
        //безымянное пр-во имён, ибо нам side не понадобится, пусть не отвлекает
        {
            APointOnBoundaryPolygon side;
            side = nearestPointOnBoundaryPolygon(point, polygons);
            discretSide = discretNearestPolygonPoint(point, side.first, Qt::Horizontal);
            /* Привязываемся к стороне только если нам нужны лишь полигоны.
             * Это потому, что пока что булевые операции с полигонами, имеющими
             * общие стороны может не работать. */
            if (mLogic == NeedPolygons) {
                if (!discretSide.first.isNull() && pointsAreCloseEnought(point, side.first.toPoint())) {
                    if (firstPointIsCloser(point, discretSide.first, corner.first)) {
                        // если точка после дискретизации оказалась точкой смены условия
                        if (discretSide.first.polygon()->conditionPoints().contains(discretSide.first.toPoint())) {
                            // то пусть привязка будет выглядеть соответствующе
                            discretSide.second = OnPolygonConditionPoint;
                        }
                        return discretSide;
                    } else {
                        return corner;
                    }
                    /* Таким образом, если сторона полигона достаточно близка
                     * и возможна дискретизированная проекция на неё, привязка
                     * сработает по ближайшему углу или по этой дискретизированной
                     * проекции, в зависимости от того, что ближе */
                }
            }
        }

        /* Если нужны лишь полигоны, а точка привязки не попала в stickRadius,
         * то привязываемся к ближайшей из этих трёх точек */
        if (mLogic == NeedPolygons) {
            if (firstPointIsCloser(point, corner.first, conditionPoint.first)
                    && firstPointIsCloser(point, corner.first, discretSide.first)) {
                return corner;
            } else if (firstPointIsCloser(point, conditionPoint.first, discretSide.first)) {
                return conditionPoint;
            } else {
                return discretSide;
            }
        }
    }
    return APointOnBoundaryPolygon(PointOnBoundaryPolygon(), Nowhere);
}

Anchor::APoint Anchor::anchorOnBlock(const QPointF &point)
{
    Q_ASSERT(mLogic.testFlag(NeedBlockCenters) || mLogic.testFlag(NeedBlockBorders));

    APoint result;
    QList<Block *> nearestBlocks;
    Block *blockContainingPoint;
    blockContainingPoint = qfScene()->block(point);
    if (blockContainingPoint != NULL) {
        QRectF blockRect = blockContainingPoint->rect();
        if (mLogic.testFlag(NeedBlockBorders)) {
            if (isInInnerPart(point, blockRect)) {
                result = nearestRectCorner(point, blockRect);
                if (result.first != QFrost::noPoint) {
                    return result;
                }
            } else {
                result = nearestRectCorner(point, blockRect);
                if (pointsAreCloseEnought(point, result.first)) {
                    return result;
                }
                result = nearestPointOnRectSide(point, blockRect);
                if (result.first != QFrost::noPoint) {
                    return result;
                }
            }
        } else {
            Q_ASSERT(mLogic.testFlag(NeedBlockCenters));
            result.first = blockRect.center();
            result.second = OnBlockCenter;
            return result;
        }
    } else {
        nearestBlocks = qfScene()->blocks(stickRect(point));
        QList<QRectF> rects;
        foreach(Block * nearestBlock, nearestBlocks) {
            rects.append(nearestBlock->rect());
        }
        result = (mLogic.testFlag(NeedBlockCenters))
                 ? nearestRectCenter(point, rects)
                 : nearestRectCorner(point, rects);
        if (pointsAreCloseEnought(point, result.first)) {
            return result;
        }
        if (mLogic.testFlag(NeedBlockBorders)) {
            result = nearestPointOnRectSide(point, rects);
            if (pointsAreCloseEnought(point, result.first)) {
                return result;
            }
        }
    }
    return APoint(QFrost::noPoint, Nowhere);
}

void Anchor::updateFromViewScale(qreal scale)
{
    mStickRadius = qfScene()->qfView()->gridSpan();
    mViewScale = scale;
}


void Anchor::updateFoundAnchor(const APoint &pointToAnchor)
{
    Q_ASSERT(QFrost::boundRectF.contains(pointToAnchor.first) || pointToAnchor.second == Nowhere);
    mFoundAnchorType = pointToAnchor.second;
    setPos(pointToAnchor.first);
    if (!mPosOnPolygon.isNull()) {
        //обнуляем поле о привязке к полигону, если надо
        mPosOnPolygon = PointOnBoundaryPolygon();
    }
    emit signalPositionChanged(pointToAnchor.first);
}

void Anchor::updateFoundAnchor(const APointOnBoundaryPolygon &pointToAnchor)
{
    Q_ASSERT(!pointToAnchor.first.isNull());

    mPosOnPolygon = pointToAnchor.first;
    mAnchorAngle = -pointToAnchor.first.segment().angle();

    // Точки на углах должны иметь правильный угол (актуально для элипсов)
    if (qFuzzyIsNull(mPosOnPolygon.distance())) {
        const qreal neighbourAngle = mPosOnPolygon.polygon()->segment(mPosOnPolygon.index() - 1).angle();
        mAnchorAngle = (mAnchorAngle - neighbourAngle) / 2.0;
    } else if (qFuzzyCompare(mPosOnPolygon.distance(), mPosOnPolygon.segment().length())) {
        const qreal neighbourAngle = mPosOnPolygon.polygon()->segment(mPosOnPolygon.index() + 1).angle();
        mAnchorAngle = (mAnchorAngle - neighbourAngle) / 2.0;
    }

    emit signalPositionChanged(pointToAnchor.first);

    mFoundAnchorType = pointToAnchor.second;
    if (mFoundAnchorType == OnPolygonCorner && mPosOnPolygon.isInEllipse()) {
        mFoundAnchorType = OnPolygonSide;
    }
    setPos(pointToAnchor.first.toPoint());
    emit signalPositionChanged(pointToAnchor.first.toPoint());
}

//*****************Привязка по полигонам граничных условий********************//

Anchor::APointOnBoundaryPolygon Anchor::nearestCornerOnBoundaryPolygon(const QPointF &p,
        const BoundaryPolygonList &polygons) const
{
    PointOnBoundaryPolygon result;
    foreach(BoundaryPolygon * polygon, polygons) {
        PointOnBoundaryPolygon currentPoint;
        currentPoint = nearestCornerOnBoundaryPolygon(p, polygon).first;
        if (firstPointIsCloser(p, currentPoint, result)) {
            result = currentPoint;
        }
    }
    APointOnBoundaryPolygon aresult;
    aresult.first = result;
    aresult.second = OnPolygonCorner;
    return aresult;
}

Anchor::APointOnBoundaryPolygon Anchor::nearestConditionPointOnBoundaryPolygon(const QPointF &p,
        const BoundaryPolygonList &polygons) const
{
    PointOnBoundaryPolygon result;
    foreach(BoundaryPolygon * polygon, polygons) {
        PointOnBoundaryPolygon currentPoint;
        currentPoint = nearestConditionPointOnBoundaryPolygon(p, polygon).first;
        if (firstPointIsCloser(p, currentPoint, result)) {
            result = currentPoint;
        }
    }
    APointOnBoundaryPolygon aresult;
    aresult.first = result;
    aresult.second = OnPolygonConditionPoint;
    return aresult;
}

Anchor::APointOnBoundaryPolygon Anchor::nearestPointOnBoundaryPolygon(const QPointF &p,
        const BoundaryPolygonList &polygons) const
{
    PointOnBoundaryPolygon result;
    foreach(BoundaryPolygon * polygon, polygons) {
        PointOnBoundaryPolygon currentPoint;
        currentPoint = nearestPointOnBoundaryPolygon(p, polygon).first;
        if (firstPointIsCloser(p, currentPoint, result)) {
            result = currentPoint;
        }
    }
    APointOnBoundaryPolygon aresult;
    aresult.first = result;
    aresult.second = OnPolygonSide;
    return aresult;
}

Anchor::APointOnBoundaryPolygon Anchor::nearestCornerOnBoundaryPolygon(const QPointF &p,
        const BoundaryPolygon *polygon) const
{
    PointOnBoundaryPolygon result;
    for (int i = 0; i < polygon->polygon().size() - 1; ++i) {
        PointOnBoundaryPolygon currentPoint;
        currentPoint = PointOnBoundaryPolygon(polygon, i, 0);
        if (firstPointIsCloser(p, currentPoint, result)) {
            result = currentPoint;
        }
    }
    APointOnBoundaryPolygon aresult;
    aresult.first = result;
    aresult.second = OnPolygonCorner;
    return aresult;
}

Anchor::APointOnBoundaryPolygon Anchor::nearestConditionPointOnBoundaryPolygon(const QPointF &p,
        const BoundaryPolygon *polygon) const
{
    PointOnBoundaryPolygon result;
    Q_ASSERT(polygon->corners().last().conditionPoints.isEmpty());
    uint i = 0;
    foreach(const Vertex &corner, polygon->corners()) {
        foreach(const ConditionPoint &conditionPoint , corner.conditionPoints) {
            PointOnBoundaryPolygon currentPoint;
            currentPoint = PointOnBoundaryPolygon(polygon, i, conditionPoint.distance);
            if (firstPointIsCloser(p, currentPoint, result)) {
                result = currentPoint;
            }
        }
        ++i;
    }
    APointOnBoundaryPolygon aresult;
    aresult.first = result;
    aresult.second = OnPolygonConditionPoint;
    return aresult;
}

Anchor::APointOnBoundaryPolygon Anchor::nearestPointOnBoundaryPolygon(const QPointF &p,
        const BoundaryPolygon *polygon) const
{
    Q_ASSERT(polygon != NULL);
    PointOnBoundaryPolygon result;
    for (int i = 0; i < polygon->polygon().size() - 1; ++i) {
        /// Отрезок, построенный на текущем и следующем углах полигона.
        QLineF segment;
        segment = polygon->segment(i);

        qreal segmentLength;
        segmentLength = segment.length();
        Q_ASSERT(!qFuzzyIsNull(segmentLength));

        /**
        * Расстояние от начальной точки отрезка (segment) до проекции
        * точки p на этот отрезок. Величина может быть отрицательной.
        */
        qreal distance;
        distance = BoundaryPolygonCalc::signedProjectionDistance(p, polygon->segment(i));
        if (distance >= 0 && distance < segmentLength) {
            // Проекция попала внутрь отрезка
            PointOnBoundaryPolygon currentPoint;
            currentPoint = PointOnBoundaryPolygon(polygon, i, distance) ;
            if (firstPointIsCloser(p, currentPoint, result)) {
                // текущая точка находится ближе к p, чем предыдущая.
                result = currentPoint;
            }
        }
    }
    APointOnBoundaryPolygon aresult;
    aresult.first = result;
    aresult.second = OnPolygonSide;
    return aresult;
}

Anchor::APointOnBoundaryPolygon Anchor::discretNearestPolygonPoint(const QPointF &p,
        const PointOnBoundaryPolygon &pointOnBoundaryPolygon,
        Qt::Orientation orientation) const
{
    APointOnBoundaryPolygon aresult;
    aresult.second = OnPolygonSide;
    if (pointOnBoundaryPolygon.polygon() == NULL) {
        return aresult;
    }
    /// Сторона полигона, которой пренадлежит @p pointOnPolygon;
    QLineF segment;
    segment = pointOnBoundaryPolygon.segment();
    Q_ASSERT(!qFuzzyIsNull(segment.length()));

    // Изменяем ориентацию для горизонтальных и вертикальных сторон полигона.
    qreal angle = segment.angle();
    if (qFuzzyIsNull(angle) || qFuzzyCompare(angle, 180)) {
        orientation = Qt::Horizontal;
    } else if (qFuzzyCompare(segment.angle(), 90) || qFuzzyCompare(segment.angle(), 270)) {
        orientation = Qt::Vertical;
    }

    qreal gridSpan;
    gridSpan = qfScene()->qfView()->gridSpan();

    /// Получившиеся после дискретизации точки.
    QPointF firstPoint;
    QPointF secondPoint;
    if (orientation == Qt::Horizontal) {
        qreal x = pointOnBoundaryPolygon.toPoint().x();
        firstPoint.setX(qFloor(x / gridSpan) * gridSpan);
        firstPoint = BoundaryPolygonCalc::pointByXOnSegment(firstPoint.x(), segment);
        secondPoint.setX(qCeil(x / gridSpan) * gridSpan);
        secondPoint = BoundaryPolygonCalc::pointByXOnSegment(secondPoint.x(), segment);
    } else {
        qreal y = pointOnBoundaryPolygon.toPoint().y();
        firstPoint.setY(qFloor(y / gridSpan) * gridSpan);
        firstPoint = BoundaryPolygonCalc::pointByYOnSegment(firstPoint.y(), segment);
        secondPoint.setY(qCeil(y / gridSpan) * gridSpan);
        secondPoint = BoundaryPolygonCalc::pointByYOnSegment(secondPoint.y(), segment);
    }

    QPointF point = closestPoint(p, firstPoint, secondPoint);
    if (point != QFrost::noPoint) {
        qreal distance = QLineF(segment.p1(), point).length();
        // Если точка попала наружу сегмента (или на его конец), она не подходит
        if (distance >= segment.length()) {
            return aresult;
        }
        aresult.first = PointOnBoundaryPolygon(pointOnBoundaryPolygon.polygon(),
                                               pointOnBoundaryPolygon.index(),
                                               distance);
    }

    return aresult;
}

Anchor::APoint Anchor::nearestRectCorner(const QPointF &p, const QList< QRectF > &rects) const
{
    QPointF result = QFrost::noPoint;
    QPointF currentPoint;
    foreach(const QRectF &rect, rects) {
        currentPoint = nearestRectCorner(p, rect).first;
        if (currentPoint != QFrost::noPoint) {
            result = closestPoint(p, currentPoint, result);
        }
    }
    APoint aresult;
    aresult.first = result;
    aresult.second = OnBlockCorner;
    return aresult;
}

Anchor::APoint Anchor::nearestRectCenter(const QPointF &p, const QList< QRectF > &rects) const
{
    QPointF result = QFrost::noPoint;
    QPointF currentPoint;
    foreach(const QRectF &rect, rects) {
        result = closestPoint(p, rect.center(), result);
    }
    APoint aresult;
    aresult.first = result;
    aresult.second = OnBlockCenter;
    return aresult;
}

Anchor::APoint Anchor::nearestRectCorner(const QPointF &p, const QRectF &rect) const
{
    QPointF result;
    if (qAbs(p.x() - rect.right()) < qAbs(p.x() - rect.left())) {
        /* ближе к правой стороне */
        result.setX(rect.right());
    } else {
        /* ближе к левой стороне */
        result.setX(rect.left());
    }
    if (qAbs(p.y() - rect.bottom()) < qAbs(p.y() - rect.top())) {
        /* ближе к низу */
        result.setY(rect.bottom());
    } else {
        /* ближе к верху */
        result.setY(rect.top());
    }
    APoint aresult;
    aresult.first = result;
    aresult.second = OnBlockCorner;
    return aresult;
}

Anchor::APoint Anchor::nearestPointOnRectSide(const QPointF &p, const QList<QRectF> &rects) const
{
    QPointF result = QFrost::noPoint;
    QPointF currentPoint;
    foreach(const QRectF &rect, rects) {
        QRectF extendedRect;
        extendedRect = rect.adjusted(-mStickRadius, -mStickRadius,
                                     mStickRadius, mStickRadius);
        if (extendedRect.contains(p)) {
            currentPoint = p;
            currentPoint.setX(qBound(rect.left(), currentPoint.x(), rect.right()));
            currentPoint.setY(qBound(rect.top(), currentPoint.y(), rect.bottom()));
            currentPoint = nearestPointOnRectSide(currentPoint, rect).first;
            result = closestPoint(p, currentPoint, result);
        }
    }
    APoint aresult;
    aresult.first = result;
    aresult.second = OnBlockSide;
    return aresult;
}

Anchor::APoint Anchor::nearestPointOnRectSide(const QPointF &p, const QRectF &rect) const
{
    // сначала привязываемся к ближайшему углу
    QPointF result = nearestRectCorner(p, rect).first;

    /// шаг дискретизации
    const int step = qfScene()->qfView()->gridSpan();

    // потом дискретно сдвигаемся так, чтобы остаться на ближайшей стороне
    if (qAbs(result.x() - p.x()) < qAbs(result.y() - p.y())) {
        result.setY(qRound(p.y() / step) * step);
    } else {
        result.setX(qRound(p.x() / step) * step);
    }

    APoint aresult;
    aresult.first = result;
    aresult.second = OnBlockSide;
    return aresult;
}

bool Anchor::isInInnerPart(const QPointF &p, const QRectF &rect) const
{
    qreal margins;
    margins = qMin(rect.width() * (1 - mInnerPart),
                   rect.height() * (1 - mInnerPart));

    // если отступ в пикселах совсем маленький, считаем, что мы внутри
    static const qreal minimumMarginPx = 8;
    if (margins * mViewScale < minimumMarginPx) {
        return true;
    }

    QRectF innerRect;
    innerRect.setSize(QSizeF(rect.width() - margins, rect.height() - margins));
    innerRect.moveCenter(rect.center());

    if (innerRect.contains(p)) {
        return true;
    }
    return false;
}

QPointF Anchor::closestPoint(const QPointF &basePoint,
                             const QPointF &point1, const QPointF &point2) const
{
    if (firstPointIsCloser(basePoint, point1, point2)) {
        return point1;
    } else {
        return point2;
    }
}

bool Anchor::firstPointIsCloser(const QPointF &basePoint, const QPointF &point1, const QPointF &point2) const
{
    if (point2 == QFrost::noPoint) {
        return true;
    }

    if (point1 == QFrost::noPoint) {
        return false;
    }

    return QLineF(basePoint, point1).length() <= QLineF(basePoint, point2).length();
}

bool Anchor::firstPointIsCloser(const QPointF &basePoint,
                                const PointOnBoundaryPolygon &point_1,
                                const PointOnBoundaryPolygon &point_2) const
{
    return firstPointIsCloser(basePoint,
                              point_1.toPoint(),
                              point_2.toPoint());
}

Anchor::APoint Anchor::nearestGridNode(const QPointF &p) const
{
    QPointF result;
    int gridSpan = qfScene()->qfView()->gridSpan();
    result.setX(qRound(p.x() / gridSpan) * gridSpan);
    result.setY(qRound(p.y() / gridSpan) * gridSpan);
    Q_ASSERT(qAbs(result.x()) <= QFrost::sceneHalfSize
             && qAbs(result.y()) <= QFrost::sceneHalfSize);
    APoint aresult;
    aresult.first = result;
    aresult.second = OnVisibleGrid;
    return aresult;
}

QVariant Anchor::itemChange(QGraphicsItem::GraphicsItemChange change,
                            const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
        if (scene() != NULL) {
            connect(scene(), SIGNAL(mainViewMouseMoved(QPointF)),
                    this, SLOT(findAnchor(QPointF)));
            connect(scene(), SIGNAL(mainViewScaleChanged(qreal)),
                    this, SLOT(updateFromViewScale(qreal)));
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void Anchor::setTool(QFrost::ToolType toolType)
{
    AnchorLogicFlags oldLogic = mLogic;

    switch (toolType) {
    case QFrost::noTool:
        mLogic = NotNeeded;
        break;
    case QFrost::blockCreator:
    case QFrost::rectangularSelection:
    case QFrost::boundaryEllipseCreator:
    case QFrost::polygonalSelection:
    case QFrost::ellipseSelection:
        mLogic = NeedBlockBorders | NeedVisibleGrid;
        break;
    case QFrost::boundaryPolygonCreator:
        mLogic = NeedBlockBorders | NeedPolygons | NeedVisibleGrid;
        break;
    case QFrost::boundaryConditionsCreator:
        mLogic = NeedPolygons;
        break;
    case QFrost::curvePlot:
        mLogic = NeedBlockCenters;
        break;
    default:
        Q_ASSERT(false);
    }

    if (oldLogic != mLogic) {
        updateFoundAnchor(APoint(QFrost::noPoint, Nowhere));
    }
}
