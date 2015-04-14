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

#include "monthstablemodel.h"

#include <QtCore/QDate>
#include <QtCore/QSize>
#include <QtCore/QLocale>
#include <QtGui/QFont>

#include "monthstableexpander.h"

using namespace qfgui;

MonthsTableModel::MonthsTableModel(Qt::Orientation orientation,
                                   QObject *parent)
    : QAbstractTableModel(parent)
    , mOrientation(orientation)
    , mIsHorizontal(orientation == Qt::Horizontal)
    , mExpanders()
{

}

Qt::ItemFlags MonthsTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result;
    if (index.isValid()) {
        result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if (dataTypeNum(index) >= 1) {
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

    if (orientation != mOrientation) {
        if (section == 0) {
            return tr("Month");
        } else {
            return mExpanders.at(section - 1)->headerText();
        }
    } else {
        return section + 1;
    }
}

bool MonthsTableModel::setData(const QModelIndex &index,
                               const QVariant &value,
                               int role)
{
    const int sectorNum = dataTypeNum(index);
    if (index.isValid() && role == Qt::EditRole) {
        if (sectorNum == 0) {
            // первую колонку менять нельзя (там месяца перечислены)
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

        // ещё здесь? запишем новое значение в соответствующий expander
        Q_ASSERT(mExpanders.size() > sectorNum - 1);
        MonthsTableExpander *expander = mExpanders[sectorNum - 1];
        return expander->setValue(monthNum(index), value.toDouble());
    }
    return false;
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

    const int sectorNum = dataTypeNum(index);
    const bool isMonthsColumn = (sectorNum == 0);
    MonthsTableExpander *expander = isMonthsColumn ? 0 : mExpanders[sectorNum - 1];
    Q_ASSERT(isMonthsColumn || expander->modelSector() == sectorNum);

    if (role == QFrost::PhysicalPropertyRole) {
        return isMonthsColumn
               ? QVariant()
               : expander->physicalProperty();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const int month = monthNum(index);
        if (isMonthsColumn) {
            const QLocale locale;
            return locale.standaloneMonthName(month + 1,
                                              mIsHorizontal
                                                ? QLocale::ShortFormat
                                                : QLocale::LongFormat);
        } else {
            Q_ASSERT(expander);
            return expander->value(month);
        }
    }

    if (role == Qt::TextAlignmentRole) {
        if (mIsHorizontal) {
            return Qt::AlignCenter;
        } else {
            return Qt::AlignVCenter
                + (isMonthsColumn ? Qt::AlignRight : Qt::AlignHCenter);
        }
    }

    // летние месяцы выделяем
    if (isMonthsColumn && role == Qt::FontRole
            && monthNum(index) >= 5 && monthNum(index) <= 7) {
        QFont font;
        font.setBold(true);
        return font;
    }

    return QVariant();
}

int MonthsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mIsHorizontal ? 12 : (1 + mExpanders.size());
}

int MonthsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mIsHorizontal ? (1 + mExpanders.size()) : 12;
}

QModelIndexList MonthsTableModel::allData() const
{
    QModelIndexList result;
    for (int monthNum = 0; monthNum < 11; ++monthNum) {
        result.append(mIsHorizontal ? index(1, monthNum) : index(monthNum, 1));
    }
    return result;
}

int MonthsTableModel::addExpander(MonthsTableExpander *expander)
{
    const int newSector = mExpanders.size() + 1;

    if (mIsHorizontal) {
        beginInsertRows(QModelIndex(), newSector, newSector);
    } else {
        beginInsertColumns(QModelIndex(), newSector, newSector);
    }

    mExpanders.append(expander);

    if (mIsHorizontal) {
        endInsertRows();
    } else {
        endInsertColumns();
    }

    connect(expander, SIGNAL(headerTextChanged()),
            SLOT(onExpanderHeaderTextChanged()));

    connect(expander, SIGNAL(physicalPropertyChanged()),
            SLOT(onExpanderPhysicalPropertyChanged()));

    connect(expander, SIGNAL(valueChanged(int)),
            SLOT(onExpanderValueChanged(int)));

    connect(expander, SIGNAL(valuesReplaced()),
            SLOT(onExpanderValuesReplaced()));

    return newSector;
}

int MonthsTableModel::sectorNum(QObject *expanderObj) const
{
    Q_ASSERT(expanderObj);
    MonthsTableExpander *expander = qobject_cast<MonthsTableExpander* >(expanderObj);
    Q_ASSERT(expander);
    return expander->modelSector();
}

void MonthsTableModel::onExpanderHeaderTextChanged()
{
    Q_ASSERT(sender());
    const int section = sectorNum(sender());
    emit headerDataChanged(mIsHorizontal ? Qt::Vertical : Qt::Horizontal,
                           section,
                           section);
}

void MonthsTableModel::onExpanderPhysicalPropertyChanged()
{
    Q_ASSERT(sender());
    const int section = sectorNum(sender());
    if (mIsHorizontal) {
        emit dataChanged(index(0, section), index(11, section));
    } else {
        emit dataChanged(index(section, 0), index(section, 11));
    }
}

void MonthsTableModel::onExpanderValueChanged(int monthNum)
{
    Q_ASSERT(sender());
    const int section = sectorNum(sender());
    const QModelIndex changedModelIndex = mIsHorizontal
                                            ? index(monthNum, section)
                                            : index(section, monthNum);
    emit dataChanged(changedModelIndex, changedModelIndex);
}

void MonthsTableModel::onExpanderValuesReplaced()
{
    Q_ASSERT(sender());
    const int section = sectorNum(sender());
    if (mIsHorizontal) {
        emit dataChanged(index(0, section), index(11, section));
    } else {
        emit dataChanged(index(section, 0), index(section, 11));
    }
}
