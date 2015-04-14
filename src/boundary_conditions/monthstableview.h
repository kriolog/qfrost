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

#ifndef QFGUI_MONTHSTABLEVIEW_H
#define QFGUI_MONTHSTABLEVIEW_H

#include <QtWidgets/QTableView>


namespace qfgui
{

class MonthsTableView : public QTableView
{
    Q_OBJECT
public:
    MonthsTableView(Qt::Orientation orientation, QWidget *parent);
    void updateSizeLimits(bool withMaxHeight);

protected:
    void closeEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

private:
    const Qt::Orientation mOrientation;

    QHeaderView *monthsHeader() const;
    QHeaderView *dataTypesHeader() const;
};

}

#endif // QFGUI_MONTHSTABLEVIEW_H
