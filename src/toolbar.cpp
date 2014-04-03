/*
 * Copyright (C) 2012  Denis Pesotsky
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

#include "toolbar.h"

#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QToolButton>

using namespace qfgui;

ToolBar::ToolBar(const QString &title, QWidget *parent)
    : QToolBar(title, parent)
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    setMovable(false);
    setFloatable(false);
    setAllowedAreas(Qt::TopToolBarArea);

    // Делаем мелкий шрифт в тулбаре (как в KDE)
    /*QFont f = font();
    f.setPointSize(f.pointSize() - 1);
    setFont(f);*/
}

void ToolBar::addAction(QAction *action, bool showText)
{
    QToolBar::addAction(action);
    if (showText) {
        QToolButton *button = qobject_cast<QToolButton * >(widgetForAction(action));
        Q_ASSERT(button != NULL);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
}
