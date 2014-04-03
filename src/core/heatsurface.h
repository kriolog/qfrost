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

#ifndef QFCORE_HEATSURFACE_H
#define QFCORE_HEATSURFACE_H
#include <exception>

namespace qfcore
{

class SoilBlock;
class BoundaryCondition;
class Domain;

/**
 * Класс для описания площадки между двумя блоками или блоком и граничным
 * условием, то есть площадки, по которой идёт теплопоток. На каждом временном
 * слое рассчитывает плотность теплового потока и передаёт её прилежащим
 * блокам.
 */
class HeatSurface
{
    /// Значение площади контакта.
    double mSquare;

    /// Являемся ли мы контактом между блоком и граничным условием.
    /// Соответственно, если не являемся, мы - контакт между 2 блоками.
    bool mHasBoundaryCondition;

    /// Указатель на граничное условие-соседа
    const BoundaryCondition *mBoundaryCondition;
    /// Указатель на 1й грунтовый блок-соседа.
    SoilBlock *mSoilBlock1;
    /// Указатель на 2й грунтовый блок-соседа.
    SoilBlock *mSoilBlock2;

    /**
     * Ось, по которой идёт теплопоток (перпендикулярная площадке).
     * Актуально только для контактов между блоками.
     * 0 - x, 1 - y, 2 - z.
     * \see SoilBlock::mDimensions
     */
    unsigned short mAxe;

    /// Мнимый радиус от центра блока до границы (для гр. усл.; r/2 в формулах)
    /// ИЛИ расстояние от оси Y до этой площадки (для вертикального контакта
    /// между двумя блоками)
    double mR;

    /// Расстояние от контакта до центра первого блока ИЛИ приведённая величина
    /// этого расстояния для верт. площадки контакта у осесимметричной задачи
    double mR1;
    /// Расстояние от контакта до центра второго блока ИЛИ приведённая величина
    /// этого расстояния для верт. площадки контакта у осесимметричной задачи
    double mR2;

    bool mIsAxiallySymmetric;

    /**
     * Расчёт теплопотока для следующего шага во времени
     * (основываясь на состоянии прилежащих блоков)
     * \see SoilBlock::moveInTime(), BoundaryCondition::moveInTime()
     */
    void moveInTime() const;

public:
    /**
     * Конструктор для контакта между 2 грунтовыми блоками.
     * @param axe ось, по которой идёт теплопоток
     * @param square площадь контакта
     * @param block1 указатель на первый грунтовый блок
     * @param block2 указатель на второй грунтовый блок
     */
    HeatSurface(unsigned short axe, double square,
                SoilBlock *block1, SoilBlock *block2,
                bool isAxiallySymmetric);

    /**
     * Конструктор для контакта между граничным условием и грунтовым блоком.
     * @param r (мнимое) расстояние от центра блока до граничного условия
     * @param square площадь контакта
     * @param block указатель на грунтовый блок
     * @param condition указатель на граничное условие
     */
    HeatSurface(double r, double square,
                SoilBlock *block, const BoundaryCondition *condition);

    /// Наибольший шаг (в секундах), при котором не нужна регуляризация.
    /// Бесконечность, если не важно.
    double maximalStep(bool is_1d) const;

    static void moveInTime2(HeatSurface &heatSurface) {
        heatSurface.moveInTime();
    }

    short axe() const {
        return mAxe;
    }
};

}

#endif // QFCORE_HEATSURFACE_H
