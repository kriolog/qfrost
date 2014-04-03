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

#include "checkboxwithundo.h"

#include <undo/nextcheckstatecommand.h>
#include <mainwindow.h>
#include <application.h>

using namespace qfgui;

CheckBoxWithUndo::CheckBoxWithUndo(const QString &text,
                                   const QString &undoTextOnChecking,
                                   const QString &undoTextOnUnchecking,
                                   QWidget *parent)
    : QCheckBox(text, parent)
    , mUndoTextOnChecking(undoTextOnChecking)
    , mUndoTextOnUnchecking(undoTextOnUnchecking)
    , mMainWindow(Application::findMainWindow(parent))
{

}

void CheckBoxWithUndo::nextCheckState()
{
    Q_ASSERT(!isTristate());
    mMainWindow->undoStack()
    ->push(new NextCheckStateCommand(this,
                                     isChecked()
                                     ? mUndoTextOnUnchecking
                                     : mUndoTextOnChecking));
}
