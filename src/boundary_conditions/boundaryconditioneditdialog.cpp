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
    , mTable1(new MonthsTableWidget(tr("T"), TablesOrienation, this))
    , mTable2(new MonthsTableWidget(tr("q"), TablesOrienation, this))
    , mTable3Temps(new MonthsTableWidget(tr("T"), TablesOrienation, this))
    , mTable3Factors(new MonthsTableWidget(tr("\316\261"), TablesOrienation, this))
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

    QWidget *values3 = new QWidget(this);
    QBoxLayout *thirdTypeParametersLayout;
    if (TablesOrienation == Qt::Vertical) {
        thirdTypeParametersLayout = new QHBoxLayout(values3);
    } else {
        thirdTypeParametersLayout = new QVBoxLayout(values3);
    }
    thirdTypeParametersLayout->setContentsMargins(QMargins());

    thirdTypeParametersLayout->addWidget(mTable3Temps);
    thirdTypeParametersLayout->addWidget(mTable3Factors);

    QStackedWidget *valuesWidget = new QStackedWidget(this);
    valuesWidget->addWidget(mTable1);
    valuesWidget->addWidget(mTable2);
    valuesWidget->addWidget(values3);
    mainLayout->addWidget(valuesWidget);

    /**************************************************************************/
    mPlot->axisRect()->setupFullAxesBox(false);
    mPlot->xAxis->setLabel(tr("Days since year start"));
    mPlot->setMinimumSize(300, 180);

    QFrame *plotFrame = new QFrame(this);
    plotFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QVBoxLayout *plotFrameLayout = new QVBoxLayout(plotFrame);
    plotFrameLayout->setContentsMargins(QMargins());
    plotFrameLayout->addWidget(mPlot);

    /*QPushButton *showPlotButton = new QPushButton(QIcon::fromTheme("office-chart-line"),
                                                  tr("Show Graphs"),
                                                  this);
    connect(showPlotButton, SIGNAL(clicked()), SLOT(showGraphs()));
    mainLayout->addWidget(showPlotButton);*/
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
            SLOT(updateTrendWidgetVisibility(int)));

    /**************************************************************************/

    mapper()->addMapping(mTypeBox, BC_Type, "currentIndex");
    mapper()->addMapping(mTable1, BC_Temperatures1);
    mapper()->addMapping(mTable2, BC_HeatFlowDensities);
    mapper()->addMapping(mTable3Temps, BC_Temperatures3);
    mapper()->addMapping(mTable3Factors, BC_HeatTransferFactors);
    mapper()->addMapping(mTrendGroupBox, BC_HasTemperatureTrend, "checked");
    mapper()->addMapping(trendValue, BC_TemperatureTrend);
    mapper()->addMapping(trendStartYear, BC_TemperatureTrendStartYear);

    mapper()->revert();

    /**************************************************************************/

    connect(mTypeBox, SIGNAL(currentIndexChanged(int)), SLOT(updatePlot()));
    connect(mTable1, SIGNAL(valuesChanged()), SLOT(updatePlot()));
    connect(mTable2, SIGNAL(valuesChanged()), SLOT(updatePlot()));
    connect(mTable3Temps, SIGNAL(valuesChanged()), SLOT(updatePlot()));
    connect(mTable3Factors, SIGNAL(valuesChanged()), SLOT(updatePlot()));
    updatePlot();

    /**************************************************************************/

    // Примем минимальный размер
    resize(QSize(0, 0));
}

void BoundaryConditionEditDialog::updateTrendWidgetVisibility(int type)
{
    const bool canHaveTrend = (type != 1);
    mTrendGroupBox->setVisible(canHaveTrend);
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
    static const int firstGraphPenWidth = 2;
    static const int secondGraphPenWidth = 1;
    static const int stepsGraphWidth = 0;

    mPlot->clearGraphs();

    QCPAxis *const dateAxis = mPlot->xAxis;
    const int type = mTypeBox->currentIndex();
    const bool hasSecondGraph = (type == 2);
    if (type != 1) {
        // Температуры (1 и 3 род)
        QCPAxis *const tAxis = mPlot->yAxis;
        if (hasSecondGraph) {
            // + коэффициенты теплообмена (3 род)
            QCPAxis *const aAxis = mPlot->yAxis2;
            Q_ASSERT(aAxis != tAxis);
            aAxis->setTickLabels(true);
            aAxis->setLabel(tr("Heat transfer factor \316\261") +
            Units::unit(this, mTable3Factors->physicalProperty()).headerSuffixOneLine());
            QCPGraph *aSteps = createStepsGraph(dateAxis, aAxis,
                                                mTable3Factors->values().toVector());
            aSteps->setPen(QPen(Qt::green, secondGraphPenWidth));
        }
        MonthsTableWidget *tempsWidget = (type == 2) ? mTable3Temps : mTable1;
        tAxis->setLabel(tr("Temperature T") +
                        Units::unit(this, tempsWidget->physicalProperty()).headerSuffixOneLine());
        const QVector<double> monthlyTemps = tempsWidget->values().toVector();
        QCPGraph *tSteps = createStepsGraph(dateAxis, tAxis, monthlyTemps);
        tSteps->setPen(QPen(Qt::blue, stepsGraphWidth, Qt::DashLine));
        QCPGraph *tSpline = createSplineGraph(dateAxis, tAxis, monthlyTemps);
        tSpline->setPen(QPen(Qt::blue, firstGraphPenWidth));
    } else {
        // Плотности теплопотока (2 род)
        QCPAxis *const qAxis = mPlot->yAxis;
        qAxis->setLabel(tr("Heat flow density q") +
                        Units::unit(this, mTable2->physicalProperty()).headerSuffixOneLine());
        QCPGraph *qSteps = createStepsGraph(dateAxis, qAxis,
                                            mTable2->values().toVector());
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
