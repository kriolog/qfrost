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

#include "soilsremovecommand.h"

#include <itemviews/item.h>
#include <soils/soil.h>

using namespace qfgui;

SoilsRemoveCommand::SoilsRemoveCommand(const QList<Item *> &items)
    : ItemsRemoveCommand(items)
    , mAffectedBlocks()
{
    foreach(Item * item, items) {
        Soil *soil = qobject_cast<Soil *>(item);
        Q_ASSERT(soil != NULL);
        foreach(Block * block, soil->blocks()) {
            mAffectedBlocks.append(BlockWithOldSoil(block));
        }
    }

    setText(names().size() == 1
            ? (QUndoStack::tr("remove soil %1")
               .arg(names().first()) + "\n"
               //: Short action name. Accusative case.
               + QUndoStack::tr("remove soil"))
            : (QUndoStack::tr("remove soils %1").arg(names().join(", ")) + "\n"
               //: Short action name. Accusative case.
               + QUndoStack::tr("remove %n soil(s)", "", names().size())));
}

void SoilsRemoveCommand::redo()
{
    ItemsRemoveCommand::redo();
    for (QList<BlockWithOldSoil>::Iterator i = mAffectedBlocks.begin();
            i != mAffectedBlocks.end(); ++i) {
        i->setSoil(NULL);
    }
}

void SoilsRemoveCommand::undo()
{
    ItemsRemoveCommand::undo();
    for (QList<BlockWithOldSoil>::Iterator i = mAffectedBlocks.begin();
            i != mAffectedBlocks.end(); ++i) {
        i->restore();
    }
}
