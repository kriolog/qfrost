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

#ifndef QFGUI_TOOLSETTINGS_H
#define QFGUI_TOOLSETTINGS_H

#include <QtCore/QObject>

namespace qfgui
{

/**
 * Абстрактный класс - настройки инструмента.
 * Медиум между виджетом настроек конкретного инструмента и самим инструментом.
 */
class ToolSettings: public QObject
{
    Q_OBJECT
public:
    ToolSettings(QObject *parent): QObject(parent) {

    }
};

}

#endif // QFGUI_TOOLSETTINGS_H
