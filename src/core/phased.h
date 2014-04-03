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

#ifndef QFCORE_PHASED_H
#define QFCORE_PHASED_H

namespace qfcore
{
/**
 * Структура для хранения какого-либо параметра, который меняется
 * в зависимости от фазового состояния воды в грунте
 */
struct Phased {
    /// Значение для грунта в талом состоянии
    double thawed;

    /// Значение для грунта в мёрзлом состоянии
    double frozen;

    Phased() :
        thawed(0),
        frozen(0) {}

    /**
     * \details Основной конструктор.
     * @param inThawed значение показателя в талом состоянии,
     * @param inFrozen значение показателя в мёрзлом состоянии
     */
    Phased(const double &inThawed, const double &inFrozen) :
        thawed(inThawed),
        frozen(inFrozen) {}

    inline bool operator== (const Phased &other) const {
        return thawed == other.thawed && frozen == other.frozen;
    }
};

}

#endif // QFCORE_PHASED_H
