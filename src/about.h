/*
 * Copyright (C) 2010-2013  Denis Pesotsky
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

#ifndef QFGUI_ABOUT_H
#define QFGUI_ABOUT_H

#include <QtWidgets/QDialog>
#include <QtWidgets/QTextBrowser>

namespace qfgui
{

class AboutTextView : public QTextBrowser
{
public:
    AboutTextView(QWidget *parent = 0);
};

/**
 * Список персон
 */
class PersonListWidget : public AboutTextView
{
public:
    PersonListWidget(QWidget *parent = 0);

    /**
    * Добавляет персону в конец @p textBrowse.
    * Если надо, в начале дописывает новую строку.
    * @param textBrowser текстовое поле
    * @param name имя персоны (или имена через запятую)
    * @param description сделанный взнос
    * @param mail почтовый адрес (с символом "@")
    * @param websiteUrl адрес веб-страницы (вместе с http://)
    */
    void appendPerson(const QString &name,
                      const QString &description,
                      const QString &mail, const QString &websiteUrl);

};

/**
 * Диалог "О программе", сделанный по образу и подобию KAboutApplicationDialog.
 * Заполнение идёт с помощью методов, аналогичных методам KAboutData.
 */
class About : public QDialog
{
    Q_OBJECT
public:
    /**
     * Создаёт диалог "О программе" со вкладками:
     * - общая информация,
     * - список авторов,
     * - список благодарностей,
     * - информация о лицензии.
     * Вкладка с общей информацией заполняется из аргументов.
     * @param description краткое описание программы
     * @param copyrightYear год (или года через дефис) копирайта
     * @param copyrightNames имя человека (организации), которому принадлежает
     *                        авторские права (или имена через запятые)
     * @param parent родительский виджет
     * Название, версия и доменное имя берутся из соответствующих статических
     * методов класса QCoreApplication: applicationName(), applicationVersion()
     * и organizationDomain()
     */
    About(const QString &description,
          QString copyrightYears, const QString &copyrightNames,
          QWidget *parent = NULL);

    /**
     * Добавляет персону к списку авторов.
     * @param name имя персоны (или имена через запятую)
     * @param description сделанный взнос
     * @param mailOrWww почтовый адрес (с символом "@") или адрес веб-страницы
     *                    (адрес страницы может быть как с http://, так и без)
     */
    void addAuthor(const QString &name, const QString &description = "",
                   const QString &mail = "", const QString &website = "");

    /**
     * Добавляет персону к списку благодарностей.
     * @param name имя персоны (или имена через запятую)
     * @param description сделанный взнос
     * @param mailOrWww почтовый адрес (с символом "@") или адрес веб-страницы
     *                    (адрес страницы может быть как с http://, так и без)
     */
    void addCredit(const QString &name, const QString &description = "",
                   const QString &mail = "", const QString &website = "");

private:
    /// Список авторов.
    PersonListWidget *mAuthors;

    /// Список благодарностей.
    PersonListWidget *mThanks;

private slots:
    /**
     * Открывает лицензию, если QUrl(urlString).sceheme().isEmpty,
     * иначе просто открывает ссылку через QDesktopServices::openUrl().
     */
    void openLink(const QString &urlString);
};

}

#endif // QFGUI_ABOUT_H
