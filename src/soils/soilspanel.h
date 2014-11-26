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

#ifndef QFGUI_SOILSPANEL_H
#define QFGUI_SOILSPANEL_H

#include <QtWidgets/QWidget>

QT_FORWARD_DECLARE_CLASS(QPushButton)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Item)
QT_FORWARD_DECLARE_CLASS(SoilsModel)
QT_FORWARD_DECLARE_CLASS(ControlPanel)
QT_FORWARD_DECLARE_CLASS(Soil)
QT_FORWARD_DECLARE_CLASS(SoilsWidget)

class SoilsPanel : public QWidget
{
    Q_OBJECT
public:
    SoilsPanel(ControlPanel *parent);
    SoilsModel *model();

private:
    SoilsWidget *mSoilsWidget;
    bool mAnyBlockIsSelected;
    bool mAnyClearBlockIsSelected;
    QPushButton *mApplySoil;
    QPushButton *mApplySoilToClear;
    QPushButton *mApplySoilFill;

public slots:
    void updateApplyButton(bool sceneSelectionIsEmpty,
                           bool selectionHasNoClearBlocks);
    void updateApplyButtons();
    void slotApplySoil();

signals:
    void signalApplySoil(const Soil *soil, bool onlyClearBlocks);
    void signalBucketFillApply(const Soil *soil);
};

}

#endif // QFGUI_SOILSPANEL_H
