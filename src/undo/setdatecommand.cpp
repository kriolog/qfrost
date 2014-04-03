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

#include <undo/setdatecommand.h>

#include <control_panel/dateeditwithundo.h>

using namespace qfgui;

SetDateCommand::SetDateCommand(DateEditWithUndo *dateEdit,
                               const QString &undoText,
                               const QDate &oldDate,
                               QUndoCommand *parent)
    : QUndoCommand(undoText, parent)
    , mDateEdit(dateEdit)
    , mNewDate(dateEdit->date())
    , mOldDate(oldDate)
    , mIgnoreRedo(true)
    , mMustMerge(true)
{
}

SetDateCommand::SetDateCommand(DateEditWithUndo *dateEdit,
                               const QDate &newDate,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDateEdit(dateEdit)
    , mNewDate(newDate)
    , mOldDate(dateEdit->date())
    , mIgnoreRedo(false)
    , mMustMerge(true)
{
}

void SetDateCommand::undo()
{
    mDateEdit->mIgnoreDateChanges = true;
    mDateEdit->setDate(mOldDate);
    mDateEdit->mIgnoreDateChanges = false;
    QUndoCommand::undo();
}

void SetDateCommand::redo()
{
    if (mIgnoreRedo) {
        mIgnoreRedo = false;
        return;
    }

    mDateEdit->mIgnoreDateChanges = true;
    mDateEdit->setDate(mNewDate);
    mDateEdit->mIgnoreDateChanges = false;
    QUndoCommand::redo();
}

bool SetDateCommand::mergeWith(const QUndoCommand *other)
{
    if (!mMustMerge) {
        return false;
    }
    if (other->id() != id()) {
        return false;
    }

    const SetDateCommand *command = static_cast<const SetDateCommand *>(other);
    if (command->mDateEdit != mDateEdit) {
        return false;
    }
    Q_ASSERT(command->mOldDate == mNewDate);
    mNewDate = command->mNewDate;
    return true;
}
