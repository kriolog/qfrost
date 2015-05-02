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

#ifndef QFGUI_SORTEDPOINTSWIDGET_H
#define QFGUI_SORTEDPOINTSWIDGET_H

#include <QtWidgets/QWidget>

#include <qfrost.h>

QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QPersistentModelIndex)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(SortedPointsModel)

typedef QMap<double, double> DoubleMap;
class SortedPointsWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(DoubleMap values READ values WRITE setValues NOTIFY valuesChanged USER true)
public:
    SortedPointsWidget(const QString &xName, const QString &yName,
                       PhysicalProperty xProp, PhysicalProperty yProp,
                       QWidget *parent);

    const DoubleMap &values() const;
    void setValues(const DoubleMap &data);

signals:
    void valuesChanged(const DoubleMap &values);

private slots:
    void emitValuesChanged();
    void updateButtons();
    void removeSelectedPoints();

private:
    /// Список устойчивых индексов по 0 столбцу всех целиком выбранных строк
    QList<QPersistentModelIndex> selectedRows() const;

    SortedPointsModel *const mModel;

    QTableView *const mView;

    QPushButton *const mNewPoint;
    QPushButton *const mRemovePoint;
};

}

#endif // QFGUI_SORTEDPOINTSWIDGET_H
