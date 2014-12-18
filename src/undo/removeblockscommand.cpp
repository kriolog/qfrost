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

#include <undo/removeblockscommand.h>

#include <graphicsviews/scene.h>
#include <graphicsviews/block.h>

using namespace qfgui;

RemoveBlocksCommand::RemoveBlocksCommand(Scene *scene,
        const QList< Block * > &blocks,
        QUndoCommand *parent)
    : QUndoCommand(parent)
    , mScene(scene)
    , mBlocks(blocks)
{
    setText(QUndoStack::tr("remove %n block(s)", "", mBlocks.size()));

    foreach(Block * block, mBlocks) {
        block->increaseUndoReferenceCount();
    }
}

void RemoveBlocksCommand::redo()
{
    QList<Block *>::iterator it;
    for (it = mBlocks.begin(); it != mBlocks.end(); ++it) {
        (*it)->destroyTopLinks(mBlocks);
        (*it)->destroyBottomLinks(mBlocks);
        (*it)->destroyLeftLinks(mBlocks);
        (*it)->destroyRightLinks(mBlocks);
    }
    mScene->replaceBlocks(mBlocks, QList<Block *>());
}

void RemoveBlocksCommand::undo()
{
    QList<Block *>::iterator it;
    for (it = mBlocks.begin(); it != mBlocks.end(); ++it) {
        (*it)->restoreTopLinks(mBlocks);
        (*it)->restoreBottomLinks(mBlocks);
        (*it)->restoreLeftLinks(mBlocks);
        (*it)->restoreRightLinks(mBlocks);
    }
    mScene->replaceBlocks(QList<Block *>(), mBlocks);
}

RemoveBlocksCommand::~RemoveBlocksCommand()
{
    foreach(Block * block, mBlocks) {
        block->decreaseUndoReferenceCount();
    }
}
