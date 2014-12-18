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

#include <undo/setblockstemperaturecommand.h>
#include <graphicsviews/block.h>
#include <units.h>

using namespace qfgui;

SetBlocksTemperatureCommand *SetBlocksTemperatureCommand::createCommand(const QList< Block * > &blocks,
        double temperature,
        QUndoCommand *parent)
{
    SetBlocksTemperatureCommand *c = new SetBlocksTemperatureCommand(blocks,
            temperature,
            parent);
    if (c->mBlocks.isEmpty()) {
        delete c;
        return NULL;
    } else {
        return c;
    }
}

SetBlocksTemperatureCommand *SetBlocksTemperatureCommand::createCommand(const QList< Block * > &blocks,
        double t1,
        double t2,
        double depth1,
        double depth2,
        QUndoCommand *parent)
{
    SetBlocksTemperatureCommand *c = new SetBlocksTemperatureCommand(blocks,
            t1, t2,
            depth1,
            depth2,
            parent);
    if (c->mBlocks.isEmpty()) {
        delete c;
        return NULL;
    } else {
        return c;
    }
}


SetBlocksTemperatureCommand::SetBlocksTemperatureCommand(
    const QList<Block *> &blocks,
    double temperature,
    QUndoCommand *parent)
    : QUndoCommand(parent)
    , mBlocks()
    , mTemperature(temperature)
    , mTemperature2()
    , mDepth1()
    , mDepth2()
    , mIsGradient(false)
{
    Q_ASSERT(!blocks.isEmpty());
    Q_ASSERT(blocks.first()->scene());
    QList<Block *>::ConstIterator i;
    for (i = blocks.constBegin(); i != blocks.constEnd(); ++i) {
        if ((*i)->soilBlock()->temperature() != temperature) {
            mBlocks.append(BlockWithOldTemperature(*i));
        }
    }
    const Unit &unit = Units::unit(blocks.first()->scene(), Temperature);
    setText(QUndoStack::tr("apply temperature %1 to %n block(s)",
                           "", mBlocks.size()).arg(unit.textFromSI(temperature))
            + "\n"
            //: Short action name. Accusative case.
            + QUndoStack::tr("apply temperature"));
}

SetBlocksTemperatureCommand::SetBlocksTemperatureCommand(
    const QList<Block *> &blocks, double t1, double t2,
    double depth1, double depth2, QUndoCommand *parent)
    : QUndoCommand(parent)
    , mBlocks()
    , mTemperature(t1)
    , mTemperature2(t2)
    , mDepth1(depth1)
    , mDepth2(depth2)
    , mIsGradient(true)
{
    Q_ASSERT(t1 != t2);
    Q_ASSERT(mDepth1 < mDepth2);
    QList<Block *>::ConstIterator i;
    for (i = blocks.constBegin(); i != blocks.constEnd(); ++i) {
        if ((*i)->soilBlock()->temperature() != temperatureFromGradient(*i)) {
            mBlocks.append(BlockWithOldTemperature(*i));
        }
    }
    setText(QUndoStack::tr("apply temperature gradient to %n block(s)",
                           "", mBlocks.size()) + "\n"
            //: Short action name. Accusative case.
            + QUndoStack::tr("apply temperature gradient"));
}

double SetBlocksTemperatureCommand::temperatureFromGradient(double depth)
{
    Q_ASSERT(mIsGradient);
    return mTemperature + (depth - mDepth1) * (mTemperature2 - mTemperature) / (mDepth2 - mDepth1);
}

double SetBlocksTemperatureCommand::temperatureFromGradient(const Block *block)
{
    return temperatureFromGradient(block->rect().center().y());
}

double SetBlocksTemperatureCommand::temperatureFromGradient(const BlockWithOldTemperature &block)
{
    return temperatureFromGradient(block.depth());
}


void SetBlocksTemperatureCommand::redo()
{
    QList<BlockWithOldTemperature>::Iterator i;
    for (i = mBlocks.begin(); i != mBlocks.end(); ++i) {
        i->setTemperature(mIsGradient
                          ? temperatureFromGradient(*i)
                          : mTemperature);
    }
}

void SetBlocksTemperatureCommand::undo()
{
    QList<BlockWithOldTemperature>::Iterator i;
    for (i = mBlocks.begin(); i != mBlocks.end(); ++i) {
        i->restore();
    }
}
