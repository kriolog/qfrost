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

#ifndef QFGUI_ITEMSREMOVECOMMAND_H
#define QFGUI_ITEMSREMOVECOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Item)
QT_FORWARD_DECLARE_CLASS(ItemsModel)

class ItemsRemoveCommand : public QUndoCommand
{

public:
    static ItemsRemoveCommand *createCommand(const QList<Item *> &items);

    virtual void redo();
    virtual void undo();

    virtual ~ItemsRemoveCommand();

protected:
    ItemsRemoveCommand(const QList<Item *> &items);

    /// Сортированный список имён изменённых элементов, каждое имя -- в кавычках
    const QStringList &names() const {
        return mNames;
    }

private:
    const QList<Item *> mItems;
    ItemsModel *const mModel;
    const QStringList mNames;
};

}

#endif // QFGUI_ITEMSREMOVECOMMAND_H
