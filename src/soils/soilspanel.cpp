/*
 * Copyright (C) 2010-2012  Denis Pesotsky
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

#include <soils/soilspanel.h>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>

#include <soils/soilswidget.h>
#include <soils/soil.h>
#include <control_panel/controlpanel.h>

using namespace qfgui;

SoilsPanel::SoilsPanel(ControlPanel *parent): QWidget(parent),
    mSoilsWidget(NULL),
    mAnyBlockIsSelected(false),
    mAnyClearBlockIsSelected(false),
    mApplySoil(new QPushButton(tr("Apply to selected blocks"))),
    mApplySoilToClear(new QPushButton(tr("Apply to selected clear blocks"))),
    mApplySoilFill(new QPushButton(tr("&Apply using fill")))
{
    mSoilsWidget = new SoilsWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addWidget(mSoilsWidget);
    mainLayout->addWidget(mApplySoil);
    mainLayout->addWidget(mApplySoilToClear);
    mainLayout->addWidget(mApplySoilFill);

    QPushButton *openTableEditor = new QPushButton(tr("&Table Editor"), this);
    mainLayout->addWidget(openTableEditor);
    connect(openTableEditor, SIGNAL(clicked()),
            mSoilsWidget, SLOT(openTableEditor()));

    connect(mApplySoil, SIGNAL(clicked()), SLOT(slotApplySoil()));
    connect(mApplySoilToClear, SIGNAL(clicked()), SLOT(slotApplySoil()));
    connect(mApplySoilFill, SIGNAL(clicked(bool)), SLOT(slotApplySoil()));

    connect(mSoilsWidget, SIGNAL(selectionChanged()),
            SLOT(updateApplyButtons()));
    updateApplyButtons();
    
    const QString shortcutText = tr("Use <b>%1</b> as shortcut.");

    mApplySoil->setShortcut(QKeySequence::InsertParagraphSeparator);
    mApplySoil->setToolTip(tr("Apply choosen soil to all selected blocks.") + "<br/>"
                           + shortcutText.arg(mApplySoil->shortcut().toString()
                                              .replace("Return", "Enter")));
    
    mApplySoilToClear->setShortcut(QKeySequence::InsertLineSeparator);
    mApplySoilToClear->setToolTip(tr("Apply choosen soil to selected blocks without soil.") + "<br/>"
                                  + shortcutText.arg(mApplySoilToClear->shortcut().toString()
                                                     .replace("Return", "Enter")));
    
    mApplySoilFill->setToolTip(tr("Apply choosen soil to blocks with bucket filling."));
}

void SoilsPanel::updateApplyButton(bool sceneSelectionIsEmpty,
                                   bool selectionHasNoClearBlocks
)
{
    mAnyBlockIsSelected = !sceneSelectionIsEmpty;
    mAnyClearBlockIsSelected = !selectionHasNoClearBlocks;
    updateApplyButtons();
}

void SoilsPanel::updateApplyButtons()
{
    const bool oneSoilIsSelected = (mSoilsWidget->selectedItem() != NULL);
    mApplySoil->setEnabled(mAnyBlockIsSelected && oneSoilIsSelected);
    mApplySoilToClear->setEnabled(mAnyClearBlockIsSelected && oneSoilIsSelected);
    mApplySoilFill->setEnabled(oneSoilIsSelected);
}

void SoilsPanel::slotApplySoil()
{
    Q_ASSERT(mSoilsWidget->selectedItem() != NULL);
    Q_ASSERT(qobject_cast<Soil *>(mSoilsWidget->selectedItem()) != NULL);
    const Soil *const soil = qobject_cast<Soil *>(mSoilsWidget->selectedItem());
    if (sender() == mApplySoilFill) {
        emit signalBucketFillApply(soil);
    } else {
        const bool onlyClearBlocks = (sender() == mApplySoilToClear);
        emit signalApplySoil(soil, onlyClearBlocks);
    }
}

SoilsModel *SoilsPanel::model()
{
    Q_ASSERT(qobject_cast< SoilsModel * >(mSoilsWidget->model()) != NULL);
    return qobject_cast< SoilsModel * >(mSoilsWidget->model());
}
