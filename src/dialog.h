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
 */

#ifndef QFGUI_DIALOG_H
#define QFGUI_DIALOG_H

#include <QtCore/QString>

#include <QtWidgets/QFileDialog>

namespace qfgui
{

class Dialog
{
public:
    static QString getOpenFileName(const QString &dirSettingName,
                                   QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &wantedPathOrName = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   QFileDialog::Options options = 0);

    static QString getSaveFileName(const QString &dirSettingName,
                                   QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &wantedPathOrName = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   QFileDialog::Options options = 0);

    static QString getOpenFileName(QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &wantedPathOrName = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   QFileDialog::Options options = 0) {
        return getOpenFileName(defaultSettingName, parent, caption,
                               wantedPathOrName, filter,
                               selectedFilter, options);
    }

    static QString getSaveFileName(QWidget *parent = 0,
                                   const QString &caption = QString(),
                                   const QString &wantedPathOrName = QString(),
                                   const QString &filter = QString(),
                                   QString *selectedFilter = 0,
                                   QFileDialog::Options options = 0) {
        return getSaveFileName(defaultSettingName, parent, caption,
                               wantedPathOrName, filter,
                               selectedFilter, options);
    }

private:
    static const QString defaultSettingName;

    Dialog() {}
};

}

#endif // QFGUI_DIALOG_H
