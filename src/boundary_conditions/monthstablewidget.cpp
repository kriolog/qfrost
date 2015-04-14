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

#include "monthstablewidget.h"

#include <cmath>

#include <QtWidgets/QVBoxLayout>

#include <boundary_conditions/monthstablemodel.h>
#include <boundary_conditions/monthstableview.h>
#include <boundary_conditions/monthstableexpander.h>
#include <mainwindow.h>

#include "monthstablesetter.h"

using namespace qfgui;

MonthsTableWidget::MonthsTableWidget(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent)
    , mView(new MonthsTableView(orientation, this))
    , mOrientation(orientation)
{
    MonthsTableModel *model = new MonthsTableModel(orientation, this);
    mView->setModel(model);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addWidget(mView);
    mainLayout->addWidget(new MonthsTableSetter(mView->selectionModel(), this));
    mainLayout->addStretch();
}

MonthsTableExpander *MonthsTableWidget::addExpander(const QString &valueName)
{
    return new MonthsTableExpander(qfModel(), valueName, this);
}

MonthsTableModel *MonthsTableWidget::qfModel()
{
    return static_cast<MonthsTableModel *>(mView->model());
}

const MonthsTableModel *MonthsTableWidget::qfModel() const
{
    return static_cast<const MonthsTableModel *>(mView->model());
}
