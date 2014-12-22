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

#ifndef QFGUI_CURVEPLOTTOOLPANEL_H
#define QFGUI_CURVEPLOTTOOLPANEL_H

#include <tools_panel/settingsbox.h>

QT_FORWARD_DECLARE_CLASS(QRadioButton)

namespace qfgui {

QT_FORWARD_DECLARE_CLASS(ToolSettings)
QT_FORWARD_DECLARE_CLASS(CurvePlotToolSettings)

class CurvePlotToolPanel : public SettingsBox
{
    Q_OBJECT
public:
    CurvePlotToolPanel(QWidget* parent = NULL);

    ToolSettings *toolSettings() {
        return mSettings;
    }

private:
    QRadioButton *mVerticalSlice;
    QRadioButton *mHorizontalSlice;

    ToolSettings *mSettings;
};
}

#endif // QFGUI_CURVEPLOTTOOLPANEL_H
