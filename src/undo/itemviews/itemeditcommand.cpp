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

#include "itemeditcommand.h"

#include "soileditcommand.h"
#include <itemviews/item.h>

#include <QtCore/QLocale>

using namespace qfgui;

ItemEditCommand *ItemEditCommand::createCommand(Item *item,
        const ItemChanges &changes)
{
    return (strcmp(item->metaObject()->className(), "qfgui::Soil") == 0)
           ? new SoilEditCommand(item, changes)
           : new ItemEditCommand(item, changes);
}

ItemEditCommand::ItemEditCommand(Item *item, const ItemChanges &changes)
    : QUndoCommand()
    , mItem(item)
    , mChanges(changes)
    , mHadFirstRedo(false)
{
}

void ItemEditCommand::updateText()
{
    QStringList manuallyChangedValueNames;
    QStringList autoChangedValueNames;
    for (ItemChanges::ConstIterator it = mChanges.constBegin(); it != mChanges.constEnd(); ++it) {
        QString propertyName = mItem->shortPropertyNameGenetive(it.key());
        if (!it->second.isValid()) {
            autoChangedValueNames << QString("[%1]").arg(propertyName);
        } else {
            manuallyChangedValueNames << propertyName;
        }
    }

    manuallyChangedValueNames.sort();
    autoChangedValueNames.sort();

    const QString oldName = mChanges.contains("name")
                            ? mChanges.constFind("name")->first.toString()
                            : mItem->name();
    /*: %1 is changed properties in genetive case. %2 is quoted soil name.
     Example: change name, w_tot, [Qph], [Tbf] of "Sand" */
    setText(QUndoStack::tr("change %1 of %2", "edit item command")
            .arg((manuallyChangedValueNames + autoChangedValueNames).join(", "))
            .arg(QLocale().quoteString(oldName)) + "\n"
            + ((strcmp(mItem->metaObject()->className(), "qfgui::Soil") == 0)
               //: Short action name. Accusative case
               ? QUndoStack::tr("edit soil")
               //: Short action name. Accusative case.
               : QUndoStack::tr("edit b. cond.")));
}

void ItemEditCommand::redo()
{
    if (mHadFirstRedo) {
        mItem->applyChanges(mChanges, true);
    } else {
        mHadFirstRedo = true;
        // Делаем это только сейчас, а не в конструкторе, т.к. метод виртуальный
        prepareChanges(mChanges);
        // А это сейчас, т.к. mChanges стало актуальным только что
        updateText();
    }
    QUndoCommand::redo();
}

void ItemEditCommand::undo()
{
    QUndoCommand::undo();
    mItem->applyChanges(mChanges, false);
}
