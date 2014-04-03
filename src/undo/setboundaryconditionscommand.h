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

#ifndef QFGUI_SETBOUNDARYCONDITIONSCOMMAND_H
#define QFGUI_SETBOUNDARYCONDITIONSCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <boundarypolygon.h>

namespace qfgui
{

class SetBoundaryConditionsCommand : public QUndoCommand
{
public:
    SetBoundaryConditionsCommand(BoundaryPolygon *polygon,
                                 const QList<Vertex> &newCorners,
                                 QUndoCommand *parent = NULL);
    void undo();
    void redo();
private:
    BoundaryPolygon *mPolygon;
    QList<Vertex> mOldCorners;
    QList<Vertex> mNewCorners;
};

}

#endif // QFGUI_SETBOUNDARYCONDITIONSCOMMAND_H
