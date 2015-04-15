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

#ifndef QFGUI_MONTHSTABLEEXPANDER_H
#define QFGUI_MONTHSTABLEEXPANDER_H

#include <QtWidgets/QWidget>

#include <qfrost.h>

namespace qfgui {

QT_FORWARD_DECLARE_CLASS(MonthsTableModel)

/// Класс, добавляющий ещё один сектор к MonthsTableWidget (и MonthsTableModel).
/// QDataWidgetMapper не даёт привязаться к нескольким свойствам одного виджета,
/// поэтому был добавлен этот класс. Бонус - неограниченное число доп. секторов.
class MonthsTableExpander : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QList<double> values
               READ values
               WRITE setValues
               NOTIFY valuesChanged
               USER true)

    Q_PROPERTY(int physicalProperty
               READ physicalProperty
               WRITE setPhysicalProperty
               NOTIFY physicalPropertyChanged)

public:
    MonthsTableExpander(MonthsTableModel *model,
                        const QString &valueName,
                        QWidget *parent);

    int modelSector() const { return mModelSector; }

    double value(int monthNum) const {
        Q_ASSERT(monthNum < mData.size());
        return mData.at(monthNum);
    }

    const QString &valueName() const { return mValueName; }

    const QList<double> &values() const { return mData; }
    const QString &headerText() const { return mValueNameWithSuffix; }
    int physicalProperty() const { return mPhysicalProperty; }

    bool setValue(int monthNum, double d);
    bool setValues(const QList<double> &data);
    bool setPhysicalProperty(int p);

signals:
    void valuesChanged(const QList<double> &data); ///< Изменение хотя бы одного значения
    void headerTextChanged(); ///< Изменение текста для заголовка
    void physicalPropertyChanged(int p); ///< Изменение физ. свойства

    void valueChanged(int monthNum); ///< Изменение одного значения
    void valuesReplaced(); ///< Изменение ВСЕХ значений

private slots:
    void updateHeaderText();

private:
    MonthsTableModel *const mModel;

    QList<double> mData;

    const QString mValueName;
    QString mValueNameWithSuffix;

    PhysicalProperty mPhysicalProperty;

    const int mModelSector;
};

}

#endif // QFGUI_MONTHSTABLEEXPANDER_H
