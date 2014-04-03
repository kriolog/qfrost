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

#ifndef QFGUI_PARSER_H
#define QFGUI_PARSER_H

#include <QtCore/QString>

#include <muParser.h>

namespace qfgui
{

/// Парсер математических выражений с доступной переменной z.
class Parser
{

public:
    Parser();
    Parser(const Parser &other);
    /// Является ли введёное в парсер выражение валидным
    bool isValid() const;
    /// Результат вычисления для @p z.
    inline double value(qreal z) const;
    /// Записана ли в нас константа
    bool isConstant() const;
    /// Если в нас записана константа, возвращает её значение, иначе 0.0.
    double value() const;
    /**
     * Устанавливает выражение парсера в @p expression.
     * @p expression должно быть в локали приложения.
     */
    void setExpression(const QString &expression);
    /// Устанавливает выражение парсера в число @p value
    void setValue(double value);
    /// Текущее выражение в локали приложения.
    QString expression() const;

    /**
     * Является ли выражение @p expression валидным
     * @p expression должно быть в локали приложения.
     */
    static bool expressionIsValid(const QString &expression);

    /// Разделитель десятичной и дробной части в локали приложения
    static QChar localeDecimalSeparator();

    static bool canWorkInCurrentLocale();

    static QString expressionForValue(double value);

private:
    qreal mZ;
    mu::Parser mParser;
};

}

double qfgui::Parser::value(qreal z) const
{
    // HACK: метод константный по логике, но по факту приходится изменять поле
    qreal *zPtr = const_cast<qreal *>(&mZ);
    *zPtr = z;
    Q_ASSERT(isValid());
    return mParser.Eval();
}

/// Сохраняет текст из парсера в @p out в локали Latin1
QDataStream &operator<< (QDataStream &out, const qfgui::Parser &parser);

/// Забирает текст из парсера в @p out, ожидается использованик локали Latin1
QDataStream &operator>> (QDataStream &in, qfgui::Parser &parser);

#endif // QFGUI_PARSER_H
