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

#include <boundary_conditions/boundaryconditioneditdialog.h>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QDataWidgetMapper>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtCore/QStringListModel>
#include <QtWidgets/QStackedWidget>

#include <boundary_conditions/boundaryconditionsmodel.h>
#include <boundary_conditions/monthstablewidget.h>
#include <boundary_conditions/monthstableexpander.h>
#include <itemviews/item.h>
#include <qfrost.h>
#include <mainwindow.h>
#include <smartdoublespinbox.h>
#include <annualspline.h>
#include "monthstableview.h"
#include "monthstableplot.h"
#include <yearlyparamswidget.h>

using namespace qfgui;

static const Qt::Orientation TablesOrienation = Qt::Horizontal;

BoundaryConditionEditDialog::BoundaryConditionEditDialog(ItemsModel *model,
        const QStringList &forbiddenNames,
        bool isNewItem, QWidget *parent)
    : ItemEditDialog(model, forbiddenNames, parent)
    , mTrendGroupBox(new QGroupBox(tr("Te&mperature trend"), this))
    , mTypeBox(new QComboBox(this))
    , mTable1(new MonthsTableWidget(TablesOrienation, this))
    , mTable2(new MonthsTableWidget(TablesOrienation, this))
    , mTable3(new MonthsTableWidget(TablesOrienation, this))
    , mYearlyParams(new YearlyParamsWidget(this))
    , mExp1(mTable1->addExpander(tr("T")))
    , mExp2(mTable2->addExpander(tr("q")))
    , mExp3t(mTable3->addExpander(tr("T")))
    , mExp3a(mTable3->addExpander(tr("\316\261")))
    , mUsesTemperatureSpline(new QCheckBox(tr("&Interpolate temperatures"), this))
    , mPlot(new MonthsTablePlot(mExp1, mExp2, mExp3t, mExp3a, this))
{
    Q_ASSERT(qobject_cast< BoundaryConditionsModel * >(model) != NULL);

    setWindowTitle((isNewItem
                    ? tr("New Boundary Condition")
                    : tr("Edit Boundary Condition %1")
                    .arg(locale().quoteString(model->itemAt(0)->name()))));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    addLayout(mainLayout);

    /**************************************************************************/

    QStringList items;
    items << tr("First") << tr("Second") << tr("Third") << tr("Third (yearly from file)");
    QStringListModel *typeModel = new QStringListModel(items, this);
    mTypeBox->setModel(typeModel);
    QFormLayout *typeLayout = new QFormLayout;
    typeLayout->addRow(tr("&Type:"), mTypeBox);
    mainLayout->addLayout(typeLayout);

    /**************************************************************************/

    QStackedWidget *valuesWidget = new QStackedWidget(this);
    valuesWidget->addWidget(mTable1);
    valuesWidget->addWidget(mTable2);
    valuesWidget->addWidget(mTable3);
    valuesWidget->addWidget(mYearlyParams);

    QGroupBox *tablesGroupBox = new QGroupBox(this);
    QVBoxLayout *tablesGroupBoxLayout = new QVBoxLayout(tablesGroupBox);
    tablesGroupBoxLayout->addWidget(valuesWidget);

    mainLayout->addWidget(tablesGroupBox);

    /**************************************************************************/
    mPlot->setMinimumSize(300, 180);
    connect(mTypeBox, SIGNAL(currentIndexChanged(int)),
            mPlot, SLOT(setConditionType(int)));
    connect(mUsesTemperatureSpline, SIGNAL(toggled(bool)),
            mPlot, SLOT(setSplineEnabled(bool)));
    /**************************************************************************/

    mUsesTemperatureSpline->setToolTip(tr("If checked <b>(recommended)</b>, spline interpolation "
                                          "is used to estimate daily temperatures for model."));

    QGroupBox *plotGroupBox = new QGroupBox(this);
    QVBoxLayout *plotLayout = new QVBoxLayout(plotGroupBox);
    plotLayout->addWidget(mPlot, 1);
    plotLayout->addWidget(mUsesTemperatureSpline);

    QHBoxLayout *secondLayout = new QHBoxLayout();
    secondLayout->addWidget(plotGroupBox, 1);
    secondLayout->addWidget(mTrendGroupBox);

    mainLayout->addLayout(secondLayout, 1);

    /**************************************************************************/

    QFormLayout *trendLayout = new QFormLayout(mTrendGroupBox);
    mTrendGroupBox->setCheckable(true);
    QSpinBox *trendStartYear = new QSpinBox(this);
    // FIXME ограничения года в панели моделирования сейчас не выставляются и
    //       сюда вписаны дефолтные ограничения QDateEdit. А лучше устанавливать
    //       ограничения там самостоятельно (и использовать то же самое здесь).
    trendStartYear->setMinimum(1773);
    trendStartYear->setMaximum(7999);
    SmartDoubleSpinBox *trendValue = new SmartDoubleSpinBox(this);
    trendValue->setDecimals(3);
    trendValue->setSingleStep(0.001);
    trendValue->setMinimum(-1.0);
    trendValue->setMaximum(1.0);
    trendValue->setSuffix(tr(" \302\260C/decade"));
    trendLayout->addRow(tr("Trend &value:"), trendValue);
    trendLayout->addRow(tr("&Reference year:"), trendStartYear);

    /**************************************************************************/

    connect(mTypeBox, SIGNAL(currentIndexChanged(int)),
            valuesWidget, SLOT(setCurrentIndex(int)));
    connect(mTypeBox, SIGNAL(currentIndexChanged(int)),
            SLOT(updateTemperatureWidgets(int)));

    /**************************************************************************/

    mapper()->addMapping(mTypeBox, BC_Type, "currentIndex");
    mapper()->addMapping(mExp1, BC_Temperatures1);
    mapper()->addMapping(mExp2, BC_HeatFlowDensities);
    mapper()->addMapping(mExp3t, BC_Temperatures3);
    mapper()->addMapping(mExp3a, BC_HeatTransferFactors);
    mapper()->addMapping(mTrendGroupBox, BC_HasTemperatureTrend, "checked");
    mapper()->addMapping(trendValue, BC_TemperatureTrend);
    mapper()->addMapping(trendStartYear, BC_TemperatureTrendStartYear);
    mapper()->addMapping(mUsesTemperatureSpline, BC_UsesTemperatureSpline);
    mapper()->addMapping(mYearlyParams, BC_YearlyParams);

    mapper()->revert();

    /**************************************************************************/

    // Теперь все данные (в т.ч. хедеры таблиц) заполнены, установим авторазмеры
    const int h1 = mTable1->view()->updateSizeLimits();
    const int h2 = mTable2->view()->updateSizeLimits();
    const int h3 = mTable3->view()->updateSizeLimits();

    const int maxHeight = qMax(h1, qMax(h2, h3));

    mTable1->view()->setMaximumHeight(maxHeight);
    mTable2->view()->setMaximumHeight(maxHeight);
    mTable3->view()->setMaximumHeight(maxHeight);

    // Примем минимальный размер
    resize(QSize(0, 0));
}

void BoundaryConditionEditDialog::updateTemperatureWidgets(int type)
{
    const bool hasTemps = (type != 1 && type != 3);
    mTrendGroupBox->setEnabled(hasTemps);
    mUsesTemperatureSpline->setEnabled(hasTemps);
}
