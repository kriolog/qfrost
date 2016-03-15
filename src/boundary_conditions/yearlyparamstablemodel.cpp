/*
 *  Copyright (C) 2016  Denis Pesotsky
 *
 *  This file is part of QFrost.
 *
 *  QFrost is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include "yearlyparamstablemodel.h"

using namespace qfgui;

YearlyParamsTableModel::YearlyParamsTableModel(const YearlyParams &yearlyParams,
                                               YearlyParamsTableType tableType,
                                               QObject* parent)
    : mYearlyParams(yearlyParams)
    , mTableType(tableType)
{
    
}

int YearlyParamsTableModel::rowCount(const QModelIndex &parent) const
{
    return mYearlyParams.size();
}

int YearlyParamsTableModel::columnCount(const QModelIndex &parent) const
{
    return 12;
}

QVariant YearlyParamsTableModel::headerData(int section,
                                            Qt::Orientation orientation,
                                            int role) const
{
    if (orientation == Qt::Horizontal) {
        return QFrost::monthHeaderData(section, role);
    } else {
        if (role == Qt::DisplayRole) {
            YearlyParams::ConstIterator it = mYearlyParams.constBegin();
            for (int i = 0; i < section; ++i) {
                ++it;
            }
            return it.key();
        }
    }
    
    return QVariant();
}

QVariant YearlyParamsTableModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole || !index.isValid()) {
        return QVariant();
    }
    
    YearlyParams::ConstIterator it = mYearlyParams.constBegin();
    for (int i = 0; i < index.row(); ++i) {
        ++it;
    }
    Q_ASSERT(it->size() == columnCount());
    const QPair<double, double> &pair = it->at(index.column());
    return mTableType == qfgui::YPTT_Temperature ? pair.first : pair.second;
}
