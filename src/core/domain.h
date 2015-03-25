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

#ifndef QFCORE_DOMAIN_H
#define QFCORE_DOMAIN_H

#include <core/soilblock.h>
#include <core/heatsurface.h>
#include <core/boundarycondition.h>

namespace qfcore
{

class Domain
{
private:
    /// Массив грунтовых блоков.
    std::vector<SoilBlock> mSoilBlocks;
    /// Массив граничных условий.
    std::vector<BoundaryCondition> mBoundaryConditions;
    /// Массив площадок теплопотока.
    std::vector<HeatSurface> mHeatSurfaces;

    /// Перемещение модель на новый временной слой.
    void moveInTime();

public:
    Domain();

    /**
     * Добавление нового элемента в массив грунтовых блоков.
     * @returns Порядковый номер блока в массиве.
     */
    std::size_t addSoilBlock(const SoilBlock &inSoilBlock);

    /**
     * Добавление нового элемента в массив грунтовых блоков (для осесимметричной задачи).
     * @returns Порядковый номер блока в массиве.
     */
    std::size_t addSoilBlock(const SoilBlock &inSoilBlock, double r);

    /**
     * Добавление нового элемента @p bc в массив граничных условий.
     * @returns Порядковый номер блока в массиве.
     */
    std::size_t addBoundaryCondition(const BoundaryCondition &bc);

    /**
     * Добавление новой площадки между блоками с номерами @p num1 и @p num2,
     * направленную перпендекулярно оси @p axe и имеющую площадь @p square.
     * @param isAxiallySymmetric должна ли создаваемая площадка использовать
     * формулы для осесимметричной задачи
     * @warning Функцию следует вызывать только после появления гарантии того,
     *          что блоки не будут перемещаться в памяти,
     *          ведь площадки теплопотока хранят в себе указатели на них. */
    void addHeatSurface(unsigned short axe, double square,
                        std::size_t num1, std::size_t num2,
                        bool isAxiallySymmetric);

    /**
     * Добавление новой площадки между блоком с номером @p num1 и граничным
     * условием с номером @p num2, и имеющую площадь @p square. Расстояние
     * от центра блока до граничного условия считается равным @p r.
     * @warning Функцию следует вызывать только после появления гарантии того,
     *          что блоки и граничные усвлоия не будут перемещаться в памяти,
     *          ведь площадки теплопотока хранят в себе указатели на них. */
    void addHeatSurface(double r, double square,
                        std::size_t num1, std::size_t num2);

    /**
     * Изменение шага по времени.
     * @param inTimeStep новое значение шага по времени.
     * @warning Запускать после создания _всех_ площадок теплопотока!
     */
    void setTimeStep(double inTimeStep);

    /**
     * Делает указанное количество шагов по времени.
     * @param inNumSteps количество шагов
     */
    void doSteps(unsigned int inNumSteps);

    /**
     * @return Константная ссылка на массив грунтовых ячеек.
     */
    const std::vector<SoilBlock> &blocks() const {
        return mSoilBlocks;
    }

    /// Обновляет все граничные условия в соответствие с указанной датой.
    void setDate(int y, int m, int d);

    /// Максимальный шаг (в секундах), при котором регуляризация не нужна
    double maximalStep() const;

    /// Перемешивает площадки теплопотока. Это чтобы не было конкуренции
    /// на считывание данных из блоков и граничных условий между потоками.
    void shuffleHeatSurfaces();
};

}

#endif // QFCORE_DOMAIN_H
