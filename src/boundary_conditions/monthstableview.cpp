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

#include "monthstableview.h"

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QApplication>

#include <units/physicalpropertydelegate.h>

using namespace qfgui;

MonthsTableView::MonthsTableView(QWidget *parent)
    : QTableView(parent)
{
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    horizontalHeader()->setHighlightSections(false);

    setAlternatingRowColors(true);

    setItemDelegateForColumn(1, new PhysicalPropertyDelegate(this, true));
}

void MonthsTableView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    // Делаем так, чтобы минималный размер соответствовал содержимому
    static const int minViewWidth = 180;

    horizontalHeader()->setStretchLastSection(false);
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents
                                | QEventLoop::ExcludeSocketNotifiers);

    setMinimumHeight(verticalHeader()->length()
                     + horizontalHeader()->height()
                     + 2 * frameWidth());

    setMinimumWidth(qMax(minViewWidth,
                         horizontalHeader()->length()
                         + verticalHeader()->width()
                         + 2 * frameWidth()));

    horizontalHeader()->setStretchLastSection(true);
}

void MonthsTableView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QAbstractItemView::closeEditor(editor, QAbstractItemDelegate::NoHint);
    if (hint == QAbstractItemDelegate::EditNextItem
            || hint == QAbstractItemDelegate::EditPreviousItem) {
        const int newRow = currentIndex().row()
                           + ((hint == QAbstractItemDelegate::EditNextItem) ? 1 : -1);
        if (newRow == model()->rowCount() || newRow == -1) {
            return;
        }
        QModelIndex newIndex = model()->index(newRow, 1);
        setCurrentIndex(newIndex);
        edit(newIndex);
    }
}
