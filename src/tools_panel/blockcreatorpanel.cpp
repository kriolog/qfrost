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

    mWidthSpinBox->setToolTip(tr("The first term of width progression (i.e., initial block's width).\n"
                                 "Zero out (%1) to occupy whole tool width (1D vertical model).")
                              .arg(mWidthSpinBox->specialValueText()));
    mHeightSpinBox->setToolTip(tr("The first term of height progression (i.e., initial block's height).\n"
                                  "Zero out (%1) to occupy whole tool height (1D horizontal model).")
                               .arg(mHeightSpinBox->specialValueText()));

    mWidthQSpinBox->setToolTip(tr("Common ratio of width progression.\n"
                                  "For fixed width use unity (minumum)."));
    mHeightQSpinBox->setToolTip(tr("Common ratio of height progression.\n"
                                   "For fixed height use unity (minimum)."));

    SettingsBox *xBox = new SettingsBox(tr("Width Progression"), this);
    SettingsBox *yBox = new SettingsBox(tr("Height Progression"), this);

    xBox->addRow(tr("&b<sub>1</sub>"), mWidthSpinBox);
    xBox->addRow(tr("&q"), mWidthQSpinBox);

    yBox->addRow(tr("&b<sub>1</sub>"), mHeightSpinBox);
    yBox->addRow(tr("&q"), mHeightQSpinBox);

    mainLayout->addWidget(xBox);
    mainLayout->addWidget(yBox);

    mainLayout->addWidget(mRectangularToolPanel);

    SettingsBox *miscBox = new SettingsBox(tr("Miscellaneous"), this);
    QCheckBox *mustChangePolygons = new QCheckBox(tr("&Append polygon"), miscBox);
    QString s;
    s = tr("If enabled, will append rectangular boundary polygon when applying tool.\n"
           "Else will create blocks without modifying (or creating) any polygons at all.");
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
    mSettings->setMustChangePolygons(state != Qt::Unchecked);
}
