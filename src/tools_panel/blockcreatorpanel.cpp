/*
 * Copyright (C) 2010-2015  Denis Pesotsky, Maxim Torgonsky
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

#include <tools_panel/blockcreatorpanel.h>

#include <cmath>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>

#include <smartdoublespinbox.h>
#include <tools_panel/rectangulartoolpanel.h>
#include <units/units.h>
#include <units/physicalpropertyspinbox.h>

using namespace qfgui;

static QDoubleSpinBox *createCommonRatioSpinBox(QWidget *parent = NULL)
{
    QDoubleSpinBox *result = new SmartDoubleSpinBox(parent);

    result->setRange(1, 2);
    result->setDecimals(3);
    result->setSingleStep(0.005);
    result->setValue(1);

    return result;
}

BlockCreatorPanel::BlockCreatorPanel(QWidget *parent)
    : QWidget(parent)
    , mWidthSpinBox(PhysicalPropertySpinBox::createBlockSizeSpinBox(this))
    , mWidthQSpinBox(createCommonRatioSpinBox(this))
    , mHeightSpinBox(PhysicalPropertySpinBox::createBlockSizeSpinBox(this))
    , mHeightQSpinBox(createCommonRatioSpinBox(this))
    , mSettings(new BlockCreatorSettings(this))
    , mRectangularToolPanel(new RectangularToolPanel(this, true, mSettings))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());

    connect(mWidthSpinBox, SIGNAL(valueChanged(double)), SLOT(slotSetBlocksSize()));
    connect(mHeightSpinBox, SIGNAL(valueChanged(double)), SLOT(slotSetBlocksSize()));
    slotSetBlocksSize();

    connect(mWidthQSpinBox, SIGNAL(valueChanged(double)), SLOT(slotSetBlocksQ()));
    connect(mHeightQSpinBox, SIGNAL(valueChanged(double)), SLOT(slotSetBlocksQ()));
    slotSetBlocksQ();

    const QString infinityTipSuffix = " " + tr("(for 1D use 0 \342\200\224 will be infinite)");
    mWidthSpinBox->setToolTip(tr("Width of first block") + infinityTipSuffix);
    mHeightSpinBox->setToolTip(tr("Height of first block") + infinityTipSuffix);

    mWidthQSpinBox->setToolTip(tr("Geometric ratio for x"));
    mHeightQSpinBox->setToolTip(tr("Geometric ratio for y"));

    SettingsBox *xBox = new SettingsBox(tr("Hor. Progression"), this);
    SettingsBox *yBox = new SettingsBox(tr("Vert. Progression"), this);

    xBox->addRow(tr("&b<sub>1</sub>"), mWidthSpinBox);
    xBox->addRow(tr("&q"), mWidthQSpinBox);

    yBox->addRow(tr("&b<sub>1</sub>"), mHeightSpinBox);
    yBox->addRow(tr("&q"), mHeightQSpinBox);

    mainLayout->addWidget(xBox);
    mainLayout->addWidget(yBox);

    mainLayout->addWidget(mRectangularToolPanel);

    SettingsBox *miscBox = new SettingsBox(tr("Miscellaneous"), this);
    QCheckBox *mustChangePolygons = new QCheckBox(tr("Change\n&polygons"), miscBox);
    QString s;
    s = tr("If checked, applying tool will append rectangular boundary polygon.<br> Else will not change any polygons.");
    mustChangePolygons->setToolTip(s);
    connect(mustChangePolygons, SIGNAL(stateChanged(int)),
            SLOT(slotSetMustChangePolygons(int)));
    mustChangePolygons->setCheckState(Qt::Checked);
    miscBox->addRow(mustChangePolygons);
    mainLayout->addWidget(miscBox);
    // Иначе при большой высоте miscBox в некоторых темах показывается внизу.
    mainLayout->addStretch(1);
}

void BlockCreatorPanel::slotSetBlocksSize()
{
    mSettings->setBlocksSize(QSizeF(mWidthSpinBox->value(),
                                    mHeightSpinBox->value()));
}

void BlockCreatorPanel::slotSetBlocksQ()
{
    mSettings->setBlocksQ(QSizeF(mWidthQSpinBox->value(),
                                 mHeightQSpinBox->value()));
}

void BlockCreatorPanel::slotSetMustChangePolygons(int state)
{
    mSettings->setMustChangePolygons(state != 0);
}
