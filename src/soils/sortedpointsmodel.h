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

#ifndef QFGUI_SORTEDPOINTSMODEL_H
#define QFGUI_SORTEDPOINTSMODEL_H

#include <QtCore/QAbstractTableModel>

#include <qfrost.h>

namespace qfgui
{

class SortedPointsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SortedPointsModel(const QString &xName, const QString &yName,
                      PhysicalProperty xProp,
                      PhysicalProperty yProp,
                      QObject *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &dataIndex, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;

    const QMap<double, double> &values() const {
        return mValues;
    }
    void setValues(const QMap<double, double> &data);

private slots:
    void updateHeaderData();

private:
    QMap<double, double> mValues;

    const QString mXName;
    const QString mYName;

    QString mXNameWithSuffix;
    QString mYNameWithSuffix;

    const PhysicalProperty mXProp;
    const PhysicalProperty mYProp;
};

}

#endif // QFGUI_SORTEDPOINTSMODEL_H
