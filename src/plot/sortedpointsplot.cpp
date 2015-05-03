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

#include "sortedpointsplot.h"

#include <soils/sortedpointsmodel.h>
#include <units/units.h>

#include <QFrame>
#include <QStackedLayout>

#include <qcustomplot.h>

using namespace qfgui;

SortedPointsPlot::SortedPointsPlot(SortedPointsModel *model, QWidget *parent)
    : QFrame(parent)
    , mModel(model)
    , mPlot(new QCustomPlot(this))
    , mGraph(mPlot->addGraph())
    , mLayout(new QStackedLayout(this))
    , mGraphExtension(new QCPItemLine(mPlot))
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

    mLayout->addWidget(new QWidget(this));
    mLayout->addWidget(mPlot);

    setMinimumSize(180, 150);

    mPlot->xAxis->setRangeReversed(true);

    QFont tickLabelFont = mPlot->xAxis->tickLabelFont();
    tickLabelFont.setPointSize(8);
    mPlot->xAxis->setTickLabelFont(tickLabelFont);
    mPlot->yAxis->setTickLabelFont(tickLabelFont);

    mPlot->axisRect()->setupFullAxesBox(true);

    mGraph->setPen(QPen(Qt::darkBlue, 3));
    mGraphExtension->setPen(QPen(Qt::darkBlue, 1, Qt::DashLine));

    mGraphExtension->setAntialiased(false);

    connect(mModel, SIGNAL(valuesChanged()), SLOT(updateGraph()));
    updateGraph();
}

void SortedPointsPlot::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange) {
        mLayout->setCurrentIndex(isEnabled());
    }
    QFrame::changeEvent(event);
}

void SortedPointsPlot::updateGraph()
{
    const QList<double> x = mModel->values().keys();
    const QList<double> y = mModel->values().values();

    if (x.isEmpty()) {
        mGraph->clearData();
    } else {
        const Unit &unitX = Units::unit(this, mModel->propertyX());
        const Unit &unitY = Units::unit(this, mModel->propertyY());

        mGraph->setData(unitX.fromSI(x).toVector(), unitY.fromSI(y).toVector());

        mGraph->rescaleAxes();
        static const double M = 5.0;
        mPlot->xAxis->setRange(qFloor(mPlot->xAxis->range().lower/M)*M - M/3.0,
                               0.0);
        mPlot->yAxis->setRange(0.0,
                               qCeil(mPlot->yAxis->range().upper/M)*M + M/3.0);

        const double extX = unitX.fromSI(x.first());
        const double extY = unitY.fromSI(y.first());
        mGraphExtension->start->setCoords(extX, extY);
        mGraphExtension->end->setCoords(mPlot->xAxis->range().lower - 100.0, extY);
    }

    mPlot->replot();
}
