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
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

QVariant MonthsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    const bool isMonth = (orientation == mOrientation);

    if (isMonth) {
        if (role == Qt::FontRole) {
            // летние месяцы выделяем
            if (section >= 5 && section <= 7) {
                QFont font;
                font.setItalic(true);
                return font;
            } else {
                return QVariant();
            }
        }

        if (role == Qt::ToolTipRole) {
            return section + 1;
        }

        if (role == Qt::DisplayRole) {
            const QLocale locale;
            return locale.standaloneMonthName(section + 1,
                                              mIsHorizontal
                                              ? QLocale::ShortFormat
                                              : QLocale::LongFormat);
        }
    } else {
        if (role == Qt::DisplayRole) {
                return mExpanders.at(section)->headerText();
        }
    }

    return QVariant();
}

bool MonthsTableModel::setData(const QModelIndex &index,
                               const QVariant &value,
                               int role)
{
    const int sectorNum = dataTypeNum(index);
    if (index.isValid() && role == Qt::EditRole) {
        if (index.data(role) == value) {
            // менять поле на то же самое значение незачем
            return false;
        }

        if (!value.canConvert(QVariant::Double)) {
            // не смогли сконвертировать в double? Значит, юзер ввёл фигню.
            return false;
        }

        // ещё здесь? запишем новое значение в соответствующий expander
        Q_ASSERT(mExpanders.size() > sectorNum);
        MonthsTableExpander *expander = mExpanders[sectorNum];
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

    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    MonthsTableExpander *expander = mExpanders[sectorNum];
    Q_ASSERT(expander->modelSector() == sectorNum);

    if (role == QFrost::PhysicalPropertyRole) {
        return expander->physicalProperty();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const int month = monthNum(index);
        return expander->value(month);
    }

    return QVariant();
}

int MonthsTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mIsHorizontal ? 12 : mExpanders.size();
}

int MonthsTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return mIsHorizontal ? mExpanders.size() : 12;
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
    const int newSector = mExpanders.size();

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

    connect(expander, SIGNAL(physicalPropertyChanged(int)),
            SLOT(onExpanderPhysicalPropertyChanged()));

    connect(expander, SIGNAL(valueChanged(int)),
            SLOT(onExpanderValueChanged(int)));

    connect(expander, SIGNAL(valuesReplaced()),
            SLOT(onExpanderValuesReplaced()));

    emit addedExpander(newSector, expander);

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
