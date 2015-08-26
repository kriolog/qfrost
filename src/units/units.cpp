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

#include "units.h"
#include "application.h"
#include "mainwindow.h"

#include <QtCore/qmath.h>
#include <QAction>


using namespace qfgui;

// Число для задания недостижимого нуля
static const double kUnreachableZero = 1337;

Units::Units(QObject *parent)
    : QObject(parent)
    , mUnitMap()
    , mSystem(Default)
{

}

void Units::setSystem(Units::System system)
{
    if (mSystem == system) {
        return;
    }
    mSystem = system;
    mUnitMap.clear();
    emit changed();
}

QString Units::meterText()
{
    //: meter
    return tr("m");
}

Unit::Unit(PhysicalProperty property, const QString &suffix,
           double k, bool suffixNeedSpace)
    : mSuffix(suffix)
    , mK(k)
    , mSuffixNeedSpace(suffixNeedSpace)
    , mProperty(property)
{
}

Unit::Unit(const Unit &other)
    : mSuffix(other.suffix())
    , mK(other.mK)
    , mSuffixNeedSpace(other.mSuffixNeedSpace)
    , mProperty(other.mProperty)
{

}

double Unit::minimum() const
{
    double result = Units::minMaxSI(mProperty).first;
    if (result == kUnreachableZero) {
        return minStepSI();
    } else {
        return result;
    }
}

double Unit::maximum() const
{
    return Units::minMaxSI(mProperty).second;
}

double Unit::minimumVisible() const
{
    double min = Units::minMaxSI(mProperty).first;
    if (min == kUnreachableZero) {
        return minStep();
    } else {
        double result = fromSI(min);
        if (log10(qAbs(result)) == static_cast<int>(log10(qAbs(result)))) {
            // Чтобы спинбоксы были покомпактнее
            result -= (result > 0) ? minStep() : -minStep();
        }
        return result;
    }
}

double Unit::maximumVisible() const
{
    double result = fromSI(Units::minMaxSI(mProperty).second);
    if (log10(result) == static_cast<int>(log10(result))) {
        // Чтобы спинбоксы были покомпактнее
        result -= minStep();
    }
    return result;
}

double Unit::singleStepVisible() const
{
    return qPow(10, -stepDecimalsVisible());
}

int Unit::stepDecimals() const
{
    return Units::stepDecimals(mProperty);
}

int Unit::stepDecimalsVisible() const
{
    return -qRound(log10(fromSI(qPow(10.0, -stepDecimals()))));
}

int Unit::decimalsVisible() const
{
    return qMax(0, -qRound(log10(fromSI(qPow(10.0, -decimalsSI())))));
}

Unit::Unit()
    : mSuffix()
    , mK(1)
    , mSuffixNeedSpace(false)
    , mProperty()
{

}

int Unit::decimalsSI() const
{
    return Units::decimals(mProperty);
}

double Unit::minStepSI() const
{
    return toSI(minStep());
}

double Unit::minStep() const
{
    return qPow(10.0, -decimalsVisible());
}

QPair<double, double> Units::minMaxSI(PhysicalProperty property)
{
    switch (property) {
    case Temperature:
        return qMakePair(-200.0, 2000.0);
    case HeatFlowDensity:
        return qMakePair(-1000.0, 1000.0);
    case HeatTransferFactor:
        return qMakePair(0.0, 1000.0);
    case Conductivity:
        return qMakePair(kUnreachableZero, 1000.0);
    case Capacity:
        return qMakePair(kUnreachableZero, 100000000.0);
    case TransitionHeat:
        return qMakePair(kUnreachableZero, 100000000000.0);
    case Moisture:
        return qMakePair(0.0, 99.999);
    case Density:
        return qMakePair(kUnreachableZero, 10000.0);
    case PowerDensity:
        return qMakePair(-10000.0, 10000.0);
    case Energy:
    case Power:
    case NoProperty:
    default:
        return qMakePair(-100000000000000.0, 100000000000000.0);
    }
}

int Units::decimals(PhysicalProperty property)
{
    switch (property) {
    case Temperature:
        return 2;
    case HeatFlowDensity:
        return 2;
    case HeatTransferFactor:
        return 3;
    case Conductivity:
        return 3;
    case Capacity:
        return -2;
    case TransitionHeat:
        return -3;
    case Moisture:
        return 3;
    case Density:
        return -1;
    case PowerDensity:
        return 3;
    case Energy:
    case Power:
    case NoProperty:
    default:
        return 0;
    }
}

