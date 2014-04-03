/*
 * Copyright (C) 2012  Denis Pesotsky
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

#ifndef QFGUI_UNFROZENWATERWIDGET_H
#define QFGUI_UNFROZENWATERWIDGET_H

#include <QtWidgets/QGroupBox>

QT_FORWARD_DECLARE_CLASS(QDataWidgetMapper)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Soil)
QT_FORWARD_DECLARE_CLASS(SortedPointsWidget)
QT_FORWARD_DECLARE_CLASS(PhysicalPropertySpinBox)

class UnfrozenWaterWidget : public QGroupBox
{
    Q_OBJECT
public:
    UnfrozenWaterWidget(QWidget *parent);

    void setMapper(QDataWidgetMapper *mapper);

    void connectTo(Soil *soil);

signals:
    void unfrozenWaterUsageToggled(bool on);

private:
    SortedPointsWidget *mUnfrozenWaterCurve;
    PhysicalPropertySpinBox *mMoistureTotal;
    PhysicalPropertySpinBox *mDryDensity;
};

}

#endif // QFGUI_UNFROZENWATERWIDGET_H
