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

#ifndef BOUNDARYCONDITIONSAPPLICATOR_H
#define BOUNDARYCONDITIONSAPPLICATOR_H

#include <tools/tool.h>

#include <qfrost.h>
#include <pointonboundarypolygon.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(BoundaryCondition)
QT_FORWARD_DECLARE_CLASS(BoundaryPolygon)
QT_FORWARD_DECLARE_CLASS(Scene)
QT_FORWARD_DECLARE_CLASS(Anchor)
QT_FORWARD_DECLARE_CLASS(Polyline)

class BoundaryConditionsApplicator: public Tool
{
    Q_OBJECT
public:
    BoundaryConditionsApplicator();

    const BoundaryPolygon *polygonToRepaint() const {
        Q_ASSERT(!mOriginPoint.isNull());
        return mOriginPoint.polygon();
    }

    /**
     * Привязались ли мы уже к определённному полигону.
     * Становится равен true при первом нажатии мыши, до этого равен false.
     */
    bool isStickedToPolygon() {
        if (mIsStickedToPolygon) {
            Q_ASSERT(!mOriginPoint.isNull());
        }
        return mIsStickedToPolygon;
    }

    void apply(bool alt) {
        Q_UNUSED(alt)
        /* Метод apply привязан к нажатию на энтер, этот инструмент будет
         * применяться по левому клику мыши. */
    }

protected:
    void onSceneHasChanged();

private:
    /**
     * Привязались ли мы уже к определённному полигону.
     * Становится равен true при первом нажатии мыши, до этого равен false.
     */
    bool mIsStickedToPolygon;

    /**
     * Показывает порядок углов полилинии, полаемой из полигона по двум точкам.
     */
    enum {
        // порядок углов полилинии совпадает с порядком углов полигона
        straight,
        // порядок углов полилинии обратный к порядку углов полигона
        reverse,
        // порядок углов ещё не определён
        undefined
    } mPolylineOrder;

    /// Начальная точка для получения полилинии
    PointOnBoundaryPolygon mOriginPoint;

    /// Конечная точка для получения полилинии
    PointOnBoundaryPolygon mEndPoint;

    /// Объект, который будет рисовать немасштабируемую полилинию
    Polyline *mPolyline;

    /**
     * Можно ли в данный момент применять граничные условия ко всему
     * полигону (а вместе с тем и рисовать замкнутые полилинии, равные всему
     * полигону).
     */
    bool mAllowApplyConditionToWhole;

    /**
     * Возвращает полилинию, порядок точек в которой совпадает с порядком точек
     * исходного полигона. Если полигона не существует, точки выходят за рамки
     * полигона (по номеру или расстоянию), или полигон является "замкнутой"
     * точкой, возвращается пустой список. Если точки совпадают, полилиния равна
     * исходному полигону.
     */
    QPolygonF straightOrderedPolyline() const;

    /**
     * Возвращает полилинию, порядок точек в которой обратный к порядку точек
     * исходного полигона. Если полигона не существует, точки выходят за рамки
     * полигона (по номеру или расстоянию), или полигон является "замкнутой"
     * точкой, возвращается пустой список. Если точки совпадают, полилиния равна
     * исходному полигону.
     */
    QPolygonF reverseOrderedPolyline() const;

    /**
     * Возвращает длину полигона.
     */
    static qreal length(const QPolygonF &polygon);

    /**
     * Указатель на сцену, приведённый к типу Scene.
     */
    Scene *qfScene() const;

    QList< BoundaryCondition * > boundaryConditions() const;

private slots:
    /**
     * Метод заменяет свойства на полигоне граничных условий на участке,
     * соответствующем полученной полилинии, если только её удалось получить.
     * Если первая точка до вызова метода ещё не была зафиксирована, она
     * фиксируется.
     */
    void tryToApplyPolyline();

    /**
     * Метод обновляет конечную или начальную (если она ещё не зафиксирована)
     * точку и вызывает метод объекта класса Polyline, чтобы нарисовать
     * полилинию с новой геометрией.
     */
    void updatePolyline(const PointOnBoundaryPolygon &p);
};

}

#endif // BOUNDARYCONDITIONSAPPLICATOR_H
