/*
 * Copyright (C) 2010-2014  Denis Pesotsky, Maxim Torgonsky
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

#include <core/soilblock.h>

#include <cassert>
#include <cmath>

#include <boost/math/constants/constants.hpp>

#include <core/heatsurface.h>

using namespace qfcore;

SoilBlock::SoilBlock(const double &width, const double &height):
    mDimensions(0),
    mTimeStepPerVolume(0),
    mEnthalpy(0),
    mTemperature(0),
    mThawedPart(0),
    mCapacity(Phased(2800000, 2150000)),
    mConductivity(Phased(1.5, 1.59)),
    mIeConductivity(0),
    mTransitionTemperature(0),
    mTransitionHeat(107400000),
    mUsesUnfrozenWaterCurve(false),
    mUnfrozenWaterCurve(),
    mMoistureTotal(0.3),
    mDryDensity(1400),
    mTemperatureCurve(),
    mHeatDiff(0),
    mAdditionalTransitionHeat(0),
    mMinBfMoisture(0),
    mInternalHeatSourcePowerDensity(0)
{
    mDimensions.push_back(width);
    mDimensions.push_back(height);

    mCapacity = Phased(1206870, 909320);
    mConductivity = Phased(1.31, 1.38);
    mMoistureTotal = 0.186;
    mDryDensity = 1550;
    mUnfrozenWaterCurve[-0.5]  = 18.6 / 100.0;
    mUnfrozenWaterCurve[-2.1]  = 8.4 / 100.0;
    mUnfrozenWaterCurve[-6.2]  = 3.3 / 100.0;
    mUnfrozenWaterCurve[-20.5] = 1.8 / 100.0;

    calcIEConductivity();
    calcEnthalpy();
}

/**
 * Используются следующие формулы:
 * - \f$ T=1/CF+T_{bf}, V=0 \f$ при \f$ I<0 \f$
 * - \f$ T=T_{bf}, V=1/L_\nu \f$ при \f$ 0 \leq I \leq L_\nu \f$
 * - \f$ T=(I-L_\nu)/C_{th}+T_{bf}, V=1 \f$ при \f$ I > L_\nu \f$
 */
void SoilBlock::calcCondition()
{
    if (!mUsesUnfrozenWaterCurve) {
        if (mEnthalpy <= 0) {
            mTemperature = mEnthalpy / mCapacity.frozen + mTransitionTemperature;
            mThawedPart = 0;
        } else if (mEnthalpy >= mTransitionHeat) {
            mTemperature = (mEnthalpy - mTransitionHeat) / mCapacity.thawed +
                           mTransitionTemperature;
            mThawedPart = 1;
        } else { /* 0 < I < LV */
            mTemperature = mTransitionTemperature;
            // обратите внимание на случай, когда теплота фазового перехода == 0
            // в таком случае, если бы мы сюда попали, то получили бы NaN или INF
            // но ведь мы попали в одно из предыдущих ветвлений, так что всё ок
            mThawedPart = mEnthalpy / mTransitionHeat;
        }
    } else {
        if (mEnthalpy < 0) {
            mTemperature = interpolate(mTemperatureCurve, mEnthalpy);
            mThawedPart = ((mEnthalpy - mCapacity.frozen * (mTemperature - mTransitionTemperature))
                           / (WaterTransitionHeat * mDryDensity) + mMinBfMoisture) / mMoistureTotal;
        } else if (mEnthalpy >= mAdditionalTransitionHeat) {
            mTemperature = (mEnthalpy - mAdditionalTransitionHeat) / mCapacity.thawed +
                           mTransitionTemperature;
            mThawedPart = 1;
        } else { /* 0 < I < LV */
            mTemperature = mTransitionTemperature;
            mThawedPart = (mEnthalpy / (WaterTransitionHeat * mDryDensity) + mMinBfMoisture) / mMoistureTotal;
        }
    }
}

/**
 * Используются следующие формулы:
 * - \f$ I=CF(T-T_{bf}) \f$ при \f$ T < T_{bf} \f$
 * - \f$ I=L_\nu V\f$ при \f$ T = T_{bf} \f$
 * - \f$ I=L_\nu + C_{th}(T-T_{bf}) \f$ при \f$ T > T{bf} \f$
 */
