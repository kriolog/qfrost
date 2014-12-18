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

#include <control_panel/startingconditions.h>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QGroupBox>

#include <units/physicalpropertyspinbox.h>

using namespace qfgui;

StartingConditions::StartingConditions(QWidget *parent)
    : QWidget(parent)
    , mTSpinBox(new PhysicalPropertySpinBox(Temperature, this))
    , mApplyTToSelection(new QPushButton(tr("&Apply to selection"), this))
    , mT1SpinBox(new PhysicalPropertySpinBox(Temperature, this))
    , mT2SpinBox(new PhysicalPropertySpinBox(Temperature, this))
    , mApplyTGradToSelection(new QPushButton(tr("A&pply to selection"), this))
    , mVSpinBox(new QSpinBox(this))
    , mApplyVToSelection(new QPushButton(tr("App&ly to selection"), this))
{
    mT2SpinBox->setValue(10.0);

    connect(mT1SpinBox, SIGNAL(valueChanged(double)), SLOT(updateApplyTGradButton()));
    connect(mT2SpinBox, SIGNAL(valueChanged(double)), SLOT(updateApplyTGradButton()));

    mVSpinBox->setRange(0, 100);
    mVSpinBox->setValue(0);
    mVSpinBox->setSingleStep(1);
    mVSpinBox->setSuffix(tr("%"));

    updateButtons(true);
    connect(mApplyTToSelection, SIGNAL(clicked()),
            SLOT(slotApplyTemperature()));
    connect(mApplyTGradToSelection, SIGNAL(clicked()),
            SLOT(slotApplyTemperatureGradient()));
    connect(mApplyVToSelection, SIGNAL(clicked()),
            SLOT(slotApplyThawedPart()));

    QGroupBox *tBox = new QGroupBox(tr("Temperature"), this);
    QFormLayout *tFormLayout = new QFormLayout(tBox);
    tFormLayout->setRowWrapPolicy(QFormLayout::WrapLongRows);
    tFormLayout->addRow(tr("&T"), mTSpinBox);
    tFormLayout->addRow(mApplyTToSelection);

    QGroupBox *tGradBox = new QGroupBox(tr("Temperature Gradient"), this);
    QFormLayout *tGradFormLayout = new QFormLayout(tGradBox);
    tGradFormLayout->setRowWrapPolicy(QFormLayout::WrapLongRows);
    mT1SpinBox->setToolTip(tr("Temperature at top of selection"));
    mT2SpinBox->setToolTip(tr("Temperature at bottom of selection"));
    tGradFormLayout->addRow(tr("T<sub>&1</sub>"),
                            mT1SpinBox);
    tGradFormLayout->addRow(tr("T<sub>&2</sub>"),
                            mT2SpinBox);
    tGradFormLayout->addRow(mApplyTGradToSelection);

    QGroupBox *vBox = new QGroupBox(tr("Thawed Volume Fraction"), this);
    QFormLayout *vFormLayout = new QFormLayout(vBox);
    mVSpinBox->setToolTip(tr("Thawed volume fraction"));
    vFormLayout->addRow(tr("&V<sub>th</sub>"), mVSpinBox);
    vFormLayout->addRow(mApplyVToSelection);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addStretch();
    mainLayout->addWidget(tBox);
    mainLayout->addStretch();
    mainLayout->addWidget(tGradBox);
    mainLayout->addStretch();
    mainLayout->addWidget(vBox);
    mainLayout->addStretch();
}

void StartingConditions::slotApplyTemperature()
{
    emit signalApplyTemperature(mTSpinBox->value());
}

void StartingConditions::slotApplyThawedPart()
{
    emit signalApplyThawedPart(double(mVSpinBox->value()) / 100);
}

void StartingConditions::slotApplyTemperatureGradient()
{
    emit signalApplyTemperatureGradient(mT1SpinBox->value(),
                                        mT2SpinBox->value());
}

void StartingConditions::updateButtons(bool selectionIsEmpty)
{
    mSelectionIsEmpty = selectionIsEmpty;
    mApplyTToSelection->setEnabled(!selectionIsEmpty);
    mApplyVToSelection->setEnabled(!selectionIsEmpty);
    updateApplyTGradButton();
}

void StartingConditions::updateApplyTGradButton()
{
    mApplyTGradToSelection->setEnabled(!mSelectionIsEmpty
                                       && mT1SpinBox->value()
                                       != mT2SpinBox->value());
}
