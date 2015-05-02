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

#ifndef QFCORE_SOILBLOCK_H
#define QFCORE_SOILBLOCK_H

#include <vector>
#include <cassert>
#include <map>

#include <core/phased.h>

namespace qfgui
{
class SetBlocksTemperatureCommand;
class BlockWithOldThawedPart;
class ChangeSoilPropertyCommand;
class BlockPortable;
class UnfrozenWaterWidget;
class SoilsModelBase;
class Soil;
}

namespace qfcore
{

static double interpolate(const std::map<double, double> &data, double x);

class HeatSurface;

/// Удельная теплота фазового перехода вода-лёд в Дж/кг
static const double WaterTransitionHeat = 335.105 * 1000;

/**
 * Класс для описания грунтового блока.
 *
 * Содержит в себе все теплофизические характеристики, умеет переходить
 * на новый временной слой после рассчёта теплопотоков прилежащих площадок
 * теплопотока.
 */
class SoilBlock
{
private:
    /// Массив размеров блока (\f$ r \f$). Элементы соответствуют осям x, y и z
    std::vector<double> mDimensions;

    /// Шаг по времени, делённый на объём блока (\f$ \tau/\nu \f$)
    double mTimeStepPerVolume;

    /// Энтальпия (\f$ H \f$)
    double mEnthalpy;

    /// Температура (\f$ T \f$)
    double mTemperature;

    /// Относительный объём талой фазы (\f$ V \f$)
    double mThawedPart;

    /// Теплоёмкость талого и мёрзлого грунта (\f$ C_{th} \f$ и \f$ CF \f$)
    Phased mCapacity;

    /** Теплопроводность талого и мёрзлого грунта
        (\f$ \lambda_{th} \f$ и \f$ \lambdaF \f$) */
    Phased mConductivity;

    /** Эффективная теплопроводность в минус первой степени \f$ 1/\lambda \f$.
        @see calcCondition() */
    double mIeConductivity;

    /// Температура фазового перехода (\f$ T_{bf} \f$)
    double mTransitionTemperature;

    /// Объёмная теплота фазовых переходов (\f$ L_\nu \f$)
    double mTransitionHeat;

    /// Использует ли кривую незамёзршей воды
    bool mUsesUnfrozenWaterCurve;
    /// Кривая незамёрзшей воды (от температуры)
    std::map<double, double> mUnfrozenWaterCurve;
    /// Влажность за счёт льда и незамёрзшей воды (\f$ W_{tot} \f$)
    double mMoistureTotal;
    /// Плотность сухого грунта (\f$ \rhoD \f$)
    double mDryDensity;
    /// Объёмная теплота фазовых переходов за вычетом той теплоты, которая
    /// заложена в кривую незамёзршей воды
    double mAdditionalTransitionHeat;

    /// Минимальная влажность за счёт незамёзршей воды при T=Tbf (расчётная)
    double mMinBfMoisture;
    /// Кривая температуры от энтальпии (расчётная)
    std::map<double, double> mTemperatureCurve;

    /// Плотность внутренних источников тепла на единицу объёма
    double mInternalHeatSourcePowerDensity;

    /// Количество тепла от внутренних источников за один шаг по времени
    double mInternalHeatPerStep;

    /**
     * Величина теплового потока, который подействует на блок при очередном
     * шаге во времени. Прилегающие поверхности теплопотока скидывают сюда
     * соответствующие значение при каждом шаге во времени.
     */
    double mHeatDiff;

    /**
     * Расчёт температуры и относительного объёма талой фазы из энтальпии
     * (с учётом температуры фазового перехода).
     */
    void calcCondition();

    /**
     * Расчёт энтальпии из температуры и относительного объёма талой фазы
     * (с учётом температуры фазового перехода).
     */
    void calcEnthalpy();

    /**
     * Пересчитывает эффективную теплопроводность в минус первой степени из
     * значений теплопроводности талой и мёрзлой фазы и относительного объёма
     * талой фазы (с учётом температуры фазового перехода).
     */
    void calcIEConductivity();
    /**
     * Делает кол-во незамёзршей воды равным 0.0 или 1.0, если температура
     * не равна температуре фазового перехода.
     */
    void normalizeThawedPart();

    /// Энтальпия для температуры @p T и влажности @p w (формула для T < Tbf)
    inline double enthalpyForFreezing(double T, double w) {
        assert(mUsesUnfrozenWaterCurve);
        assert(T <= mTransitionTemperature);
        return mCapacity.frozen * (T - mTransitionTemperature) +
               WaterTransitionHeat * mDryDensity * (w - mMinBfMoisture);
    }

    /// Расчётный (из кривой н.в.) относительный объём талой воды.
    /// @warning вызывать только при использовании кривой незамёзршей воды!
    inline double curThawedPart() const {
        assert(mUsesUnfrozenWaterCurve);
        return interpolate(mUnfrozenWaterCurve, mTemperature) / mMoistureTotal;
    }

    friend class qfgui::SetBlocksTemperatureCommand;
    friend class qfgui::BlockWithOldThawedPart;
    friend class qfgui::ChangeSoilPropertyCommand;
    friend class qfgui::BlockPortable;
    friend class qfgui::UnfrozenWaterWidget;
    friend class qfgui::SoilsModelBase;
    friend class qfgui::Soil;
public:
    /**
     * В результате работы этого конструктора, температура и относительный
     * объём талой фазы равны нулю, а прочие свойства -- образцовым значениям.
     */
    SoilBlock(const double &width = 1, const double &height = 1);

