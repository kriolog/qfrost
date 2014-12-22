/*
 * Copyright (C) 2014  Denis Pesotsky
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

#include "curveplotdialog.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include <units/physicalpropertyspinbox.h>
#include <graphicsviews/block.h>

using namespace qfgui;

CurvePlotDialog::CurvePlotDialog(Block *block,
                                 Qt::Orientation orientation,
                                 QWidget *parent)
    : QDialog(parent)  
    , mMinT(new PhysicalPropertySpinBox(Temperature, this))
    , mMaxT(new PhysicalPropertySpinBox(Temperature, this))
    , mMinCoord(PhysicalPropertySpinBox::createSceneCoordinateSpinBox())
    , mMaxCoord(PhysicalPropertySpinBox::createSceneCoordinateSpinBox())
    , mSlice(block->slice(orientation))
    , mTemperatures()
    , mThawedParts()
    , mTransitionTemperatures()
    , mCoordsMain()
    , mCoordsNormal()
    , mDialogButtons(new QDialogButtonBox(QDialogButtonBox::Close, this))
{
    Q_ASSERT(!mSlice.isEmpty());

    mTemperatures.reserve(mSlice.size());
    mThawedParts.reserve(mSlice.size());
    mTransitionTemperatures.reserve(mSlice.size());
    mCoordsMain.reserve(mSlice.size());
    mCoordsNormal.reserve(mSlice.size());

    foreach (Block *block, mSlice) {
        mTemperatures.append(block->soilBlock()->temperature());
        mThawedParts.append(block->soilBlock()->temperature());
        mTransitionTemperatures.append(block->soilBlock()->temperature());

        const QPointF center = block->metersCenter();
        mCoordsMain.append(orientation == Qt::Horizontal
                           ? center.x() : center.y());
        mCoordsNormal.append(orientation == Qt::Horizontal
                             ? center.y() : center.x());

        block->showArrows(); // TMP
    }

    mMinCoord->setMinimum(mCoordsMain.first());
    if (mMinCoord->minimum() > 0.0 && mMinCoord->minimum() < 1.0) {
        mMinCoord->setMinimum(0.0);
    }
    mMaxCoord->setMaximum(mCoordsMain.last());

    mMinCoord->setValue(mMinCoord->minimum());
    mMaxCoord->setValue(mMaxCoord->maximum());

    //: automatically set t limits
    QPushButton *tAutoLimit = new QPushButton(tr("Auto t"));
    //connect(tAutoLimit, SIGNAL(clicked()), this, SLOT(autoLimitsT()));
    //: automatically set z limits
    QPushButton *zAutoLimit = new QPushButton(tr("Auto z"));
    //connect(zAutoLimit, SIGNAL(clicked()), this, SLOT(autoLimitsZ()));

    QHBoxLayout *tLimits = new QHBoxLayout;
    tLimits->addWidget(mMinT);
    tLimits->addWidget(mMaxT);
    tLimits->addWidget(tAutoLimit);
    QHBoxLayout *zLimits = new QHBoxLayout;
    zLimits->addWidget(mMinCoord);
    zLimits->addWidget(mMaxCoord);
    zLimits->addWidget(zAutoLimit);

    QFormLayout *limits = new QFormLayout;
    limits->addRow(tr("t:"), tLimits);
    limits->addRow(tr("z:"), zLimits);

    QCheckBox *plotTemperatesBox = new QCheckBox(tr("Draw t/v"));
    connect(plotTemperatesBox, SIGNAL(toggled(bool)),
            mMinT, SLOT(setEnabled(bool)));
    connect(plotTemperatesBox, SIGNAL(toggled(bool)),
            mMaxT, SLOT(setEnabled(bool)));
    connect(plotTemperatesBox, SIGNAL(toggled(bool)),
            mMaxT, SLOT(setEnabled(bool)));
    plotTemperatesBox->setChecked(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(plotTemperatesBox);
    mainLayout->addLayout(limits);
    mainLayout->addWidget(mDialogButtons);
}
