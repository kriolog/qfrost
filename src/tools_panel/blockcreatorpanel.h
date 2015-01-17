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

#ifndef QFGUI_BLOCKCREATORPANEL_H
#define QFGUI_BLOCKCREATORPANEL_H

#include <QtWidgets/QWidget>

#include <qfrost.h>
#include <tools_panel/blockcreatorsettings.h>

QT_FORWARD_DECLARE_CLASS(QButtonGroup)
QT_FORWARD_DECLARE_CLASS(QDoubleSpinBox)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(RectangularToolPanel)

class BlockCreatorPanel : public QWidget
{
    Q_OBJECT
public:
    BlockCreatorPanel(QWidget *parent);
    ToolSettings *toolSettings() {
        return mSettings;
    }

signals:
    /// Выбран новый желаемый размер блоков
    void signalChangeBlocksSize(const QSizeF &blockSize);

    /// Выбран новый коэффициент геометрической прогресии
    void signalChangeBlocksQ(const QSizeF &newQ);

private:
    QDoubleSpinBox *mWidthSpinBox;
    QDoubleSpinBox *mWidthQSpinBox;

    QDoubleSpinBox *mHeightSpinBox;
    QDoubleSpinBox *mHeightQSpinBox;

    BlockCreatorSettings *mSettings;

    RectangularToolPanel *mRectangularToolPanel;

private slots:
    void slotSetBlocksSize();
    void slotSetBlocksQ();
    void slotSetMustChangePolygons(int state);
};

}

#endif // QFGUI_BLOCKCREATORPANEL_H
