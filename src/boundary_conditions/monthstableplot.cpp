/*
 * Copyright (C) 2015-2016  Denis Pesotsky
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

#include "monthstableplot.h"

#include <QVBoxLayout>

#include "annualspline.h"
#include "monthstableexpander.h"
#include "units/units.h"

using namespace qfgui;

MonthsTableGraph::MonthsTableGraph(QCPAxis *keyAxis, QCPAxis *valueAxis,
                                   MonthsTableExpander *exp,
                                   const QPen &penPrimary,
                                   const QPen &penSecondary)
    : QObject(keyAxis->parentPlot())
    , mValues(exp->values())
    , mPhysicalProperty(exp->physicalProperty())
    , mValuesConverted(Units::unit(this, mPhysicalProperty).fromSI(mValues))
    , mStepsGraph(new QCPGraph(keyAxis, valueAxis))
    , mSplineGraph(new QCPGraph(keyAxis, valueAxis))
    , mSplineEnabled(false)
    , mPenPrimary(penPrimary)
    , mPenSecondary(penSecondary)
{
    mSplineGraph->setVisible(false);

    mStepsGraph->setAntialiased(false);
    mStepsGraph->setLineStyle(QCPGraph::lsStepLeft);

    mSplineGraph->setPen(mPenPrimary);
    mStepsGraph->setPen(mPenPrimary);

    updateSplineData();
    updateStepsData();

    connect(exp, SIGNAL(valuesChanged(QList<double>)), SLOT(setValues(QList<double>)));
    connect(exp, SIGNAL(physicalPropertyChanged(int)), SLOT(setPhysicalProperty(int)));
}

void MonthsTableGraph::setValues(const QList<double> &values)
{
    if (mValues == values) {
        return;
    }

    mValues = values;
    mValuesConverted = Units::unit(this, mPhysicalProperty).fromSI(values);

    bool needReplot = false;

    if (mStepsGraph->visible()) {
        updateStepsData();
        needReplot = true;
    } else {
        mNeedStepsGraphUpdate = true;
    }

    if (mSplineGraph->visible()) {
        updateSplineData();
        needReplot = true;
    } else {
        mNeedSplineGraphUpdate = true;
    }

    if (needReplot) {
        rescaleValueAxis();
        mStepsGraph->parentPlot()->replot();
    }
}

void MonthsTableGraph::setVisible(bool b)
{
    if (isVisible() == b) {
        return;
    }

    if (b) {
        if (mNeedStepsGraphUpdate) {
            updateStepsData();
        }
        if (mNeedSplineGraphUpdate && mSplineEnabled) {
            updateSplineData();
        }
    }

    mStepsGraph->setVisible(b);
    mSplineGraph->setVisible(mSplineEnabled ? b : false);

    rescaleValueAxis();
    mStepsGraph->parentPlot()->replot();
}

void MonthsTableGraph::setSplineEnabled(bool b)
{
    if (isSplineEnabled() == b) {
        return;
    }

    mSplineEnabled = b;

    const bool splineGraphNowVisible = mSplineEnabled && isVisible();

    mSplineGraph->setVisible(splineGraphNowVisible);
    mStepsGraph->setPen(b ? mPenSecondary : mPenPrimary);

    if (splineGraphNowVisible && mNeedSplineGraphUpdate) {
        updateSplineData();
    }

    rescaleValueAxis();
    mSplineGraph->parentPlot()->replot();
}

void MonthsTableGraph::setPhysicalProperty(int p)
{
    if (mPhysicalProperty == p) {
        return;
    }

    mPhysicalProperty = p;
    mValuesConverted = Units::unit(this, p).fromSI(mValues);

    if (isVisible()) {
        updateStepsData();
        if (mSplineEnabled) {
            updateSplineData();
        }
        rescaleValueAxis();
        mStepsGraph->parentPlot()->replot();
    } else {
        mNeedSplineGraphUpdate = true;
        mNeedStepsGraphUpdate = true;
    }
}

void MonthsTableGraph::updateStepsData()
{
    mNeedStepsGraphUpdate = false;

    if (mValues.isEmpty()) {
        mStepsGraph->clearData();
        return;
    }

    QVector<double> additionalVal;
    additionalVal.append(mValuesConverted.first());

    Q_ASSERT(AnnualSpline::MonthlyKeys.size() == mValues.size() + 1);

    mStepsGraph->setData(AnnualSpline::MonthlyKeys,
                         mValuesConverted.toVector() + additionalVal);
}

void MonthsTableGraph::updateSplineData()
{
    mNeedSplineGraphUpdate = false;

    if (mValues.isEmpty()) {
        mSplineGraph->clearData();
        return;
    }

    AnnualSpline spline(mValuesConverted);

    const QVector<double> valsDaily = spline.dailyValues();

    Q_ASSERT(AnnualSpline::DailyKeys.size() == valsDaily.size());

    mSplineGraph->setData(AnnualSpline::DailyKeys, valsDaily);
}

/// Слегка расширает границы @p axis (до ближайших соседних целых)
static void enlargeAxisRange(QCPAxis *axis)
{
        static const double delta = 0.2;
        axis->setRange(qFloor(axis->range().lower - delta),
                       qCeil(axis->range().upper + delta));
}

void MonthsTableGraph::rescaleValueAxis()
{
    if (!isVisible()) {
        return;
    }

    if (mSplineEnabled) {
        mSplineGraph->rescaleValueAxis();
    } else {
        mStepsGraph->rescaleValueAxis();
    }

    enlargeAxisRange(mStepsGraph->valueAxis());
}

////////////////////////////////////////////////////////////////////////////////

static const int FirstGraphPenWidth = 3;
static const int SecondGraphPenWidth = 2;
static const int StepsGraphWidth = 1;

MonthsTablePlot::MonthsTablePlot(MonthsTableExpander *exp1,
                                 MonthsTableExpander *exp2,
                                 MonthsTableExpander *exp3t,
                                 MonthsTableExpander *exp3a,
                                 QWidget* parent)
    : QFrame(parent)
    , mCurrentType(0)
    , mExp1(exp1)
    , mExp2(exp2)
    , mExp3t(exp3t)
    , mExp3a(exp3a)
    , mPlot(new QCustomPlot(this))
    , mTemperatures1(new MonthsTableGraph(mPlot->xAxis, mPlot->yAxis, exp1,
                                          QPen(Qt::darkBlue, FirstGraphPenWidth),
                                          QPen(Qt::darkBlue, StepsGraphWidth, Qt::DashLine)))
    , mHeatFlowDensities(new MonthsTableGraph(mPlot->xAxis, mPlot->yAxis, exp2,
                                              QPen(Qt::red, FirstGraphPenWidth)))
    , mHeatTransferFactors(new MonthsTableGraph(mPlot->xAxis, mPlot->yAxis2, exp3a,
                                                QPen(Qt::green, SecondGraphPenWidth)))
    , mTemperatures3(new MonthsTableGraph(mPlot->xAxis, mPlot->yAxis, exp3t,
                                          QPen(Qt::blue, FirstGraphPenWidth),
                                          QPen(Qt::blue, StepsGraphWidth, Qt::DashLine)))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->addWidget(mPlot);

    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    QVector<double> xTicks;
    QVector<QString> xTicksLabels;
    for (int month = 1; month <= 12; ++month) {
        xTicks.append(AnnualSpline::MonthlyKeys.at(month - 1));
        xTicksLabels.append(QFrost::romanNumeral(month));
    }
    mPlot->xAxis->setAutoTicks(false);
    mPlot->xAxis->setAutoTickLabels(false);
    mPlot->xAxis->setTickVector(xTicks);
    mPlot->xAxis->setTickVectorLabels(xTicksLabels);

    mPlot->xAxis->setRange(AnnualSpline::DailyKeys.first(),
                           AnnualSpline::DailyKeys.last());

    mPlot->axisRect()->setupFullAxesBox();

    mHeatFlowDensities->setVisible(false);
    mHeatTransferFactors->setVisible(false);
    mTemperatures3->setVisible(false);

    connect(exp1, SIGNAL(physicalPropertyChanged(int)), SLOT(updateValueAxes()));
    connect(exp2, SIGNAL(physicalPropertyChanged(int)), SLOT(updateValueAxes()));
    connect(exp3t, SIGNAL(physicalPropertyChanged(int)), SLOT(updateValueAxes()));
    connect(exp3a, SIGNAL(physicalPropertyChanged(int)), SLOT(updateValueAxes()));

    updateValueAxes();
}

void MonthsTablePlot::setConditionType(int type)
{
    if (mCurrentType == type) {
        return;
    }
    setGraphsVisibile(mCurrentType, false);
    mCurrentType = type;
    setGraphsVisibile(mCurrentType, true);
}

void MonthsTablePlot::setGraphsVisibile(int type, bool visible)
{
    if (type == 3) {
        return;
    }
    switch(type) {
    case 0:
        mTemperatures1->setVisible(visible);
        break;
    case 1:
        mHeatFlowDensities->setVisible(visible);
        break;
    case 2:
        mHeatTransferFactors->setVisible(visible);
        mTemperatures3->setVisible(visible);
        break;
    default:
        Q_ASSERT(false);
    }

    updateValueAxes();
}

void MonthsTablePlot::setSplineEnabled(bool b)
{
    if (mTemperatures1->isSplineEnabled() == b) {
        return;
    }

    mTemperatures1->setSplineEnabled(b);
    mTemperatures3->setSplineEnabled(b);
}

void MonthsTablePlot::updateValueAxes()
{
    if (mCurrentType == 2) {
        mPlot->yAxis2->setTickLabels(true);
        mPlot->yAxis2->setLabel(tr("Heat transfer factor\n\316\261") +
                                Units::unit(this, mExp3a->physicalProperty()).headerSuffixOneLine());
    } else {
        mPlot->yAxis2->setTickLabels(false);
        mPlot->yAxis2->setLabel(QString());
    }

    if (mCurrentType == 0 || mCurrentType == 2) {
        MonthsTableExpander *tempsExp = (mCurrentType == 2) ? mExp3t : mExp1;
        mPlot->yAxis->setLabel(tr("Temperature T") +
                               Units::unit(this, tempsExp->physicalProperty()).headerSuffixOneLine());
    } else {
        mPlot->yAxis->setLabel(tr("Heat flow density q") +
                               Units::unit(this, mExp2->physicalProperty()).headerSuffixOneLine());
    }

    const bool rangesMustBeSame = (mCurrentType != 2);
    if (rangesMustBeSame) {
        connect(mPlot->yAxis, SIGNAL(rangeChanged(QCPRange)),
                mPlot->yAxis2, SLOT(setRange(QCPRange)),
                Qt::UniqueConnection);
    } else {
        disconnect(mPlot->yAxis, SIGNAL(rangeChanged(QCPRange)),
                   mPlot->yAxis2, SLOT(setRange(QCPRange)));
    }

    ((mCurrentType == 0)
      ? mTemperatures1
      : ((mCurrentType == 1)
         ? mHeatFlowDensities
         : mTemperatures3))->rescaleValueAxis();

    if (mCurrentType == 2) {
        mHeatTransferFactors->rescaleValueAxis();
    }

    mPlot->replot();
}
