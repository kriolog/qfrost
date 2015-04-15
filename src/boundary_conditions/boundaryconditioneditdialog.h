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

#ifndef QFGUI_BOUNDARYCONDITIONEDITDIALOG_H
#define QFGUI_BOUNDARYCONDITIONEDITDIALOG_H

#include <itemviews/itemeditdialog.h>

QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QCustomPlot)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(BoundaryConditionsModel)
QT_FORWARD_DECLARE_CLASS(MonthsTableWidget)
QT_FORWARD_DECLARE_CLASS(MonthsTableExpander)
QT_FORWARD_DECLARE_CLASS(MonthsTablePlot)

class BoundaryConditionEditDialog : public ItemEditDialog
{
    Q_OBJECT
public:
    Q_INVOKABLE BoundaryConditionEditDialog(ItemsModel *model,
                                            const QStringList &forbiddenNames,
                                            bool isNewItem,
                                            QWidget *parent);

private slots:
    /// Обновляет виджеты, касающиеся температуры (mTrendGroupBox + содержимое и
    /// mUsesTemperatureSpline), чтобы учесть, есть ли для @p type температуры.
    void updateTemperatureWidgets(int type);

private:
    QGroupBox *mTrendGroupBox;

    QComboBox *mTypeBox;

    MonthsTableWidget *mTable1;
    MonthsTableWidget *mTable2;
    MonthsTableWidget *mTable3;

    MonthsTableExpander *mExp1;
    MonthsTableExpander *mExp2;
    MonthsTableExpander *mExp3t;
    MonthsTableExpander *mExp3a;

    QCheckBox *mUsesTemperatureSpline;

    MonthsTablePlot *mPlot;
};

}

#endif // QFGUI_BOUNDARYCONDITIONEDITDIALOG_H