int Units::stepDecimals(PhysicalProperty property)
{
    switch (property) {
    case Temperature:
        return 1;
    case HeatFlowDensity:
        return 0;
    case HeatTransferFactor:
        return 2;
    case Conductivity:
        return 2;
    case Capacity:
        return -3;
    case TransitionHeat:
        return -5;
    case Moisture:
        return 2;
    case Density:
        return -1;
    case PowerDensity:
        return 2;
    case Energy:
    case Power:
    case NoProperty:
    default:
        return 0;
    }
}

Unit Units::createUnit(PhysicalProperty property, Units::System system)
{
    // Кол-во Дж к 1 ккал
    static const double kcalToJ = 4190.0;
    // Кол-во секунд в 1 ч
    static const double hourToS = 3600.0;

    static const QString oC = "\302\260C";
    static const QString K = "K";

    //: watt
    static const QString W = tr("W");
    //: joule
    static const QString J = tr("J");
    //: gram
    static const QString g = tr("g");
    //: calory
    static const QString cal = tr("cal");
    //: hour
    static const QString h = tr("h");
    //: second
    static const QString s = tr("s");


    //: kilo
    static const QString kilo = tr("k");
    //: mega
    static const QString mega = tr("M");

    //: multiplication mark
    static const QString x = tr("\302\267");

    static const QString m2 = meterText() + "\302\262";
    static const QString m3 = meterText() + "\302\263";
    static const QString sm3 = tr("cm")+ "\302\263";

    if (property == NoProperty) {
        return Unit(property, QString(), 1, false);
    }

    switch (system) {
    case SI:
        switch (property) {
        case Temperature:
            return Unit(property,
                        oC,
                        1, false);
        case HeatFlowDensity:
            return Unit(property,
                        W + "/" + m2,
                        1);
        case HeatTransferFactor:
            return Unit(property,
                        W + "/(" + m3 + x + K + ")",
                        1);
        case Conductivity:
            return Unit(property,
                        W + "/(" + meterText() + x + K + ")",
                        1);
        case Capacity:
            return Unit(property,
                        J + "/(" + m3 + x + K + ")",
                        1);
        case TransitionHeat:
            return Unit(property,
                        J + "/" + m3,
                        1);
        case Moisture:
            return Unit(property,
                        "%",
                        0.01, false);
        case Density:
            return Unit(property,
                        g + "/" + sm3,
                        1000);
        case Energy:
            return Unit(property,
                        J, 1);
        case Power:
            return Unit(property,
                        W, 1);
        case PowerDensity:
            return Unit(property,
                        W + "/" + m3, 1);
        case NoProperty:
        default:
            Q_ASSERT(false);
            return Unit();
        }
    case Default:
        switch (property) {
        case Temperature:
        case HeatFlowDensity:
        case HeatTransferFactor:
        case Conductivity:
        case Power:
        case PowerDensity:
            return createUnit(property, SI);
        case Capacity:
            return Unit(property,
                        kilo + J + "/(" + m3 + x + K + ")",
                        1000);
        case TransitionHeat:
            return Unit(property,
                        mega + J + "/" + m3,
                        1000 * 1000);
        case Density:
            return Unit(property,
                        g + "/" + sm3,
                        1000);
        case Moisture:
            return Unit(property,
                        "%",
                        0.01, false);
        case Energy:
            return Unit(property,
                        kilo + J,
                        1000);
        default:
        case NoProperty:
            Q_ASSERT(false);
            return Unit();
        }
    case Heat:
        switch (property) {
        case Temperature:
        case HeatFlowDensity:
        case HeatTransferFactor:
        case Conductivity:
        case Moisture:
        case Density:
        case Power:
        case PowerDensity:
            return createUnit(property, Default);
        case Capacity:
            return Unit(property,
                        W + x + h + "/(" + m3 + x + K + ")",
                        hourToS);
        case TransitionHeat:
            return Unit(property,
                        W + x + h + "/" + m3,
                        hourToS);
        case Energy:
            return Unit(property,
                        W + x + h,
                        hourToS);
        case NoProperty:
        default:
            Q_ASSERT(false);
            return Unit();
        }
    case Warm:
        switch (property) {
        case Temperature:
        case Moisture:
        case Density:
            return createUnit(property, Default);
        case HeatFlowDensity:
            return Unit(property,
                        kilo + cal + "/(" + h + x + m2 + ")",
                        kcalToJ / hourToS);
        case HeatTransferFactor:
            return Unit(property,
                        kilo + cal + "/(" + h + x + m2 + x + K + ")",
                        kcalToJ / hourToS);
        case Conductivity:
            return Unit(property,
                        kilo + cal + "/(" + h + x + meterText() + x + K + ")",
                        kcalToJ / hourToS);
        case Capacity:
            return Unit(property,
                        kilo + cal + "/(" + m3 + x + K + ")",
                        kcalToJ);
        case TransitionHeat:
            return Unit(property,
                        kilo + cal + "/" + m3,
                        kcalToJ);
        case Energy:
            return Unit(property,
                        kilo + cal,
                        kcalToJ);
        case Power:
            return Unit(property,
                        kilo + cal + "/" + h,
                        kcalToJ / hourToS);
        case PowerDensity:
            return Unit(property,
                        kilo + cal + "/(" + m3 + x + h + ")", kcalToJ / hourToS);
        case NoProperty:
        default:
            Q_ASSERT(false);
            return Unit();
        }
    default:
        Q_ASSERT(false);
        return Unit();
    }
}

