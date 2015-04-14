/*
 * Copyright (C) 2012-2015  Denis Pesotsky
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

#ifndef QFGUI_MONTHSTABLEMODEL_H
#define QFGUI_MONTHSTABLEMODEL_H

#include <QtCore/QAbstractTableModel>

#include <qfrost.h>

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(MonthsTableExpander)

class MonthsTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    MonthsTableModel(Qt::Orientation orientation, QObject* parent);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /// Список индексов всех хранящихся в модели значений (не дат!)
    QModelIndexList allData() const;

    /// Месяц (от 0 до 11) для @p index
    int monthNum(const QModelIndex &index) const {
        return mIsHorizontal ? index.column() : index.row();
    }

    /// Номер типа данных для @p index (0 - название месяца, 1 - сами данные)
    int dataTypeNum(const QModelIndex &index) const {
        return mIsHorizontal ? index.row() : index.column();
    }

    /// Добавление обрабатываемого @p expander нового сектора с данными.
    /// @warning на момент срабатывания сигнала, сектор ещё не записан в него!
    int addExpander(MonthsTableExpander* expander);

signals:
    /// Сигнал о том, что только что был добавлен @p expander
    void addedExpander(int sector, MonthsTableExpander* expander);

private:
    int sectorNum(QObject *expanderObj) const;

    const Qt::Orientation mOrientation;
    const bool mIsHorizontal;

    QList<MonthsTableExpander*> mExpanders;

private slots:
    void onExpanderHeaderTextChanged();
    void onExpanderPhysicalPropertyChanged();
    void onExpanderValueChanged(int monthNum);
    void onExpanderValuesReplaced();
};

}

#endif // QFGUI_MONTHSTABLEMODEL_H
