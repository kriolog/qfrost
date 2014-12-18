/*
 * Copyright (C) 2010-2012  Denis Pesotsky
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

#include "soil.h"

#include <QtCore/QLocale>
#include <boundary_conditions/boundarycondition.h>
#include <graphicsviews/block.h>

using namespace qfgui;

Soil::Soil(const QString &name, const QColor &color)
    : Item(name, color)
    , mBlock()
    , mBlocks()
    , mOldMoistureTotalMinimum(moistureTotalMinimum())
    , mOldMoistureTotalMaximum(moistureTotalMaximum())
{
}

Soil::Soil(const Item *other,
           const QString &name,
           const QColor &color)
    : Item(name, color)
    , mBlock(static_cast<const Soil *>(other)->mBlock)
    , mBlocks()
    , mOldMoistureTotalMinimum(static_cast<const Soil *>(other)->mOldMoistureTotalMinimum)
    , mOldMoistureTotalMaximum(static_cast<const Soil *>(other)->mOldMoistureTotalMaximum)
{

}

Soil::Soil(const Item *other)
    : Item(other->name(), other->color())
    , mBlock()
    , mBlocks()
{
    const Soil *otherSoil = qobject_cast<const Soil * >(other);
    Q_ASSERT(otherSoil != NULL);
    mBlock = otherSoil->mBlock;
    mOldMoistureTotalMinimum = otherSoil->mOldMoistureTotalMinimum;
    mOldMoistureTotalMaximum = otherSoil->mOldMoistureTotalMaximum;
}

Soil::Soil()
    : Item()
    , mBlock()
    , mBlocks()
    , mOldMoistureTotalMinimum()
    , mOldMoistureTotalMaximum()
{

}

void Soil::setConductivityThawed(double d)
{
    if (conductivityThawed() != d) {
        mBlock.mConductivity.thawed = d;
        emit conductivityThawedChanged(conductivityThawed());
    }
}

void Soil::setConductivityFrozen(double d)
{
    if (conductivityFrozen() != d) {
        mBlock.mConductivity.frozen = d;
        emit conductivityFrozenChanged(conductivityFrozen());
    }
}

void Soil::setCapacityThawed(double d)
{
    if (capacityThawed() != d) {
        mBlock.mCapacity.thawed = d;
        emit capacityThawedChanged(capacityThawed());
    }
}

void Soil::setCapacityFrozen(double d)
{
    if (capacityFrozen() != d) {
        mBlock.mCapacity.frozen = d;
        emit capacityFrozenChanged(capacityFrozen());
    }
}

void Soil::setTransitionTemperature(double d)
{
    Q_ASSERT(!usesUnfrozenWaterCurve());
    if (transitionTemperature() != d) {
        mBlock.mTransitionTemperature = d;
        emit transitionTemperatureChanged(transitionTemperature());
    }
}

void Soil::setTransitionHeat(double d)
{
    Q_ASSERT(!usesUnfrozenWaterCurve());
    if (transitionHeat() != d) {
        mBlock.mTransitionHeat = d;
        emit transitionHeatChanged(transitionHeat());
    }
}
void Soil::setUsesUnfrozenWaterCurve(bool b)
{
    if (usesUnfrozenWaterCurve() != b) {
        mBlock.mUsesUnfrozenWaterCurve = b;
        emit usesUnfrozenWaterCurveChanged(usesUnfrozenWaterCurve());
        if (b) {
            updateFromWaterCurve();
        }
    }
}

void Soil::setUnfrozenWaterCurve(const DoubleMap &m)
{
    std::map<double, double> newCurve = m.toStdMap();
    if (mBlock.mUnfrozenWaterCurve != newCurve) {
        mOldMoistureTotalMinimum = moistureTotalMinimum();
        mOldMoistureTotalMaximum = moistureTotalMaximum();
        mBlock.mUnfrozenWaterCurve = newCurve;
        emit unfrozenWaterCurveChanged(unfrozenWaterCurve());
        updateFromWaterCurve();
    }
}

void Soil::setMoistureTotal(double d)
{
    if (moistureTotal() != d) {
        mBlock.mMoistureTotal = d;
        emit moistureTotalChanged(moistureTotal());
        updateFromWaterCurve();
    }
}

void Soil::setDryDensity(double d)
{
    if (dryDensity() != d) {
        mBlock.mDryDensity = d;
        emit dryDensityChanged(dryDensity());
        updateFromWaterCurve();
    }
}

void Soil::setInternalHeatSourcePowerDensity(double d)
{
    if (internalHeatSourcePowerDensity() != d) {
        mBlock.mInternalHeatSourcePowerDensity = d;
        emit internalHeatSourcePowerDensityChanged(internalHeatSourcePowerDensity());
    }
}

void Soil::updateFromWaterCurve()
{
    if (usesUnfrozenWaterCurve()) {
        const double minW = moistureTotalMinimum();
        const double maxW = moistureTotalMaximum();
        Q_ASSERT(minW < maxW);
        const double oldW = mBlock.mMoistureTotal;
        mBlock.mMoistureTotal = qBound(minW, oldW, maxW);
        if (minW != mOldMoistureTotalMinimum) {
            emit moistureTotalMinimumChanged(minW);
        }
        if (maxW != mOldMoistureTotalMaximum) {
            emit moistureTotalMaximumChanged(maxW);
        }
        if (moistureTotal() != oldW) {
            emit moistureTotalChanged(moistureTotal());
        }
        const double oldT = transitionTemperature();
        const double oldQ = transitionHeat();
        mBlock.updateFromWaterCurve();
        if (transitionTemperature() != oldT) {
            emit transitionTemperatureChanged(transitionTemperature());
        }
        if (transitionHeat() != oldQ) {
            emit transitionHeatChanged(transitionHeat());
        }
    }
}

double Soil::moistureTotalMinimum() const
{
    std::map<double, double>::const_iterator it;
    it = mBlock.mUnfrozenWaterCurve.begin();
    return it->second;
}

double Soil::moistureTotalMaximum() const
{
    std::map<double, double>::const_iterator it;
    it = mBlock.mUnfrozenWaterCurve.end();
    return (--it)->second;
}

QList< QString > Soil::priorityProperties()
{
    QList<QString> result = Item::priorityProperties();
    result << "usesUnfrozenWaterCurve";
    return result;
}

QString Soil::shortPropertyNameGenetive(const QString &propertyName)
{
    if (propertyName == "conductivityThawed") {
        return tr("\316\273th");
    } else if (propertyName == "conductivityFrozen") {
        return tr("\316\273fr");
    } else if (propertyName == "capacityThawed") {
        return tr("Cth");
    } else if (propertyName == "capacityFrozen") {
        return tr("Cfr");
    } else if (propertyName == "transitionTemperature") {
        return tr("Tbf");
    } else if (propertyName == "transitionHeat") {
        return tr("Qph");
    } else if (propertyName == "usesUnfrozenWaterCurve") {
        //: In genetive case. There are 2 types: [not]uses unfrozen water curve.
        return tr("phase transition type", "genetive");
    } else if (propertyName == "unfrozenWaterCurve") {
        //: In genetive case
        return tr("unfrozen water curve", "genetive");
    } else if (propertyName == "moistureTotal") {
        return tr("w_tot");
    } else if (propertyName == "dryDensity") {
        return tr("\317\201d");
    } else if (propertyName == "internalHeatSourcePowerDensity") {
        return tr("F");
    } else {
        return Item::shortPropertyNameGenetive(propertyName);
    }
}
