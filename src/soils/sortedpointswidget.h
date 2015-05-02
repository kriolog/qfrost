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
#include <QtWidgets/QDialog>

#include <qfrost.h>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QPersistentModelIndex)
QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(SortedPointsModel)
QT_FORWARD_DECLARE_CLASS(PhysicalPropertySpinBox)

typedef QMap<double, double> DoubleMap;
class SortedPointsWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(DoubleMap values READ values WRITE setValues NOTIFY valuesChanged USER true)
public:
    SortedPointsWidget(PhysicalProperty xProp, const QString& xName, const QString& xNameFull,
                       PhysicalProperty yProp, const QString& yName, const QString& yNameFull,
                       QWidget* parent);

    const DoubleMap &values() const;
    void setValues(const DoubleMap &data);

signals:
    void valuesChanged(const DoubleMap &values);

private slots:
    void emitValuesChanged();
    void updateButtons();

    void openNewPointDialog();
    void removeSelectedPoints();

private:
    /// Список устойчивых индексов по 0 столбцу всех целиком выбранных строк
    QList<QPersistentModelIndex> selectedRows() const;

    SortedPointsModel *const mModel;

    QTableView *const mView;

    QPushButton *const mNewPoint;
    QPushButton *const mRemovePoint;

    const QString mXNameFull;
    const QString mYNameFull;
};

/**
 * Диалог для добавления новой точки в SortedPointsModel.
 * Соблюдает валидность добавляемых значений (в соответствие с текущими).
 */
class SortedPointsNewPointDialog : public QDialog
{
    Q_OBJECT
public:
    SortedPointsNewPointDialog(const SortedPointsModel *model,
                               const QString &xName,
                               const QString &yName,
                               QWidget * parent = 0);

    double x() const;
    double y() const;

private slots:
    void checkInput();

private:
    const SortedPointsModel *mModel;

    PhysicalPropertySpinBox *const mSpinBoxX;
    PhysicalPropertySpinBox *const mSpinBoxY;

    QLabel *const mInvalidInputNotice;

    QDialogButtonBox *const mButtons;

    const QString mXNameForLabel;
    const QString mYNameForLabel;

    const QString kLabelMaskInvalidYBothBounds;
};

}

#endif // QFGUI_SORTEDPOINTSWIDGET_H
