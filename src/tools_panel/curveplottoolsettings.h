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

#ifndef QFGUI_CURVEPLOTTOOLSETTINGS_H
#define QFGUI_CURVEPLOTTOOLSETTINGS_H

#include <tools_panel/toolsettings.h>

namespace qfgui {

class CurvePlotToolSettings :  public ToolSettings
{
    Q_OBJECT
public:
    CurvePlotToolSettings(QObject *parent);

    Qt::Orientation orientation() const { return mOrientation; }

public slots:
    void setOrientation(int orientation) {
        Q_ASSERT(orientation == Qt::Horizontal || orientation == Qt::Vertical);
        mOrientation = static_cast<Qt::Orientation>(orientation); 
    }

private:
    Qt::Orientation mOrientation;
};
}

#endif // QFGUI_CURVEPLOTTOOLSETTINGS_H
