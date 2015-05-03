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

#include "annualspline.h"

#include <QDate>

#include <QMessageBox>

#ifdef Q_OS_WIN // HACK почему-то без этого релиз-сборка в винде падает
#define eigen_assert(X) if(!X) { QString("Eigen assert failed! %1 at\n%2, line %3.").arg(#X).arg(__FILE__).arg(__LINE__); }
#endif //Q_OS_WIN
#define EIGEN_INITIALIZE_MATRICES_BY_ZERO
#include <Eigen/Sparse>

using namespace qfgui;

static const int MainYear = 2001;
static const QDate MainYearFirstDate = QDate(MainYear, 1, 1);
static const int MainYearDaysNum = MainYearFirstDate.daysTo(MainYearFirstDate.addYears(1));

static QVector<double> dailyKeys()
{
    QVector<double> x;
    x.reserve(MainYearDaysNum);

    int secsNum = 0;
    x.append(secsNum);
    for (int i = 1; i < MainYearDaysNum; ++i) {
        secsNum += 24 * 60 * 60;
        x.append(secsNum);
    }

    Q_ASSERT(x.size() == MainYearDaysNum);

    return x;
}

static QVector<double> monthlyKeys()
{
    QVector<double> x;
    x.reserve(13);

    x.append(0);
    for (int month = 2; month <= 12; ++month) {
        const QDate curDate(MainYear, month, 1);
        x.append(MainYearFirstDate.daysTo(curDate) * 24 * 60 * 60);
    }
    x.append(MainYearDaysNum * 24 * 60 * 60);

    Q_ASSERT(x.size() == 13);

    return x;
}

const QVector<double> AnnualSpline::DailyKeys = dailyKeys();
const QVector<double> AnnualSpline::MonthlyKeys = monthlyKeys();

/***
 * Коэффициенты сплайна периодической функции, заданной средними значениями.
 *
 * Пример применения: получение значений температуры по дням из среднемесячных.
 *
 * Параметры:
 * x: N+1 значений аргумента, задающих N интервалов. Должен быть возрастающим!
 * y: N средних значений функции для каждого из выделенных интервалов.
 *
 * То есть из N средних значений функции {y_i} на интервалах {[x_i .. x_{i+1}]}
 * получаем коэффициенты квадратичного сплайна f_i - N наборов {a_i, b_i, c_i},
 * задащих его внутри каждого интервала как f_i(x) = a_i + b_i * x + c_i * x^2.
 * Для этого решается система уравнений, составленная из накладываемых условий.
 *
 * Основные условия (корректность и соответствие средних):
 * 1) Непрерывность - f_i(x_i) = f_{i+1}(x_i).
 * 2) Гладкость первого порядка - f'_i(x_i) = f'_{i+1}(x_i).
 * 3) Известные средние - ∫ [x_{i-1} .. x_i] f_i(x)dx = y_i * (x_1 - x_{i-1}).
 *
 * Дополнительные условия (делают сплайн периодическим):
 * 1) Непрерывность на стыке периодов - f_0(x_0) = f_N(x_N).
 * 2) Гладкость первого порядка на стыке периодов - f'_0(x_0) = f'_N(x_N).
 *
 * Возвращается массив коэффициентов искомого сплайна: N троек {a_i, b_i, c_i}.
 */
