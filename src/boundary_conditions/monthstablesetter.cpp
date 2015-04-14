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

#include <units/physicalpropertyspinbox.h>

#include <monthstableexpander.h>

using namespace qfgui;

MonthsTableSetter::MonthsTableSetter(QItemSelectionModel *selectionModel,
                                     QWidget *parent)
  : QWidget(parent)
  , mSetSelectedButton(new QPushButton(tr("Set selected..."), this))
  , mSetAllButtons()
  , mSetAllMapper(new QSignalMapper(this))
  , mSetAllButtonsLayout(new QHBoxLayout(this))
{
    connect(selectionModel->model(), SIGNAL(addedExpander(int,MonthsTableExpander*)),
            SLOT(onExpanderAdded(int,MonthsTableExpander*)));

    connect(mSetAllMapper, SIGNAL(mapped(int)), SLOT(doSetAll(int)));

    mSetAllButtonsLayout->setContentsMargins(QMargins());
    mSetAllButtonsLayout->addWidget(mSetSelectedButton);

    mSetSelectedButton->setVisible(false);
}


void MonthsTableSetter::onExpanderAdded(int sector, MonthsTableExpander *expander)
{
    Q_ASSERT(!mExpanders.contains(sector));

    mExpanders.insert(sector, expander);

    QPushButton *button = new QPushButton(tr("Set all %1...").arg(expander->valueName()));
    mSetAllButtons.insert(sector, button);
    mSetAllMapper->setMapping(button, sector);
    connect(button, SIGNAL(clicked()), mSetAllMapper, SLOT(map()));
    mSetAllButtonsLayout->addWidget(button);
}

void MonthsTableSetter::doSetAll(int sector)
{
    QPair<bool, double> editorResult = execEditor(sector);
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

void MonthsTableSetter::onSelectionChanged(const QItemSelection &selected)
{

}

QPair<bool, double> MonthsTableSetter::execEditor(int sector)
{
    MonthsTableExpander *expander = mExpanders.value(sector);
    Q_ASSERT(expander);

    QDialog *dialog = new QDialog(this);

    PhysicalPropertySpinBox *spinBox = new PhysicalPropertySpinBox(static_cast<PhysicalProperty>(expander->physicalProperty()),
                                                                   dialog);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                     dialog);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    // FIXME: надпись можно бы и получше придумать
    formLayout->addRow(tr("Batch set %1:").arg(expander->valueName()), spinBox);

    //: Batch set dialog title
    dialog->setWindowTitle(tr("Batch Set"));

    connect(buttons, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), dialog, SLOT(reject()));

    connect(dialog, SIGNAL(accepted()), SLOT(batchSetData()));

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
