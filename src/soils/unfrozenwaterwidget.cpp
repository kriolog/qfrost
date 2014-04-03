/*
 * Copyright (C) 2012  Denis Pesotsky
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

#include "unfrozenwaterwidget.h"

#include <QtWidgets/QFormLayout>
#include <QtWidgets/QDataWidgetMapper>
#include <QtWidgets/QVBoxLayout>

#include <physicalpropertyspinbox.h>
#include <soils/sortedpointswidget.h>
#include <soils/soilsmodel.h>
#include <soils/soil.h>
#include <core/soilblock.h>

using namespace qfgui;

UnfrozenWaterWidget::UnfrozenWaterWidget(QWidget *parent)
    : QGroupBox(tr("Unfrozen Water Curve"), parent)
    , mUnfrozenWaterCurve(new SortedPointsWidget(tr("T"), tr("W_w"),
                          Temperature,
                          Moisture,
                          this))
    , mMoistureTotal(new PhysicalPropertySpinBox(this))
    , mDryDensity(new PhysicalPropertySpinBox(this))
{
    QFormLayout *boxLayout = new QFormLayout(this);
    setCheckable(true);
    setChecked(false);

    boxLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    boxLayout->addRow(mUnfrozenWaterCurve);
    boxLayout->addRow(tr("H&umidity Total w<sub>tot</sub>"),
                      mMoistureTotal);
    boxLayout->addRow(tr("D&ry Density \317\201<sub>d</sub>"),
                      mDryDensity);

    connect(this, SIGNAL(toggled(bool)), SIGNAL(unfrozenWaterUsageToggled(bool)));
}

void UnfrozenWaterWidget::setMapper(QDataWidgetMapper *mapper)
{
    mapper->addMapping(this, SM_UsesUnfrozenWaterCurve, "checked");
    mapper->addMapping(mUnfrozenWaterCurve, SM_UnfrozenWaterCurve);
    mapper->addMapping(mMoistureTotal, SM_MoistureTotal);
    mapper->addMapping(mDryDensity, SM_DryDensity);
}

void UnfrozenWaterWidget::connectTo(Soil *soil)
{
    connect(this, SIGNAL(unfrozenWaterUsageToggled(bool)),
            soil, SLOT(setUsesUnfrozenWaterCurve(bool)));
    connect(mUnfrozenWaterCurve, SIGNAL(valuesChanged(DoubleMap)),
            soil, SLOT(setUnfrozenWaterCurve(DoubleMap)));
    connect(mMoistureTotal, SIGNAL(valueChanged(double)),
            soil, SLOT(setMoistureTotal(double)));
    connect(mDryDensity, SIGNAL(valueChanged(double)),
            soil, SLOT(setDryDensity(double)));

    connect(soil, SIGNAL(moistureTotalMinimumChanged(double)),
            mMoistureTotal, SLOT(setForcedMinimum(double)));
    connect(soil, SIGNAL(moistureTotalMaximumChanged(double)),
            mMoistureTotal, SLOT(setForcedMaximum(double)));
}
