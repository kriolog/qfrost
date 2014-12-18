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

#include "naturalsortfilterproxymodel.h"
#include "boundary_conditions/boundarycondition.h"

#include <QtCore/qmath.h>

using namespace qfgui;

/// Берёт число начиная с @p it и не раньше @p end.
/// @p it после выполнения функции указывает на последнюю цифру.
static int getNumber(QString::ConstIterator &it,
                     const QString::ConstIterator &end)
{
    Q_ASSERT(it->isDigit());
    Q_ASSERT(it->digitValue() != 0);
    QList <int> digits;
    while (it != end && it->isDigit()) {
        digits << (it++)->digitValue();
    }
    --it;
    int result = 0;
    while (!digits.isEmpty()) {
        result += digits.takeFirst() * qPow(10, digits.size());
    }
    return result;
}

static int naturalCompare(const QString &s1, const QString &s2,
                          Qt::CaseSensitivity caseSensitivity = Qt::CaseSensitive,
                          bool localeAware = false)
{
    QString::ConstIterator it1 = s1.begin();
    QString::ConstIterator it2 = s2.begin();
    while (it1 != s1.end() && it2 != s2.end()) {
        // Сравниваем числа, если их можно получить и если они не начинаются с 0
        // Иначе сравниваем буквы исходя из caseSensitivity и localeAware
        if (it1->digitValue() > 0 && it2->digitValue() > 0) {
            int num1 = getNumber(it1, s1.end());
            int num2 = getNumber(it2, s2.end());
            if (num1 != num2) {
                return (num1 > num2) ? 1 : -1;
            }
        } else {
            if (localeAware) {
                int compare = QString(*it1).localeAwareCompare(*it2);
                if (compare != 0) {
                    return compare;
                }
            } else {
                int compare = QString(*it1).compare(*it2, caseSensitivity);
                if (compare != 0) {
                    return compare;
                }
            }
        }
        ++it1;
        ++it2;
    }
    // Сюда мы попали, если строки равны или если кол-во букв различается.
    // Строка считается меньше, если в ней меньше слов
    bool s1Ended = (it1 == s1.end());
    bool s2Ended = (it2 == s2.end());
    if (s1Ended && s2Ended) {
        return 0;
    } else if (s1Ended) {
        return -1;
    } else {
        Q_ASSERT(s2Ended);
        return 1;
    }
}

NaturalSortFilterProxyModel::NaturalSortFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    Q_ASSERT(naturalCompare("A", "B") < 0);
    Q_ASSERT(naturalCompare("", "") == 0);
    Q_ASSERT(naturalCompare("", "a") < 0);
    Q_ASSERT(naturalCompare("a", "") > 0);
    Q_ASSERT(naturalCompare("a", "a") == 0);
    Q_ASSERT(naturalCompare("", "9") < 0);
    Q_ASSERT(naturalCompare("9", "") > 0);
    Q_ASSERT(naturalCompare("1", "1") == 0);
    Q_ASSERT(naturalCompare("1", "2") < 0);
    Q_ASSERT(naturalCompare("3", "2") > 0);
    Q_ASSERT(naturalCompare("a1", "a1") == 0);
    Q_ASSERT(naturalCompare("a1", "a2") < 0);
    Q_ASSERT(naturalCompare("a2", "a1") > 0);
    Q_ASSERT(naturalCompare("a1a2", "a1a3") < 0);
    Q_ASSERT(naturalCompare("a1a2", "a1a0") > 0);
    Q_ASSERT(naturalCompare("a1a2a2", "a1a2a3") < 0);
    Q_ASSERT(naturalCompare("a1a2a2", "a1a2a0") > 0);
    Q_ASSERT(naturalCompare("a1a2a02", "a1a2a03") < 0);
    Q_ASSERT(naturalCompare("a1a2a02", "a1a2a01") > 0);
    Q_ASSERT(naturalCompare("a1a2a01", "a1a2a12") < 0);
    Q_ASSERT(naturalCompare("a1a2a09", "a1a2a12") < 0);
    Q_ASSERT(naturalCompare("134", "122") > 0);
    Q_ASSERT(naturalCompare("12a3", "12a3") == 0);
    Q_ASSERT(naturalCompare("12a1", "12a0") > 0);
    Q_ASSERT(naturalCompare("12a1", "12a2") < 0);
    Q_ASSERT(naturalCompare("a", "aa") < 0);
    Q_ASSERT(naturalCompare("aaa", "aa") > 0);
    Q_ASSERT(naturalCompare("Alpha 2", "Alpha 2") == 0);
    Q_ASSERT(naturalCompare("Alpha 2", "Alpha 2A") < 0);
    Q_ASSERT(naturalCompare("Alpha 2 B", "Alpha 2") > 0);
    Q_ASSERT(naturalCompare("Alpha 2", "Alpha 10") < 0);
    Q_ASSERT(naturalCompare("Alpha 20", "Alpha 10") > 0);
    Q_ASSERT(naturalCompare("A", "Z") < 0);
    Q_ASSERT(naturalCompare("A", "No") < 0);
    Q_ASSERT(naturalCompare("V", "W") < 0);
    Q_ASSERT(naturalCompare("V", "Boun") > 0);
    Q_ASSERT(naturalCompare("А", "Я") < 0);
    Q_ASSERT(naturalCompare("Ю", "Я") < 0);
    Q_ASSERT(naturalCompare("Z", "Я") < 0);
    Q_ASSERT(naturalCompare("Я", "А") > 0);
    Q_ASSERT(naturalCompare("П", "Я") < 0);
}

static bool isVoidBoundaryCondition(const QModelIndex &index)
{
    if (index.internalPointer() != NULL) {
        BoundaryCondition *bc = qobject_cast<BoundaryCondition *>(static_cast<QObject *>(index.internalPointer()));
        if (bc != NULL) {
            return bc->isVoid();
        }
    } else {
        qWarning("NaturalSortFilterProxyModel: got model index without internal pointer!");
    }
    return false;
}

bool NaturalSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (isVoidBoundaryCondition(left)) {
        return true;
    }
    if (isVoidBoundaryCondition(right)) {
        return false;
    }

    if (left.data().type() == QVariant::String && right.data().type() == QVariant::String) {
        QString s1 = left.data(sortRole()).toString();
        QString s2 = right.data(sortRole()).toString();
        return naturalCompare(s1, s2, sortCaseSensitivity(), isSortLocaleAware()) < 0;
    } else {
        return QSortFilterProxyModel::lessThan(left, right);
    }
}
