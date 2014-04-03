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

#include "recentfilesmenu.h"

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QFileSystemWatcher>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>

#include <mainwindow.h>

using namespace qfgui;

RecentFilesMenu *RecentFilesMenu::kMenu = NULL;

const int kMaxRecentFiles = 9;

void RecentFilesMenu::addRecentFilesMenu(QMenu *parent)
{
    if (kMenu == NULL) {
        kMenu = new RecentFilesMenu;
    }
    MainWindow *mainWin = qobject_cast<MainWindow *>(parent->parentWidget()->window());
    connect(mainWin, SIGNAL(destroyed()), kMenu, SLOT(deleteIfNeeded()));
    Q_ASSERT(mainWin != NULL);

    QAction *menuAct = parent->addMenu(kMenu);
    connect(kMenu, SIGNAL(cleanChanged(bool)), menuAct, SLOT(setDisabled(bool)));
    menuAct->setDisabled(kMenu->mIsClean);
}

void RecentFilesMenu::deleteIfNeeded()
{
    bool hasMainWindows = false;
    foreach(QWidget * w, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(w);
        if (mainWin != NULL) {
            hasMainWindows = true;
            connect(mainWin, SIGNAL(destroyed()), kMenu, SLOT(deleteIfNeeded()));
            break;
        }
    }
    if (!hasMainWindows) {
        delete kMenu;
        kMenu = NULL;
    }
}

void RecentFilesMenu::prependFile(const QString &fileName)
{
    Q_ASSERT(QFile::exists(fileName));
    QString absolutePath = QFileInfo(fileName).absoluteFilePath();
    QStringList files = kMenu->recentFilesInSettings();
    files.removeOne(absolutePath);
    files.prepend(absolutePath);
    while (files.size() > kMaxRecentFiles) {
        files.removeLast();
    }
    kMenu->setRecentFiles(files);
}

void RecentFilesMenu::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    Q_ASSERT(action != NULL);
    MainWindow *activeMainwin = qobject_cast< MainWindow * >(QApplication::activeWindow());
    Q_ASSERT(activeMainwin != NULL);
    activeMainwin->open(action->data().toString());
}

RecentFilesMenu::RecentFilesMenu()
    : QMenu(tr("Open &Recent"))
    , mRecentFilesActs()
    , mRecentFilesWatcher(new QFileSystemWatcher(this))
    , mIsClean(true)
{
    Q_ASSERT(kMenu == NULL);
    setIcon(QIcon::fromTheme("document-open-recent"));

    for (int i = 0; i < kMaxRecentFiles; ++i) {
        mRecentFilesActs.append(new QAction(this));
        mRecentFilesActs.last()->setVisible(false);
        connect(mRecentFilesActs.last(), SIGNAL(triggered()), SLOT(openRecentFile()));
        addAction(mRecentFilesActs.last());
    }
    QAction *clearAct = new QAction(tr("Clear &List"), this);
    connect(clearAct, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));

    addSeparator();
    addAction(clearAct);

    updateRecentFileActions();

    checkRecentFiles();
    connect(mRecentFilesWatcher, SIGNAL(fileChanged(QString)),
            this, SLOT(checkFile(QString)));
}

void RecentFilesMenu::updateRecentFileActions()
{
    QStringList files = recentFilesInSettings();

    int numRecentFiles = qMin(files.size(), kMaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        //: Menu entry for recent file. %1 is number, %2 is filename, %3 is path
        QString text = tr("&%1: %2 [%3]")
                       .arg(i + 1)
                       .arg(QFileInfo(files[i]).fileName())
                       .arg(files[i]);
        mRecentFilesActs[i]->setText(text);
        mRecentFilesActs[i]->setData(files[i]);
        mRecentFilesActs[i]->setVisible(true);
    }

    for (int i = numRecentFiles; i < kMaxRecentFiles; ++i) {
        mRecentFilesActs[i]->setVisible(false);
    }

    bool nowClean = (numRecentFiles == 0);
    if (mIsClean != nowClean) {
        mIsClean = nowClean;
        emit cleanChanged(nowClean);
    }

    QStringList oldWatcherFiles = mRecentFilesWatcher->files();
    if (!oldWatcherFiles.isEmpty()) {
        mRecentFilesWatcher->removePaths(mRecentFilesWatcher->files());
    }

    for (QStringList::Iterator i = files.begin(); i != files.end(); ++i) {
        if (!QFile::exists(*i)) {
            i = files.erase(i) - 1;
        }
    }

    if (!files.isEmpty()) {
        mRecentFilesWatcher->addPaths(files);
    }
}

void RecentFilesMenu::checkRecentFiles()
{
    QStringList files = recentFilesInSettings();
    bool filesChanged = false;
    for (QStringList::Iterator i = files.begin(); i != files.end(); ++i) {
        if (!QFile::exists(*i)) {
            i = files.erase(i) - 1;
            filesChanged = true;
        }
    }
    if (filesChanged) {
        setRecentFiles(files);
    }
}

void RecentFilesMenu::checkFile(const QString &fileName)
{
    if (!QFile::exists(fileName)) {
        QStringList files = recentFilesInSettings();
        files.removeOne(fileName);
        setRecentFiles(files);
    }
}

QStringList RecentFilesMenu::recentFilesInSettings()
{
    QSettings settings;
    settings.beginGroup(MainWindow::settingsGroup);
    QStringList result = settings.value("recentFileList").toStringList();
    settings.endGroup();
    return result;
}

void RecentFilesMenu::setRecentFiles(const QStringList &files)
{
    QSettings settings;
    settings.beginGroup(MainWindow::settingsGroup);
    settings.setValue("recentFileList", files);
    settings.endGroup();
    updateRecentFileActions();
}

void RecentFilesMenu::clearRecentFiles()
{
    setRecentFiles(QStringList());
}
