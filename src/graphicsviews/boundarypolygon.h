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

#ifndef QFGUI_BOUNDARYPOLYGON_H
#define QFGUI_BOUNDARYPOLYGON_H

#include <graphicsviews/nonscalableitem.h>
#include <qfrost.h>
#include <boundary_conditions/boundarycondition.h>
#include <graphicsviews/block.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Block)

/**
 * Точка смены свойств на полигоне граничных условий
 */
struct ConditionPoint {
    ConditionPoint(): distance(0), condition(BoundaryCondition::voidCondition()) {}
    ConditionPoint(qreal distance, const BoundaryCondition *condition):
        distance(distance),
        condition(condition) {
        Q_ASSERT(condition != NULL);
    }
    qreal distance;
    const BoundaryCondition *condition;
};

/**
 * Угол полигона граничных условий. Содержит информацию о его угловых точках
 * и точках смены условия
 */
struct Vertex {
    QPointF point;
    QList<ConditionPoint> conditionPoints;
};

/**
 * Часть полигона граничных условий с определёнными свойствами
 */
struct BoundaryPolyline {
    QPolygonF polygon;
    const BoundaryCondition *condition;
};

/**
 * Полигон с точками смены типа граничных условий.
 */
class BoundaryPolygon : public NonScalableItem
{
    Q_OBJECT
public:
    /**
     * Создание полигона  граничных условий  с заданным свойством по
     * данному полигону.
     */
    BoundaryPolygon(const QPolygonF &polygon,
                    const BoundaryCondition *condition);

    /**
     * Создание граничного условия по списку углов.
     * Проверяется наличие точек смены условия. Если точка смены условия одна,
     * она переносится в нулевой угол.
     */
    BoundaryPolygon(const QList<Vertex> &corners);

    /**
     * Конструктор, копирующий углы из @p other.
     */
    BoundaryPolygon(const BoundaryPolygon *other);

    DEFINETYPE(BOUNDARYPOLYGONTYPE)

    QPainterPath shape() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    /// Представление в виде обычного полигона
    QPolygonF polygon() const;

    /// Список точек смены условия
    QList<QPointF> conditionPoints() const;

    /**
     * Список углов полигона граничных условий, содержащий
     * информацию о его угловых точках и точках смены условия.
     * FIXME надо бы сделать отдельную функцию только для углов
     *       (получать отсюда с симплификацией прямых участков)
     */
    const QList<Vertex> &corners() const {
        return mCorners;
    }

    /**
     * Конвертирует список Vertex'ов (отражающий все свойства полигона) в
     * полилинии, каждая из которых имеет своё условия.
     */
    static QList<BoundaryPolyline> boundaryPolylines(const QList<Vertex> &corners);

    /// Полилинии, на которые возможно разбить полигон (разбивка по гр.условиям)
    QList<BoundaryPolyline> boundaryPolylines() const;

    /// Пересекается ли полигон граничных условий с данным полигоном.
    bool intersects(const QPolygonF &polygon) const;

    /**
     * Пересекается ли полигон граничных условий с данным полигоном.
     * Пересечение не может состоять только из изолированых точек.
     */
    bool intersectsExcludingOnlyPoints(const QPolygonF &polygon) const;

    /**
     * Пересекается ли полигон граничных условий с данным полигоном.
     * Пересечение в изолированных точках по отрезкам запрещено
     * (т.о. пересечением могут быть только фигуры).
     */
    bool intersectsExcludingBorder(const QPolygonF &polygon) const;

    /**
     * Содержит ли полигон граничных условий данный полигон. Если да, то у
     * полигонов не может быть общих границ
     */
    bool contains(const QPolygonF &polygon) const;

    /// Нужно ибо contains(const QPolygonF&) скрывает этот метод.
    bool contains(const QPointF &point) const {
        return QGraphicsItem::contains(point);
    }

    /// Список полигонов внутри этого полигона (дырок)
    QList<BoundaryPolygon *> childBoundaryPolygonItems() const {
        QList<BoundaryPolygon *> polygonItems;
        foreach(QGraphicsItem * item, childItems()) {
            if (BoundaryPolygon *polygonItem = qgraphicsitem_cast<BoundaryPolygon *>(item)) {
                polygonItems.append(polygonItem);
            }
        }
        return polygonItems;
    }

    /**
     * Отрезок, соединяющий i-ю и i+1-ю точки полигона.
     * @param safe допускает i вне [0; mCornersNum-1): учёт цикличности полигона
     */
    QLineF segment(int i, bool safe = false) const;

    bool isInner() const {
        return parent() != NULL;
    }
    bool isOuter() const {
        return !isInner();
    }

private:
    /// Список углов
    QList<Vertex> mCorners;

    /**
     * Задаёт список углов полигона граничных условий, содержащий
     * информацию о его угловых точках и точках смены условия.
     */
    void setCorners(const QList<Vertex> &corners) {
        prepareGeometryChange();
        mCorners = corners;
    }

    friend class SetBoundaryConditionsCommand;
    friend class BoundaryConditionsRemoveCommand;

protected:
    QRectF baseRect() const {
        return polygon().boundingRect();
    }

    /**
     * Тут мы отслеживаем смену сцены, чтобы обновлять связь с блоками
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

bool operator== (const qfgui::Vertex &v1, const qfgui::Vertex &v2);
bool operator== (const qfgui::ConditionPoint &c1, const qfgui::ConditionPoint &c2);

}


#endif // QFGUI_BOUNDARYPOLYGON_H
