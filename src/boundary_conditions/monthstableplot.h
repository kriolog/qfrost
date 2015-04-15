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

#ifndef QFGUI_MONTHSTABLEPLOT_H
#define QFGUI_MONTHSTABLEPLOT_H

#include <QFrame>

#include "qcustomplot.h"

namespace qfgui {
    QT_FORWARD_DECLARE_CLASS(MonthsTableExpander);

class MonthsTableGraph : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<double> values
               READ values
               WRITE setValues)

    Q_PROPERTY(bool visible
               READ isVisible
               WRITE setVisible)

    Q_PROPERTY(bool splineEnabled
               READ isSplineEnabled
               WRITE setSplineEnabled)

    Q_PROPERTY(int physicalPropertyChanged
               READ physicalProperty
               WRITE setPhysicalProperty)

public :
    MonthsTableGraph(QCPAxis* keyAxis, QCPAxis* valueAxis,
                     MonthsTableExpander *exp,
                     const QPen &penPrimary,
                     const QPen &penSecondary = QPen());

    const QList<double> &values() const { return mValues; }
    bool isVisible() const { return mStepsGraph->visible(); }
    bool isSplineEnabled() const { return mSplineEnabled; }
    int physicalProperty() const { return mPhysicalProperty; }

    void rescaleValueAxis();

public slots:
    void setValues(const QList<double> &values);
    void setVisible(bool b);
    void setSplineEnabled(bool b);
    void setPhysicalProperty(int p);

private:
    void updateStepsData();
    void updateSplineData();

    QList<double> mValues;

    int mPhysicalProperty;
    QList<double> mValuesConverted; ///< Наши величины после Unit::fromSI

    QCPGraph *const mStepsGraph;
    QCPGraph *const mSplineGraph;

    bool mSplineEnabled;

    bool mNeedStepsGraphUpdate;
    bool mNeedSplineGraphUpdate;

    const QPen mPenPrimary;
    const QPen mPenSecondary;
};

class MonthsTablePlot : public QFrame
{
    Q_OBJECT

public:
    MonthsTablePlot(MonthsTableExpander *exp1,
                    MonthsTableExpander *exp2,
                    MonthsTableExpander *exp3t,
                    MonthsTableExpander *exp3a,
                    QWidget* parent = 0);

public slots:
    void setConditionType(int type);
    void setSplineEnabled(bool b);

private slots:
    void updateValueAxes();

private:
    void setGraphsVisibile(int type, bool visible);

    MonthsTableExpander *const mExp1;
    MonthsTableExpander *const mExp2;
    MonthsTableExpander *const mExp3t;
    MonthsTableExpander *const mExp3a;

    QCustomPlot *const mPlot;

    int mCurrentType;

    MonthsTableGraph *const mTemperatures1;

    MonthsTableGraph *const mHeatFlowDensities;

    MonthsTableGraph *const mHeatTransferFactors;
    MonthsTableGraph *const mTemperatures3;
};
}

#endif // QFGUI_MONTHSTABLEPLOT_H