    /// Даёт нам размер по z для осесимметричной задачи.
    /// @param r расстояние до оси Y
    void setZForAxiallySymmetricProblem(double r);

    /**
     * Функция, которую надо вызывать при изменении шага по времени.
     * Пересчитывает значения @a mTimeStepPerVolume и @a mInternalHeatPerStep.
     * @param inTimeStep новый шаг по времени
     */
    void setTimeStep(double inTimeStep);

    /**
     * Расчёт новых параметров для следующего слоя времени.
     * Вызывается после проведения рассчётов в прилежащих площадках теплопотока.
     * \see HeatSurface::moveInTime(), BoundaryCondition::moveInTime()
     */
    void moveInTime();

    inline double temperature() const {
        return mTemperature;
    }
    inline const Phased &conductivity() const {
        return mConductivity;
    }
    inline const Phased &capacity() const {
        return mCapacity;
    }
    inline double transitionTemperature() const {
        return mTransitionTemperature;
    }
    inline double transitionHeat() const {
        return mTransitionHeat;
    }
    inline double thawedPart() const {
        return mThawedPart;
    }

    /**
     * Возвращает значение размера по заданной оси.
     * @param inAxe номер оси
     */
    double dimension(short unsigned int inAxe) const {
        return mDimensions.at(inAxe);
    }

    /**
     * Возвращает эффективную теплопроводность в минус первой степени
     */
    double invertedEffectiveConductivity() const {
        return mIeConductivity;
    }

    /**
     * Добавляет @p incomingHeat к @a mHeatDiff.
     */
    void addHeat(double incomingHeat) {
        mHeatDiff += incomingHeat;
    }

    /**
     * Считает энтальпию и эффективную теплопроводность и берёт теплопроводость
     * из @p other. Должна запускаться один раз перед началом рассчётов.
     */
    void prepareForComputation(const SoilBlock &other);

    /**************************************************************************
     * Почти все нижеперечисленные функции не заботятся об энтальпии и об     *
     * эффективной теплопроводности!                                          *
     * Они предназначены для GUI и не должны использоваться при рассчётах!    *
     * TODO: лучше сделать соотв. функции friend                              */
    void setCapacityFr(double value) {
        mCapacity.frozen = value;
    }
    void setCapacityTh(double value) {
        mCapacity.thawed = value;
    }
    void setConductivityFr(double value) {
        mConductivity.frozen = value;
    }
    void setConductivityTh(double value) {
        mConductivity.thawed = value;
    }
    void setTransitionHeat(double value) {
        mTransitionHeat = value;
    }
    void setTransitionTemperature(double value) {
        mTransitionTemperature = value;
    }

    /// Устанавливает температуру в @p value.
    /// @p mustNormalizeThawedPart проверять ли кол-во незамёрзшей воды
    void setTemperature(double value, bool mustNormalizeThawedPart = true);

    /**
     * Устанавливает кол-во незамёрзшей воды в @p value.
     * @warning @p valueдолжно быть от 0.0 до 1.0
     */
    void setThawedPart(double value);

    /**
     * Выключает использование кривой незамёрзшей воды, устанавливает
     * температуру фазового перехода в 0 и делает normalizeThawedPart()
     */
    void resetTransitionParameters();

    /**
     * Берёт все параметры, кроме теплопроводности, из @p other.
     * Теплопроводность возьмётся в prepareForComputation()
     * @param mustNormalizeThawedPart нужно ли нормализовать относительный объём
     *                                талой фазы (это надо делать всегда, кроме
     *                                если она не устанавливается отдельно)
     */
    void setAllProperties(const SoilBlock &other, bool mustNormalizeThawedPart = true);

    /// Соответствует ли @a mThawedPart текущей @a mTemperature.
    bool thawedPartIsOk() const;

    /// Находится ли блок в состоянии фазового перехода.
    inline bool isAtPhaseTransition() const {
        assert(thawedPartIsOk());
        return (mUsesUnfrozenWaterCurve && mTemperature <= mTransitionTemperature)
               || mTemperature == mTransitionTemperature;
    }

    /// Можно ли менять кол-во незамёрзшей воды в блоке
    inline bool canChangeThawedPart() const {
        return mTemperature == mTransitionTemperature;
    }

    /// Минимальное кол-во незамёрзшей воды при T = Tbf
    inline double minBFThawedPart() const {
        assert(mUsesUnfrozenWaterCurve);
        return mMinBfMoisture / mMoistureTotal;
    }

    bool usesUnfrozenWaterCurve() const {
        return mUsesUnfrozenWaterCurve;
    }

    const std::map<double, double> &unfrozenWaterCurve() const {
        return mUnfrozenWaterCurve;
    }

    double moistureTotal() const {
        return mMoistureTotal;
    }
    double dryDensity() const {
        return mDryDensity;
    }

    double internalHeatSourcePowerDensity() const {
        return mInternalHeatSourcePowerDensity;
    }

    void updateFromWaterCurve();
    void getTransitionTemperatureFromWaterCurve();
    /********************** Конец функций для GUI ****************************/
};

}

double qfcore::interpolate(const std::map< double, double > &data, double x)
{
    if (data.empty()) {
        return 0.0;
    } else {
        typedef std::map<double, double>::const_iterator IT;

        IT i = data.upper_bound(x);

        if (i == data.end()) {
            return (--i)->second;
        }
        if (i == data.begin()) {
            return i->second;
        }

        IT l = i;
        --l;

        const double delta = (x - l->first) / (i->first - l->first);
        return delta * i->second + (1 - delta) * l->second;
    }
}

#endif // QFCORE_SOILBLOCK_H
