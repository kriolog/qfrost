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

#include <core/domain.h>

#include <algorithm>
#include <cassert>
#include <limits>

//#include <tbb/parallel_for_each.h>
//#include "tbb/task_scheduler_init.h"

#include <core/soilblock.h>
#include <core/heatsurface.h>
#include <core/boundarycondition.h>

using namespace qfcore;

Domain::Domain() :
    mSoilBlocks(),
    mBoundaryConditions(),
    mHeatSurfaces()
{
}

void moveBlockInTime(SoilBlock &b)
{
    b.moveInTime();
}

void Domain::moveInTime()
{
    /*  for (int i = 0; i < mHeatSurfaces.size(); ++i) {
          mHeatSurfaces[i].moveInTime();
      }
      for (int i = 0; i < mSoilBlocks.size(); ++i) {
          mSoilBlocks[i].moveInTime();
      }*/

    std::for_each(mHeatSurfaces.begin(),
                  mHeatSurfaces.end(),
                  HeatSurface::moveInTime2);
    std::for_each(mSoilBlocks.begin(),
                  mSoilBlocks.end(),
                  moveBlockInTime);


    /*tbb::task_scheduler_init init(1);

    tbb::parallel_for_each(mHeatSurfaces.begin(),
                           mHeatSurfaces.end(),
                           HeatSurface::moveInTime2);

    tbb::parallel_for_each(mSoilBlocks.begin(),
                           mSoilBlocks.end(),
                           moveBlockInTime);*/
}

void Domain::setTimeStep(double inTimeStep)
{
    for (std::vector<SoilBlock>::iterator it = mSoilBlocks.begin();
            it != mSoilBlocks.end(); ++it) {
        it->setTimeStep(inTimeStep);
    }
}

std::size_t Domain::addSoilBlock(const SoilBlock &inSoilBlock)
{
    mSoilBlocks.push_back(inSoilBlock);
    return mSoilBlocks.size() - 1;
}

std::size_t Domain::addSoilBlock(const SoilBlock &inSoilBlock, double r)
{
    mSoilBlocks.push_back(inSoilBlock);
    mSoilBlocks.back().setZForAxiallySymmetricProblem(r);
    return mSoilBlocks.size() - 1;
}

std::size_t Domain::addBoundaryCondition(const BoundaryCondition &bc)
{
    mBoundaryConditions.push_back(bc);
    return mBoundaryConditions.size() - 1;
}

void Domain::addHeatSurface(unsigned short axe, double square,
                            std::size_t num1, std::size_t num2,
                            bool isAxiallySymmetric)
{
    assert(square > 0);
    mHeatSurfaces.push_back(HeatSurface(axe, square,
                                        &(mSoilBlocks.at(num1)),
                                        &(mSoilBlocks.at(num2)),
                                        isAxiallySymmetric));
}

void Domain::addHeatSurface(double r, double square,
                            std::size_t num1, std::size_t num2)
{
    assert(square > 0);
    mHeatSurfaces.push_back(HeatSurface(r, square,
                                        &(mSoilBlocks.at(num1)),
                                        &(mBoundaryConditions.at(num2))));
}

void Domain::doSteps(unsigned int inNumSteps)
{
    for (unsigned int i = 0; i < inNumSteps; ++i) {
        moveInTime();
    }
}

void Domain::setMonth(int month)
{
    for (std::vector<BoundaryCondition>::iterator it = mBoundaryConditions.begin();
            it != mBoundaryConditions.end(); ++it) {
        it->setMonth(month);
    }
}

double Domain::maximalStep() const
{
    if (mHeatSurfaces.empty()) {
        // Нет ни одного контакта между блоками? Ответ очевиден.
        return std::numeric_limits<double>::infinity();
    }

    // определяем, является ли наша задача одномерной
    // FIXME: это известно из гуя заранее, ни к чему узнавать это снова
    //        (хотя не совсем, гуй считает горизонтальные одномерные задачи
    //         двухмерными, после исправления этого нужно передавать сюда
    //         параметр dimensionsNum - 1, 2, или 3)
    bool is_1d = true;
    std::vector <HeatSurface>::const_iterator it = mHeatSurfaces.begin();
    short firstAxe = it->axe();
    ++it;
    for (; it != mHeatSurfaces.end(); ++it) {
        if (it->axe() != firstAxe) {
            is_1d = false;
            break;
        }
    }

    // считаем критический шаг
    double result = std::numeric_limits<double>::infinity();
    for (it = mHeatSurfaces.begin(); it != mHeatSurfaces.end(); ++it) {
        double t = it->maximalStep(is_1d);
        if (t < result) {
            result = t;
        }
    }
    return result;
}

void Domain::shuffleHeatSurfaces()
{
    std::random_shuffle(mHeatSurfaces.begin(), mHeatSurfaces.end());
}
