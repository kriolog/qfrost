/*
 * Copyright (C) 2011-2015  Denis Pesotsky
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

#ifndef QFGUI_MONTHSTABLEWIDGET_H
#define QFGUI_MONTHSTABLEWIDGET_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QAbstractItemDelegate>

#include <qfrost.h>

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(MonthsTableModel)
QT_FORWARD_DECLARE_CLASS(MonthsTableView)
QT_FORWARD_DECLARE_CLASS(MonthsTableExpander)

class MonthsTableWidget : public QWidget
{
    Q_OBJECT

public:
    MonthsTableWidget(Qt::Orientation orientation,
                      QWidget *parent);

    MonthsTableExpander *addExpander(const QString &valueName);

    void updateSizeLimits(bool withMaxHeight = false);

private:
    MonthsTableModel *qfModel();
    const MonthsTableModel *qfModel() const;

    MonthsTableView *mView;

    Qt::Orientation mOrientation;
};

}

#endif // QFGUI_MONTHSTABLEWIDGET_H
