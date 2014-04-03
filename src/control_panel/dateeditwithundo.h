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

#ifndef QFGUI_DATEEDITWITHUNDO_H
#define QFGUI_DATEEDITWITHUNDO_H

#include <QtWidgets/QDateEdit>

QT_FORWARD_DECLARE_CLASS(QUndoStack)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(SetDateCommand)

class DateEditWithUndo : public QDateEdit
{
    Q_OBJECT
public:
    DateEditWithUndo(const QDate &date,
                     const QString &undoText,
                     QWidget *parent = NULL);

    void save(QDataStream &out);
    void load(QDataStream &in);

private slots:
    /**
     * Создаёт undo-комманду, если не выставлено mIgnoreDateChanges.
     * Та команда начнёт менять нашу дату только _после_ первого redo.
     */
    void createCommand(const QDate &date);

protected:
    void focusOutEvent(QFocusEvent *event);

private:
    QUndoStack *const mUndoStack;
    const QString mUndoText;
    bool mIgnoreDateChanges;

    /**
     * Дата, которая была у нас до очередного попадания в createCommand.
     * Нужна затем, что у сигнала dateChanged аргумент -- новая дата, и в слоте,
     * подсоединённому к этому сигналу, date() возвращает ту же (новую) дату.
     */
    QDate mPreviousDate;

    /// Последняя созданная команда изменения нашей даты.
    SetDateCommand *mLastCommand;

    friend class SetDateCommand;
};

}

#endif // QFGUI_DATEEDITWITHUNDO_H
