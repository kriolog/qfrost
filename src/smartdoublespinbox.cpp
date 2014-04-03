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

#include "smartdoublespinbox.h"

#include <QtCore/qmath.h>

using namespace qfgui;

SmartDoubleSpinBox::SmartDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
    , mDenyStepingToNonmultipleBounds(false)
{
}

bool SmartDoubleSpinBox::isSingleStepMultiple(double v) const
{
    double stepsCount = v / singleStep();
    return qAbs((stepsCount - qRound(stepsCount)) * singleStep()) < 0.001;
}

void SmartDoubleSpinBox::stepBy(int steps)
{
    double roundedValue;
    if (!isSingleStepMultiple(value())) {
        if (steps < 0) {
            roundedValue = static_cast<double>(qCeil(value() / singleStep())) * singleStep();
        } else {
            roundedValue = static_cast<double>(qFloor(value() / singleStep())) * singleStep();
        }
    } else {
        roundedValue = value();
    }

    double newValue = roundedValue + static_cast<double>(steps) * singleStep();

    if (mDenyStepingToNonmultipleBounds) {
        if (newValue < minimum() && !isSingleStepMultiple(minimum())) {
            setValue(minimum());
            stepBy(1);
            return;
        } else if (newValue > maximum() && !isSingleStepMultiple(maximum())) {
            setValue(maximum());
            stepBy(-1);
            return;
        }
    }

    setValue(newValue);
}

void SmartDoubleSpinBox::readProperties(QDoubleSpinBox *other)
{
    setRange(other->minimum(), other->maximum());
    setValue(other->value());
    setSingleStep(other->singleStep());
    setSuffix(other->suffix());
    setDecimals(other->decimals());
    setSpecialValueText(other->specialValueText());
}
