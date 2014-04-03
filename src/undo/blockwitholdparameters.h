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

#ifndef QFGUI_BLOCKWITHOLDTHAWEDPART_H
#define QFGUI_BLOCKWITHOLDTHAWEDPART_H

#include <QtCore/QObject>

namespace qfcore
{
QT_FORWARD_DECLARE_CLASS(SoilBlock)
}

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Soil)
QT_FORWARD_DECLARE_CLASS(Block)

/**
 * Блок, запомнивший своё изначальное кол-во незамёрзшей воды.
 * Может менять кол-во незамёрзшей воды и восстанавливать изначальное.
 */
class BlockWithOldThawedPart
{
public:
    BlockWithOldThawedPart(Block *block);
    void restore();
    /// Устанавливает кол-во незамёрзшей воды в блоке равным v
    void setThawedPart(double newV);
    /** Устанавливает кол-во незамёрзшей воды исходя из v.
     *  Если блок использует кривую незамёрзшей воды, то если v меньше
     *  минимального допустимого значения, то используется оно */
    void smartSetThawedPart(double newV);
protected:
    inline Block *block() {
        return mBlock;
    }
    inline const Block *block() const {
        return mBlock;
    }
private:
    Block *mBlock;
    double mOldV;
};

/**
 * Блок, запомнивший свою изначальную температуру и кол-во незамёрзшей воды.
 * Может менять температуру и восстанавливать изначальное состояние.
 */
class BlockWithOldTemperature: protected BlockWithOldThawedPart
{
public:
    BlockWithOldTemperature(Block *block);
    void restore();
    void setTemperature(double newT);
    double depth() const;
private:
    double mOldT;
};

/**
 * Блок, запомнивший своё изначальное кол-во незамёрзшей воды.
 * Может обновляться из грунта для поддержания актуальности.
 */
class BlockWithOldTransitionParameters: protected BlockWithOldThawedPart
{
public:
    BlockWithOldTransitionParameters(Block *block);
    /**
     * Обновляет параметры для блока из его грунта.
     * Если грунт == NULL, берёт для параметров фазового перехода стандартное
     * значение. Перерисовывает блок при необходимости.
     */
    void getNewPropertiesFromSoil();
    /// Блок берёт из грунта параметры фазового перехода и восстанавливает
    /// кол-во незамёрзшей воды.
    void restore();
};

/**
 * Блок, запомнивший свой изначальный грунт и кол-во незамёрзшей воды.
 * Может менять грунт и восстанавливать изначальное состояние.
 */
class BlockWithOldSoil: protected BlockWithOldTransitionParameters
{
public:
    BlockWithOldSoil(Block *block);
    void restore();
    void setSoil(const Soil *soil);
private:
    const Soil *mOldSoil;
};

}

#endif // QFGUI_BLOCKWITHOLDTHAWEDPART_H
