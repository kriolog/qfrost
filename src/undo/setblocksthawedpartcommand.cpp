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

#include <undo/setblocksthawedpartcommand.h>
#include <graphicsviews/block.h>

using namespace qfgui;

SetBlocksThawedPartCommand *SetBlocksThawedPartCommand::createCommand(const QList< Block * > &blocks,
        double thawedPart,
        QUndoCommand *parent)
{
    SetBlocksThawedPartCommand *c = new SetBlocksThawedPartCommand(blocks,
            thawedPart,
            parent);
    if (c->mBlocks.isEmpty()) {
        delete c;
        return NULL;
    } else {
        return c;
    }
}

SetBlocksThawedPartCommand::SetBlocksThawedPartCommand(const QList <Block *> &blocks,
        double thawedPart,
        QUndoCommand *parent)
    : QUndoCommand(parent)
    , mBlocks()
    , mNewV(thawedPart)
{
    QList<Block *>::ConstIterator i;
    for (i = blocks.constBegin(); i != blocks.constEnd(); ++i) {
        const qfcore::SoilBlock *s = (*i)->soilBlock();
        if (s->canChangeThawedPart() && s->thawedPart() != thawedPart) {
            if (s->usesUnfrozenWaterCurve()) {
                double minV = s->minBFThawedPart();
                if (s->thawedPart() == minV && thawedPart <= minV) {
                    continue;
                }
            }
            mBlocks.append(BlockWithOldThawedPart(*i));
        }
    }
    setText(QUndoStack::tr("apply thawed part to %n block(s)",
                           "", mBlocks.size()) + "\n"
            //: Short action name. Accusative case.
            + QUndoStack::tr("apply thawed part"));
}

void SetBlocksThawedPartCommand::redo()
{
    QList<BlockWithOldThawedPart>::Iterator i;
    for (i = mBlocks.begin(); i != mBlocks.end(); ++i) {
        i->smartSetThawedPart(mNewV);
    }
}

void SetBlocksThawedPartCommand::undo()
{
    QList<BlockWithOldThawedPart>::Iterator i;
    for (i = mBlocks.begin(); i != mBlocks.end(); ++i) {
        i->restore();
    }
}
