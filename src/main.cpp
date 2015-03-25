/*
 * Copyright (C) 2010-2012  Denis Pesotsky, Maxim Torgonsky
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

#include <main.h>

#include <application.h>
#include <boundary_conditions/boundarycondition.h>

#include <QtCore/QTextCodec>
#include <QtGui/QIcon>
#include <QtWidgets/QMessageBox>
#include <QtCore/QLocale>
#include <QtCore/QTranslator>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>

using namespace qfgui;

int main(int argc, char **argv)
{
    qsrand(QDateTime::currentDateTime().toTime_t());

    QApplication::setColorSpec(QApplication::CustomColor);
    Application app(argc, argv);

    QStringList files = app.arguments();
    files.removeAt(0);
    for (QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
        *it = QFileInfo(*it).absoluteFilePath();
    }
    QString message = files.join("DEADBOOBISSODEAD");

    if (app.sendMessage(message)) {
        return 0;
    }

    QObject::connect(&app, SIGNAL(messageReceived(QString)),
                     &app, SLOT(handleMessage(QString)));

    //QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    //QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    //QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    app.setApplicationName(APP_NAME);
    app.setApplicationDisplayName(APP_NAME);
    app.setApplicationVersion(VERSION_STR);
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setOrganizationDomain(ORGANIZATION_DOMAIN);

    /* TODO: авто-обновления:
     * - http://stackoverflow.com/questions/2077550/how-can-i-enable-auto-updates-in-a-qt-cross-platform-app
     * - https://github.com/pypt/fervor/tree/autoupdate
     */

    // QTBUG-16697
    if (QIcon::themeName().isEmpty()) {
        QIcon::setThemeName("/");
    }

    ////////////////////////////////////////////////////////////////////////////
    QTranslator qtTranslator;
    qtTranslator.load(QLocale::system(), "qtbase", "_",
#ifdef EMBED_QT_L10N
                      ":/",
#else
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath),
#endif
                      ".qm");
    app.installTranslator(&qtTranslator);

    QTranslator translator;
    translator.load(QLocale::system(), "qfrost", "_", ":/", ".qm");
    app.installTranslator(&translator);
    ////////////////////////////////////////////////////////////////////////////

    app.handleMessage(message);

    /* HACK: почему-то, если оно не смогло открыть файл(ы), то несмотря на то,
     *       что открытых окон нет, не закрывается. И мало того, app.quit() не
     *       срабатывает (также см. баг #56, возможно, дело в лишнем виджете) */
    if (app.topLevelWindows().isEmpty()) {
        return 0;
    }

    return app.exec();
}
