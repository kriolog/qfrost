/*
 * Copyright (C) 2011-2012  Denis Pesotsky
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

#ifndef QFGUI_BOUNDARYPOLYGONPORTABLE_H
#define QFGUI_BOUNDARYPOLYGONPORTABLE_H

#include <boundarypolygon.h>

namespace qfgui
{

/**
 * Точка смены свойств на полигоне граничных условий (вместо указателей
 * на граничные условия гранит их номера)
 */
struct PortableConditionPoint {
    qreal distance;
    qint32 conditionID;
};

/**
 * Угол полигона граничных условий (вместо указателей
 * на граничные условия гранит их номера).
 */
struct PortableVertex {
    QPointF point;
    QList<PortableConditionPoint> conditionPoints;
};

/**
 * Информация о граничном полигоне, которая нужна для его записи/считывания.
 */
struct BoundaryPolygonPortable {
    BoundaryPolygonPortable(BoundaryPolygon *boundaryPolygon);
    BoundaryPolygonPortable() {
        // Для QList
    }
    QList<PortableVertex> corners;
    QList<BoundaryPolygonPortable> children;

    /// Создаёт полигон исходя из граничных условий @p conditions
    BoundaryPolygon *createPolygon(const QList<const BoundaryCondition *>conditions);
};

}

QDataStream &operator<< (QDataStream &out, const qfgui::PortableVertex &p);
QDataStream &operator>> (QDataStream &in, qfgui::PortableVertex &p);
QDataStream &operator<< (QDataStream &out, const qfgui::PortableConditionPoint &p);
QDataStream &operator>> (QDataStream &in, qfgui::PortableConditionPoint &p);
QDataStream &operator<< (QDataStream &out, const qfgui::BoundaryPolygonPortable &p);
QDataStream &operator>> (QDataStream &in, qfgui::BoundaryPolygonPortable &p);

#endif // QFGUI_BOUNDARYPOLYGONPORTABLE_H
