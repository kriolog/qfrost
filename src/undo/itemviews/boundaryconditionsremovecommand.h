/*
 * Copyright (C) 2012  Denis Pesotsky
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

#ifndef QFGUI_BOUNDARYCONDITIONSREMOVECOMMAND_H
#define QFGUI_BOUNDARYCONDITIONSREMOVECOMMAND_H

#include <undo/itemviews/itemsremovecommand.h>

#include <QtCore/QPair>
#include <QtCore/QMap>


namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(BoundaryPolygon)
QT_FORWARD_DECLARE_CLASS(Vertex)

class BoundaryConditionsRemoveCommand : public ItemsRemoveCommand
{

public:
    BoundaryConditionsRemoveCommand(const QList<Item *> &items);
    void redo();
    void undo();

private:
    typedef QMap<BoundaryPolygon *, QPair<QList<Vertex>, QList<Vertex> > > ChangedPolygons;
    /// Список затронутых удалением граничного условия полигонов со старыми
    /// и новыми раскрашенностями.
    ChangedPolygons mAffectedPolygons;
};

}

#endif // QFGUI_BOUNDARYCONDITIONSREMOVECOMMAND_H
