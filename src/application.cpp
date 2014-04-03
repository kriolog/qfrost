/*
 * Copyright (C) 2011-2013  Denis Pesotsky
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

#include <application.h>

#include <QtWidgets/QSplashScreen>
#include <QtCore/QFileInfo>

#include <mainwindow.h>
#include <boundary_conditions/boundarycondition.h>
#include <unistd.h> //tmp

using namespace qfgui;

Application::Application(int &argc, char **argv)
    : QtSingleApplication(argc, argv)
{
}

void Application::handleMessage(const QString &message)
{
    bool had_windows = !topLevelWidgets().isEmpty();
    QSplashScreen splash(QPixmap(":/splash.png"));
    if (!had_windows) {
        splash.show();
        // HACK из-за некого QTBUG приходится ждать и делать 2 раза обработку
        //      евентов, когда как в Qt4 приходилось делать лишь 1 processEvents
        usleep(10000);
        processEvents();
        processEvents();
    }

    MainWindow *mainWindow = NULL;
    if (message.isEmpty()) {
        // если аргументов нет, то запускаем одно главное окно (без файла)
        mainWindow = new MainWindow;
        qDebug("lets show it");
        mainWindow->show();
        qDebug("done");
    } else {
        const QStringList fileNames = message.split("DEADBOOBISSODEAD");

        // в противном случае запускаем по файлу на каждый аргумент
        foreach(const QString & fileName, fileNames) {
            Q_ASSERT(QFileInfo(fileName).isAbsolute());
            MainWindow *existing = findMainWindow(fileName);
            if (existing != NULL) {
                // такой файл уже открыт? покажем окно, в котором он открыт.
                existing->show();
                existing->raise();
                existing->activateWindow();
                continue;
            }

            mainWindow = new MainWindow(fileName);
            if (mainWindow->isUntitled()) {
                // окошко не смогло открыть файл? удалим его.
                // HACK: надо именно удалять, а не делать close, см. main.cpp
                delete mainWindow;
                mainWindow = NULL;
                continue;
            }
            mainWindow->show();
        }
    }

    if (mainWindow != NULL) {
        splash.finish(mainWindow);
    } else {
        splash.close();
    }
}

MainWindow *Application::findMainWindow(const QString &fileName)
{
    QString filePath = QFileInfo(fileName).canonicalFilePath();

    foreach(QWidget * widget, topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin != NULL
                && !mainWin->isUntitled()
                && mainWin->currentFilePath() == filePath) {
            return mainWin;
        }
    }
    return NULL;
}

void Application::installTranslator(QTranslator *translator)
{
    QtSingleApplication::installTranslator(translator);
    BoundaryCondition::updateVoidConditionName();
}

MainWindow *Application::findMainWindow(const QWidget *widget)
{
    Q_ASSERT(widget != NULL);
    const QWidget *currentWidget = widget;
    while (qobject_cast<const MainWindow *>(currentWidget) == NULL) {
        const QWidget *nextWidget = currentWidget->window();
        if (nextWidget == currentWidget) {
            currentWidget = currentWidget->parentWidget();
        } else {
            currentWidget = nextWidget;
        }
        Q_ASSERT(currentWidget != NULL);
    }
    return const_cast<MainWindow *>(qobject_cast<const MainWindow *>(currentWidget));
}

MainWindow *Application::findMainWindow(const QObject *object)
{
    Q_ASSERT(object != NULL);
    Q_ASSERT(object->parent() != NULL);
    const QObject *currentObject = object;
    while (qobject_cast<const MainWindow *>(currentObject) == NULL) {
        currentObject = currentObject->parent();
        Q_ASSERT(currentObject != NULL);
        const QWidget *widget = qobject_cast<const QWidget * >(currentObject);
        if (widget != NULL) {
            return findMainWindow(widget);
        }
    }
    return const_cast<MainWindow *>(qobject_cast<const MainWindow *>(currentObject));
}