void SoilBlock::calcEnthalpy()
{
    if (!mUsesUnfrozenWaterCurve) {
        if (mTemperature < mTransitionTemperature) {
            mEnthalpy = mCapacity.frozen * (mTemperature - mTransitionTemperature);
        } else if (mTemperature > mTransitionTemperature) {
            mEnthalpy = mTransitionHeat + mCapacity.thawed *
                        (mTemperature - mTransitionTemperature);
        } else { /* T = Tbf */
            mEnthalpy = mTransitionHeat * mThawedPart;
        }
    } else {
        if (mTemperature < mTransitionTemperature) {
            mEnthalpy = enthalpyForFreezing(mTemperature,
                                            interpolate(mUnfrozenWaterCurve, mTemperature));
        } else if (mTemperature > mTransitionTemperature) {
            mEnthalpy = mAdditionalTransitionHeat + mCapacity.thawed *
                        (mTemperature - mTransitionTemperature);
        } else {
            mEnthalpy = WaterTransitionHeat * mDryDensity
                        * (mThawedPart * mMoistureTotal - mMinBfMoisture);
        }
    }
}

void SoilBlock::setZForAxiallySymmetricProblem(double r)
{
    assert(mDimensions.size() == 2);
    mDimensions.push_back(2.0 * boost::math::constants::pi<double>() * r);
}

void SoilBlock::setTimeStep(double inTimeStep)
{
    /* Шаг по времени не хранится напрямую: используется
     * только отношение шага по времени к объёму.
     * Это нужно для того, чтобы не считать этот параметр
     * каждый раз. Сам же шаг по времени после этого не нужен. */
    mTimeStepPerVolume = inTimeStep;
    for (std::size_t  i = 0; i < mDimensions.size(); ++i) {
        mTimeStepPerVolume /= mDimensions.at(i);
    }
}

void SoilBlock::moveInTime()
{
    mEnthalpy += ((mHeatDiff + mInternalHeatSourcePowerDensity) * mTimeStepPerVolume);
    mHeatDiff = 0.0;
    calcCondition();
    calcIEConductivity();
}

/**
 * \f$ 1/\lambda = V/\lambda_{th} + (1-V)/\lambdaF \f$.
 */
void SoilBlock::calcIEConductivity()
{
    if (mThawedPart == 0) {
        mIeConductivity = 1 / mConductivity.frozen;
    } else if (mThawedPart == 1) {
        mIeConductivity = 1 / mConductivity.thawed;
    } else {
        mIeConductivity = mThawedPart / mConductivity.thawed +
                          (1 - mThawedPart) / mConductivity.frozen;
    }
}

void SoilBlock::setTemperature(double value, bool mustNormalizeThawedPart)
{
    mTemperature = value;
    if (mustNormalizeThawedPart) {
        normalizeThawedPart();
    }
}

void SoilBlock::setThawedPart(double value)
{
    assert(value >= 0.0 && value <= 1.0);
    mThawedPart = value;
    assert(thawedPartIsOk());
}

void SoilBlock::setAllProperties(const SoilBlock &other,
                                 bool mustNormalizeThawedPart)
{
    mCapacity = other.mCapacity;
    mTransitionHeat = other.mTransitionHeat;
    mTransitionTemperature = other.mTransitionTemperature;
    mUsesUnfrozenWaterCurve = other.mUsesUnfrozenWaterCurve;
    mUnfrozenWaterCurve = other.mUnfrozenWaterCurve;
    mTemperatureCurve = other.mTemperatureCurve;
    mDryDensity = other.mDryDensity;
    mMoistureTotal = other.mMoistureTotal;
    mMinBfMoisture = other.mMinBfMoisture;
    mAdditionalTransitionHeat = other.mAdditionalTransitionHeat;
    mInternalHeatSourcePowerDensity = other.mInternalHeatSourcePowerDensity;
    if (mustNormalizeThawedPart) {
        normalizeThawedPart();
    } else {
        assert(thawedPartIsOk());
    }
}

void SoilBlock::resetTransitionParameters()
{
    mUsesUnfrozenWaterCurve = false;
    mTransitionTemperature = 0.0;
    normalizeThawedPart();
}

