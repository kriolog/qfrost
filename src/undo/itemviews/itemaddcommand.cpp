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

#include "itemaddcommand.h"

#include <QtCore/QLocale>

#include <itemviews/item.h>
#include <itemviews/itemsmodel.h>

using namespace qfgui;

ItemAddCommand::ItemAddCommand(Item *item,
                               ItemsModel *model,
                               QString originalItemName)
    : QUndoCommand()
    , mItem(item)
    , mModel(model)
{
    Q_ASSERT(mModel != NULL);
    mItem->increaseUndoReferenceCount();
    if (strcmp(item->metaObject()->className(), "qfgui::Soil") == 0) {
        setText(originalItemName.isNull()
                ? (QUndoStack::tr("new soil %1")
                   .arg(QLocale().quoteString(item->name())) + "\n"
                   //: Short action name. Accusative case.
                   + QUndoStack::tr("create soil"))

                : (QUndoStack::tr("duplicate soil %1")
                   .arg(QLocale().quoteString(originalItemName)) + "\n" +
                   //: Short action name. Accusative case.
                   QUndoStack::tr("duplicate soil")));
    } else {
        Q_ASSERT(strcmp(item->metaObject()->className(),
                        "qfgui::BoundaryCondition") == 0);
        setText(originalItemName.isNull()
                ? (QUndoStack::tr("new boundary condition %1")
                   .arg(QLocale().quoteString(item->name())) + "\n"
                   //: Short action name. Accusative case.
                   + QUndoStack::tr("create b. cond."))

                : (QUndoStack::tr("duplicate boundary condition %1")
                   .arg(QLocale().quoteString(originalItemName)) + "\n" +
                   //: Short action name. Accusative case.
                   QUndoStack::tr("duplicate b. cond.")));
    }
}

void ItemAddCommand::redo()
{
    mItem->setParent(mModel);
}

void ItemAddCommand::undo()
{
    mItem->setParent(NULL);
}

ItemAddCommand::~ItemAddCommand()
{
    mItem->decreaseUndoReferenceCount();
}
