/*
 * Copyright (C) 2010-2012  Denis Pesotsky, Maxim Torgonsky
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

#include <core/heatsurface.h>

#include <algorithm>
#include <cmath>
#include <limits>

#include <core/soilblock.h>
#include <core/boundarycondition.h>

using namespace qfcore;

HeatSurface::HeatSurface(unsigned short axe, double square,
                         SoilBlock *block1, SoilBlock *block2,
                         bool isAxiallySymmetric)
    : mSquare(square)
    , mHasBoundaryCondition(false)
    , mBoundaryCondition(NULL)
    , mSoilBlock1(block1)
    , mSoilBlock2(block2)
    , mAxe(axe)
    , mR()
    , mR1(block1->dimension(axe) / 2.0)
    , mR2(block2->dimension(axe) / 2.0)
    , mIsAxiallySymmetric(isAxiallySymmetric)
{ }

HeatSurface::HeatSurface(double r, double square,
                         SoilBlock *block, const BoundaryCondition *condition)
    : mSquare(square)
    , mHasBoundaryCondition(true)
    , mBoundaryCondition(condition)
    , mSoilBlock1(block)
    , mSoilBlock2(NULL)
    , mAxe()
    , mR(r)
    , mR1()
    , mR2()
    , mIsAxiallySymmetric(false)
{ }

void HeatSurface::moveInTime() const
{
    double h;

    // Рассчитываем плотность теплопотока.
    if (mHasBoundaryCondition) {
        switch (mBoundaryCondition->type()) {
        case BoundaryCondition::FirstType:
            h = (mBoundaryCondition->temperature() - mSoilBlock1->temperature())
                / (mR * mSoilBlock1->invertedEffectiveConductivity());
            break;
        case BoundaryCondition::SecondType:
            h = mBoundaryCondition->heatFlowDensity();
            break;
        case BoundaryCondition::ThirdType:
            h = (mBoundaryCondition->temperature() - mSoilBlock1->temperature())
                / (mR * mSoilBlock1->invertedEffectiveConductivity()
                   + mBoundaryCondition->resistivity());
            break;
        default:
            // не должны сюда попасть
            assert(false);
            h = 0.0; // а это чтоб компилятор не ругался
        }
    } else {
        h = (mSoilBlock2->temperature() - mSoilBlock1->temperature())
            / (mR1 * mSoilBlock1->invertedEffectiveConductivity()
               + mR2 * mSoilBlock2->invertedEffectiveConductivity());
    }

    // Переводим плотность теплопотока в теплопоток
    h *= mSquare;
    // и передаём его в ячейки
    mSoilBlock1->addHeat(h);
    if (!mHasBoundaryCondition) {
        mSoilBlock2->addHeat(-h);
    }
}

double HeatSurface::maximalStep(bool is_1d) const
{
    if (mHasBoundaryCondition) {
        return std::numeric_limits<double>::infinity();
    }
    // Температуропроводность = \lambda / C
    double a = mSoilBlock1->conductivity().thawed
               / mSoilBlock1->capacity().thawed;
    a = std::max(mSoilBlock1->conductivity().frozen
                 / mSoilBlock1->capacity().frozen, a);
    a = std::max(mSoilBlock2->conductivity().thawed
                 / mSoilBlock2->capacity().thawed, a);
    a = std::max(mSoilBlock2->conductivity().frozen
                 / mSoilBlock2->capacity().frozen, a);

    // Квадрат шага сетки
    double h2;

    if (is_1d) {
        // h*h для одномерной задачи
        h2 = std::min(mSoilBlock1->dimension(mAxe), mSoilBlock2->dimension(mAxe));
        h2 *= h2;
    } else {
        // 1/(1/h1^2 + 1/h2^2) для двухмерной задачи
        h2 = 1.0 / (1.0 / (mSoilBlock1->dimension(0) * mSoilBlock1->dimension(0)) +
                    1.0 / (mSoilBlock1->dimension(1) * mSoilBlock1->dimension(1)));
        h2 = std::min(1.0 / (1.0 / (mSoilBlock2->dimension(0) * mSoilBlock2->dimension(0)) +
                             1.0 / (mSoilBlock2->dimension(1) * mSoilBlock2->dimension(1))), h2);
    }
    return h2 / (2.0 * a);
}
