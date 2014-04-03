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

#include "itemsremovecommand.h"

#include <QtCore/QLocale>

#include "soilsremovecommand.h"
#include "boundaryconditionsremovecommand.h"
#include <itemviews/item.h>
#include <itemviews/itemsmodel.h>

using namespace qfgui;

ItemsRemoveCommand *ItemsRemoveCommand::createCommand(const QList<Item *> &items)
{
    Q_ASSERT(!items.isEmpty());
    if (strcmp(items.first()->metaObject()->className(), "qfgui::Soil") == 0) {
        return new SoilsRemoveCommand(items);
    } else {
        return new BoundaryConditionsRemoveCommand(items);
    }
}

static QStringList quotedNamesList(const QList< Item * > &items)
{
    QStringList result;
    foreach(Item * item, items) {
        result << QLocale().quoteString(item->name());
    }
    result.sort();
    return result;
}

ItemsRemoveCommand::ItemsRemoveCommand(const QList< Item * > &items)
    : QUndoCommand()
    , mItems(items)
    , mModel(qobject_cast<ItemsModel *>(mItems.first()->parent()))
    , mNames(quotedNamesList(items))
{
    Q_ASSERT(mModel != NULL);

    foreach(Item * item, mItems) {
        item->increaseUndoReferenceCount();
    }
}

void ItemsRemoveCommand::undo()
{
    foreach(Item * item, mItems) {
        item->setParent(mModel);
    }
}

void ItemsRemoveCommand::redo()
{
    foreach(Item * item, mItems) {
        item->setParent(NULL);
    }
}

ItemsRemoveCommand::~ItemsRemoveCommand()
{
    foreach(Item * item, mItems) {
        item->decreaseUndoReferenceCount();
    }
}
