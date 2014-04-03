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

#include <boundary_conditions/boundaryconditionspanel.h>

#include <QtWidgets/QVBoxLayout>

#include <boundary_conditions/boundaryconditionswidget.h>
#include <boundary_conditions/boundaryconditionsmodel.h>
#include <boundary_conditions/boundarycondition.h>
#include <control_panel/controlpanel.h>
#include <itemviews/itemsmodel.h>

using namespace qfgui;

BoundaryConditionsPanel::BoundaryConditionsPanel(ControlPanel *parent)
    : QWidget(parent)
    , mBoundaryConditionsWidget(new BoundaryConditionsWidget(this))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addWidget(mBoundaryConditionsWidget);
}

BoundaryConditionsModel *BoundaryConditionsPanel::model()
{
    Q_ASSERT(qobject_cast<BoundaryConditionsModel *>(mBoundaryConditionsWidget->model()) != NULL);
    return qobject_cast<BoundaryConditionsModel *>(mBoundaryConditionsWidget->model());
}

QList<BoundaryCondition *> BoundaryConditionsPanel::boundaryConditionsSorted()
{
    QList<BoundaryCondition *> result;
    foreach(Item * item, mBoundaryConditionsWidget->sortedItems()) {
        result << qobject_cast<BoundaryCondition * >(item);
        Q_ASSERT(result.last() != NULL);
    }
    return result;
}
