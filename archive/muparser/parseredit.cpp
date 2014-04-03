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

#include <parseredit.h>

#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QStyleOptionFrameV2>
#include <QtGui/QApplication>
#include <QtCore/QEvent>

#include <soils/parser.h>

using namespace qfgui;

ParserEdit::ParserEdit(QWidget *parent)
    : QWidget(parent)
    , mLineEdit(new QLineEdit(this))
    , mValidityLabel(new QLabel(this))
    , mIsValid()
{
    mValidityLabel->setMinimumWidth(15);

    mLineEdit->setAlignment(Qt::AlignCenter);
    connect(mLineEdit, SIGNAL(textChanged(QString)), SLOT(check(QString)));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins());
    layout->addWidget(mLineEdit);
    layout->addWidget(mValidityLabel);

    check(mLineEdit->text());
}

void ParserEdit::check(const QString &text)
{
    bool isValidNow = Parser::expressionIsValid(text);
    if (isValidNow != mIsValid) {
        mIsValid = isValidNow;
        mValidityLabel->setText(mIsValid ? "" : "<b>(!)</b>");
        emit validityChanged(mIsValid);
    }
}

void ParserEdit::setText(const QString &text)
{
    mLineEdit->setText(text);
}

void ParserEdit::setValue(double value)
{
    mLineEdit->setText(Parser::expressionForValue(value));
}

QString ParserEdit::text() const
{
    return mLineEdit->text();
}

#include "parseredit.moc"