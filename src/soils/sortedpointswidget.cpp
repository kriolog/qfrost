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

#include "sortedpointswidget.h"

#include <soils/sortedpointsmodel.h>
#include <physicalpropertydelegate.h>

#include <QtWidgets/QTableView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>

using namespace qfgui;

SortedPointsWidget::SortedPointsWidget(const QString &xName,
                                       const QString &yName,
                                       PhysicalProperty xProp,
                                       PhysicalProperty yProp,
                                       QWidget *parent)
    : QWidget(parent)
    , mView(new QTableView(this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addWidget(mView);

    SortedPointsModel *model = new SortedPointsModel(xName, yName, xProp, yProp, this);
    mView->setModel(model);

    mView->setItemDelegate(new PhysicalPropertyDelegate(mView));

    mView->setSelectionMode(QAbstractItemView::NoSelection);

    mView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mView->verticalHeader()->hide();
    mView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(model, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            SLOT(emitValuesChanged()));
    connect(model, SIGNAL(rowsInserted(QModelIndex, int, int)),
            SLOT(emitValuesChanged()));
    connect(model, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            SLOT(emitValuesChanged()));
}

void SortedPointsWidget::setValues(const DoubleMap &data)
{
    static_cast<SortedPointsModel *>(mView->model())->setValues(data);
}

const DoubleMap &SortedPointsWidget::values() const
{
    Q_ASSERT(mView->model() != NULL);
    return static_cast<SortedPointsModel *>(mView->model())->values();
}

void SortedPointsWidget::emitValuesChanged()
{
    emit valuesChanged(values());
}
