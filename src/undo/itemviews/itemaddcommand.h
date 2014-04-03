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

#ifndef QFGUI_ITEMADDCOMMAND_H
#define QFGUI_ITEMADDCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <QtCore/QSharedPointer>


namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Item)
QT_FORWARD_DECLARE_CLASS(ItemsModel)

class ItemAddCommand : public QUndoCommand
{

public:
    /// Конструктор команды добавления элемента @p item в @p model.
    /// При дублировании грунта следует указывать @p originalItemName --
    /// имя элемента, дубликатом которого является @p item.
    ItemAddCommand(Item *item,
                   ItemsModel *model,
                   QString originalItemName = QString());

    virtual void redo();
    virtual void undo();

    virtual ~ItemAddCommand();

private:
    Item *const mItem;
    ItemsModel *const mModel;
};

}

#endif // QFGUI_ITEMADDCOMMAND_H
