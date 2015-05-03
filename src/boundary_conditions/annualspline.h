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

#ifndef QFGUI_ANNUALSPLINE_H
#define QFGUI_ANNUALSPLINE_H

#include <QObject>
#include <QVector>

namespace qfgui {

struct SplineCoeffs {
    double a;
    double b;
    double c;

    double value(const double key) const { return a + b * key + c * key * key; }
};

class AnnualSpline : public QObject
{
    Q_OBJECT

public:
    AnnualSpline(const QList<double> &montlyValues);

    QVector<double> dailyValues();

    static const QVector<double> DailyKeys; ///< 365 дней [0 .. 365) формата QCP
    static const QVector<double> MonthlyKeys; ///< 13 дней-границ меж месяцев

private:
    const QList<double> &mMontlyValues;
    const QVector<SplineCoeffs> mCoeffs;
};

}

#endif // QFGUI_ANNUALSPLINE_H
