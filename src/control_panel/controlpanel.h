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

#ifndef QFGUI_CONTROLPANEL_H
#define QFGUI_CONTROLPANEL_H

#include <QtWidgets/QDockWidget>
#include <qfrost.h>

QT_FORWARD_DECLARE_CLASS(QGraphicsItem)
QT_FORWARD_DECLARE_CLASS(QTabWidget)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Block)
QT_FORWARD_DECLARE_CLASS(ComputationControl)
QT_FORWARD_DECLARE_CLASS(Soil)
QT_FORWARD_DECLARE_CLASS(SoilsPanel)
QT_FORWARD_DECLARE_CLASS(StartingConditions)
QT_FORWARD_DECLARE_CLASS(MainWindow)
QT_FORWARD_DECLARE_CLASS(BoundaryConditionsPanel)

class ControlPanel : public QDockWidget
{
    Q_OBJECT
public:
    ControlPanel(MainWindow *parent);
    SoilsPanel *soilsPanel();
    BoundaryConditionsPanel *boundaryConditionsPanel();
    ComputationControl *computationControl();
    StartingConditions *startingConditions();

    QString computationTabText() const {
        return tr("Computations");
    }

private:
    /// Стиль блоков, выбранный во вкладке вычислений
    QFrost::BlockStyle mStyleForComputations;

    QTabWidget *mControls;
    ComputationControl *mComputation;
    SoilsPanel *mSoilsPanel;
    StartingConditions *mStartingConditions;
    BoundaryConditionsPanel *mBoundaryConditions;

signals:
    void signalChangeBlockStyle(QFrost::BlockStyle);

public slots:
    void slotTabChanged(int index);
    void slotComputationStateChanged(bool computationIsNowOn);
    /// Слот для реагирования на смену стиля блоков во вкладке вычислений
    void changeBlocksStyleForComputation(int style);
};

}

#endif // QFGUI_CONTROLPANEL_H
