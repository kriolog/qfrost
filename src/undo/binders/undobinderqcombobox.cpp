/*
 * Copyright (C) 2013  Denis Pesotsky
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

#include "undobinderqcombobox.h"
#include <qfrost.h>

#include <QtWidgets/QComboBox>

using namespace qfgui;

UndoBinderQComboBox::UndoBinderQComboBox(QUndoStack *undoStack, QComboBox *parent)
    : QObject(parent)
    , mLastIndex(parent->currentIndex())
    , mUndoStack(undoStack)
{
    parent->setProperty(QFrost::UndoBinderIsEnabled, true);
    connect(parent, SIGNAL(currentIndexChanged(int)), SLOT(onCurrentIndexChanged(int)));
}

void UndoBinderQComboBox::onCurrentIndexChanged(int index)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(parent());
    Q_ASSERT(comboBox != NULL);
    Q_ASSERT(sender() == parent());
    if (comboBox->property(QFrost::UndoBinderIsEnabled).toBool()) {
        mUndoStack->push(new SetQComboBoxIndexCommand(static_cast<QComboBox *>(parent()),
                         mLastIndex, index));
        mLastIndex = index;
    }
}

UndoBinderQComboBox::SetQComboBoxIndexCommand::SetQComboBoxIndexCommand(QComboBox *comboBox,
        int oldIndex,
        int newIndex,
        QUndoCommand *parent)
    : QUndoCommand(parent)
    , mComboBox(comboBox)
    , mOldIndex(oldIndex)
    , mNewIndex(newIndex)
    , mHadFirstRedo(false)
{
    const QVariant undoTextData = comboBox->itemData(newIndex, QFrost::UndoTextRole);
    if (undoTextData.type() != QVariant::String) {
        qWarning("No data for undo text role!");
        setText("$change combo box value$");
    } else {
        setText(undoTextData.toString());
    }
}

void UndoBinderQComboBox::SetQComboBoxIndexCommand::redo()
{
    Q_ASSERT(mComboBox->property(QFrost::UndoBinderIsEnabled).toBool());
    if (mHadFirstRedo) {
        mComboBox->setProperty(QFrost::UndoBinderIsEnabled, false);
        mComboBox->setCurrentIndex(mNewIndex);
        mComboBox->setProperty(QFrost::UndoBinderIsEnabled, true);
    } else {
        mHadFirstRedo = true;
    }
    QUndoCommand::redo();
}

void UndoBinderQComboBox::SetQComboBoxIndexCommand::undo()
{
    QUndoCommand::undo();
    Q_ASSERT(mHadFirstRedo);
    Q_ASSERT(mComboBox->property(QFrost::UndoBinderIsEnabled).toBool());
    mComboBox->setProperty(QFrost::UndoBinderIsEnabled, false);
    mComboBox->setCurrentIndex(mOldIndex);
    mComboBox->setProperty(QFrost::UndoBinderIsEnabled, true);
}
