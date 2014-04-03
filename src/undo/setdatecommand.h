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

#ifndef QFGUI_SETDATECOMMAND_H
#define QFGUI_SETDATECOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <QtCore/QDate>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(DateEditWithUndo)

class SetDateCommand : public QUndoCommand
{
public:
    /**
     * Меняет время у @p dateEdit с @p oldDate на то время, которое у него
     * сейчас возвращает date(). Первое redo игнорируется.
     */
    SetDateCommand(DateEditWithUndo *dateEdit,
                   const QString &undoText,
                   const QDate &oldDate,
                   QUndoCommand *parent = NULL);
    virtual void undo();
    virtual void redo();

    virtual int id() const {
        return 4;
    }

    virtual bool mergeWith(const QUndoCommand *other);

    /// Перестать сливаться с другими командами.
    void stopMerging() {
        mMustMerge = false;
    }

private:
    DateEditWithUndo *mDateEdit;
    QDate mNewDate;
    QDate mOldDate;
    bool mIgnoreRedo;
    bool mMustMerge;

protected:
    /**
     * Меняет время у @p dateEdit на @p newDate.
     */
    SetDateCommand(DateEditWithUndo *dateEdit,
                   const QDate &newDate,
                   QUndoCommand *parent = NULL);

    const QDate &oldDate() const {
        return mOldDate;
    }
    const QDate &newDate() const {
        return mNewDate;
    }
};

}

#endif // QFGUI_SETDATECOMMAND_H
