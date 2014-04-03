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

#ifndef QFGUI_UNDOBINDERQCOMBOBOX_H
#define QFGUI_UNDOBINDERQCOMBOBOX_H

#include <QtCore/QObject>
#include <qabstractitemmodel.h>
#include <QtWidgets/QUndoCommand>

QT_FORWARD_DECLARE_CLASS(QComboBox)

namespace qfgui
{

class UndoBinderQComboBox : public QObject
{
    Q_OBJECT
public:
    UndoBinderQComboBox(QUndoStack *undoStack, QComboBox *parent);

private slots:
    void onCurrentIndexChanged(int index);

private:
    int mLastIndex;
    QUndoStack *const mUndoStack;

    class SetQComboBoxIndexCommand : public QUndoCommand
    {
    public:
        SetQComboBoxIndexCommand(QComboBox *comboBox,
                                 int oldIndex, int newIndex,
                                 QUndoCommand *parent = 0);
        void undo();
        void redo();

    private:
        QComboBox *const mComboBox;
        const int mOldIndex;
        const int mNewIndex;
        bool mHadFirstRedo;
    };
};

}

#endif // QFGUI_UNDOBINDERQCOMBOBOX_H
