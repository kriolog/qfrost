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

#ifndef QFGUI_MONTHSTABLESETTER_H
#define QFGUI_MONTHSTABLESETTER_H

#include <QtWidgets/QWidget>

QT_FORWARD_DECLARE_CLASS(QSignalMapper)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QItemSelection)
QT_FORWARD_DECLARE_CLASS(QItemSelectionModel)
QT_FORWARD_DECLARE_CLASS(QPushButton)

namespace qfgui {
QT_FORWARD_DECLARE_CLASS(PhysicalPropertySpinBox)
QT_FORWARD_DECLARE_CLASS(MonthsTableExpander)

/**
 * Виджет для batch-установки значений в MonthsTableModel.
 * Поддерживает неограниченное кол-во секторов с данными (MonthsTableExpander).
 *
 * Если ничего не выделено, показывает по кнопке "установить всё" на сектор.
 * Если есть выделение, показывает кнопку "установить выбранное", которую можно
 * нажать, только если выделены данные из одного сектора.
 */
class MonthsTableSetter : public QWidget
{
    Q_OBJECT

public:
    MonthsTableSetter(QItemSelectionModel *selectionModel,
                      QWidget *parent);

private slots:
    /// Открывает и обрабатывает диалог изменения всех значений для @p sector
    void doSetAll(int sector);

    /// Обработчик добавления дополнительного сектора данных через @p expander
    void onExpanderAdded(int sector, MonthsTableExpander* expander);

    /// Обработчик изменения активного выделения
    void onSelectionChanged(const QItemSelection &selected);

private:
    /// Вызывает диалог выбора значений для @p sector.
    /// Возвращает пару значений: был ли диалог принят + что было введено в нём.
    QPair<bool, double> execEditor(int sector);

    /// Кнопка установки значений для выбранного.
    /// Видна при выделении. Доступна, если выделены значения одного сектора.
    QPushButton *mSetSelectedButton;

    /// Кнопки установки всех значений (посекторно).
    QHash<int, QPushButton *> mSetAllButtons;

    /// Все наши источники данных
    QHash<int, MonthsTableExpander *> mExpanders;

    /// Маппер, разделяющий сигналы от кнопок по значению сектора модели
    QSignalMapper *mSetAllMapper;

    QHBoxLayout *mSetAllButtonsLayout;
};

}

#endif // QFGUI_MONTHSTABLESETTER_H
