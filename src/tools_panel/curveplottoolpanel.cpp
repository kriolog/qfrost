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

#include "curveplottoolpanel.h"

#include <tools_panel/curveplottoolsettings.h>

#include <QRadioButton>
#include <QButtonGroup>

using namespace qfgui;

CurvePlotToolPanel::CurvePlotToolPanel(QWidget *parent):
    SettingsBox(tr("Slice Orientation"), parent),
    mVerticalSlice(new QRadioButton(tr("&Vertical"))),
    mHorizontalSlice(new QRadioButton(tr("&Horizontal"))),
    mSettings(new CurvePlotToolSettings(this))
{
    QButtonGroup *sliceOrientations = new QButtonGroup(this);
    sliceOrientations->addButton(mVerticalSlice, Qt::Vertical);
    sliceOrientations->addButton(mHorizontalSlice, Qt::Horizontal);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    mVerticalSlice->setChecked(true);

    addRow(mVerticalSlice);
    addRow(mHorizontalSlice);

    connect(sliceOrientations, SIGNAL(buttonClicked(int)),
            mSettings, SLOT(setOrientation(int)));
}
