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

#include <control_panel/dateeditwithundo.h>

#include <QtWidgets/QUndoStack>

#include <undo/setdatecommand.h>
#include <application.h>
#include <mainwindow.h>

using namespace qfgui;

DateEditWithUndo::DateEditWithUndo(const QDate &date,
                                   const QString &undoText,
                                   QWidget *parent)
    : QDateEdit(date, parent)
    , mUndoStack(Application::findMainWindow(parent)->undoStack())
    , mUndoText(undoText)
    , mIgnoreDateChanges(false)
    , mPreviousDate(date)
    , mLastCommand(NULL)
{
    connect(this, SIGNAL(dateChanged(QDate)), SLOT(createCommand(QDate)));
}

void DateEditWithUndo::createCommand(const QDate &date)
{
    if (!mIgnoreDateChanges) {
        SetDateCommand *newCommand = new SetDateCommand(this, mUndoText,
                mPreviousDate);
        mUndoStack->push(newCommand);
        if (mUndoStack->command(mUndoStack->index() - 1) == newCommand) {
            // новая команда не поглотилась, запомним её
            mLastCommand = newCommand;
        }
    }
    mPreviousDate = date;
}

void DateEditWithUndo::focusOutEvent(QFocusEvent *event)
{
    // если мы ещё помним последнюю созданную нами команду, скажем ей больше не
    // сливаться с другими командами и забудем о ней
    if (mLastCommand != NULL) {
        mLastCommand->stopMerging();
        mLastCommand = NULL;
    }
    QDateEdit::focusOutEvent(event);
}

void DateEditWithUndo::load(QDataStream &in)
{
    Q_ASSERT(in.status() == QDataStream::Ok);
    QDate date;

    in >> date;
    if (in.status() != QDataStream::Ok) {
        qWarning("Load failed: cannot read date");
        throw false;
    }
    mIgnoreDateChanges = true;
    setDate(date);
    mIgnoreDateChanges = false;
}

void DateEditWithUndo::save(QDataStream &out)
{
    out << date();
}
