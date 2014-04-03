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

#include "parser.h"

#include <QtCore/QLocale>
#include <QtCore/QDataStream>

using namespace qfgui;

static const QChar kDefaultArgSeparator = ';';

Parser::Parser()
    : mZ()
    , mParser()
{
    mParser.DefineVar("z", &mZ);
    Q_ASSERT(canWorkInCurrentLocale());
    mParser.SetArgSep(kDefaultArgSeparator.toAscii());
    mParser.SetDecSep(localeDecimalSeparator().toAscii());
    mParser.SetThousandsSep(QLocale().groupSeparator().toAscii());
    setExpression("0");
}

bool Parser::canWorkInCurrentLocale()
{
    return localeDecimalSeparator() != kDefaultArgSeparator;
}

Parser::Parser(const Parser &other)
    : mZ()
    , mParser()
{
    mParser.DefineVar("z", &mZ);
    setExpression(other.expression());
}

void Parser::setExpression(const QString &expression)
{
    mParser.SetExpr(expression.toStdString());
}

QString Parser::expressionForValue(double value)
{
    QString result;
    result = QString::number(value);
    result.replace('.', localeDecimalSeparator());
    return result;
}

void Parser::setValue(double value)
{
    mParser.SetExpr(expressionForValue(value).toStdString());
}

bool Parser::isConstant() const
{
    bool result;
    QLocale().toDouble(expression(), &result);
    return result;
}

double Parser::value() const
{
    Q_ASSERT(isConstant());
    return QLocale().toDouble(expression());
}

QString Parser::expression() const
{
    QString result;
    result = mParser.GetExpr().c_str();
    result = result.simplified();
    return result;
}

/// Разделитель десятичной и дробной части в локали приложения
QChar Parser::localeDecimalSeparator()
{
    return QLocale().decimalPoint();
}

bool Parser::isValid() const
{
    try {
        mParser.Eval();
        return true;
    } catch (mu::Parser::exception_type &e) {
        return false;
    }
}

bool Parser::expressionIsValid(const QString &expression)
{
    Parser tmp;
    tmp.setExpression(expression);
    return tmp.isValid();
}

QDataStream &operator<<(QDataStream &out, const Parser &parser)
{
    QString string = parser.expression();
    string.replace(Parser::localeDecimalSeparator(), '.');
    out << string;
    return out;
}

QDataStream &operator>>(QDataStream &in, Parser &parser)
{
    QString string;
    in >> string;
    // сохранённое выражение обязано быть валидным
    Q_ASSERT(!string.contains(','));
    string.replace('.', Parser::localeDecimalSeparator());
    parser.setExpression(string);
    Q_ASSERT(parser.isValid());
    return in;
}
