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

#include "nextcheckstatecommand.h"

#include <QtWidgets/QCheckBox>

using namespace qfgui;

static Qt::CheckState nextCheckState(QCheckBox *checkBox)
{
    if (checkBox->checkState() == Qt::Checked) {
        return Qt::Unchecked;
    } else if (!checkBox->isTristate() || checkBox->checkState() == Qt::PartiallyChecked) {
        return Qt::Checked;
    } else {
        return Qt::PartiallyChecked;
    }
}

NextCheckStateCommand::NextCheckStateCommand(QCheckBox *checkBox,
        const QString &text,
        QUndoCommand *parent)
    : QUndoCommand(text, parent)
    , mCheckBox(checkBox)
    , mInitialCheckState(checkBox->checkState())
    , mNewCheckState(nextCheckState(checkBox))
{
    Q_ASSERT(!checkBox->isTristate());
}

void NextCheckStateCommand::undo()
{
    mCheckBox->setCheckState(mInitialCheckState);
}

void NextCheckStateCommand::redo()
{
    mCheckBox->setCheckState(mNewCheckState);
}
