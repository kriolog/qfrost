/*
 * Copyright (C) 2010-2012  Denis Pesotsky
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

#include <undo/setblockssoilcommand.h>

#include <undo/blockwitholdparameters.h>

#include <graphicsviews/block.h>

using namespace qfgui;

SetBlocksSoilCommand::SetBlocksSoilCommand(const QList<Block *> &blocks,
        const Soil *soil,
        QUndoCommand *parent)
    : QUndoCommand(parent)
    , mBlocks()
    , mSoil(soil)
{
    QList<Block *>::ConstIterator i;
    for (i = blocks.constBegin(); i != blocks.constEnd(); ++i) {
        if ((*i)->mSoil != soil) {
            mBlocks.push_back(BlockWithOldSoil(*i));
        }
    }

    setText(QUndoStack::tr("apply soil to %n block(s)", "", mBlocks.size()) + "\n"
            //: Short action name. Accusative case.
            + QUndoStack::tr("apply soil"));
}

void SetBlocksSoilCommand::redo()
{
    QList<BlockWithOldSoil>::Iterator i;
    for (i = mBlocks.begin(); i != mBlocks.end(); ++i) {
        i->setSoil(mSoil);
    }
}

void SetBlocksSoilCommand::undo()
{
    QList<BlockWithOldSoil>::Iterator i;
    for (i = mBlocks.begin(); i != mBlocks.end(); ++i) {
        i->restore();
    }
}
