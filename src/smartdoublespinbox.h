/*
 * Copyright (C) 2010-2015  Denis Pesotsky
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

#ifndef QFGUI_SMARTDOUBLESPINBOX_H
#define QFGUI_SMARTDOUBLESPINBOX_H

#include <QtWidgets/QDoubleSpinBox>

namespace qfgui
{

/**
 * QDoubleSpinBox, который при совершении шагов принимает значения, кратные
 * signleStep(). Причём изменяет в сторону, противоположную направлению шага.
 * То есть при value()==0.01, singleStep()==0.5 и изменении в большую
 * сторону (steps>0), newValue станет равен 0, а при изменении в меньшую
 * сторону и тех же прочих начальных данных, он станет равен 0.5.
 */
class SmartDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    SmartDoubleSpinBox(QWidget *parent = 0);
    virtual void stepBy(int steps);

    /**
     * Запретить или разрешить принятие значения minimum() и maximum()
     * при stepBy(), если это ограничение не кратно singleStep().
     */
    void setDenySteppingToNonMultipleBound(bool deny) {
        mDenyStepingToNonmultipleBounds = deny;
    }

private:
    /**
     * Если после stepBy() значение выйдет за пределы [min; max], стать
     * равным ближайшему допустимому кратному singleStep() значению.
     */
    bool mDenyStepingToNonmultipleBounds;

    bool isSingleStepMultiple(double v) const;

    friend class PhysicalPropertySpinBox;
};

}

#endif // QFGUI_SMARTDOUBLESPINBOX_H
