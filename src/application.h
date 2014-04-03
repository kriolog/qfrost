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

#ifndef QFGUI_APPLICATION_H
#define QFGUI_APPLICATION_H

#include <QtSingleApplication>

#define qfApp (static_cast<Application *>(qApp))

#include "qfrost.h"

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(MainWindow)

class Application : public QtSingleApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);

    /// Указатель на главное окно, в котором открыт файл с именем @p fileName
    /// или NULL, если такого окна нет.
    static MainWindow *findMainWindow(const QString &fileName);

    /// Указатель на главное окно, к которому относится @p widget
    static MainWindow *findMainWindow(const QWidget *widget);
    /// Указатель на главное окно, к которому относится @p object
    static MainWindow *findMainWindow(const QObject *object);

    void installTranslator(QTranslator *translator);
public slots:
    void handleMessage(const QString &message);
};

}

#endif // QFGUI_APPLICATION_H
