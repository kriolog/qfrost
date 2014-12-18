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

#ifndef QFGUI_ANCHOR_H
#define QFGUI_ANCHOR_H

#include <QtWidgets/QGraphicsObject>
#include <QtCore/qmath.h>

#include <qfrost.h>
#include <graphicsviews/scene.h>
#include <pointonboundarypolygon.h>

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(Block)
QT_FORWARD_DECLARE_CLASS(BoundaryPolygon)
QT_FORWARD_DECLARE_CLASS(View)

/**
 * Привязка вместе с логикой собственной работы.
 */
class Anchor: public QGraphicsObject
{
    Q_OBJECT
public:
    Anchor();

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = NULL);
    QRectF boundingRect() const;

    /**
     * Радиус прилипания.
     * Может использоваться, если нужна аналогичная функционтальность где-либо
     */
    qreal stickRadius() const {
        return mStickRadius;
    }

    const PointOnBoundaryPolygon &posOnPolygon() const {
        return mPosOnPolygon;
    }

    /// Изменяет способ привязки в зависимости от @p toolType.
    void setTool(QFrost::ToolType toolType);

    enum AnchorLogicFlag {
        NotNeeded = 0x0,
        /// Нужна привязка к блокам (к их углам и сторонам)
        NeedBlocks = 0x1,
        /// Нужна привязка к видимой сетке
        NeedVisibleGrid = 0x2,
        /// Нужна привязка к полигонам (к их углам, сторонам и точкам смены условия)
        NeedPolygons = 0x4
    };
    Q_DECLARE_FLAGS(AnchorLogicFlags, AnchorLogicFlag)

public slots:
    /**
     * Перестаёт реагировать на сигналы об изменении позиции курсора
     */
    void freeze() {
        mIsNeeded = false;
    }

    /**
     * Начинает реагировать на сигналы об изменении позиции курсора.
     */
    void unfreeze() {
        mIsNeeded = true;
    }

    /// Поиск привязки, которым можно пользоваться, если сделан freeze()
    void findAnchorFreezed(const QPointF &point);

private slots:
    /**
     * Находит анкер по активному алгоритму поиска привязки в зависимости от
     * текущего инструмента.
     * @param point - положение курсора в координатах сцены.
     */
    void findAnchor(const QPointF &point);

    /**
     * Обновляет mStickRadius исходя из шага сетки отображения и
     * mViewScale исходя из @p scale
     */
    void updateFromViewScale(qreal scale);

signals:
    /**
     * Сигнал о том, что позиция привязки изменилась.
     * @param newPosition новая позиция.
     */
    void signalPositionChanged(const QPointF &newPosition);

    void signalPositionChanged(const PointOnBoundaryPolygon &newPosition);

