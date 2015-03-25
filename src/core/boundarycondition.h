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
     * @param params 12 значений характеристики условия, т.е. температур для
     *               условия I рода и плотностей теплопотока для условия II рода
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
        , mHasTemperatureTrend(hasTemperatureTrend)
        , mTemperatureTrend(temperatureTrend)
        , mTemperatureTrendStartYear(temperatureTrendStartYear)
        , mCurrentMonth(-1) {
        assert(mParams1.size() == 12);
        assert(mType != ThirdType);
        assert(mType != SecondType || !hasTemperatureTrend);
    }

    /**
     * Конструктор для условий III рода.
     * @param temperatures 12 значений температур
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
        , mHasTemperatureTrend(hasTemperatureTrend)
        , mTemperatureTrend(temperatureTrend)
        , mTemperatureTrendStartYear(temperatureTrendStartYear)
        , mCurrentMonth(-1) {
        assert(mParams1.size() == 12);
        assert(mResistivities.size() == 12);
    }

    /**
     * Изменение параметров при переходе в новую дату.
     * @see SoilBlock::moveInTime(), HeatSurface::moveInTime().
     */
    inline void setDate(int year, int month, int day) {
        if (mCurrentMonth == month) {
            return;
        }
        mCurrentMonth = month;
        --month;
        mCurrentParam1 = mParams1.at(month);
        if (mType == ThirdType) {
            mCurrentResistivity = mResistivities.at(month);
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
