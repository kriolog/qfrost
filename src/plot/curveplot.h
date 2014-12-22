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

#ifndef QFGUI_CURVEPLOT_H
#define QFGUI_CURVEPLOT_H

#include <QWidget>

QT_FORWARD_DECLARE_CLASS(QCustomPlot)
QT_FORWARD_DECLARE_CLASS(QCPPlotTitle)
QT_FORWARD_DECLARE_CLASS(QCPGraph)

namespace qfgui {

class CurvePlot : public QWidget
{
    Q_OBJECT
public:
    CurvePlot(Qt::Orientation coordsAxeOrientation, QWidget *parent = NULL);

public slots:
    void setCoords(const QVector<double> &data);

    void setModelDate(const QDate &date);
    void setTemperature(const QVector<double> &data);
    void setThawedPart(const QVector<double> &data);
    void setTransitionTemperature(const QVector<double> &data);

    void setModelDateVisible(bool visible);
    void setTemperatureVisible(bool visible);
    void setThawedPartVisible(bool visible);
    void setTransitionTemperatureVisible(bool visible);

    void setCoordsAxisRange(double lower, double upper);
    void setTemperatureAxisRange(double lower, double upper);

private:
    QCustomPlot *mPlot;

    QCPPlotTitle *mModelDate;

    QCPGraph *mTemperature;
    QCPGraph *mThawedPart;
    QCPGraph *mTransitionTemperature;

    QVector<double> mCoords;;
};
}

#endif // QFGUI_CURVEPLOT_H
