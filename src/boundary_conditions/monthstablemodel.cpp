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

#include "monthstablemodel.h"

#include <units.h>

#include <QtCore/QDate>
#include <QtCore/QSize>
#include <QtCore/QLocale>
#include <QtGui/QFont>

using namespace qfgui;

MonthsTableModel::MonthsTableModel(const QString &valueName, QObject *parent)
    : QAbstractTableModel(parent)
    , mData()
    , mValueName(valueName)
    , mPhysicalProperty(NoProperty)
{
    for (int i = 0; i != 12; ++i) {
        mData.append(0);
    }
    updateHeaderData();
    connect(Units::units(this), SIGNAL(changed()), SLOT(updateHeaderData()));
}

Qt::ItemFlags MonthsTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result;
    if (index.isValid()) {
        result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if (index.column() == 1) {
            result |= Qt::ItemIsEditable;
        }
    }
    return result;
}

QVariant MonthsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        return (section == 0) ? tr("Month") : mValueNameWithSuffix;
    } else {
        return section + 1;
    }
}

void MonthsTableModel::updateHeaderData()
{
    mValueNameWithSuffix = mValueName;
    if (mPhysicalProperty != NoProperty) {
        mValueNameWithSuffix += Units::unit(this, mPhysicalProperty).headerSuffixOneLine();
    }
    emit headerDataChanged(Qt::Horizontal, 1, 1);
}

bool MonthsTableModel::setData(const QModelIndex &index,
                               const QVariant &value,
                               int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        if (index.column() != 1) {
            // первую колонку менять нельзя (там месяца перечислены)
            // как и отличные от второй колонки (там нет данных)
            Q_ASSERT(false);
            return false;
        }

        if (index.data(role) == value) {
            // менять поле на то же самое значение незачем
            return false;
        }

        if (!value.canConvert(QVariant::Double)) {
            // не смогли сконвертировать в double? Значит, юзер ввёл фигню.
            return false;
        }

        Q_ASSERT(mData.size() == 12);
        // ещё здесь? запишем новое значение и просигналим об изменении данных
        mData[index.row()] = value.toDouble();
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QString MonthsTableModel::standaloneMonthName(int month)
{
    Q_ASSERT(month >= 1 && month <= 12);
    switch (month) {
    case 1: return tr("January");
    case 2: return tr("February");
    case 3: return tr("March");
    case 4: return tr("April");
    case 5: return tr("May");
    case 6: return tr("June");
    case 7: return tr("July");
    case 8: return tr("August");
    case 9: return tr("September");
    case 10: return tr("October");
    case 11: return tr("November");
    case 12: return tr("December");
    default: return "";
    }
}

QVariant MonthsTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        Q_ASSERT(false);
        return QVariant();
    }

    if (index.row() >= rowCount() || index.column() >= columnCount()) {
        Q_ASSERT(false);
        return QVariant();
    }

    bool isMonthsColumn = (index.column() == 0);

    if (role == QFrost::PhysicalPropertyRole) {
        return isMonthsColumn
               ? QVariant()
               : mPhysicalProperty;
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (isMonthsColumn) {
            return standaloneMonthName(index.row() + 1);
        } else {
            return mData.at(index.row());
        }
    }

    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignVCenter
               + (isMonthsColumn
                  ? Qt::AlignRight
                  : Qt::AlignHCenter);
    }

    // летние месяцы выделяем
    if (isMonthsColumn && role == Qt::FontRole
            && index.row() >= 5 && index.row() <= 7) {
        QFont font;
        font.setBold(true);
        return font;
    }

    return QVariant();
}

int MonthsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 2;
}

int MonthsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 12;
}

void MonthsTableModel::setValues(const QList<double> &data)
{
    Q_ASSERT(data.size() == 12);
    mData = data;
    emit dataChanged(index(1, 0), index(1, 11));
}

const QList<double> &MonthsTableModel::values() const
{
    Q_ASSERT(mData.size() == 12);
    return mData;
}

QModelIndexList MonthsTableModel::allData() const
{
    QModelIndexList result;
    for (int row = 0; row < rowCount(); ++row) {
        result << createIndex(row, 1);
    }
    return result;
}

void MonthsTableModel::setPhysicalProperty(PhysicalProperty property)
{
    if (mPhysicalProperty != property) {
        mPhysicalProperty = property;
    }
    updateHeaderData();
    emit dataChanged(index(1, 0), index(1, 11));
}
