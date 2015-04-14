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
#include <units/units.h>

#include <qcustomplot.h>

using namespace qfgui;

static const Qt::Orientation TablesOrienation = Qt::Horizontal;

BoundaryConditionEditDialog::BoundaryConditionEditDialog(ItemsModel *model,
        const QStringList &forbiddenNames,
        bool isNewItem, QWidget *parent)
    : ItemEditDialog(model, forbiddenNames, parent)
    , mTrendGroupBox(new QGroupBox(tr("Temperature trend"), this))
    , mTypeBox(new QComboBox(this))
    , mTable1(new MonthsTableWidget(TablesOrienation, this))
    , mTable2(new MonthsTableWidget(TablesOrienation, this))
    , mTable3(new MonthsTableWidget(TablesOrienation, this))
    , mExp1(mTable1->addExpander(tr("T")))
    , mExp2(mTable2->addExpander(tr("q")))
    , mExp3t(mTable3->addExpander(tr("T")))
    , mExp3a(mTable3->addExpander(tr("\316\261")))
    , mUsesTemperatureSpline(new QCheckBox(tr("Use spline interpolation for temperatures (recommended)"), this))
    , mPlot(new QCustomPlot(this))
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
    items << tr("First") << tr("Second") << tr("Third");
    QStringListModel *typeModel = new QStringListModel(items, this);
    mTypeBox->setModel(typeModel);
    QFormLayout *typeLayout = new QFormLayout;
    typeLayout->addRow(tr("&Type"), mTypeBox);
    mainLayout->addLayout(typeLayout);

    /**************************************************************************/

    QStackedWidget *valuesWidget = new QStackedWidget(this);
    valuesWidget->addWidget(mTable1);
    valuesWidget->addWidget(mTable2);
    valuesWidget->addWidget(mTable3);
    mainLayout->addWidget(valuesWidget);

    /**************************************************************************/
    mPlot->axisRect()->setupFullAxesBox(false);
    //mPlot->xAxis->setLabel(tr("Days since year start"));
    mPlot->setMinimumSize(300, 180);

    QFrame *plotFrame = new QFrame(this);
    plotFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QVBoxLayout *plotFrameLayout = new QVBoxLayout(plotFrame);
    plotFrameLayout->setContentsMargins(QMargins());
    plotFrameLayout->addWidget(mPlot);

    mainLayout->addWidget(mUsesTemperatureSpline);
    mainLayout->addWidget(plotFrame, 1);

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
    trendLayout->addRow(tr("Trend value:"), trendValue);
    trendLayout->addRow(tr("Reference year:"), trendStartYear);
    mainLayout->addWidget(mTrendGroupBox);

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

    mapper()->revert();

    /**************************************************************************/

    // Теперь все данные (в т.ч. хедеры таблиц) заполнены, установим авторазмеры
    mTable1->updateSizeLimits();
    mTable2->updateSizeLimits();
    mTable3->updateSizeLimits(true);

    /**************************************************************************/

    connect(mTypeBox, SIGNAL(currentIndexChanged(int)), SLOT(updatePlot()));
    connect(mExp1, SIGNAL(valuesChanged()), SLOT(updatePlot()));
    connect(mExp2, SIGNAL(valuesChanged()), SLOT(updatePlot()));
    connect(mExp3t, SIGNAL(valuesChanged()), SLOT(updatePlot()));
    connect(mExp3a, SIGNAL(valuesChanged()), SLOT(updatePlot()));
    connect(mUsesTemperatureSpline, SIGNAL(stateChanged(int)), SLOT(updatePlot()));
    updatePlot();

    /**************************************************************************/

    // Примем минимальный размер
    resize(QSize(0, 0));
}

void BoundaryConditionEditDialog::updateTemperatureWidgets(int type)
{
    const bool hasTemps = (type != 1);
    mTrendGroupBox->setEnabled(hasTemps);
    mUsesTemperatureSpline->setEnabled(hasTemps);
}

static QCPGraph *createStepsGraph(QCPAxis *keyAxis, QCPAxis *valAxis,
                                  const QVector<double> &monthlyVals)
{
    QCPGraph *result = keyAxis->parentPlot()->addGraph(keyAxis, valAxis);
    result->setAntialiased(false);
    result->setLineStyle(QCPGraph::lsStepLeft);

    QVector<double> additionalVal;
    additionalVal << monthlyVals.first();

    result->setData(AnnualSpline::MonthlyKeys, monthlyVals + additionalVal);

    return result;
}

