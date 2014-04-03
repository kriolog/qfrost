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

#ifndef QFGUI_BOUNDARYCONDITIONSPANEL_H
#define QFGUI_BOUNDARYCONDITIONSPANEL_H

#include <QtWidgets/QWidget>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(ControlPanel)
QT_FORWARD_DECLARE_CLASS(BoundaryConditionsWidget)
QT_FORWARD_DECLARE_CLASS(BoundaryConditionsModel)
QT_FORWARD_DECLARE_CLASS(BoundaryCondition)

class BoundaryConditionsPanel : public QWidget
{
    Q_OBJECT
public:
    BoundaryConditionsPanel(ControlPanel *parent);
    BoundaryConditionsModel *model();

    /// Список граничных условий (в том же порядке, что и во view)
    QList<BoundaryCondition *> boundaryConditionsSorted();

private:
    BoundaryConditionsWidget *mBoundaryConditionsWidget;
};

}

#endif // QFGUI_BOUNDARYCONDITIONSPANEL_H
