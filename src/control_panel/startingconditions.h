/*
 * Copyright (C) 2010-2012  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_STARTINGCONDITIONS_H
#define QFGUI_STARTINGCONDITIONS_H

#include <QtWidgets/QWidget>

QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(PhysicalPropertySpinBox)

class StartingConditions : public QWidget
{
    Q_OBJECT
public:
    StartingConditions(QWidget *parent = 0);

private:
    PhysicalPropertySpinBox *mTSpinBox;
    QPushButton *mApplyTToSelection;

    PhysicalPropertySpinBox *mT1SpinBox;
    PhysicalPropertySpinBox *mT2SpinBox;
    QPushButton *mApplyTGradToSelection;

    QSpinBox *mVSpinBox;
    QPushButton *mApplyVToSelection;

    bool mSelectionIsEmpty;

public slots:
    void slotApplyTemperature();
    void slotApplyTemperatureGradient();
    void slotApplyThawedPart();
    void updateButtons(bool selectionIsEmpty);
    /// Делает кнопку применения температурного градиента доступной только если
    /// значения в mT1SpinBox и mT2SpinBox отличаются и имеется выделения.
    void updateApplyTGradButton();

signals:
    void signalApplyTemperature(double t);
    void signalApplyTemperatureGradient(double t1, double t2);
    void signalApplyThawedPart(double v);
};

}

#endif // QFGUI_STARTINGCONDITIONS_H
