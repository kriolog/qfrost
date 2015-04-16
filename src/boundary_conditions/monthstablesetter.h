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

#include <QtCore/QSet>

QT_FORWARD_DECLARE_CLASS(QSignalMapper)
QT_FORWARD_DECLARE_CLASS(QHBoxLayout)
QT_FORWARD_DECLARE_CLASS(QItemSelection)
QT_FORWARD_DECLARE_CLASS(QItemSelectionModel)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QStackedLayout)
QT_FORWARD_DECLARE_CLASS(QItemSelectionModel)

namespace qfgui {
QT_FORWARD_DECLARE_CLASS(PhysicalPropertySpinBox)
QT_FORWARD_DECLARE_CLASS(MonthsTableExpander)

/**
 * Виджет для batch-установки значений в MonthsTableModel.
 * Поддерживает неограниченное кол-во секторов с данными (MonthsTableExpander).
 *
 * Если ничего не выделено, показывает по кнопке "установить всё" на сектор.
 * Если есть выделение, показывает кнопки "установить выбранное" - они выделят
 * соответствующие величины за выбранные месяца.
 */
class MonthsTableSetter : public QWidget
{
    Q_OBJECT

public:
    MonthsTableSetter(QItemSelectionModel *selectionModel,
                      Qt::Orientation orientation,
                      QWidget *parent);

private slots:
    /// Пусеает и обрабатывает диалог изменения всех значений для @p sector
    void doSetAll(int sector);

    /// Пускает и обрабатывает диалог изменения @p sector по выделенным месяцам
    void doSetMonthly(int sector);

    /// Обработчик добавления дополнительного сектора данных через @p expander
    void onExpanderAdded(int sector, MonthsTableExpander* expander);

    /// Обработчик изменения активного выделения
    void onSelectionChanged();

private:
    QItemSelectionModel *const mSelectionModel;

    /// Вызывает диалог выбора значений для @p sector.
    /// Возвращает пару значений: был ли диалог принят + что было введено в нём.
    QPair<bool, double> execEditor(int sector, bool setAll);

    const Qt::Orientation mOrientation;

    /// Все наши источники данных
    QHash<int, MonthsTableExpander *> mExpanders;

    /// Лэйоут из (1) кнопок установки всего и (2) кнопок установки по месяцам
    QStackedLayout *mLayout;

    /// Кнопки установки всех значений (посекторно).
    QHash<int, QPushButton *> mSetAllButtons;
    /// Кнопки установки значений в выделенных месяцах (посекторно).
    QHash<int, QPushButton *> mSetMonthlyButtons;

    QSignalMapper *mSetAllMapper;
    QSignalMapper *mSetMonthlyMapper;

    QHBoxLayout *mSetAllButtonsLayout;
    QHBoxLayout *mSetMonthlyButtonsLayout;

    int mSetAllButtonsIndex;
    int mSetMonthlyButtonsIndex;

    /// Выделенные месяцы (0 .. 11)
    QSet<int> mSelectedMonths;
};

}

#endif // QFGUI_MONTHSTABLESETTER_H
