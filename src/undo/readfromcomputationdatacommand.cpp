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

#include <undo/readfromcomputationdatacommand.h>

#include <QtWidgets/QDateEdit>

#include <mainwindow.h>
#include <control_panel/controlpanel.h>
#include <control_panel/computationcontrol.h>

using namespace qfgui;

ReadFromComputationDataCommand::ReadFromComputationDataCommand(Scene *scene,
        const ComputationData &data,
        bool isFinal,
        QUndoCommand *parent)
    : SetDateCommand(scene->mainWindow()->controlPanel()->computationControl()->mInitialDateEdit,
                     data.date(), parent)
    , mOldData(scene)
    , mNewData(data)
    , mBlocksAndNums()
    , mIsFinal(isFinal)
{
    Q_ASSERT(mOldData.numOfBlocks() == mNewData.numOfBlocks());

    QList<Block *>::Iterator it;
    for (it = scene->mBlocksInDomain.begin(); it != scene->mBlocksInDomain.end(); ++it) {
        Q_ASSERT((*it)->mIsInDomain);
        mBlocksAndNums.append(BlockAndNum(*it, (*it)->mSoilBlockInDomainNum));
    }

    updateText();
}

void ReadFromComputationDataCommand::updateText()
{
    //: %1 is computations start date, %2 is end date
    setText(QUndoStack::tr("computation (%1 \342\200\223 %2)")
            .arg(oldDate().toString(QFrost::dateFormat()))
            .arg(newDate().toString(QFrost::dateFormat())) + "\n"
            //: Short action name. Accusative case.
            + QUndoStack::tr("computation"));
}

void ReadFromComputationDataCommand::undo()
{
    foreach(BlockAndNum b, mBlocksAndNums) {
        b.first->readFromBlockData(mOldData.soilBlockDataAt(b.second));
    }
    SetDateCommand::undo();
}

void ReadFromComputationDataCommand::redo()
{
    foreach(BlockAndNum b, mBlocksAndNums) {
        b.first->readFromBlockData(mNewData.soilBlockDataAt(b.second));
    }
    SetDateCommand::redo();
}

bool ReadFromComputationDataCommand::mergeWith(const QUndoCommand *other)
{
    if (mIsFinal || other->id() != id()) {
        return false;
    }

    const ReadFromComputationDataCommand *castedOther;
    castedOther = static_cast<const ReadFromComputationDataCommand *>(other);

    if (castedOther->mIsFinal) {
        mIsFinal = true;
    }

    Q_ASSERT(castedOther->oldDate() == newDate());

    mNewData = castedOther->mNewData;
    bool mergeSuccessful = SetDateCommand::mergeWith(other);
    Q_ASSERT(mergeSuccessful);
    updateText();
    return true;
}
