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

#ifndef QFGUI_WELCOMEDIALOG_H
#define QFGUI_WELCOMEDIALOG_H

#include <QtWidgets/QDialog>


namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(MainWindow);

class WelcomeDialog : public QDialog
{
    Q_OBJECT
public:
    WelcomeDialog(MainWindow *parent);

    ~WelcomeDialog() {
        enableUndoBinders();
    }

private:
    /// Для вызова при открытии, отключает undo binders связанных виджетов
    void disableUndoBinders();
    /// Для вызова при закрытии, снова включает undo binders связанных виджетов
    void enableUndoBinders();
};

}

#endif // QFGUI_WELCOMEDIALOG_H
