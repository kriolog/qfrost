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

#ifndef QFGUI_SOILEDITDIALOG_H
#define QFGUI_SOILEDITDIALOG_H

#include <itemviews/itemeditdialog.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(ItemsModel)
QT_FORWARD_DECLARE_CLASS(PhysicalPropertySpinBox)
QT_FORWARD_DECLARE_CLASS(UnfrozenWaterWidget)

class SoilEditDialog : public ItemEditDialog
{
    Q_OBJECT
public:
    Q_INVOKABLE SoilEditDialog(ItemsModel *model,
                               const QStringList &forbiddenNames,
                               bool isNewItem,
                               QWidget *parent);

private slots:
    void updateTransitionHeatAndTemperatureConnections(bool usesUnfrozenWaterCurve);

private:
    PhysicalPropertySpinBox *mConductivityTh;
    PhysicalPropertySpinBox *mConductivityFr;

    PhysicalPropertySpinBox *mCapacityTh;
    PhysicalPropertySpinBox *mCapacityFr;

    PhysicalPropertySpinBox *mTransitionTemperature;
    PhysicalPropertySpinBox *mTransitionHeat;

    PhysicalPropertySpinBox *mInternalHeatSourcePowerDensity;

    UnfrozenWaterWidget *mUnfrozenWater;
};

}

#endif // QFGUI_SOILEDITDIALOG_H
