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

#ifndef QFGUI_SOILEDITCOMMAND_H
#define QFGUI_SOILEDITCOMMAND_H

#include <undo/itemviews/itemeditcommand.h>

#include <undo/blockwitholdparameters.h>


namespace qfgui
{

class SoilEditCommand : public ItemEditCommand
{

public:
    SoilEditCommand(Item *item, const ItemChanges &changes);
    void redo();
    void undo();

protected:
    void prepareChanges(qfgui::ItemChanges &changes) const;

private:
    /// Список блоков, затронутых этим изменением
    QList<BlockWithOldTransitionParameters> mAffectedBlocks;
};

}

#endif // QFGUI_SOILEDITCOMMAND_H
