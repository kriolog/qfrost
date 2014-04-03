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

#include <undo/changeboundarypolygonscommand.h>

#include <scene.h>

using namespace qfgui;

ChangeBoundaryPolygonsCommand::ChangeBoundaryPolygonsCommand(
    Scene *scene, QPair<BoundaryPolygonList, BoundaryPolygonList> diff,
    const QString &text,  QUndoCommand *parent)
    : QUndoCommand(text, parent)
    , mPolygonsToRemove(diff.second)
    , mPolygonsToAdd(diff.first)
    , mScene(scene)
{
    Q_ASSERT(!mPolygonsToAdd.empty() || !mPolygonsToRemove.empty());
}

void ChangeBoundaryPolygonsCommand::redo()
{
    mScene->removeItems(mPolygonsToRemove);
    mScene->addItems(mPolygonsToAdd);
}

void ChangeBoundaryPolygonsCommand::undo()
{
    mScene->removeItems(mPolygonsToAdd);
    mScene->addItems(mPolygonsToRemove);
}
