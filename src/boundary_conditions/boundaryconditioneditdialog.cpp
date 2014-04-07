/*
 * Copyright (C) 2011-2014  Denis Pesotsky
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

#include <boundary_conditions/boundaryconditioneditdialog.h>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDataWidgetMapper>
#include <QtWidgets/QComboBox>
#include <QtCore/QStringListModel>
#include <QtWidgets/QStackedWidget>

#include <boundary_conditions/boundaryconditionsmodel.h>
#include <boundary_conditions/monthstablewidget.h>
#include <itemviews/item.h>
#include <qfrost.h>
#include <mainwindow.h>

using namespace qfgui;

BoundaryConditionEditDialog::BoundaryConditionEditDialog(ItemsModel *model,
        const QStringList &forbiddenNames,
        bool isNewItem, QWidget *parent)
    : ItemEditDialog(model, forbiddenNames, parent)
{
    Q_ASSERT(qobject_cast< BoundaryConditionsModel * >(model) != NULL);

    setWindowTitle((isNewItem
                    ? tr("New Boundary Condition")
                    : tr("Edit Boundary Condition %1")
                    .arg(locale().quoteString(model->itemAt(0)->name()))));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    addLayout(mainLayout);

    QStringList items;
    items << tr("First") << tr("Second") << tr("Third");
    QComboBox *typeBox = new QComboBox(this);
    QStringListModel *typeModel = new QStringListModel(items, this);
    typeBox->setModel(typeModel);
    QFormLayout *typeLayout = new QFormLayout;
    typeLayout->addRow(tr("&Type"), typeBox);
    mainLayout->addLayout(typeLayout);

    MonthsTableWidget *values1 = new MonthsTableWidget(tr("T"), this);

    MonthsTableWidget *values2 = new MonthsTableWidget(tr("q"), this);

    QWidget *values3 = new QWidget(this);
    QHBoxLayout *thirdTypeParametersLayout = new QHBoxLayout(values3);
    thirdTypeParametersLayout->setContentsMargins(QMargins());
    MonthsTableWidget *temperatures3 = new MonthsTableWidget(tr("T"), this);

    MonthsTableWidget *heatTranferFactors = new MonthsTableWidget(tr("\316\261"), this);
    thirdTypeParametersLayout->addWidget(temperatures3);
    thirdTypeParametersLayout->addWidget(heatTranferFactors);

    QStackedWidget *valuesWidget = new QStackedWidget(this);
    valuesWidget->addWidget(values1);
    valuesWidget->addWidget(values2);
    valuesWidget->addWidget(values3);
    mainLayout->addWidget(valuesWidget);

    connect(typeBox, SIGNAL(currentIndexChanged(int)),
            valuesWidget, SLOT(setCurrentIndex(int)));
    /**************************************************************************/

    mapper()->addMapping(typeBox, BC_Type, "currentIndex");
    mapper()->addMapping(values1, BC_Temperatures1);
    mapper()->addMapping(values2, BC_HeatFlowDensities);
    mapper()->addMapping(temperatures3, BC_Temperatures3);
    mapper()->addMapping(heatTranferFactors, BC_HeatTransferFactors);

    mapper()->revert();

    // Примем минимальный размер
    resize(QSize(0, 0));
}
