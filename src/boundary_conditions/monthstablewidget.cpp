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

#include <QtWidgets/QDialog>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFormLayout>

#include <boundary_conditions/monthstablemodel.h>
#include <boundary_conditions/monthstableview.h>
#include <units/physicalpropertyspinbox.h>
#include <mainwindow.h>

using namespace qfgui;

MonthsTableWidget::MonthsTableWidget(const QString &valueName,
                                     Qt::Orientation orientation,
                                     QWidget *parent)
    : QWidget(parent)
    , mView(new MonthsTableView(orientation, this))
    , mDataSetterSpinbox(new PhysicalPropertySpinBox(this))
    , mOpenDataSetter(new QPushButton(this))
    , mOrientation(orientation)
{
    MonthsTableModel *model = new MonthsTableModel(valueName, orientation, this);
    mView->setModel(model);

    QDialogButtonBox *dialogButtons;
    dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    QDialog *dataSetterDialog = new QDialog(this);

    QFormLayout *dialogFormLayout = new QFormLayout;
    dialogFormLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    dialogFormLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    // FIXME: надпись можно бы и получше придумать
    dialogFormLayout->addRow(tr("Batch set %1:")
                             .arg(valueName),
                             mDataSetterSpinbox);

    //: Batch set dialog title
    dataSetterDialog->setWindowTitle(tr("Batch Set"));

    connect(dialogButtons, SIGNAL(accepted()),
            dataSetterDialog, SLOT(accept()));
    connect(dialogButtons, SIGNAL(rejected()),
            dataSetterDialog, SLOT(reject()));

    connect(dataSetterDialog, SIGNAL(accepted()), SLOT(batchSetData()));

    QVBoxLayout *dialogLayout = new QVBoxLayout(dataSetterDialog);

    dialogLayout->addStretch();
    dialogLayout->addLayout(dialogFormLayout);
    dialogLayout->addStretch();
    dialogLayout->addWidget(dialogButtons);

    connect(mOpenDataSetter, SIGNAL(clicked(bool)), SLOT(setDataSetterValue()));

    connect(mOpenDataSetter, SIGNAL(clicked(bool)),
            dataSetterDialog, SLOT(exec()));

    // Пусть примет минимальный размер по вертикали и красивый по горизонтали
    dataSetterDialog->resize(QSize(300, 0));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addWidget(mView);
    mainLayout->addWidget(mOpenDataSetter);

    connect(mView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(updateDataSetterButton()));

    updateDataSetterButton();

    connect(model,
            SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
            SIGNAL(valuesChanged()));
}

void MonthsTableWidget::setValues(const QList<double> &data)
{
    qfModel()->setValues(data);
}

QList< double > MonthsTableWidget::values() const
{
    return qfModel()->values();
}

MonthsTableModel *MonthsTableWidget::qfModel()
{
    return static_cast<MonthsTableModel *>(mView->model());
}

const MonthsTableModel *MonthsTableWidget::qfModel() const
{
    return static_cast<const MonthsTableModel *>(mView->model());
}

void MonthsTableWidget::updateDataSetterButton()
{
    QSet<int> selectedMonths;
    foreach(const QModelIndex &index, mView->selectionModel()->selectedIndexes()) {
        selectedMonths << qfModel()->monthNum(index);
    }
    mOpenDataSetter->setText(selectedMonths.size() > 1
                             ? tr("Set Selected")
                             : tr("Set All"));
}

QModelIndexList MonthsTableWidget::indexesToBatchSet() const
{
    QModelIndexList r;
    if (mView->selectionModel()->selectedIndexes().size() > 1) {
        r = mView->selectionModel()->selectedIndexes();
        for (QModelIndexList::Iterator i = r.begin(); i != r.end(); ++i) {
            if (i->column() != 1) {
                *i = mView->model()->index(i->row(), 1);
            }
        }
        // Оставляем только уникальные значения
        r = QSet<QModelIndex>::fromList(r).toList();
    } else {
        r = qfModel()->allData();
    }
    return r;
}

void MonthsTableWidget::batchSetData()
{
    foreach(const QModelIndex & index, indexesToBatchSet()) {
        mView->model()->setData(index, mDataSetterSpinbox->value());
    }
}

void MonthsTableWidget::setDataSetterValue()
{
    bool valuesAreSame = true;
    QModelIndexList indexList = indexesToBatchSet();
    QModelIndexList::ConstIterator it = indexList.constBegin();
    QAbstractItemModel *model = mView->model();
    double lastValue = model->data(*(it++)).toDouble();
    for (; it != indexList.end(); ++it) {
        if (model->data(*it).toDouble() != lastValue) {
            valuesAreSame = false;
            break;
        }
    }

    if (valuesAreSame) {
        mDataSetterSpinbox->setValue(lastValue);
    } else {
        mDataSetterSpinbox->setValue(0.0);
    }
}

int MonthsTableWidget::physicalProperty()
{
    Q_ASSERT(mDataSetterSpinbox->physicalProperty() == qfModel()->physicalProperty());
    return mDataSetterSpinbox->physicalProperty();
}

void MonthsTableWidget::setPhysicalProperty(int p)
{
    mDataSetterSpinbox->setPhysicalProperty(p);
    qfModel()->setPhysicalProperty(static_cast<PhysicalProperty>(p));
    mView->updateSizeLimits(); // теперь суффикс (единицы измерения) влезут
}
