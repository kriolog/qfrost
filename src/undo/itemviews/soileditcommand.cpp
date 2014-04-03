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

#include "soileditcommand.h"

#include <soils/soil.h>

using namespace qfgui;

SoilEditCommand::SoilEditCommand(Item *item, const ItemChanges &changes)
    : ItemEditCommand(item, changes)
{
    Q_ASSERT(qobject_cast<Soil *>(item) != NULL);
    foreach(Block * block, qobject_cast<Soil *>(item)->blocks()) {
        mAffectedBlocks.append(BlockWithOldTransitionParameters(block));
    }
}

void SoilEditCommand::prepareChanges(ItemChanges &changes) const
{
    const Soil *soil = qobject_cast<const Soil * >(item());
    Q_ASSERT(soil != NULL);

    const bool nowUsingUnfrozenWaterCurve = soil->usesUnfrozenWaterCurve();
    const bool unfrozenWaterUsageToggled = changes.contains("usesUnfrozenWaterCurve");
    const bool wasUsingUnfrozenWaterCurve = (nowUsingUnfrozenWaterCurve && !unfrozenWaterUsageToggled)
                                            || (!nowUsingUnfrozenWaterCurve && unfrozenWaterUsageToggled);

    const ItemChanges::Iterator t = changes.find("transitionTemperature");
    const ItemChanges::Iterator q = changes.find("transitionHeat");
    if (nowUsingUnfrozenWaterCurve) {
        // Грунт после redo использует кривую незамёрзшей воды
        // Температура и теплота ф.п. сами пересчитываются при redo
        if (t != changes.end()) {
            t->second = QVariant();
        }
        if (q != changes.end()) {
            q->second = QVariant();
        }
    }

    if (wasUsingUnfrozenWaterCurve) {
        // Грунт до redo использовал кривую незамёрзшей воды
        // Температура и теплота ф.п. сами пересчитываются при undo
        if (t != changes.end()) {
            t->first = QVariant();
        }
        if (q != changes.end()) {
            q->first = QVariant();
        }
    }

    ItemEditCommand::prepareChanges(changes);
}

void SoilEditCommand::redo()
{
    ItemEditCommand::redo();
    QList<BlockWithOldTransitionParameters>::Iterator i;
    for (i = mAffectedBlocks.begin(); i != mAffectedBlocks.end(); ++i) {
        i->getNewPropertiesFromSoil();
    }
}

void SoilEditCommand::undo()
{
    ItemEditCommand::undo();
    QList<BlockWithOldTransitionParameters>::Iterator i;
    for (i = mAffectedBlocks.begin(); i != mAffectedBlocks.end(); ++i) {
        i->restore();
    }
}