void SoilBlock::normalizeThawedPart()
{
    assert(mThawedPart >= 0.0 && mThawedPart <= 1.0);
    if (mTemperature > mTransitionTemperature) {
        mThawedPart = 1.0;
    } else if (mTemperature < mTransitionTemperature) {
        mThawedPart = mUsesUnfrozenWaterCurve
                      ? curThawedPart()
                      : 0.0;
    }
    assert(thawedPartIsOk());
}

bool SoilBlock::thawedPartIsOk() const
{
    return true;
    if (mTemperature > mTransitionTemperature) {
        return mThawedPart == 1;
    }
    if (mUsesUnfrozenWaterCurve) {
        if (mTemperature < mTransitionTemperature) {
            return std::abs(mThawedPart - curThawedPart()) < 0.0001;
        } else {
            return mThawedPart >= minBFThawedPart() && mThawedPart <= 1;
        }
    } else {
        if (mTemperature < mTransitionTemperature) {
            return mThawedPart == 0;
        } else {
            return mThawedPart >= 0 && mThawedPart <= 1;
        }
    }
}

void SoilBlock::prepareForComputation(const SoilBlock &other)
{
    if (!mUsesUnfrozenWaterCurve) {
        setAllProperties(other);
    }
    mConductivity = other.mConductivity;

    assert(mConductivity == other.mConductivity);
    assert(mCapacity == other.mCapacity);
    assert(mTransitionHeat == other.mTransitionHeat);
    assert(mTransitionTemperature == other.mTransitionTemperature);
    assert(mAdditionalTransitionHeat == other.mAdditionalTransitionHeat);
    assert(mDryDensity == other.mDryDensity);
    assert(mMinBfMoisture == other.mMinBfMoisture);
    assert(mMoistureTotal == other.mMoistureTotal);
    assert(mUsesUnfrozenWaterCurve == other.mUsesUnfrozenWaterCurve);
    assert(mUnfrozenWaterCurve == other.mUnfrozenWaterCurve);
    assert(mTemperatureCurve == other.mTemperatureCurve);
    assert(mInternalHeatSourcePowerDensity == other.mInternalHeatSourcePowerDensity);

    calcEnthalpy();
    calcIEConductivity();
}

void SoilBlock::updateFromWaterCurve()
{
    if (!mUsesUnfrozenWaterCurve) {
        return;
    }
    std::map<double, double>::const_iterator i = mUnfrozenWaterCurve.end();
    --i;

    /*mTransitionTemperature = i->first;
    mMinBfMoisture = i->second;
    assert(mMoistureTotal >= mMinBfMoisture);
    mAdditionalTransitionHeat = waterTransitionHeat * mDryDensity * (mMoistureTotal - mMinBfMoisture);*/

    mTransitionHeat = WaterTransitionHeat * mDryDensity * mMoistureTotal;

    mMinBfMoisture = mMoistureTotal;
    mAdditionalTransitionHeat = 0.0;

    getTransitionTemperatureFromWaterCurve();
    mTemperatureCurve.clear();

    static const double tBound = -1000;
    double IBound = enthalpyForFreezing(tBound, i->second);

    mTemperatureCurve[IBound] = tBound;

    for (i = mUnfrozenWaterCurve.begin(); i->first < mTransitionTemperature; ++i) {
        double I = enthalpyForFreezing(i->first, i->second);
        mTemperatureCurve[I] = i->first;
    }
    mTemperatureCurve[enthalpyForFreezing(mTransitionTemperature, mMinBfMoisture)] = mTransitionTemperature;
}

void SoilBlock::getTransitionTemperatureFromWaterCurve()
{
    assert(mUsesUnfrozenWaterCurve);
    assert(!mUnfrozenWaterCurve.empty());
    std::map<double, double> invertedUnfrozenWaterCurve;
    std::map<double, double>::const_reverse_iterator j = mUnfrozenWaterCurve.rbegin();
    assert(j->second >= mMoistureTotal);
    for (; j != mUnfrozenWaterCurve.rend(); ++j) {
        invertedUnfrozenWaterCurve[j->second] = j->first;
    }
    mTransitionTemperature = interpolate(invertedUnfrozenWaterCurve, mMinBfMoisture);
}