private:
    /**
     * Размер ограничивающего прямоугольника привязки.
     * Размер должен давать в остатке 2 после деления на 4 (более, чем в корень
     * из двух раз) и делиться на 8 = 4 * 2.
     * Используется при расчёте boundingRect() и при рисовании.
     */
    static const int mBoundingSize = 17;

    /**
     * Отношение размеров внутреннего и внешнего прямоугольников блока.
     */
    const qreal mInnerPart;

    /**
     * Привязка по полигону  граничных условий в терминах полигона с указателем
     * на него.
     */
    PointOnBoundaryPolygon mPosOnPolygon;

    /**
     * Угол (в градусах) поворота изображения привязки в соответствии с
     * направлением отрезка.
     * Актуально для привязки по отрезку полигона граничных условий.
     */
    qreal mAnchorAngle;

    /**
     * Радиус, "прилипания" используемый для некоторых типов привязки.
     * Например относящихся к полигонам. На этом расстоянии может произойти
     * срабатывание привязки.
     */
    qreal mStickRadius;

    /// Нужна привязка к блокам (к их углам и сторонам)
    bool mNeedBlocks;

    /// Нужна привязка к полигонам (к их углам, сторонам и точкам смены условия)
    bool mNeedPolygons;

    /// Нужна привязка к видимой сетке
    bool mNeedVisibleGrid;

    /// Необходимые привязки
    AnchorLogicFlags mLogic;

    /// Типы привязки (низкоуровневые)
    enum FoundAnchorType {
        Nowhere,
        OnBlockCorner,
        OnBlockSide,
        OnVisibleGrid,
        OnPolygonSide,
        OnPolygonCorner,
        OnPolygonConditionPoint
    };

    FoundAnchorType mFoundAnchorType;

    bool mIsNeeded;

    /// Масштаб главного отображения
    qreal mViewScale;

    typedef QPair<QPointF, FoundAnchorType> APoint;
    typedef QPair<PointOnBoundaryPolygon, FoundAnchorType> APointOnBoundaryPolygon;

    /**
     * Обновляет привязку.
     */
    void updateFoundAnchor(const APoint &pointToAnchor);

    /**
     * Перегруженный метод. Обновляет привязку, если она была найдена по
     * полигону граничных условий.
     */
    void updateFoundAnchor(const APointOnBoundaryPolygon &pointToAnchor);

    Scene *qfScene() const {
        Scene *result = qobject_cast<Scene *>(scene());
        Q_ASSERT(result != NULL);
        return result;
    }

    /**
     * Привязка по полигонам.
     */
    APointOnBoundaryPolygon anchorOnBoundaryPolygon(const QPointF &point);

    /**
     * Привязка по блокам.
     */
    APoint anchorOnBlock(const QPointF &point);

    /**
     * Прямоугольник со стороной 2*mStickRadius и центром в @p point.
     */
    QRectF stickRect(const QPointF &point) {
        return QRectF(-mStickRadius + point.x(), -mStickRadius + point.y(),
                      2 * mStickRadius, 2 * mStickRadius);
    }

    /**
     * Ближайший к @a p из углов списка полигонов @a polygons
     */
    APointOnBoundaryPolygon nearestCornerOnBoundaryPolygon(const QPointF &p,
            const BoundaryPolygonList &polygons) const;

    /**
     * Ближайшая к @a p из точек смены условия списка полигонов @a polygons
     */
    APointOnBoundaryPolygon nearestConditionPointOnBoundaryPolygon(const QPointF &p,
            const BoundaryPolygonList &polygons) const;

    /**
     * Ближайшая к @a p из точек списка полигонов @a polygons. Если таковая
     * найдена, обновляется угол наклона привязки.
     */
    APointOnBoundaryPolygon nearestPointOnBoundaryPolygon(const QPointF &p,
            const BoundaryPolygonList &polygons) const;

    /**
     * Ближайший к @a p из углов полигона @a polygon
     */
    APointOnBoundaryPolygon nearestCornerOnBoundaryPolygon(const QPointF &p,
            const BoundaryPolygon *polygon) const;

    /**
     * Ближайшая к @a p из точек смены условия полигона @a polygon
     */
    APointOnBoundaryPolygon nearestConditionPointOnBoundaryPolygon(const QPointF &p,
            const BoundaryPolygon *polygon) const;

    /**
     * Ближайшая к @a p из точек полигона @a polygon
     */
    APointOnBoundaryPolygon nearestPointOnBoundaryPolygon(const QPointF &p,
            const BoundaryPolygon *polygon) const;

    /**
     * Выполняет дискритезацию @a pointOnPolygon по видимой сетке, то есть
     * сдвигает её по стороне полигона, которой она принадлежит, в двух
     * противоположных направлениях до тех пор, пока заданная по @a orientation
     * координата не станет кратной gridSpan. Получившиеся две точки
     * сравниваются по принципу наименьшего расстояния до @a p.
     *
     * @param p Точка, по расстоянию до которой делается выбор результата.
     *
     * @param pointOnPolygon Точка на полигоне граничных условий, которую
     * нужно сдвинуть по стороне, которой она пренадлежит так, чтобы одна
     * (заданная) из её координат округлилась по видимой сетке.
     *
     * @param orientation Задаёт координату, по которой ведётся дискретизация.
     *
     */
    APointOnBoundaryPolygon discretNearestPolygonPoint(const QPointF &p,
            const PointOnBoundaryPolygon &pointOnPolygon,
            Qt::Orientation orientation) const;

    /**
     * Ближайший к @a p из углов списка прямоугольников @a rects
     */
    APoint nearestRectCorner(const QPointF &p, const QList<QRectF> &rects) const;

    /**
     * Ближайший к @a p из углов прямоугольника @a rect
     */
    APoint nearestRectCorner(const QPointF &p, const QRectF &rect) const;

    /**
     * Ближайшая точка (округлённая с точностью до шага сетки) на ближайшей к
     * @a p стороне из списка прямоугольников @a rects.
     */
    APoint nearestPointOnRectSide(const QPointF &p, const QList<QRectF> &rects) const;

    /**
     * Ближайшая точка (округлённая с точностью до шага сетки) на ближайшей к
     * @a p стороне прямоугольника @a rect, которому принадлежит @a p.
     */
    APoint nearestPointOnRectSide(const QPointF &p, const QRectF &rect) const;

    /**
     * Находится ли точка во внутренней части прямоугольника
     */
    bool isInInnerPart(const QPointF &p, const QRectF &rect) const;

    /**
     * Ближайшая из @a point_1 и @a point_2 к @a basePoint точка.
     * Если одна из точек это @c QFrost::noPoint, возвращает другую.
     */
    QPointF closestPoint(const QPointF &basePoint,
                         const QPointF &point_1,
                         const QPointF &point_2) const;

    /**
     * Находится ли @a point_1 ближе к @a basePoint, чем @a point_2.
     * Если расстояния равны, возвращает true.
     * Если @a point_1 это QFrost::noPoint, возвращает false.
     * Если @a point_2 это QFrost::noPoint, возвращает true.
     */
    bool firstPointIsCloser(const QPointF &basePoint,
                            const QPointF &point_1,
                            const QPointF &point_2) const;

    /**
     * Перегруженный метод. Сравниваемые точки задаются в терминах полигона
     * граничных условий.
     * Находится ли @a point_1 ближе к @a basePoint, чем @a point_2.
     * Если @a point_2 это QFrost::noPoint, возвращает true.
     * Если @a point_1 это QFrost::noPoint, возвращает false.
     */
    bool firstPointIsCloser(const QPointF &basePoint,
                            const PointOnBoundaryPolygon &point_1,
                            const PointOnBoundaryPolygon &point_2) const;

    /**
     * Ближайший к @a p узел сетки, рисуемой во view, без учёта перекрытия
     * сетки блоками и т.п.
     */
    APoint nearestGridNode(const QPointF &p) const;

    /// Тут мы отслеживаем смену сцены, чтобы установить connect'ы.
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    /**
     * Расстояние между точками не превышает @a mStickRadius.
     * Возвращает false, если одна из точек является несуществующей
     */
    bool pointsAreCloseEnought(const QPointF &p1, const QPointF &p2) const {
        return !(p1 == QFrost::noPoint || p2 == QFrost::noPoint)
               && QLineF(p1, p2).length() <= mStickRadius;
    }

    bool pointsAreCloseEnought(const PointOnBoundaryPolygon &p1,
                               const PointOnBoundaryPolygon &p2) const {
        return pointsAreCloseEnought(p1.toPoint(), p2.toPoint());
    }
};

}
Q_DECLARE_OPERATORS_FOR_FLAGS(qfgui::Anchor::AnchorLogicFlags)

#endif // QFGUI_ANCHOR_H
