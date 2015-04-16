/*
 * Copyright (C) 2015  Denis Pesotsky
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
 *
 */

#include "monthstablesetter.h"

#include <QtCore/QSignalMapper>
#include <qitemselectionmodel.h>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedLayout>

#include <units/physicalpropertyspinbox.h>

#include <monthstableexpander.h>

using namespace qfgui;

MonthsTableSetter::MonthsTableSetter(QItemSelectionModel *selectionModel,
                                     Qt::Orientation orientation,
                                     QWidget *parent)
  : QWidget(parent)
  , mSelectionModel(selectionModel)
  , mOrientation(orientation)
  , mExpanders()
  , mLayout(new QStackedLayout(this))
  , mSetAllButtons()
  , mSetMonthlyButtons()
  , mSetAllMapper(new QSignalMapper(this))
  , mSetMonthlyMapper(new QSignalMapper(this))
  , mSetAllButtonsLayout(new QHBoxLayout())
  , mSetMonthlyButtonsLayout(new QHBoxLayout())
  , mSetAllButtonsIndex(/* will set later*/)
  , mSetMonthlyButtonsIndex(/* will set later*/)
  , mSelectedMonths()
{
    connect(selectionModel->model(),
            SIGNAL(addedExpander(int,MonthsTableExpander*)),
            SLOT(onExpanderAdded(int,MonthsTableExpander*)));

    connect(selectionModel,
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(onSelectionChanged()));

    QWidget *setAllButtonsWidget = new QWidget(this);
    setAllButtonsWidget->setLayout(mSetAllButtonsLayout);
    mSetAllButtonsLayout->setContentsMargins(QMargins());
    mSetAllButtonsIndex = mLayout->addWidget(setAllButtonsWidget);
    connect(mSetAllMapper, SIGNAL(mapped(int)), SLOT(doSetAll(int)));

    QWidget *setMonthlyButtonsWidget = new QWidget(this);
    setMonthlyButtonsWidget->setLayout(mSetMonthlyButtonsLayout);
    mSetMonthlyButtonsLayout->setContentsMargins(QMargins());
    mSetMonthlyButtonsIndex = mLayout->addWidget(setMonthlyButtonsWidget);
    connect(mSetMonthlyMapper, SIGNAL(mapped(int)), SLOT(doSetMonthly(int)));
}

void MonthsTableSetter::onExpanderAdded(int sector, MonthsTableExpander *expander)
{
    Q_ASSERT(!mExpanders.contains(sector));

    mExpanders.insert(sector, expander);

    QPushButton *setAll = new QPushButton(tr("Set all %1...").arg(expander->valueName()));
    mSetAllButtons.insert(sector, setAll);
    mSetAllMapper->setMapping(setAll, sector);
    connect(setAll, SIGNAL(clicked()), mSetAllMapper, SLOT(map()));
    mSetAllButtonsLayout->addWidget(setAll);

    QPushButton *setMonthly = new QPushButton(tr("Set selected %1...").arg(expander->valueName()));
    mSetMonthlyButtons.insert(sector, setMonthly);
    mSetMonthlyMapper->setMapping(setMonthly, sector);
    connect(setMonthly, SIGNAL(clicked()), mSetMonthlyMapper, SLOT(map()));
    mSetMonthlyButtonsLayout->addWidget(setMonthly);
}

void MonthsTableSetter::doSetAll(int sector)
{
    QPair<bool, double> editorResult = execEditor(sector, true);
    if (!editorResult.first) {
        return;
    }

    MonthsTableExpander *expander = mExpanders.value(sector);
    Q_ASSERT(expander);
    QList<double> newData;
    for (int i = 1; i <= 12; ++i) {
        newData.append(editorResult.second);
    }
    expander->setValues(newData);
}

void MonthsTableSetter::doSetMonthly(int sector)
{
    QPair<bool, double> editorResult = execEditor(sector, false);
    if (!editorResult.first) {
        return;
    }

    MonthsTableExpander *expander = mExpanders.value(sector);
    Q_ASSERT(expander);
    QList<double> newData = expander->values();
    Q_ASSERT(newData.size() == 12);
    foreach (const int month, mSelectedMonths) {
        newData[month] = (editorResult.second);
    }
    expander->setValues(newData);
}

void MonthsTableSetter::onSelectionChanged()
{
    mSelectedMonths.clear();

    foreach(const QModelIndex &index, mSelectionModel->selectedIndexes()) {
        const int month = (mOrientation == Qt::Vertical) ? index.row()
                                                         : index.column();
        mSelectedMonths.insert(month);
    }

    mLayout->setCurrentIndex((mSelectedMonths.size() < 2)
                             ? mSetAllButtonsIndex
                             : mSetMonthlyButtonsIndex);
}

QPair<bool, double> MonthsTableSetter::execEditor(int sector, bool setAll)
{
    MonthsTableExpander *expander = mExpanders.value(sector);
    Q_ASSERT(expander);

    QList<int> monthsToSet;
    if (setAll) {
        for (int i = 0; i < 12; ++i) {
            monthsToSet << i;
        }
    } else {
        monthsToSet = mSelectedMonths.toList();
        if (monthsToSet.isEmpty()) {
            qCritical("%s called with no months selected", Q_FUNC_INFO);
            return qMakePair(false, 0.0);
        }
    }

    const double firstVal = expander->value(monthsToSet.first());
    bool allValsAreSame = true;
    foreach (const int month, monthsToSet) {
        if (!qFuzzyCompare(firstVal, expander->value(month))) {
            allValsAreSame = false;
            break;
        }
    }

    QDialog *dialog = new QDialog(this);

    PhysicalPropertySpinBox *spinBox = new PhysicalPropertySpinBox(static_cast<PhysicalProperty>(expander->physicalProperty()),
                                                                   dialog);
    if (allValsAreSame) {
        spinBox->setValue(firstVal);
    }

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                     dialog);
    buttons->button(QDialogButtonBox::Ok)->setText(tr("Set %n values", 0,
                                                      monthsToSet.size()));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    // FIXME: надпись можно бы и получше придумать
    formLayout->addRow((setAll
                        ? tr("New value for all %1:")
                        : tr("New value for selected %1:"))
                       .arg(expander->valueName()), spinBox);

    //: Batch set dialog title
    dialog->setWindowTitle(tr("Batch Set"));

    connect(buttons, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

    QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);

    dialogLayout->addStretch();
    dialogLayout->addLayout(formLayout);
    dialogLayout->addStretch();
    dialogLayout->addWidget(buttons);

    // Пусть примет минимальный размер по вертикали и красивый по горизонтали
    dialog->resize(QSize(300, 0));

    const int dialogResult = dialog->exec();
    return qMakePair(dialogResult == QDialog::Accepted, spinBox->value());
}
