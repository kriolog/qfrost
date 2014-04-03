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

#ifndef QFGUI_SETTINGSBOX_H
#define QFGUI_SETTINGSBOX_H

#include <QtWidgets/QGroupBox>

QT_FORWARD_DECLARE_CLASS(QFormLayout)

namespace qfgui
{

class SettingsBox : public QGroupBox
{
public:
    SettingsBox(const QString &title, QWidget *parent = 0);
    void addRow(const QString &labelText, QWidget *field);
    void addRow(const QString &labelText, QLayout *field);
    void addRow(QWidget *field);
    void addSpacing();
    QWidget *labelForField(QWidget *field) const;
    QWidget *labelForField(QLayout *field) const;
private:
    QFormLayout *mLayout;
};

}

#endif // QFGUI_SETTINGSBOX_H
