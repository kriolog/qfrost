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

using namespace qfgui;

BlockCreatorPanel::BlockCreatorPanel(QWidget *parent)
    : QWidget(parent)
    , mWidthSpinBox(new SmartDoubleSpinBox(this))
    , mWidthQSpinBox(new SmartDoubleSpinBox(this))
    , mHeightSpinBox(new SmartDoubleSpinBox(this))
    , mHeightQSpinBox(new SmartDoubleSpinBox(this))
    , mSettings(new BlockCreatorSettings(this))
    , mRectangularToolPanel(new RectangularToolPanel(this, true, mSettings))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());

    connect(mWidthSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetBlocksSize()));
    connect(mHeightSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetBlocksSize()));

    connect(mWidthQSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetBlocksQ()));
    connect(mHeightQSpinBox, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetBlocksQ()));

    mWidthSpinBox->setRange(0, 10);
    mWidthSpinBox->setDecimals(QFrost::meterDecimalsBlockSize);
    mWidthSpinBox->setValue(0.1);
    mWidthSpinBox->setSingleStep(0.01);
    mWidthSpinBox->setSuffix(Units::meterSuffix());
    mWidthSpinBox->setSpecialValueText(tr("\342\210\236"));
    mWidthSpinBox->setToolTip(tr("Width of first block"));

    mHeightSpinBox->readProperties(mWidthSpinBox);
    mHeightSpinBox->setToolTip(tr("Height of first block"));

    mWidthQSpinBox->setRange(1, 2);
    mWidthQSpinBox->setDecimals(3);
    mWidthQSpinBox->setValue(1);
    mWidthQSpinBox->setSingleStep(0.005);
    mWidthQSpinBox->setToolTip(tr("Geometric ratio for x"));

    mHeightQSpinBox->readProperties(mWidthQSpinBox);
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
