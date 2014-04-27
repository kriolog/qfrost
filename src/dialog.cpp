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

#include <dialog.h>

using namespace qfgui;

#include <QtCore/QSettings>

const QString Dialog::defaultSettingName = "fileDialogsDir";

static QString getOpenSaveFileName(bool open,
                                   const QString &dirSettingName,
                                   QWidget *parent, const QString &caption,
                                   const QString &wantedPathOrName,
                                   const QString &filter,
                                   QString *selectedFilter,
                                   QFileDialog::Options options)
{
    QSettings settings;
    const QFileInfo fileInfo = QFileInfo(wantedPathOrName);
    const bool hasWantedPathOrName = !wantedPathOrName.isEmpty();
    const QString fileName = hasWantedPathOrName
                             ? fileInfo.fileName()
                             : QString();
    const bool hasDirName = hasWantedPathOrName && fileName != wantedPathOrName;
    const QString dir = (!hasDirName || !hasWantedPathOrName)
                        ? settings.value(dirSettingName).toString()
                        : fileInfo.absolutePath();

    QString filePath = dir;
    if (!fileName.isEmpty()) {
        if (!filePath.isEmpty()) {
            filePath.append('/');
        }
        filePath.append(fileName);
    }

    const QString result = open ? QFileDialog::getOpenFileName(parent, caption,
                                                               filePath, filter,
                                                               selectedFilter,
                                                               options)
                                : QFileDialog::getSaveFileName(parent, caption,
                                                               filePath, filter,
                                                               selectedFilter,
                                                               options);

    if (!result.isEmpty()) {
        settings.setValue(dirSettingName,
                          QFileInfo(result).absoluteDir().absolutePath());
    }

    return result;
}

QString Dialog::getOpenFileName(const QString &dirSettingName,
                                QWidget *parent, const QString &caption,
                                const QString &wantedPathOrName,
                                const QString &filter,
                                QString *selectedFilter,
                                QFileDialog::Options options)
{
    return getOpenSaveFileName(true, dirSettingName,
                               parent, caption,
                               wantedPathOrName, filter,
                               selectedFilter, options);
}

QString Dialog::getSaveFileName(const QString &dirSettingName,
                                QWidget *parent,
                                const QString &caption,
                                const QString &wantedPathOrName,
                                const QString &filter,
                                QString *selectedFilter,
                                QFileDialog::Options options)
{
    return getOpenSaveFileName(false, dirSettingName,
                               parent, caption,
                               wantedPathOrName, filter,
                               selectedFilter, options);
}
