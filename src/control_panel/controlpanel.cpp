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

#include <control_panel/controlpanel.h>

#include <QtWidgets/QTabWidget>

#include <core/phased.h>

#include <graphicsviews/block.h>
#include <boundary_conditions/boundaryconditionspanel.h>
#include <control_panel/computationcontrol.h>
#include <control_panel/startingconditions.h>
#include <soils/soilspanel.h>
#include <mainwindow.h>

using namespace qfgui;

ControlPanel::ControlPanel(MainWindow *parent): QDockWidget(tr("Control Panel"), parent),
    mStyleForComputations(),
    mControls(new QTabWidget(this)),
    mComputation(new ComputationControl(this)),
    mSoilsPanel(NULL),
    mStartingConditions(new StartingConditions(this)),
    mBoundaryConditions(new BoundaryConditionsPanel(this))
{
    mSoilsPanel = new SoilsPanel(this);
    setMaximumWidth(300);
    setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea +
                                        Qt::RightDockWidgetArea));
    setFeatures(DockWidgetFeatures(QDockWidget::DockWidgetMovable +
                                   QDockWidget::DockWidgetFloatable));

    mControls->setTabPosition(QTabWidget::West);
    mControls->setUsesScrollButtons(false);

    mControls->addTab(mSoilsPanel, tr("Soils"));
    mControls->addTab(mBoundaryConditions, tr("Boundary cond."));
    mControls->addTab(mStartingConditions, tr("Initial cond."));
    mControls->addTab(mComputation, computationTabText());

    connect(mControls, SIGNAL(currentChanged(int)), SLOT(slotTabChanged(int)));

    setWidget(mControls);
}

void ControlPanel::slotTabChanged(int index)
{
    QFrost::BlockStyle newStyle;
    switch (index) {
    case 0:
        newStyle = QFrost::blockShowsSoil;
        break;
    case 1:
        newStyle = QFrost::blockShowsBoundaryConditions;
        break;
    case 2:
        newStyle = QFrost::blockShowsTemperature;
        break;
    case 3:
        newStyle = mStyleForComputations;
        break;
    default:
        Q_ASSERT(false);
        newStyle = QFrost::blockShowsSoil;
        break;
    }
    emit signalChangeBlockStyle(newStyle);
}

void ControlPanel::slotComputationStateChanged(bool computationIsNowOn)
{
    mSoilsPanel->setDisabled(computationIsNowOn);
    mBoundaryConditions->setDisabled(computationIsNowOn);
    mComputation->onComputationStateChanged(computationIsNowOn);
}

void ControlPanel::changeBlocksStyleForComputation(int style)
{

    mStyleForComputations = QFrost::BlockStyle(style);
    if (mControls->currentIndex() == 3) {
        emit signalChangeBlockStyle(mStyleForComputations);
    }
}

BoundaryConditionsPanel *ControlPanel::boundaryConditionsPanel()
{
    return mBoundaryConditions;
}

SoilsPanel *ControlPanel::soilsPanel()
{
    return mSoilsPanel;
}

ComputationControl *ControlPanel::computationControl()
{
    return mComputation;
}

StartingConditions *ControlPanel::startingConditions()
{
    return mStartingConditions;
}
