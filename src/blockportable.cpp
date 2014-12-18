/*
 * Copyright (C) 2011-2013  Denis Pesotsky
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

#include "blockportable.h"
#include <soils/soil.h>

using namespace qfgui;

BlockPortable::NumAndContact::NumAndContact(const BlockContact &bs)
    : otherBlockNum(bs.block()->num())
    , square(bs.square())
    , r(bs.r())
{

}

QDataStream &operator<<(QDataStream &out, const BlockPortable::NumAndContact &ns)
{
    out << ns.otherBlockNum
        << ns.square
        << ns.r;
    return out;
}

QDataStream &operator>>(QDataStream &in, BlockPortable::NumAndContact &ns)
{
    // на случай, если мы что-то не смогли считать до этого (размер массива)
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    in >> ns.otherBlockNum
       >> ns.square
       >> ns.r;
    // а это на случай, если мы что-то не смогли считать сейчас
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    return in;
}

BlockPortable::BlockPortable(Block *block)
    : mBlock(block)
    , mTemperature(block->soilBlock()->temperature())
    , mThawedPart(block->soilBlock()->thawedPart())
    , mRect(block->rect())
    , mBlocksRight()
    , mBlocksTop()
    , mSoilID(block->mSoil == NULL ? -1 : block->mSoil->id())
    , mTransitionTemperature(block->soilBlock()->transitionTemperature())
{
    foreach(const BlockContact & bs, block->mContactsRight) {
        mBlocksRight << NumAndContact(bs);
    }

    foreach(const BlockContact & bs, block->mContactsTop) {
        mBlocksTop << NumAndContact(bs);
    }
}

BlockPortable::BlockPortable()
    : mBlock(NULL)
    , mTemperature()
    , mThawedPart()
    , mRect()
    , mBlocksRight()
    , mBlocksTop()
    , mSoilID()
    , mTransitionTemperature()
{

}

QDataStream &operator<<(QDataStream &out, const BlockPortable &bp)
{
    Q_ASSERT(bp.mBlock != NULL);
    out << bp.mRect
        << bp.mTemperature
        << bp.mThawedPart
        << bp.mBlocksRight
        << bp.mBlocksTop
        << bp.mSoilID
        << bp.mTransitionTemperature;
    return out;
}

QDataStream &operator>>(QDataStream &in, BlockPortable &bp)
{
    Q_ASSERT(bp.mBlock == NULL);
    // на случай, если мы что-то не смогли считать до этого (размер массива)
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    in >> bp.mRect
       >> bp.mTemperature
       >> bp.mThawedPart
       >> bp.mBlocksRight
       >> bp.mBlocksTop
       >> bp.mSoilID
       >> bp.mTransitionTemperature;
    // а это на случай, если мы что-то не смогли считать сейчас
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    return in;
}

Block *BlockPortable::createBlock(const QList<const Soil *> &soils,
                                  const ColorGenerator *colorGenerator)
{
    Q_ASSERT(mBlock == NULL);
    mBlock = new Block(mRect, colorGenerator);

    mBlock->mSoilBlock.mTransitionTemperature = mTransitionTemperature;
    mBlock->mSoilBlock.mTemperature = mTemperature;
    mBlock->mSoilBlock.mThawedPart = mThawedPart;
    if (mSoilID != -1) {
        Q_ASSERT(soils.size() > mSoilID);
        Soil *const soil = const_cast<Soil *>(soils.at(mSoilID));
        mBlock->mSoil = soil;
        mBlock->mSoilBlock.setAllProperties(soil->block(), false);
    }

    mBlock->updateFromTemperature();
    mBlock->updateFromThawedPart();
    return mBlock;
}

void BlockPortable::fillBlockNeighbors(const QList<Block *> &blocks)
{
    Q_ASSERT(mBlock != NULL);
    foreach(const NumAndContact & ns, mBlocksRight) {
        mBlock->mContactsRight << BlockContact(blocks.at(ns.otherBlockNum),
                                               ns.square,
                                               ns.r);
    }
    mBlock->restoreRightLinks();

    foreach(const NumAndContact & ns, mBlocksTop) {
        mBlock->mContactsTop << BlockContact(blocks.at(ns.otherBlockNum),
                                             ns.square,
                                             ns.r);
    }
    mBlock->restoreTopLinks();
}
