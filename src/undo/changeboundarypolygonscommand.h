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

#ifndef QFGUI_CHANGEBOUNDARYPOLYGONSCOMMAND_H
#define QFGUI_CHANGEBOUNDARYPOLYGONSCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <QtCore/QPair>

#include <qfrost.h>

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(Scene)

class ChangeBoundaryPolygonsCommand: public QUndoCommand
{
public:
    ChangeBoundaryPolygonsCommand(Scene *scene,
                                  QPair<BoundaryPolygonList, BoundaryPolygonList> diff,
                                  const QString &text, QUndoCommand *parent = NULL);
    void undo();
    void redo();
private:
    BoundaryPolygonList mPolygonsToRemove;
    BoundaryPolygonList mPolygonsToAdd;
    Scene *mScene;
};

}

#endif // QFGUI_CHANGEBOUNDARYPOLYGONSCOMMAND_H