static QCPGraph *createSplineGraph(QCPAxis *keyAxis, QCPAxis *valAxis,
                                   const QVector<double> &monthlyVals)
{
    QCPGraph *result = keyAxis->parentPlot()->addGraph(keyAxis, valAxis);

    AnnualSpline spline(monthlyVals.toList());

    const QVector<double> valsDaily = spline.dailyValues();
    QVector<double> daysDaily;
    daysDaily.reserve(valsDaily.size());
    for (int i = 0; i < valsDaily.size(); ++i) {
        daysDaily << i;
    }

    Q_ASSERT(daysDaily.size() == valsDaily.size());
    result->setData(daysDaily, valsDaily);

    return result;
}

/// Слегка расширает границы @p axis (до ближайших соседних целых)
static void enlargeAxisRange(QCPAxis *axis)
{
    static const double delta = 0.2;
    axis->setRange(qFloor(axis->range().lower - delta),
                   qCeil(axis->range().upper + delta));
}

void BoundaryConditionEditDialog::updatePlot()
{
    static const int firstGraphPenWidth = 3;
    static const int secondGraphPenWidth = 2;
    static const int stepsGraphWidth = 1;

    mPlot->clearGraphs();

    QCPAxis *const dateAxis = mPlot->xAxis;
    const int type = mTypeBox->currentIndex();
    const bool hasSecondGraph = (type == 2);
    if (!hasSecondGraph) {
        mPlot->yAxis2->setLabel(QString());
    }
    if (type != 1) {
        // Температуры (1 и 3 род)
        QCPAxis *const tAxis = mPlot->yAxis;
        if (hasSecondGraph) {
            // + коэффициенты теплообмена (3 род)
            QCPAxis *const aAxis = mPlot->yAxis2;
            Q_ASSERT(aAxis != tAxis);
            aAxis->setTickLabels(true);
            aAxis->setLabel(tr("Heat transfer factor\n\316\261") +
            Units::unit(this, mExp3a->physicalProperty()).headerSuffixOneLine());
            QCPGraph *aSteps = createStepsGraph(dateAxis, aAxis,
                                                mExp3a->values().toVector());
            aSteps->setPen(QPen(Qt::green, secondGraphPenWidth));
        }
        MonthsTableExpander *tempsExp = (type == 2) ? mExp3t : mExp1;
        tAxis->setLabel(tr("Temperature T") +
                        Units::unit(this, tempsExp->physicalProperty()).headerSuffixOneLine());
        const QVector<double> monthlyTemps = tempsExp->values().toVector();
        QCPGraph *tSteps = createStepsGraph(dateAxis, tAxis, monthlyTemps);
        if (!mUsesTemperatureSpline->isChecked()) {
            tSteps->setPen(QPen(Qt::blue, firstGraphPenWidth));
        } else {
            tSteps->setPen(QPen(Qt::blue, stepsGraphWidth, Qt::DashLine));
            QCPGraph *tSpline = createSplineGraph(dateAxis, tAxis, monthlyTemps);
            tSpline->setPen(QPen(Qt::blue, firstGraphPenWidth));
        }
    } else {
        // Плотности теплопотока (2 род)
        QCPAxis *const qAxis = mPlot->yAxis;
        qAxis->setLabel(tr("Heat flow density q") +
                        Units::unit(this, mExp2->physicalProperty()).headerSuffixOneLine());
        QCPGraph *qSteps = createStepsGraph(dateAxis, qAxis,
                                            mExp2->values().toVector());
        qSteps->setPen(QPen(Qt::red, firstGraphPenWidth));
    }

    mPlot->rescaleAxes();
    enlargeAxisRange(mPlot->yAxis);

    mPlot->yAxis2->setTickLabels(hasSecondGraph);
    if (hasSecondGraph) {
        enlargeAxisRange(mPlot->yAxis2);
    } else {
        mPlot->yAxis2->setRange(mPlot->yAxis->range());
    }

    mPlot->replot();
}
