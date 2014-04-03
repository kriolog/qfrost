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

#ifndef QFGUI_RECENTFILESMENU_H
#define QFGUI_RECENTFILESMENU_H

#include <QtWidgets/QMenu>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QStringList)
QT_FORWARD_DECLARE_CLASS(QFileSystemWatcher)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(MainWindow)

class RecentFilesMenu: public QMenu
{
    Q_OBJECT
public:
    static void addRecentFilesMenu(QMenu *parent);
    static void prependFile(const QString &fileName);

private:
    RecentFilesMenu();
    /// Считывает список недавних файлов из настроек и обновляет меню
    void updateRecentFileActions();

    /// Изменяет список недавних файлов в настройках и обновляет меню
    void setRecentFiles(const QStringList &files);
    /// Список недавних файлов из настроек
    QStringList recentFilesInSettings();
    /// Удаляет несуществующие недавние файлы из настроек и обновляет меню
    void checkRecentFiles();

    static RecentFilesMenu *kMenu;

    QList<QAction *> mRecentFilesActs;
    QFileSystemWatcher *mRecentFilesWatcher;
    bool mIsClean;

private slots:
    /// Очищает список последних открытых файлов
    void clearRecentFiles();
    /// Проверяет файл на существование
    void checkFile(const QString &fileName);
    /// Открывает файл исходя из отправителя сигнала в текущем активном окне
    void openRecentFile();
    /// Удаляет kMenu, если больше не осталось главных окон
    void deleteIfNeeded();

signals:
    /// Сигнал об изменении наличия недавних файлов
    void cleanChanged(bool clean);
};

}

#endif // QFGUI_RECENTFILESMENU_H
