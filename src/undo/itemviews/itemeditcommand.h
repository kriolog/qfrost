/*
 * Copyright (C) 2012-2015  Denis Pesotsky
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

#ifndef QFGUI_ITEMEDITCOMMAND_H
#define QFGUI_ITEMEDITCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QVariant>

#include <qfrost.h>


namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Item)

class ItemEditCommand : public QUndoCommand
{
public:
    static ItemEditCommand *createCommand(Item *item,
                                          const ItemChanges &changes);

    virtual void redo();
    virtual void undo();

protected:
    ItemEditCommand(Item *item, const ItemChanges &changes);

    const Item *item() const {
        return mItem;
    }

    /// Подготовление к изменениям при первом redo ДО формирования текста.
    /// Унаследованные классы могут модифицировать здесь @p changes.
    /// По умолчанию ничего не делает.
    virtual void prepareChanges(ItemChanges &changes) const {
        Q_UNUSED(changes)
    }

private:
    void updateText();

    Item *const mItem;
    ItemChanges mChanges;
    bool mHadFirstRedo;
};

}

#endif // QFGUI_ITEMEDITCOMMAND_H
