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

#include <undo/addblockscommand.h>

#include <block.h>
#include <scene.h>
#include <undo/changeboundarypolygonscommand.h>
#include <boundarypolygoncalc.h>
#include <boundarypolygon.h>
#include <mainwindow.h>

using namespace qfgui;

#include <QDebug>
AddBlocksCommand::AddBlocksCommand(Scene *scene,
                                   const QList<QList<QRectF> > &blocksRects,
                                   bool mustChangeOuterPolygons,
                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , mScene(scene)
    , mAddedBlocks()
    , mAddedBlocksTop()
    , mAddedBlocksBottom()
    , mAddedBlocksLeft()
    , mAddedBlocksRight()
    , mRemovedBlocks()
    , mRemovedBlocksTop()
    , mRemovedBlocksBottom()
    , mRemovedBlocksLeft()
    , mRemovedBlocksRight()
{
    Q_ASSERT(!blocksRects.isEmpty());
    Q_ASSERT(!blocksRects.first().isEmpty());

    QRectF boundingRect;
    boundingRect.setTopLeft(blocksRects.first().first().topLeft());
    boundingRect.setBottomRight(blocksRects.last().last().bottomRight());

    mRemovedBlocks = scene->blocks(boundingRect);
    foreach(Block * block, mRemovedBlocks) {
        block->increaseUndoReferenceCount();
    }

    QRectF topRect = boundingRect;
    topRect.setTop(boundingRect.top() + QFrost::microSizeF / 2);
    topRect.setBottom(boundingRect.top() + QFrost::microSizeF);
    mRemovedBlocksTop = scene->blocks(topRect);

    QRectF bottomRect = boundingRect;
    bottomRect.setTop(boundingRect.bottom() - QFrost::microSizeF);
    bottomRect.setBottom(boundingRect.bottom() - QFrost::microSizeF / 2);
    mRemovedBlocksBottom = scene->blocks(bottomRect);

    QRectF leftRect = boundingRect;
    leftRect.setLeft(boundingRect.left() + QFrost::microSizeF / 2);
    leftRect.setRight(boundingRect.left() + QFrost::microSizeF);
    mRemovedBlocksLeft = scene->blocks(leftRect);

    QRectF rightRect = boundingRect;
    rightRect.setLeft(boundingRect.right() - QFrost::microSizeF);
    rightRect.setRight(boundingRect.right() - QFrost::microSizeF / 2);
    mRemovedBlocksRight = scene->blocks(rightRect);

    QList<QList<Block *> > blocks;
    const int numX = blocksRects.first().size();

    const ColorGenerator *colorGenerator = scene->mainWindow()->colorGenerator();
    for (int i = 0; i < blocksRects.size(); ++i) {
        QList<Block *> row;
        for (int j = 0; j < numX; ++j) {
            Block *block = new Block(blocksRects.at(i).at(j), colorGenerator);
            block->increaseUndoReferenceCount();
            row.append(block);
            if (i == 0) {
                mAddedBlocksTop.append(block);
                foreach(Block * otherBlock, mScene->blocks(block->mTopRect)) {
                    block->mContactsTop.append(BlockContact(block, otherBlock,
                                                            BlockContact::Top));
                }
            }
            if (i == blocksRects.size() - 1) {
                mAddedBlocksBottom.append(block);
                foreach(Block * otherBlock, mScene->blocks(block->mBottomRect)) {
                    block->mContactsBottom.append(BlockContact(block, otherBlock,
                                                  BlockContact::Bottom));
                }
            }

            if (j == 0) {
                mAddedBlocksLeft.append(block);
                foreach(Block * otherBlock, mScene->blocks(block->mLeftRect)) {
                    block->mContactsLeft.append(BlockContact(block, otherBlock,
                                                BlockContact::Left));
                }
            }
            if (j == numX - 1) {
                mAddedBlocksRight.append(block);
                foreach(Block * otherBlock, mScene->blocks(block->mRightRect)) {
                    block->mContactsRight.append(BlockContact(block, otherBlock,
                                                 BlockContact::Right));
                }
            }
        }
        mAddedBlocks.append(row);
        blocks.append(row);
    }

    for (int i = 0; i < blocksRects.size(); ++i) {
        for (int j = 0; j < numX; ++j) {
            Block *b = blocks.at(i).at(j);
            if (i != 0) {
                b->mContactsTop.append(BlockContact(blocks.at(i - 1).at(j),
                                                    b->metersRect().width(),
                                                    qAbs(b->metersCenter().x())));
            }
            if (i != blocksRects.size() - 1) {
                b->mContactsBottom.append(BlockContact(blocks.at(i + 1).at(j),
                                                       b->metersRect().width(),
                                                       qAbs(b->metersCenter().x())));
            }
            if (j != 0) {
                b->mContactsLeft.append(BlockContact(blocks.at(i).at(j - 1),
                                                     b->metersRect().height(),
                                                     qAbs(b->metersRect().left())));
            }
            if (j != numX - 1) {
                b->mContactsRight.append(BlockContact(blocks.at(i).at(j + 1),
                                                      b->metersRect().height(),
                                                      qAbs(b->metersRect().right())));
            }
        }
    }

    QString text = QUndoStack::tr("add %n block(s)", "", mAddedBlocks.size());
    if (!mRemovedBlocks.isEmpty()) {
        text += " ";
        text += QUndoStack::tr("(removing %n block(s))", "", mRemovedBlocks.size());
    }

    QString actionText;
    if (mAddedBlocks.size() == 1) {
        //: Short action name. Accusative case.
        actionText = QUndoStack::tr("add block");
    } else {
        //: Short action name. Accusative case.
        actionText = QUndoStack::tr("add blocks");
    }
    setText(text + "\n" + actionText);

    if (mustChangeOuterPolygons) {
        BoundaryPolygonCalc calc(mScene);
        QPair<BoundaryPolygonList, BoundaryPolygonList> diff;
        diff = calc.uniteOperation(boundingRect, false);
        if (!diff.first.isEmpty() || !diff.second.isEmpty()) {
            new ChangeBoundaryPolygonsCommand(mScene, diff, "", this);
        }
    }
}

void AddBlocksCommand::redo()
{
    QList<Block *>::Iterator i;
    for (i = mAddedBlocksBottom.begin(); i != mAddedBlocksBottom.end(); ++i) {
        (*i)->restoreBottomLinks();
    }
    for (i = mAddedBlocksTop.begin(); i != mAddedBlocksTop.end(); ++i) {
        (*i)->restoreTopLinks();
    }
    for (i = mAddedBlocksLeft.begin(); i != mAddedBlocksLeft.end(); ++i) {
        (*i)->restoreLeftLinks();
    }
    for (i = mAddedBlocksRight.begin(); i != mAddedBlocksRight.end(); ++i) {
        (*i)->restoreRightLinks();
    }

    for (i = mRemovedBlocksBottom.begin(); i != mRemovedBlocksBottom.end(); ++i) {
        (*i)->destroyBottomLinks();
    }
    for (i = mRemovedBlocksTop.begin(); i != mRemovedBlocksTop.end(); ++i) {
        (*i)->destroyTopLinks();
    }
    for (i = mRemovedBlocksLeft.begin(); i != mRemovedBlocksLeft.end(); ++i) {
        (*i)->destroyLeftLinks();
    }
    for (i = mRemovedBlocksRight.begin(); i != mRemovedBlocksRight.end(); ++i) {
        (*i)->destroyRightLinks();
    }

    mScene->replaceBlocks(mRemovedBlocks, mAddedBlocks);
    QUndoCommand::redo();
}

void AddBlocksCommand::undo()
{
    QList<Block *>::Iterator i;
    for (i = mAddedBlocksBottom.begin(); i != mAddedBlocksBottom.end(); ++i) {
        (*i)->destroyBottomLinks();
    }
    for (i = mAddedBlocksTop.begin(); i != mAddedBlocksTop.end(); ++i) {
        (*i)->destroyTopLinks();
    }
    for (i = mAddedBlocksLeft.begin(); i != mAddedBlocksLeft.end(); ++i) {
        (*i)->destroyLeftLinks();
    }
    for (i = mAddedBlocksRight.begin(); i != mAddedBlocksRight.end(); ++i) {
        (*i)->destroyRightLinks();
    }

    for (i = mRemovedBlocksBottom.begin(); i != mRemovedBlocksBottom.end(); ++i) {
        (*i)->restoreBottomLinks();
    }
    for (i = mRemovedBlocksTop.begin(); i != mRemovedBlocksTop.end(); ++i) {
        (*i)->restoreTopLinks();
    }
    for (i = mRemovedBlocksLeft.begin(); i != mRemovedBlocksLeft.end(); ++i) {
        (*i)->restoreLeftLinks();
    }
    for (i = mRemovedBlocksRight.begin(); i != mRemovedBlocksRight.end(); ++i) {
        (*i)->restoreRightLinks();
    }

    mScene->replaceBlocks(mAddedBlocks, mRemovedBlocks);

    QUndoCommand::undo();
}

AddBlocksCommand::~AddBlocksCommand()
{
    foreach(Block * block, mAddedBlocks) {
        block->decreaseUndoReferenceCount();
    }

    foreach(Block * block, mRemovedBlocks) {
        block->decreaseUndoReferenceCount();
    }
}
