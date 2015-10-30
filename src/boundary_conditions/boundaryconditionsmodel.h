/*
 * Copyright (C) 2011-2015  Denis Pesotsky
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

#ifndef QFGUI_BOUNDARYCONDITIONSSMODEL_H
#define QFGUI_BOUNDARYCONDITIONSSMODEL_H

#include <itemviews/itemsmodel.h>

namespace qfgui
{

enum BoundaryConditionsModelColumn {
    BC_Color,
    BC_Name,
    BC_Type,
    BC_Temperatures1,
    BC_HeatFlowDensities,
    BC_Temperatures3,
    BC_HeatTransferFactors,
    BC_HasTemperatureTrend,
    BC_TemperatureTrend,
    BC_TemperatureTrendStartYear,
    BC_UsesTemperatureSpline,
    BC_YearlyParams
};

QT_FORWARD_DECLARE_CLASS(MainWindow)

class BoundaryConditionsModel : public ItemsModel
{
    Q_OBJECT
public:
    BoundaryConditionsModel(QWidget *parent);
    Q_INVOKABLE BoundaryConditionsModel(ItemsModel *other, int row);

    QString newItemName() const {
        //: Default name for new b.c. (like "New Folder" for folders in Windows)
        return tr("New Condition");
    }
    QString newItemNameTemplate() const {
        //: Default name for new boundary condition (%1 is number)
        return tr("New Condition %1");
    }
};

}

#endif // QFGUI_BOUNDARYCONDITIONSSMODEL_H
