/*
 * Copyright (C) 2012-2015  Denis Pesotsky
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

#ifndef QFGUI_UNITY_H
#define QFGUI_UNITY_H

#include <qfrost.h>

#include <QtCore/QObject>
#include <QtWidgets/QActionGroup>

namespace qfgui
{

class Unit
{
public:
    Unit(const Unit &other);

    const QString &suffix() const {
        return mSuffix;
    }

    QString spinBoxSuffix() const {
        return (mSuffixNeedSpace ? " " : "") + mSuffix;
    }

    QString headerSuffix() const {
        return mSuffix.isEmpty()
               ? QString()
               : (((mSuffix.size() <= 2) ? ", " : ",\n") + mSuffix);
    }

    QString headerSuffixOneLine() const {
        return mSuffix.isEmpty()
               ? QString()
               : (", " + mSuffix);
    }

    /// Значение знаков после запятой (в СИ).
    /// Не изменяется при смене системы единиц.
    int decimalsSI() const;

    /// Видимое значение знаков после запятой
    int decimalsVisible() const;

    inline double toSI(double value) const {
        return value * mK;
    }

    inline double fromSI(double value) const {
        return value / mK;
    }

    QString textFromSI(double value) const;

    /// Минимальное значение величины (в СИ).
    /// Если минимум - это выколотый ноль, это @m minStepSI
    double minimum() const;

    /// Максимальное значение величины (в СИ).
    /// Не меняется при изменении системы единиц.
    double maximum() const;

    /// Минимальное значение величины (в наших единицах).
    /// Если минимум - выколотый ноль, это @m minStep.
    double minimumVisible() const;

    /// Максимальное значение величины (в наших единицах).
    /// Если максимум в наших единицах - степень десяти, оно ниже на @m minStep.
    /// Это нужно для более компактного представления спинбоксов.
    double maximumVisible() const;

    /// Шаг для спинбоксов (в наших единицах)
    double singleStepVisible() const;

    /**
     * Минимально возможный шаг (в СИ).
     * Это минимально видимый шаг, никак не связанный с @m singleStep, он
     * получается возведением 0.1 в степень @m decimals (и переводом в СИ).
     */
    double minStepSI() const;

    /**
     * Минимально возможный шаг (в наших единицах).
     * Это минимально видимый шаг, никак не связанный с @m singleStep, он
     * получается возведением 0.1 в степень @m decimalsVisible.
     * Не меняется при изменении системы единиц.
     */
    double minStep() const;

    /// Видимое значение знаков после запятой у шага.
    /// Не меняется при смене системы единиц.
    int stepDecimalsVisible() const;

private:
    Unit(PhysicalProperty property, const QString &suffix,
         double k, bool suffixNeedSpace = true);
    Unit();

    /// Значение знаков после запятой у шага.
    /// Не меняется при смене системы единиц.
    int stepDecimals() const;

    QString mSuffix;
    double mK;
    bool mSuffixNeedSpace;
    PhysicalProperty mProperty;

    friend class Units;
    friend class QMap<PhysicalProperty, Unit>; // ему нужен копирующий конструктор
};

class Units: public QObject
{
    Q_OBJECT
public:
    Units(QObject *parent);

    /// Выбранная на данный момент система единиц для @p propery
    const Unit &unit(PhysicalProperty property) const;
    /// Выбранная на данный момент система единиц для @p propery
    const Unit &unit(int property) const;

    static const Unit &unit(const QObject *object, PhysicalProperty property) {
        return units(object)->unit(property);
    }

    static const Unit &unit(const QObject *object, int property) {
        return units(object)->unit(property);
    }

    static const Units *units(const QObject *object);

    /// Перечисление возможных систем единиц.
    /// Соответствующие значения не изменяются для обратной совместимости.
    enum System {
        /// Работа в кДж, мощность в кВт
        Default = 0x00,
        /// Работа в ккал, мощность в ккал/час
        Heat = 0x01,
        /// Работа в Вт*ч, мощность в Вт
        Warm = 0x02,
        /// Работа в Дж, мощность в Вт
        SI = 0x03
    };

    static const int SystemsNum = 4;

    System system() const {
        return mSystem;
    }
    void setSystem(System system);

    /// Название системы единиц @p system со знаком "&"
    static QString systemName(System system);

    /// Название системы единиц @p system со знаком "&" и единицы измерения
    /// энергии и мощности в скобках.
    static QString systemText(System system);

    /// Аббревиатура метра (для суффиксов в спинбоксах и пр.).
    static QString meterText();

    /// С точностью до этого знака изменяются значения величины.
    static int decimals(PhysicalProperty property);

    void save(QDataStream &out);
    void load(QDataStream &in);

signals:
    void changed();

private:
    /// Минимальное и максимальное значения величины @p property в СИ
    static QPair<double, double> minMaxSI(PhysicalProperty property);

    /// В этом разряде при каждом шаге изменяются значения величины.
    static int stepDecimals(PhysicalProperty property);

    /// Создаёт Unit исходя из @p property и @m mSystem
    static Unit createUnit(PhysicalProperty property, System system);

    QMap<PhysicalProperty, Unit> mUnitMap;
    System mSystem;

    friend class Unit;
};

class UnitsSystemActionGroup: public QActionGroup
{
    Q_OBJECT
public:
    UnitsSystemActionGroup(QObject *parent);

private slots:
    /// Делает checked тому action'у, который отвечает за ныне выбранную систему
    void checkCurrentUnitSystem();
    /// Выбирает ту систему, за которую отвечает (QAction) sender
    void changeUnitSystem();
};

}

#endif // QFGUI_UNITY_H
