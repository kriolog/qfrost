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

#include "sortedpointsmodel.h"

#include <units/units.h>

#include <QtCore/qmath.h>

using namespace qfgui;

SortedPointsModel::SortedPointsModel(const QString &xName, const QString &yName,
                                     PhysicalProperty xProp,
                                     PhysicalProperty yProp,
                                     QObject *parent)
    : QAbstractTableModel(parent)
    , mValues()
    , mXName(xName)
    , mYName(yName)
    , mXNameWithSuffix()
    , mYNameWithSuffix()
    , mXProp(xProp)
    , mYProp(yProp)
{
    updateHeaderData();
    connect(Units::units(this), SIGNAL(changed()), SLOT(updateHeaderData()));
}

void SortedPointsModel::updateHeaderData()
{
    mXNameWithSuffix = mXName + Units::unit(this, mXProp).headerSuffixOneLine();
    mYNameWithSuffix = mYName + Units::unit(this, mYProp).headerSuffixOneLine();
    emit headerDataChanged(Qt::Horizontal, 0, 2);
}

int SortedPointsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

int SortedPointsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mValues.size();
}

Qt::ItemFlags SortedPointsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.isValid()) {
        return flags | Qt::ItemIsEditable;
    }
    return flags;
}

QVariant SortedPointsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return (section == 0) ? mXNameWithSuffix : mYNameWithSuffix;
    } else {
        return section + 1;
    }
}

QVariant SortedPointsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        Q_ASSERT(false);
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole) {
        return  Qt::AlignCenter;
    }

    if (role == QFrost::PhysicalPropertyRole) {
        return (index.column() == 0) ? mXProp : mYProp;
    }

    if (role == QFrost::MinimumRole && index.row() != rowCount() - 1) {
        const Unit &u = Units::unit(this, (index.column() == 0) ? mXProp : mYProp);
        const double d = u.minStepSI();
        QMap<double, double>::ConstIterator i = mValues.constEnd() - 1 - index.row() - 1;
        Q_ASSERT(*i < * (i + 1));
        return ((index.column() == 0) ? i.key() : i.value()) + d;
    }

    if (role == QFrost::MaximumRole) {
        if (index.row() == 0) {
            if (index.column() == 0) {
                return 0.0;
            }
        } else {
            const Unit &u = Units::unit(this, (index.column() == 0) ? mXProp : mYProp);
            const double d = u.minStepSI();
            QMap<double, double>::ConstIterator i = mValues.constEnd() - 1 - index.row() + 1;
            Q_ASSERT(*i > *(i - 1));
            return ((index.column() == 0) ? i.key() : i.value()) - d;
        }
    }

    if (index.row() >= mValues.size() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        QMap<double, double>::ConstIterator i = mValues.constEnd() - 1 - index.row();
        return (index.column() == 0) ? i.key() : i.value();
    }

    return QVariant();
}

bool SortedPointsModel::setData(const QModelIndex &dataIndex,
                                const QVariant &value, int role)
{
    if (dataIndex.isValid() && role == Qt::EditRole) {
        if (data(dataIndex) == value) {
            // менять поле на то же самое значение незачем
            return false;
        }

        if (value.type() != QVariant::Double) {
            Q_ASSERT(false);
            return false;
        }

        QMap<double, double>::Iterator i = mValues.end() - 1 - dataIndex.row();
        if (dataIndex.column() == 0) {
            mValues.insertMulti(value.toDouble(), i.value());
            mValues.erase(i);
            emit dataChanged(index(0, 0), index(rowCount() - 1, 1));
        } else {
            i.value() = value.toDouble();
            emit dataChanged(dataIndex, dataIndex);

            // У соседних величин изменились минимумы и максимумы
            int prevRow = qMax(0, dataIndex.row() - 1);
            QModelIndex prevIndex = index(prevRow, dataIndex.column());
            emit dataChanged(prevIndex, prevIndex);

            int nextRow = qMin(rowCount() - 1, dataIndex.row() + 1);
            QModelIndex nextIndex = index(nextRow, dataIndex.column());
            emit dataChanged(nextIndex, nextIndex);
        }
        return true;
    }
    return false;
}

void SortedPointsModel::setValues(const QMap< double, double > &data)
{
    if (mValues == data) {
        return;
    }
    beginRemoveRows(index(0, 0), 0, mValues.size());
    endRemoveRows();
    beginInsertRows(index(0, 0), 0, data.size() - 1);
    mValues = data;
    endInsertRows();
}
