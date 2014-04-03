/*
 * Copyright (C) 2011-2012  Denis Pesotsky
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

#include "boundaryconditionsmodel.h"

#include "boundarycondition.h"

using namespace qfgui;

static QList<QColor> defaultColors()
{
    QList<QColor> colors;
    colors << QColor(0, 156, 255)
           << QColor(212, 7, 99)
           << QColor(255, 255, 0)
           << QColor(255, 0, 0)
           << QColor(11, 224, 0)
           << QColor(0, 0, 190)
           << QColor(149, 0, 181)
           << QColor(255, 96, 0);
    return colors;
}

static QHash<int, PhysicalProperty> physicalProperties()
{
    QHash<int, PhysicalProperty> result;
    result.insert(BC_Temperatures1, Temperature);
    result.insert(BC_HeatFlowDensities, HeatFlowDensity);
    result.insert(BC_Temperatures3, Temperature);
    result.insert(BC_HeatTransferFactors, HeatTransferFactor);
    return result;
}

BoundaryConditionsModel::BoundaryConditionsModel(QWidget *parent)
    : ItemsModel(BoundaryCondition::staticMetaObject, parent,
                 defaultColors(), physicalProperties(),
                 BoundaryCondition::voidCondition())
{

}

BoundaryConditionsModel::BoundaryConditionsModel(ItemsModel *other,
        int row)
    : ItemsModel(other, row)
{
    Q_ASSERT(qobject_cast< BoundaryConditionsModel * >(other) != NULL);
}
