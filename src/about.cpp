/*
 * Copyright (C) 2010-2016  Denis Pesotsky
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

#include <about.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtGui/QPalette>
#include <QtGui/QDesktopServices>

using namespace qfgui;

/**
 * Ссылка (в формате HTML) на @p string, начинающаяся с переноса строки (<br/>).
 * Если из @p string невозможно составить ссылку, возвращает QString().
 * @param string http или https адрес (без строгого форматирования)
 */
QString httpLink(const QString &string)
{
    if (string.isEmpty()) {
        return QString();
    } else {
        QUrl url = QUrl::fromUserInput(string);
        QByteArray encodedUrl = url.toEncoded();
        QString scheme = url.scheme();
        if (encodedUrl.isEmpty() || !url.isValid()
                || (scheme != "http" && scheme != "https")) {
            qWarning("About dialogue error. Bad http/https URL: %s",
                     qPrintable(string));
            return QString();
        }
        QString niceString =  url.toString(QUrl::StripTrailingSlash);
        return "<br/><a href=\"" + encodedUrl + "\">" + niceString + "</a>";
    }
}

/**
 * Ссылка (в формате HTML) на @p string, начинающаяся с переноса строки (<br/>).
 * Если из @p string невозможно составить ссылку, возвращает QString().
 * @param string почтовый адрес (без mailto:)
 */
QString mailLink(const QString &string)
{
    if (string.isEmpty() || string.isNull()) {
        return QString();
    } else {
        QUrl url = QUrl::fromUserInput("mailto:" + string);
        QByteArray encodedUrl = url.toEncoded();
        if (encodedUrl.isEmpty() || !url.isValid()
                || url.scheme() != "mailto") {
            qWarning("About dialogue error. Bad mail URL: %s",
                     qPrintable(string));
            return QString();
        }
        return "<br/><a href=\"" + encodedUrl + "\">" + string + "</a>";
    }
}

About::About(const QString &description,
             QString copyrightNotice,
             QWidget *parent):
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    mAuthors(new PersonListWidget(this)),
    mThanks(new PersonListWidget(this))
{
    setWindowTitle(tr("About %1").arg(QCoreApplication::applicationName()));

    copyrightNotice.replace("-", "–");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    QGridLayout *titleLayout = new QGridLayout();
    titleLayout->setColumnStretch(1, 1);

    QLabel *pictureLabel = new QLabel(this);
    pictureLabel->setPixmap(QApplication::windowIcon().pixmap(72));
    titleLayout->addWidget(pictureLabel, 0 , 0,
                           Qt::AlignHCenter | Qt::AlignVCenter);

    QLabel *titleLabel = new QLabel(this);
    titleLabel->setText("<font size=5>" + QCoreApplication::applicationName()
                        + "</font><br>" + "<b>"
                        + tr("Version %1").arg(QCoreApplication::applicationVersion())
                        + "</b><br>" + tr("Using Qt %1").arg(QT_VERSION_STR));
    titleLayout->addWidget(titleLabel, 0, 1, Qt::AlignLeft | Qt::AlignVCenter);

    mainLayout->addLayout(titleLayout);

    QTabWidget *tabs = new QTabWidget(this);
    mainLayout->addWidget(tabs);

    QWidget *infoWidget = new QWidget(this);
    QVBoxLayout *infoLayout = new QVBoxLayout(infoWidget);
    QLabel *info = new QLabel(infoWidget);
    //  info->setOpenExternalLinks(true);
    infoLayout->addWidget(info);
    info->setText(description
                  + "<br><br>"
                  + copyrightNotice + "<br/>"
                  + httpLink(QCoreApplication::organizationDomain()) + "<br/>"
                  + "<a href=\"openLicense\">"
                  + tr("License: GNU General Public License version 3")
                  + "</a>");
    info->setTextInteractionFlags(info->textInteractionFlags()
                                  | Qt::LinksAccessibleByKeyboard);
    info->setWordWrap(true);
    tabs->addTab(infoWidget, tr("&About"));
    tabs->addTab(mAuthors, tr("A&uthors"));
    tabs->addTab(mThanks, tr("&Thanks To"));

    connect(info, SIGNAL(linkActivated(QString)), SLOT(openLink(QString)));

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close,
            Qt::Horizontal, this);
    connect(buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(close()));
    buttons->button(QDialogButtonBox::Close)->setAutoDefault(true);
    buttons->button(QDialogButtonBox::Close)->setFocus();
    mainLayout->addWidget(buttons);

    setMinimumSize(250, 300);
    resize(333, 350);
}

void About::openLink(const QString &urlString)
{
    QUrl url(urlString);
    if (url.scheme() != "") {
        QDesktopServices::openUrl(url);
    } else {
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle(tr("%1 License").arg(QApplication::applicationName()));

        QTextBrowser *license = new QTextBrowser(dialog);
        license->setSource(QUrl("qrc:/gpl-3.0-standalone.html"));
        license->setOpenExternalLinks(true);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close,
                Qt::Horizontal, this);
        connect(buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
                dialog, SLOT(close()));
        buttons->button(QDialogButtonBox::Close)->setAutoDefault(true);
        buttons->button(QDialogButtonBox::Close)->setFocus();

        QVBoxLayout *dialogLayout = new QVBoxLayout(dialog);
        dialogLayout->addWidget(license);
        dialogLayout->addWidget(buttons);

        dialog->resize(625, 450);

        dialog->exec();
    }
}

void About::addAuthor(const QString &name, const QString &description,
                      const QString &mail, const QString &website)
{
    mAuthors->appendPerson(name, description, mail, website);
}

void About::addCredit(const QString &name, const QString &description,
                      const QString &mail, const QString &website)
{
    mThanks->appendPerson(name, description, mail, website);
}

AboutTextView::AboutTextView(QWidget *parent): QTextBrowser(parent)
{
    setOpenExternalLinks(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPalette newPalette = palette();
    newPalette.setBrush(QPalette::Base, QBrush());

    setPalette(newPalette);

    setFrameShape(QFrame::NoFrame);
}


PersonListWidget::PersonListWidget(QWidget *parent)
    : AboutTextView(parent)
{
}

void PersonListWidget::appendPerson(const QString &name,
                                    const QString &description,
                                    const QString &mail,
                                    const QString &websiteUrl)
{
    static const int infoMargin = 15;
    QString text;
    if (!toPlainText().isEmpty()) {
        text = toHtml() + "<br>";
    } else {
        text = "";
    }
    text += "<b>" + name + "</b>"
            + QString("<div style='margin-left:%1px'>").arg(infoMargin)
            + description
            + mailLink(mail)
            + httpLink(websiteUrl)
            + "</div>";
    setHtml(text);
}
