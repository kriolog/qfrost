/*
 * Copyright (C) 2012-2015  Denis Pesotsky
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
 * along with this program.  If not, see <http://www.gnmUnit->org/licenses/>
 */

#include "physicalpropertyspinbox.h"

#include <units.h>

#include <QtCore/qmath.h>

#include <limits>

using namespace qfgui;

PhysicalPropertySpinBox::PhysicalPropertySpinBox(PhysicalProperty property,
        QWidget *parent)
    : SmartDoubleSpinBox(parent)
    , mProperty()
    , mNeedSuffix(true)
    , mMinimumIsForced(false)
    , mMaximumIsForced(false)
    , mHelperSpinBox(new SmartDoubleSpinBox(this))
    , mUnit(NULL)
{
    mHelperSpinBox->setDenySteppingToNonMultipleBound(false);
    mHelperSpinBox->hide();

    connect(Units::units(this), SIGNAL(changed()), SLOT(updateUnit()));
    connect(this, SIGNAL(valueChanged(double)), SLOT(updateHelperValue(double)));

    setPhysicalProperty(property);
}

PhysicalPropertySpinBox::PhysicalPropertySpinBox(QWidget *parent)
    : SmartDoubleSpinBox(parent)
    , mProperty()
    , mNeedSuffix(true)
    , mMinimumIsForced(false)
    , mMaximumIsForced(false)
    , mHelperSpinBox(new SmartDoubleSpinBox(this))
    , mUnit(NULL)
{
    mHelperSpinBox->setDenySteppingToNonMultipleBound(false);
    mHelperSpinBox->hide();

    connect(Units::units(this), SIGNAL(changed()), SLOT(updateUnit()));
    connect(this, SIGNAL(valueChanged(double)), SLOT(updateHelperValue(double)));
}

void PhysicalPropertySpinBox::setPhysicalProperty(int p)
{
    PhysicalProperty property = static_cast<PhysicalProperty>(p);
    if (mProperty == property) {
        return;
    }
    Q_ASSERT(property != NoProperty);
    mProperty = property;
    const Unit *newUnit = &(Units::unit(this, mProperty));
    if (newUnit == mUnit) {
        return;
    }
    mUnit = newUnit;
    Q_ASSERT(mUnit != NULL);
    setDecimals(qMax(0, mUnit->decimalsSI()));
    getPropertiesFromUnit();
}

QString PhysicalPropertySpinBox::textFromValue(double val) const
{
    if (mUnit == NULL) {
        return SmartDoubleSpinBox::textFromValue(val);
    }
    return mHelperSpinBox->textFromValue(mUnit->fromSI(val));
}

double PhysicalPropertySpinBox::valueFromText(const QString &text) const
{
    if (mUnit == NULL) {
        Q_ASSERT(false);
        return SmartDoubleSpinBox::valueFromText(text);
    }
    return mUnit->toSI(mHelperSpinBox->valueFromText(text));
}

void PhysicalPropertySpinBox::hideSuffix()
{
    mNeedSuffix = false;
    mHelperSpinBox->setSuffix(QString());
    setSuffix(QString());
}

void PhysicalPropertySpinBox::setForcedMinimum(double min)
{
    mMinimumIsForced = true;
    if (minimum() == min) {
        return;
    }
    setMinimum(min);
    if (mUnit == NULL) {
        return;
    }
    mHelperSpinBox->setMinimum(mUnit->fromSI(min));
}

void PhysicalPropertySpinBox::setForcedMaximum(double max)
{
    mMaximumIsForced = true;
    if (maximum() == max) {
        return;
    }
    setMaximum(max);
    if (mUnit == NULL) {
        return;
    }
    mHelperSpinBox->setMaximum(mUnit->fromSI(max));
}

