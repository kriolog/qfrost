/*
 * Copyright (C) 2010-2015  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFCORE_BOUNDARYCONDITION_H
#define QFCORE_BOUNDARYCONDITION_H

#include <vector>
#include <cassert>

#include <boost/date_time/gregorian/gregorian.hpp>

namespace qfcore
{

/**
 * Класс для описания граничных условий
 */
class BoundaryCondition
{
public:
    /// Род граничных условий
    enum Type {
        FirstType = 0,  ///< I -- температура среды
        SecondType = 1, ///< II -- плотность теплопотока
        ThirdType = 2   ///< III -- температура среды и коэффициент теплоотдачи
    };

    /**
     * Конструктор для условий I и II рода.
     * @param type тип граничного условия
     * @param params значения характеристики условия (т.е. 12 или 365 температур
     *               для I рода или же 12 плотностей теплопотока для II рода)
     * @param hasTemperatureTrend используется ли температурный тренд
     * @param temperatureTrend величина температурного тренда
     * @param temperatureTrendStartYear год, являющейся точкой отсчёта тренда
     */
    BoundaryCondition(Type type,
                      const std::vector<double> &params,
                      bool hasTemperatureTrend = false,
                      double temperatureTrend = 0,
                      int temperatureTrendStartYear = 0)
        : mType(type)
        , mParams1(params)
        , mResistivities()
        , mHasAnnualTemperatures(params.size() != 12)
        , mHasTemperatureTrend(hasTemperatureTrend)
        , mTemperatureTrend(temperatureTrend)
        , mTemperatureTrendStartYear(temperatureTrendStartYear)
        , mCurrentMonth(-1)
    {
        assert(mParams1.size() == 12 || mParams1.size() == 365);
        assert(mType != ThirdType);
        assert(mType != SecondType || !hasTemperatureTrend);
        assert(mType != SecondType || !mHasAnnualTemperatures);
    }

    /**
     * Конструктор для условий III рода.
     * @param temperatures 12 или 365 значений температуры
     * @param resistivities 12 значений термического сопротивления
     * @param hasTemperatureTrend используется ли температурный тренд
     * @param temperatureTrend величина температурного тренда
     * @param temperatureTrendStartYear год, являющейся точкой отсчёта тренда
     */
    BoundaryCondition(const std::vector<double> &temperatures,
                      const std::vector<double> &resistivities,
                      bool hasTemperatureTrend = false,
                      double temperatureTrend = 0,
                      int temperatureTrendStartYear = 0)
        : mType(ThirdType)
        , mParams1(temperatures)
        , mResistivities(resistivities)
        , mHasAnnualTemperatures(temperatures.size() != 12)
        , mHasTemperatureTrend(hasTemperatureTrend)
        , mTemperatureTrend(temperatureTrend)
        , mTemperatureTrendStartYear(temperatureTrendStartYear)
        , mCurrentMonth(-1)
    {
        assert(mParams1.size() == 12 || mParams1.size() == 365);
        assert(mResistivities.size() == 12);
    }

    /**
     * Номер элемента в массиве из 365 значений, соответствующий заданной дате.
     * Для високосных лет 28 и 29 февраля считаются одним днём - 28 февраля.
     */
    inline static int annualArrayIndex(int year, int month, int day) {
        if (boost::gregorian::gregorian_calendar::is_leap_year(year)) {
            year = 2001;
            if (month == 2 && day == 29) {
                --day;
            }
        }
        assert(!boost::gregorian::gregorian_calendar::is_leap_year(year));
        try {
            boost::gregorian::date date(year, month, day);
            const int dayNumber = date.day_of_year();
            assert(dayNumber >= 1 && dayNumber <= 365);
            return dayNumber - 1;
        } catch (std::out_of_range& e) {
            std::cerr << "Bad date: " << e.what() << std::endl;
            return 0;
        }
    }

    /**
     * Изменение параметров при переходе в новую дату.
     * @see SoilBlock::moveInTime(), HeatSurface::moveInTime().
     */
    inline void setDate(int year, int month, int day) {
        if (!mHasAnnualTemperatures && mCurrentMonth == month) {
            return;
        }
        mCurrentMonth = month;
        mCurrentParam1 = mParams1.at(mHasAnnualTemperatures
                                     ? annualArrayIndex(year, month, day)
                                     : month - 1);
        if (mType == ThirdType) {
            mCurrentResistivity = mResistivities.at(month - 1);
        }
        if (mHasTemperatureTrend) {
            assert(mType != SecondType);
            mCurrentParam1 += mTemperatureTrend * double(year - mTemperatureTrendStartYear);
        }
    }

    /// Тип граничного условия.
    inline Type type() const {
        return mType;
    }
    /// Текущая температура среды (\f$ T_с \f$)
    inline double temperature() const {
        assert(mType != SecondType);
        return mCurrentParam1;
    }
    /// Текущее термическое сопротивление (\f$ R=1/\alpha \f$)
    inline double resistivity() const {
        assert(mType == ThirdType);
        return mCurrentResistivity;
    }
    /// Текущая плотность теплоптока (\f$ q \f$)
    inline double heatFlowDensity() const {
        assert(mType == SecondType);
        return mCurrentParam1;
    }

private:
    /// Род граничного условия
    Type mType;

    /// Температуры (для I и III рода) или плотности теплопотока (для II рода)
    std::vector<double> mParams1;
    /// Термические сопротивления (для III рода)
    std::vector<double> mResistivities;

    /// Есть ли в mParams1 365 значений температуры (false - 12 среднемесячных)
    bool mHasAnnualTemperatures;

    /// Текущее термическое сопротивление
    double mCurrentResistivity;
    /// Текущая температура или плотность теплопотока (@sa mParams1)
    double mCurrentParam1;

    /// Включен ли тренд температуры
    bool mHasTemperatureTrend;
    /// Тренд температуры (градусов за год)
    double mTemperatureTrend;
    /// Год, являющейся точкой отсчёта для тренда (на его протяжении вклад = 0)
    double mTemperatureTrendStartYear;

    /// Текущий месяц (от 1 до 12) или -1, если setDate() не вызывался
    int mCurrentMonth;
};

}

#endif // QFCORE_BOUNDARYCONDITION_H