QVector<SplineCoeffs> avgSplineCoeffs(const QVector<double> &x, const QVector<double> &y)
{
    const int numIntervals = y.size();
    const int numEquations = 3 * numIntervals;

    Q_ASSERT(x.size() == numIntervals + 1);

    QVector<SplineCoeffs> result;
    if (numIntervals < 2) {
        return result;
    }

    bool inputIsHorizontal = true;
    foreach(const double value, y) {
        if (value != y.first()) {
            inputIsHorizontal = false;
            break;
        }
    }
    if (inputIsHorizontal) {
        return result;
    }

    result.reserve(numIntervals);

    typedef Eigen::SparseMatrix<double, Eigen::ColMajor> Matrix;
    typedef Eigen::VectorXd Matrix1D;
    typedef Eigen::SparseLU<Matrix, Eigen::COLAMDOrdering<int> > Solver;

    Matrix a = Matrix(numEquations, numEquations);
    Matrix1D b = Matrix1D(numEquations);

    double prevX = x.first();
    for (int i = 0, j = 0; i < numIntervals; ++i, j += 3) {
        const double nextX = x.at(i + 1);

        const double h = nextX - prevX;

        if (h <= 0.0) {
            qFatal("%s: x should be sorted in ascending order!", Q_FUNC_INFO);
        }

        const double h2 = h * h;

        a.coeffRef(j, j) = 1.0;
        a.coeffRef(j, j+1) = h;
        a.coeffRef(j, j+2) = h2;

        Q_ASSERT(a.coeffRef(j+1, j) == 0.0);
        a.coeffRef(j+1, j+1) = 1.0;
        a.coeffRef(j+1, j+2) = 2.0 * h;

        a.coeffRef(j+2, j) = 1.0;
        a.coeffRef(j+2, j+1) = h / 2.0;
        a.coeffRef(j+2, j+2) = h2 / 3.0;

        Q_ASSERT(b.coeffRef(j) == 0.0);
        Q_ASSERT(b.coeffRef(j+1) == 0.0);
        b.coeffRef(j+2) = y.at(i);

        prevX = nextX;
    }

    for (int j = 0; j < numEquations - 3; j += 3) {
        a.coeffRef(j, j+3) = -1.0;
        a.coeffRef(j+1, j+4) = -1.0;
    }

    a.coeffRef(numEquations - 3, 0) = -1.0;
    a.coeffRef(numEquations - 2, 1) = -1.0;

    a.makeCompressed();

    Solver solver;
    solver.compute(a);
    const Matrix1D resultFlat = solver.solve(b);

    if (solver.info() == Eigen::Success) {
        for (int j = 0; j < numEquations; j += 3) {
            SplineCoeffs coeffs;
            coeffs.a = resultFlat.coeff(j);
            coeffs.b = resultFlat.coeff(j+1);
            coeffs.c = resultFlat.coeff(j+2);
            result.append(coeffs);
        }
    } else {
        QMessageBox::critical(NULL, AnnualSpline::tr("Spline Error"),
                              AnnualSpline::tr("Can not solve spline! Solver info: %1").arg(solver.info()));
    }

    return result;
}

QVector<SplineCoeffs> annualSplineCoeffs(const QList<double> &monthlyValues)
{
    Q_ASSERT(monthlyValues.size() == 12);

    return avgSplineCoeffs(AnnualSpline::MonthlyKeys, monthlyValues.toVector());
}

AnnualSpline::AnnualSpline(const QList<double> &monthlyValues)
    : mMontlyValues(monthlyValues)
    , mCoeffs(annualSplineCoeffs(monthlyValues))
{
    Q_ASSERT(!QDate::isLeapYear(MainYear));
    Q_ASSERT(MainYearDaysNum == 365);
    Q_ASSERT(monthlyValues.size() == 12);
}

QVector< double > AnnualSpline::dailyValues()
{
    QVector<double> result;
    result.reserve(MainYearDaysNum);

    if (mCoeffs.isEmpty()) {
        QDate date(MainYear, 1, 1);
        for (int i = 0; i < MainYearDaysNum; ++i) {
            result.append(mMontlyValues.at(date.month() - 1));
            date = date.addDays(1);
        }
    } else {
        Q_ASSERT(mCoeffs.size() == 12);
        QVector<SplineCoeffs>::ConstIterator coeffIt = mCoeffs.constBegin();
        int daysSinceMonthStart = 0;

        QDate date = MainYearFirstDate;
        while (date.year() == MainYear) {
            result << coeffIt->value(daysSinceMonthStart * 24 * 60 * 60);
            ++daysSinceMonthStart;
            date = date.addDays(1);
            if (date.day() == 1) {
                daysSinceMonthStart = 0;
                ++coeffIt; // сейчас перейдём в следующий месяц
            }
        }

        Q_ASSERT(result.size() == MainYearDaysNum);
        }

    return result;
}
