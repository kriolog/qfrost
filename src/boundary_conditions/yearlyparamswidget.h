/*
 * Copyright (C) 2015  Denis Pesotsky
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

#ifndef QFGUI_YEARLYPARAMSWIDGET_H
#define QFGUI_YEARLYPARAMSWIDGET_H

#include <QtWidgets/QWidget>

#include "boundarycondition.h"

QT_FORWARD_DECLARE_CLASS(QLabel)

namespace qfgui {

class YearlyParamsWidget : public QWidget
{
    Q_OBJECT
    
    Q_PROPERTY(YearlyParams values
               READ values
               WRITE setValues
               NOTIFY valuesChanged
               USER true)

public:
    YearlyParamsWidget(QWidget* parent);
    
    const YearlyParams &values() const { return mValues; }
    void setValues(const YearlyParams &v);
    
    void updateLabel();
    
private slots:
    void loadFromFile();
    void showHelp();
    
signals:
    void valuesChanged();
    
private:
    YearlyParams mValues;
    
    QLabel *mLabel;
};
}

#endif // QFGUI_YEARLYPARAMSWIDGET_H
