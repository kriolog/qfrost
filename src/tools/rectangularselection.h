/*
 * Copyright (C) 2011-2012  Denis Pesotsky
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

#ifndef QFGUI_RECTANGULARSELECTION_H
#define QFGUI_RECTANGULARSELECTION_H

#include <tools/rectangulartool.h>

namespace qfgui
{

class RectangularSelection: public RectangularTool
{
    Q_OBJECT
public:
    RectangularSelection(ToolSettings *settings = NULL):
        RectangularTool(settings) { }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void apply(bool alt) {
        Q_UNUSED(alt)
        /* Для выделения нет понятия "применить".
         * Точнее, оно применяется сразу после остановки изменений. */
    }

protected:
    /// Сбрасываем выделение
    void onStartChange();
    /// Применяем выделение
    void onStopChange();

    void onSceneHasChanged();
    void beforeSceneChange();

private:
    void selectBlocks();
};

}

#endif // QFGUI_RECTANGULARSELECTION_H
