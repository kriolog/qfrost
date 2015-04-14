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

MonthsTableView::MonthsTableView(Qt::Orientation orientation, QWidget *parent)
    : QTableView(parent)
    , mOrientation(orientation)
{
    const double isVertical = (orientation == Qt::Vertical);
    dataTypesHeader()->setSectionResizeMode(isVertical
                                            ? QHeaderView::ResizeToContents
                                            : QHeaderView::Fixed);
    monthsHeader()->setSectionResizeMode(isVertical
                                         ? QHeaderView::Fixed
                                         : QHeaderView::Stretch);

    dataTypesHeader()->setHighlightSections(false);

    if (isVertical) {
        setAlternatingRowColors(true);
        setItemDelegateForColumn(1, new PhysicalPropertyDelegate(this, true));
    } else {
        QFont monthNamesFont = monthsHeader()->font();
        monthNamesFont.setBold(true);
        QFontMetrics metrics(monthNamesFont);
        const QLocale locale = this->locale();
        int maxMonthWidth = 0;
        for (int month = 1; month <= 12; ++month) {
            const QString text = locale.standaloneMonthName(month,
                                                            QLocale::ShortFormat);
            const int w = metrics.width(" " + text + " ");
            if (w > maxMonthWidth) {
                maxMonthWidth = w;
            }
        }
        monthsHeader()->setMinimumSectionSize(maxMonthWidth);
        setItemDelegateForRow(1, new PhysicalPropertyDelegate(this, true));
    }
}

QHeaderView *MonthsTableView::monthsHeader() const
{
    return (mOrientation == Qt::Vertical)
           ? verticalHeader()
           : horizontalHeader();
}

QHeaderView *MonthsTableView::dataTypesHeader() const
{
    return (mOrientation == Qt::Horizontal)
           ? verticalHeader()
           : horizontalHeader();
}

void MonthsTableView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    updateSizeLimits();
}

void MonthsTableView::updateSizeLimits()
{
    // Делаем так, чтобы минималный размер соответствовал содержимому
    static const int minViewWidth = 180;

    const bool needStretchLastSection = (mOrientation == Qt::Vertical);
    if (needStretchLastSection) {
        dataTypesHeader()->setStretchLastSection(false);
    }
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents
                                | QEventLoop::ExcludeSocketNotifiers);

    verticalHeader()->updateGeometry();
    const int heightHint = verticalHeader()->length()
                           + horizontalHeader()->sizeHint().height()
                           + 2 * frameWidth();
    const int widthHint = horizontalHeader()->length()
                          + verticalHeader()->sizeHint().width()
                          + 2 * frameWidth();

    setMinimumHeight(heightHint);

    if (mOrientation == Qt::Vertical) {
        setMinimumWidth(qMax(minViewWidth, widthHint));
    } else {
        setMinimumWidth(widthHint);
        setMaximumHeight(heightHint);
    }

    if (needStretchLastSection) {
        dataTypesHeader()->setStretchLastSection(true);
    }
}

void MonthsTableView::closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QAbstractItemView::closeEditor(editor, QAbstractItemDelegate::NoHint);
    if (hint == QAbstractItemDelegate::EditNextItem
            || hint == QAbstractItemDelegate::EditPreviousItem) {
        const bool isVertical = (mOrientation == Qt::Vertical);
        const int newMonthNum = (isVertical ? currentIndex().row() : currentIndex().column())
                                + ((hint == QAbstractItemDelegate::EditNextItem) ? 1 : -1);
        if (newMonthNum == 12 || newMonthNum == -1) {
            return;
        }
        QModelIndex newIndex = isVertical ? model()->index(newMonthNum, 1)
                                          : model()->index(1, newMonthNum);
        setCurrentIndex(newIndex);
        edit(newIndex);
    }
}
