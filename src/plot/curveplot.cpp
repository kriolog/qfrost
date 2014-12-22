/*
 * Copyright (C) 2014  Denis Pesotsky
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

#include "curveplot.h"

#include <QFrame>

#include <qcustomplot.h>

using namespace qfgui;

QCPGraph *createGraph(QCustomPlot *plot,
                      Qt::Orientation coordsAxeOrientation,
                      bool useAdditionalAxis = false)
{
    QCPAxis *const xAxis = useAdditionalAxis ? plot->xAxis2 : plot->xAxis;
    QCPAxis *const yAxis = useAdditionalAxis ? plot->yAxis2 : plot->yAxis;

    const bool xIsCoord = (coordsAxeOrientation == Qt::Horizontal);

    QCPAxis *coordAxis = xIsCoord ? xAxis : yAxis;
    QCPAxis *dataAxis = xIsCoord ? yAxis : xAxis;

    return plot->addGraph(coordAxis, dataAxis);
}

CurvePlot::CurvePlot(Qt::Orientation coordsAxeOrientation, QWidget *parent)
    : QFrame(parent)
    , mPlot(new QCustomPlot(this))
    , mModelDate(new QCPPlotTitle(mPlot, ""))
    , mTemperature(createGraph(mPlot, coordsAxeOrientation))
    , mThawedPart(createGraph(mPlot, coordsAxeOrientation, true))
    , mTransitionTemperature(createGraph(mPlot, coordsAxeOrientation))
    , mCoords()
{
    mPlot->plotLayout()->insertRow(0);
    mPlot->plotLayout()->addElement(0, 0, mModelDate);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mPlot);
    layout->setMargin(0);

    mPlot->setMinimumSize(300, 300);

    const QFont titleFont(QFont().family(), 12, QFont::Bold);
    mModelDate->setFont(titleFont);

    QCPAxis *const coordAxis = mTemperature->keyAxis();
    QCPAxis *const coordAxis2 = coordsAxeOrientation == Qt::Horizontal
                                ? mPlot->xAxis2
                                : mPlot->yAxis2;

    QCPAxis *const temperatureAxis = mTemperature->valueAxis();
    QCPAxis *const thawedPartAxis = mThawedPart->valueAxis();

    if (coordsAxeOrientation == Qt::Vertical) {
        coordAxis->setRangeReversed(true);
        coordAxis2->setRangeReversed(true);
    }

    coordAxis2->setVisible(true);
    coordAxis2->setRange(coordAxis->range());
    connect(coordAxis, SIGNAL(rangeChanged(QCPRange)),
            coordAxis2, SLOT(setRange(QCPRange)));

    const QFont labelsFont(QFont().family(), 8);
    coordAxis->setTickLabelFont(labelsFont);
    coordAxis2->setTickLabelFont(labelsFont);
    temperatureAxis->setTickLabelFont(labelsFont);
    thawedPartAxis->setTickLabelFont(labelsFont);

    coordAxis->setAutoTickStep(true);
    coordAxis2->setAutoTickStep(true);
    temperatureAxis->setAutoTickStep(true);
    thawedPartAxis->setAutoTickStep(false);

    const double d = 0.005;
    thawedPartAxis->setRange(0.0 - d, 1.0 + d);
    thawedPartAxis->setTickStep(0.2);
    thawedPartAxis->setVisible(true);
    thawedPartAxis->setTickLabels(true);

    QPen gridPen = mPlot->xAxis->grid()->pen();
    gridPen.setColor(Qt::darkGray);

    mPlot->xAxis->grid()->setSubGridVisible(true);
    mPlot->xAxis->grid()->setPen(gridPen);

    mPlot->yAxis->grid()->setSubGridVisible(true);
    mPlot->yAxis->grid()->setPen(gridPen);

    //mPlot->xAxis->setTickLabelRotation(-48);

    mPlot->setAntialiasedElement(QCP::aePlottables, true);

    mTemperature->setPen(QPen(Qt::black, 2));
    mThawedPart->setPen(QPen(Qt::blue, 2));
    mTransitionTemperature->setPen(QPen(Qt::black, 2, Qt::DashLine));

    const QPen thawedPartAxisPen(Qt::blue);
    thawedPartAxis->setBasePen(thawedPartAxisPen);
    thawedPartAxis->setTickPen(thawedPartAxisPen);
    thawedPartAxis->setSubTickPen(thawedPartAxisPen);
    thawedPartAxis->setLabelColor(thawedPartAxisPen.color());
    thawedPartAxis->setTickLabelColor(thawedPartAxisPen.color());

    //new CoordsTooltip(mPlot);

    temperatureAxis->setLabel(tr("Temperature, \302\260C"));
    coordAxis->setLabel(coordsAxeOrientation == Qt::Vertical
                        ? tr("Depth, m")
                        : tr("X coordinate, m"));
    thawedPartAxis->setLabel(tr("Thawed part"));

    setFrameStyle(QFrame::Panel | QFrame::Sunken);

    mPlot->replot();
}

void CurvePlot::setCoords(const QVector<double> &data)
{
    Q_ASSERT(mCoords.isEmpty());
    mCoords = data;
}

void CurvePlot::setModelDate(const QDate &date)
{
    mModelDate->setText(date.toString(Qt::DefaultLocaleShortDate));
}

void CurvePlot::setTemperature(const QVector<double> &data)
{
    Q_ASSERT(mCoords.size() == data.size());
    mTemperature->setData(mCoords, data);
    mPlot->replot();
}

void CurvePlot::setThawedPart(const QVector<double> &data)
{
    Q_ASSERT(mCoords.size() == data.size());
    mThawedPart->setData(mCoords, data);
    mPlot->replot();
}

void CurvePlot::setTransitionTemperature(const QVector<double> &data)
{
    Q_ASSERT(mCoords.size() == data.size());
    mTransitionTemperature->setData(mCoords, data);
    mPlot->replot();
}

void CurvePlot::setModelDateVisible(bool visible)
{
    mModelDate->setVisible(visible);
    mPlot->replot();
}

void CurvePlot::setTemperatureVisible(bool visible)
{
    mTemperature->setVisible(visible);
    mPlot->replot();
}

void CurvePlot::setThawedPartVisible(bool visible)
{
    mThawedPart->setVisible(visible);
    mPlot->replot();
}

void CurvePlot::setTransitionTemperatureVisible(bool visible)
{
    mTransitionTemperature->setVisible(visible);
    mPlot->replot();
}

void CurvePlot::setCoordsAxisRange(double lower, double upper)
{
    mTemperature->keyAxis()->setRange(lower, upper);
    mPlot->replot();
}

void CurvePlot::setTemperatureAxisRange(double lower, double upper)
{
    mTemperature->valueAxis()->setRange(lower, upper);
    mPlot->replot();
}