void PhysicalPropertySpinBox::getPropertiesFromUnit()
{
    if (mUnit == NULL) {
        Q_ASSERT(false);
        return;
    }

    if (mNeedSuffix) {
        setSuffix(mUnit->spinBoxSuffix());
    }

    mHelperSpinBox->setSingleStep(mUnit->singleStepVisible());
    mHelperSpinBox->setSuffix(suffix());
    mHelperSpinBox->setDecimals(mUnit->decimalsVisible());
    if (!mMinimumIsForced) {
        mHelperSpinBox->setMinimum(mUnit->minimumVisible());
        setMinimum(mUnit->minimum());
    } else {
        // На случай, если мы получили минимум до присвоения physicalproperty
        // (или же если единицы измерения изменились и мы обновляаемся)
        mHelperSpinBox->setMinimum(mUnit->fromSI(minimum()));
    }
    if (!mMaximumIsForced) {
        mHelperSpinBox->setMaximum(mUnit->maximumVisible());
        setMaximum(mUnit->maximum());
    } else {
        // На случай, если мы получили максимум до присвоения physicalproperty
        // (или же если единицы измерения изменились и мы обновляаемся)
        mHelperSpinBox->setMaximum(mUnit->fromSI(maximum()));
    }

    // Чтобы изменить размер в соответствии с новым sizeHint
    // FIXME: сделать это получше?
    QString suf = suffix();
    setSuffix("hi");
    setSuffix(suf);
}

QSize PhysicalPropertySpinBox::sizeHint() const
{
    return mHelperSpinBox->sizeHint();
}

QValidator::State PhysicalPropertySpinBox::validate(QString &input, int &pos) const
{
    return mHelperSpinBox->validate(input, pos);
}

void PhysicalPropertySpinBox::stepBy(int steps)
{
    mHelperSpinBox->stepBy(steps);
    Q_ASSERT(mUnit != NULL);
    setValue(mUnit->toSI(mHelperSpinBox->value()));
}

QAbstractSpinBox::StepEnabled PhysicalPropertySpinBox::stepEnabled() const
{
    return mHelperSpinBox->stepEnabled();
}

void PhysicalPropertySpinBox::updateHelperValue(double d)
{
    if (mUnit == NULL) {
        return;
    }
    mHelperSpinBox->setValue(mUnit->fromSI(d));
}

void PhysicalPropertySpinBox::updateUnit()
{
    if (mProperty == NoProperty) {
        return;
    }
    mUnit = &(Units::unit(this, mProperty));
    getPropertiesFromUnit();
}

double PhysicalPropertySpinBox::forcedMaximum() const
{
    if (!mMaximumIsForced) {
        return std::numeric_limits<double>::infinity();
    } else {
        return maximum();
    }
}

double PhysicalPropertySpinBox::forcedMinimum() const
{
    if (!mMinimumIsForced) {
        return -std::numeric_limits<double>::infinity();
    } else {
        return minimum();
    }
}

QDoubleSpinBox *PhysicalPropertySpinBox::createSceneCoordinateSpinBox(QWidget *parent)
{
    QDoubleSpinBox *result = new SmartDoubleSpinBox(parent);

    result->setSuffix(Units::meterSuffix());

    result->setDecimals(QFrost::meterDecimals);
    result->setRange(-QFrost::sceneHalfSizeInMeters, QFrost::sceneHalfSizeInMeters);
    result->setSingleStep(1);

    return result;
}

QDoubleSpinBox *PhysicalPropertySpinBox::createBlockSizeSpinBox(QWidget *parent)
{
    Q_ASSERT(QFrost::meterDecimalsBlockSize <= QFrost::meterDecimals);

    QDoubleSpinBox *result = new SmartDoubleSpinBox(parent);

    result->setSuffix(Units::meterSuffix());

    result->setDecimals(QFrost::meterDecimalsBlockSize);
    result->setRange(0, 10);
    result->setSingleStep(0.01);

    result->setSpecialValueText("\342\210\236");

    result->setValue(0.1); // дефолтные размеры блока - 10x10см

    return result;
}
