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

#include "sortedpointswidget.h"

#include <soils/sortedpointsmodel.h>
#include <units/physicalpropertydelegate.h>

#include <QtWidgets/QTableView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtCore/QPersistentModelIndex>

using namespace qfgui;

SortedPointsWidget::SortedPointsWidget(const QString &xName,
                                       const QString &yName,
                                       PhysicalProperty xProp,
                                       PhysicalProperty yProp,
                                       QWidget *parent)
    : QWidget(parent)
    , mModel(new SortedPointsModel(xName, yName, xProp, yProp, this))
    , mView(new QTableView(this))
    , mNewPoint(new QPushButton(QIcon::fromTheme("list-add"), "", this))
    , mRemovePoint(new QPushButton(QIcon::fromTheme("list-remove"), "", this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addWidget(mView);

    mView->setModel(mModel);

    mView->setItemDelegate(new PhysicalPropertyDelegate(mView));

    mView->setSelectionBehavior(QAbstractItemView::SelectRows);

    mView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mView->verticalHeader()->hide();
    mView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(mModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            SLOT(emitValuesChanged()));
    connect(mModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
            SLOT(emitValuesChanged()));
    connect(mModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            SLOT(emitValuesChanged()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(QMargins());
    mainLayout->addLayout(buttonsLayout);
    buttonsLayout->addWidget(mNewPoint);
    buttonsLayout->addWidget(mRemovePoint);

    mNewPoint->setToolTip(tr("Add new point"));
    mRemovePoint->setToolTip(tr("Remove selected points"));

    connect(mView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(updateButtons()));
    updateButtons();

    connect(mRemovePoint, SIGNAL(clicked()), SLOT(removeSelectedPoints()));
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

void SortedPointsWidget::updateButtons()
{
    const bool anythingIsSelected = mView->selectionModel()->hasSelection();
    mRemovePoint->setEnabled(anythingIsSelected);
}

void SortedPointsWidget::removeSelectedPoints()
{
    foreach (const QPersistentModelIndex &index, selectedRows()) {
        mModel->removeRow(index.row());
    }
}

QList<QPersistentModelIndex> SortedPointsWidget::selectedRows() const
{
    Q_ASSERT(mView->selectionModel()->model() == mModel);
    QList<QPersistentModelIndex> result;
    // HACK: вместо selectedRows приходится использовать такой велосипед, ибо
    //       Qt может развыделять ячейки без флага ItemIsEnabled при
    //       пересортировке QItemSelectionModel, после чего selectedRows
    //       возвращает пустой список.
    foreach(const QModelIndex & index, mView->selectionModel()->selectedIndexes()) {
        if (index.column() == 0) {
            result << QPersistentModelIndex(index);
        }
    }
    return result;
}
