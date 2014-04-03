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

#ifndef QFGUI_CHECKBOXDELEGATE_H
#define QFGUI_CHECKBOXDELEGATE_H

#include <QtWidgets/QStyledItemDelegate>

namespace qfgui
{

class CheckBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    CheckBoxDelegate(QAbstractItemView *parent);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index);
    bool mCheckboxMouseOver;
    bool mCheckboxIsPressedDown;
    bool mCheckboxHasFocus;
};

}

#endif // QFGUI_CHECKBOXDELEGATE_H
