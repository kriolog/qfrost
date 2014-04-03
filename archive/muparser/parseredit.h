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

#ifndef QFGUI_PARSEREDIT_H
#define QFGUI_PARSEREDIT_H

#include <QtGui/QWidget>
#include <QtGui/QLineEdit>

QT_FORWARD_DECLARE_CLASS(QLabel)

namespace qfgui
{

/// Редактор математических выражений, сигнализирующий о валидности ввода
class ParserEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText USER true)
public:
    explicit ParserEdit(QWidget *parent);

    QString text() const;
    bool isValid() const {
        return mIsValid;
    }

public slots:
    void setText(const QString &text);
    void setValue(double value);

signals:
    /// Сигнал о том, что изменилась валидность введённого текста
    void validityChanged(bool isValid);

private slots:
    /// Если надо, обновляет @a mValidityLabel и испускает validityChanged()
    void check(const QString &text);

private:
    QLineEdit *mLineEdit;
    QLabel *mValidityLabel;
    bool mIsValid;
};

}

#endif // QFGUI_PARSEREDIT_H
