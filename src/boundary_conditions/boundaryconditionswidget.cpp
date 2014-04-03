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

#include "boundaryconditionswidget.h"

#include <boundary_conditions/boundaryconditioneditdialog.h>
#include <boundary_conditions/boundaryconditionsmodel.h>

using namespace qfgui;

BoundaryConditionsWidget::BoundaryConditionsWidget(QWidget *parent,
        bool isCompact,
        ItemsModel *model)
    : ItemsWidget(isCompact,
                  // FIXME: почему если тут сделать BoundaryConditionsModel(this), оно падает?
                  model == NULL ? new BoundaryConditionsModel(parent) : model,
                  BoundaryConditionEditDialog::staticMetaObject,
                  parent)
{
    // HACK: если бы мы сделали в списке инициализации new BoundaryConditionsModel(this),
    //       оно бы упало, так что нормального пэрента даём здесь
    if (model == NULL) {
        this->model()->setParent(this);
    }
    Q_ASSERT(qobject_cast<BoundaryConditionsModel *>(model) != NULL || model == NULL);

    setToolTips(tr("Create new boundary condition"),
                tr("Duplicate selected boundary condition"),
                tr("Edit selected boundary conditions"),
                tr("Remove selected boundary conditions"));
}
