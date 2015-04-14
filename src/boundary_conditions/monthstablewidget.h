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

#ifndef QFGUI_MONTHSTABLEWIDGET_H
#define QFGUI_MONTHSTABLEWIDGET_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QAbstractItemDelegate>

#include <qfrost.h>

QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QModelIndex)

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(PhysicalPropertySpinBox)
QT_FORWARD_DECLARE_CLASS(MonthsTableModel)

class MonthsTableWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QList<double> values
               READ values
               WRITE setValues
               NOTIFY valuesChanged
               USER true)
    Q_PROPERTY(int physicalProperty
               READ physicalProperty
               WRITE setPhysicalProperty)
public:
    MonthsTableWidget(const QString &valueName, QWidget *parent);
    QList<double> values() const;
    void setValues(const QList<double> &data);

    int physicalProperty();

public slots:
    void setPhysicalProperty(int p);

signals:
    void valuesChanged();

private:
    MonthsTableModel *qfModel();
    const MonthsTableModel *qfModel() const;

    QTableView *mView;

    /// Спинбокс, с помощью которого пользователь задаёт значения на выделение
    PhysicalPropertySpinBox *mDataSetterSpinbox;

    /// Кнопка, открывающая диалог с вышеупомянутым спинбоксом
    QPushButton *mOpenDataSetter;

    /// Индесы, которые будут изменены при использовании batchSetData()
    QList <QModelIndex> indexesToBatchSet() const;

private slots:
    /// Делает @a mOpenDataSetter активной, только если есть выделение
    void updateDataSetterButton();

    /// Применяет значение, записанное в @a mDataSetterSpinbox к выделению
    void batchSetData();

    /**
     * Если значения всех выбранных ячеек одинаковы, устанавливает значение
     * в @a mDataSetter равным этому значению. Иначе -- равным нулю
     */
    void setDataSetterValue();
};

}

#endif // QFGUI_MONTHSTABLEWIDGET_H