QString Units::systemName(Units::System system)
{
    switch (system) {
    case SI:
        return tr("&SI");
    case Default:
        return tr("&Default");
    case Heat:
        return tr("&Heat");
    case Warm:
        return tr("&Warm");
    default:
        Q_ASSERT(false);
        return "";
    }
}

QString Units::systemText(Units::System system)
{
    return QString("%1 (%2, %3)")
           .arg(systemName(system))
           .arg(createUnit(Energy, system).suffix())
           .arg(createUnit(Power, system).suffix());
}

const Unit &Units::unit(PhysicalProperty property) const
{
    QMap<PhysicalProperty, Unit>::ConstIterator it = mUnitMap.find(property);
    if (it == mUnitMap.constEnd()) {
        Units *u = const_cast<Units*>(this);
        it = u->mUnitMap.insert(property,
                                u->createUnit(property, u->mSystem));
    }
    return *it;
}

const Unit &Units::unit(int property) const
{
    return unit(static_cast<PhysicalProperty>(property));
}

const Units *Units::units(const QObject *object)
{
    return Application::findMainWindow(object)->units();
}

void Units::save(QDataStream &out)
{
    out << qint32(mSystem);
    if (out.status() != QDataStream::Ok) {
        throw false;
    }
}

void Units::load(QDataStream &in)
{
    qint32 systemNum;
    in >> systemNum;
    if (systemNum >= SystemsNum || systemNum < 0 || in.status() != QDataStream::Ok) {
        throw false;
    }
    setSystem(static_cast<System>(systemNum));
}

UnitsSystemActionGroup::UnitsSystemActionGroup(QObject *parent)
    : QActionGroup(parent)
{
    setExclusive(true);

    for (int i = 0; i < Units::SystemsNum; ++i) {
        Units::System system = static_cast<Units::System>(i);
        QAction *action = addAction(Units::systemText(system));
        action->setCheckable(true);
        action->setData(system);
        addAction(action);

        connect(action, SIGNAL(toggled(bool)), SLOT(changeUnitSystem()));
    }
    checkCurrentUnitSystem();

    connect(Units::units(this), SIGNAL(changed()), SLOT(checkCurrentUnitSystem()));
}

void UnitsSystemActionGroup::checkCurrentUnitSystem()
{
    actions().at(Units::units(this)->system())->toggle();
}

void UnitsSystemActionGroup::changeUnitSystem()
{
    Q_ASSERT(sender() != NULL);
    QAction *action = qobject_cast<QAction *>(sender());
    Q_ASSERT(action != NULL);
    Q_ASSERT(action->data().type() == QVariant::Int);
    if (action != NULL) {
        const Units::System s = static_cast<Units::System>(action->data().toInt());
        const_cast<Units*>(Units::units(this))->setSystem(s);
    }
}

QString Unit::textFromSI(double value) const
{
    const QLocale locale;
    QString valueText = locale.toString(fromSI(value), 'f', decimalsVisible());
    if (valueText.contains(locale.decimalPoint())) {
        // удаляем все нули (если получилось дробное число)
        while (valueText.endsWith('0')) {
            valueText.chop(1);
        }
        // если мы теперь кончаемся на точку, удалим её
        if (valueText.endsWith(locale.decimalPoint())) {
            valueText.chop(1);
        }
    }
    return valueText + spinBoxSuffix();
}
