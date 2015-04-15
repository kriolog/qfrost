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
 *
 */

#include "monthstableexpander.h"
#include "monthstablemodel.h"

#include "units/units.h"

using namespace qfgui;

static QList<double> emptyMonthlyData()
{
    static QList<double> result;
    if (result.isEmpty()) {
        for (int i = 1; i <= 12; ++i) {
            result.append(0.0);
        }
    }
    return result;
}

MonthsTableExpander::MonthsTableExpander(MonthsTableModel *model,
                                         const QString &valueName,
                                         QWidget *parent)
    : QWidget(parent)
    , mModel(model)
    , mData(emptyMonthlyData())
    , mValueName(valueName)
    , mValueNameWithSuffix(valueName)
    , mPhysicalProperty()
    , mModelSector(mModel->addExpander(this))
{
    connect(Units::units(this), SIGNAL(changed()), SLOT(updateHeaderText()));
    setVisible(false);
}

bool MonthsTableExpander::setValue(int monthNum, double d)
{
    Q_ASSERT(monthNum < mData.size());
    double &valRef = mData[monthNum];
    if (valRef == d) {
        return false;
    }
    valRef = d;
    emit valueChanged(monthNum);
    emit valuesChanged(mData);
    return true;
}

bool MonthsTableExpander::setValues(const QList<double> &data)
{
    Q_ASSERT(data.size() == 12);
    if (mData == data) {
        return false;
    }
    mData = data;
    emit valuesChanged(mData);
    emit valuesReplaced();
    return true;
}

bool MonthsTableExpander::setPhysicalProperty(int p)
{
    if (mPhysicalProperty == p) {
        return false;
    }
    mPhysicalProperty = static_cast<PhysicalProperty>(p);
    Q_ASSERT(int(mPhysicalProperty) == p);
    emit physicalPropertyChanged(p);
    updateHeaderText();
    return true;
}

void MonthsTableExpander::updateHeaderText()
{
    QString newText = mValueName;
    if (mPhysicalProperty != NoProperty) {
        newText += Units::unit(this, mPhysicalProperty).headerSuffixOneLine();
    }

    if (newText == mValueNameWithSuffix) {
        return;
    }

    mValueNameWithSuffix = newText;
    emit headerTextChanged();
}
