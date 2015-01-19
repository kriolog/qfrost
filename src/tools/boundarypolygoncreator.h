/*
 * Copyright (C) 2011-2015  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_BOUNDARYPOLYGONCREATOR_H
#define QFGUI_BOUNDARYPOLYGONCREATOR_H

#include <tools/tool.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(PointOnBoundaryPolygon)
QT_FORWARD_DECLARE_CLASS(GrowingPolygon)

/**
 * Создавалка полигонов. Принимает клики из сцены и ставит там точки полигона.
 * Никак не ограничивает пользователя в создании полигона, т.е. пользователь
 * может создать и самопересекающйся полигон.
 */
class BoundaryPolygonCreator : public Tool
{
    Q_OBJECT
public:
    BoundaryPolygonCreator();

    /**
     * Прибавляет или вычитает результирующий полигон из полигонов
     * сцены в зависимости от модификатора, после чего самоудаляется.
     * @param alt - true, если надо вычесть.
     */
    void apply(bool alt);

    void cancelLastChange();

    QPointF visualCenter() const;

protected:
    void onSceneHasChanged();

private:
    GrowingPolygon *mGrowingPolygon;

private slots:
    void addPoint(const QPointF &point);
    void addPoint(const PointOnBoundaryPolygon &point);
};

}

#endif // QFGUI_BOUNDARYPOLYGONCREATOR_H
