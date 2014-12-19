/*
 * Copyright (C) 2010-2014  Denis Pesotsky
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

#ifndef QFGUI_RECTANGULARTOOLPANEL_H
#define QFGUI_RECTANGULARTOOLPANEL_H

#include <tools_panel/settingsbox.h>

#include <qfrost.h>
#include <tools_panel/rectangulartoolsettings.h>

QT_FORWARD_DECLARE_CLASS(QButtonGroup)
QT_FORWARD_DECLARE_CLASS(QDoubleSpinBox)

namespace qfgui
{

class RectangularToolPanel : public SettingsBox
{
    Q_OBJECT
public:
    RectangularToolPanel(QWidget *parent, bool showHeader = false,
                         RectangularToolSettings *settings = NULL);
    ToolSettings *toolSettings() {
        return mSettings;
    }

private:
    QDoubleSpinBox *mRectX;
    QDoubleSpinBox *mRectY;

    QDoubleSpinBox *mRectWidth;
    QDoubleSpinBox *mRectHeight;

    QButtonGroup *mBasepoints;

    void updateSizesMaximums();

    /// Нужно высылать сигналы об изменении геометрии.
    bool mMustEmitRectChanges;

    RectangularToolSettings *mSettings;

    Qt::Corner selectedBasepoint() const;


private slots:
    /************************** Изменяют здесь ********************************/
    void slotChangeSize(bool needThisSignal);
    void slotChangeBasepointPos(bool needThisSignal);

    /*********************** Изменяют mSettings ******************************/
    void slotSetSize();
    void slotSetBasepointPos();
    void slotSetBasepoint();

    void setGeometrySettingsEnabled(bool enabled);

};

}

#endif // QFGUI_RECTANGULARTOOLPANEL_H
