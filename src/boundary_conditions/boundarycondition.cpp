/*
 * Copyright (C) 2011-2015  Denis Pesotsky
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

#include <boundary_conditions/boundarycondition.h>

#include <core/domain.h>

using namespace qfgui;

const BoundaryCondition *const BoundaryCondition::kmVoidCondition =
    new BoundaryCondition(QString(), Qt::gray);

void BoundaryCondition::updateVoidConditionName()
{
    const_cast<BoundaryCondition *>(kmVoidCondition)->setName(tr("Void Condition"));
}

BoundaryCondition::BoundaryCondition(const QString &name,
                                     const QColor &color)
    : Item(name, color)
    , mType(kmVoidCondition == NULL ? qfcore::BoundaryCondition::SecondType
            : qfcore::BoundaryCondition::ThirdType)
    , mTemperatures1()
    , mHeatFlowDensities()
    , mTemperatures3()
    , mHeatTransferFactors()
    , mNumInDomain()
{
    fillTemperaturesList(mTemperatures1);
    fillTemperaturesList(mTemperatures3);
    fillList(mHeatFlowDensities);
    fillHeatTransferFactorsList(mHeatTransferFactors);

    /* пустое граничное условие всегда имеет номер -1 */
    setID(-1);
}

BoundaryCondition::BoundaryCondition(const Item *other,
                                     const QString &name,
                                     const QColor &color)
    : Item(name, color)
    , mType(static_cast<const BoundaryCondition *>(other)->mType)
    , mTemperatures1(static_cast<const BoundaryCondition *>(other)->mTemperatures1)
    , mHeatFlowDensities(static_cast<const BoundaryCondition *>(other)->mHeatFlowDensities)
    , mTemperatures3(static_cast<const BoundaryCondition *>(other)->mTemperatures3)
    , mHeatTransferFactors(static_cast<const BoundaryCondition *>(other)->mHeatTransferFactors)
    , mNumInDomain()
{
}

BoundaryCondition::BoundaryCondition(const Item *other)
    : Item(other->name(), other->color())
    , mType(static_cast<const BoundaryCondition *>(other)->mType)
    , mTemperatures1(static_cast<const BoundaryCondition *>(other)->mTemperatures1)
    , mHeatFlowDensities(static_cast<const BoundaryCondition *>(other)->mHeatFlowDensities)
    , mTemperatures3(static_cast<const BoundaryCondition *>(other)->mTemperatures3)
    , mHeatTransferFactors(static_cast<const BoundaryCondition *>(other)->mHeatTransferFactors)
    , mNumInDomain()
{

}

BoundaryCondition::BoundaryCondition()
    : Item()
    , mType()
    , mTemperatures1()
    , mHeatFlowDensities()
    , mTemperatures3()
    , mHeatTransferFactors()
    , mNumInDomain()
{

}

void BoundaryCondition::fillList(QList<double> &list, double value)
{
    static const int num = 12;
    list.reserve(num);
    for (int i = 0; i < 12; ++i) {
        list.append(value);
    }
}

void BoundaryCondition::fillTemperaturesList(QList<double> &list)
{
    // Данные из отчёта ООО ПНИИИС-изыскания по Байдаре за 2007 год (Марре-Сале)
    list.append(-20.7); // 1
    list.append(-21.7); // 2
    list.append(-19.5); // 3
    list.append(-12.9); // 4
    list.append(-5.2);  // 5
    list.append(1.9);   // 6
    list.append(7.4);   // 7
    list.append(6.9);   // 8
    list.append(3.4);   // 9
    list.append(-3.9);  // 10
    list.append(-12.5); // 11
    list.append(-18.1); // 12
}

void BoundaryCondition::fillHeatTransferFactorsList(QList< double > &list)
{
    // Данные из моей (Дениса Песоцкого) бакалаврской
    list.append(1.16); // 1
    list.append(1.16); // 2
    list.append(1.16); // 3
    list.append(1.16); // 4
    list.append(1.16); // 5
    list.append(12);   // 6
    list.append(12);   // 7
    list.append(12);   // 8
    list.append(12);   // 9
    list.append(1.16); // 10
    list.append(1.16); // 11
    list.append(1.16); // 12
}


void BoundaryCondition::moveDataToDomain(qfcore::Domain *domain)
{
    Q_ASSERT(mHeatFlowDensities.size() == 12);
    Q_ASSERT(mTemperatures1.size() == 12);
    Q_ASSERT(mTemperatures3.size() == 12);
    Q_ASSERT(mHeatTransferFactors.size() == 12);

    if (isVoid()) {
        return;
    }
    mNumInDomain = domain->addBoundaryCondition(boundaryCondition());
}

qfcore::BoundaryCondition BoundaryCondition::boundaryCondition() const
{
    switch (mType) {
    case qfcore::BoundaryCondition::FirstType:
        return qfcore::BoundaryCondition(mType,
                                         stdVector(mTemperatures1));
    case qfcore::BoundaryCondition::SecondType:
        return qfcore::BoundaryCondition(mType,
                                         stdVector(mHeatFlowDensities));
    case qfcore::BoundaryCondition::ThirdType:
        return qfcore::BoundaryCondition(stdVector(mTemperatures3),
                                         stdVector(resistivities()));
    default:
        Q_ASSERT(false);
        return qfcore::BoundaryCondition(std::vector<double>(), std::vector<double>());
    }
}

std::vector<double> BoundaryCondition::stdVector(const QList<double> &list)
{
    std::vector<double> result;
    QList<double>::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
        result.push_back(*it);
    }
    return result;
}

QList<double> BoundaryCondition::resistivities() const
{
    QList<double> result;

    QList<double>::ConstIterator it;
    for (it = mHeatTransferFactors.constBegin();
            it != mHeatTransferFactors.constEnd(); ++it) {
        result.append(1 / (*it));
    }

    return result;
}

void BoundaryCondition::setType(int t)
{
    if (type() == t) {
        return;
    }
    if (t < 0 || t > 2) {
        Q_ASSERT(false);
        return;
    }
    mType = static_cast<qfcore::BoundaryCondition::Type>(t);
    emit typeChanged();
}

void BoundaryCondition::setTemperatures1(const QList<double> &v)
{
    if (temperatures1() == v) {
        return;
    }
    mTemperatures1 = v;
    emit temperatures1Changed();
}

void BoundaryCondition::setHeatFlowDensities(const QList<double> &v)
{
    if (heatFlowDensities() == v) {
        return;
    }
    mHeatFlowDensities = v;
    emit heatFlowDensitiesChanged();
}

void BoundaryCondition::setTemperatures3(const QList<double> &v)
{
    if (temperatures3() == v) {
        return;
    }
    mTemperatures3 = v;
    emit temperatures3Changed();
}

void BoundaryCondition::setHeatTransferFactors(const QList<double> &v)
{
    if (heatTransferFactors() == v) {
        return;
    }
    mHeatTransferFactors = v;
    emit heatTransferFactorsChanged();
}

void BoundaryCondition::setHasTemperatureTrend(bool v)
{
    if (hasTemperatureTrend() == v) {
        return;
    }
    mHasTemperatureTrend = v;
    emit hasTemperatureTrendChanged();
}

void BoundaryCondition::setTemperatureTrend(double v)
{
    if (temperatureTrend() == v) {
        return;
    }
    mTemperatureTrend = v;
    emit temperatureTrendChanged();
}

void BoundaryCondition::setTemperatureTrendStartYear(double v)
{
    if (temperatureTrendStartYear() == v) {
        return;
    }
    mTemperatureTrendStartYear = v;
    emit temperatureTrendStartYearChanged();
}

void BoundaryCondition::setTemperatureTrendMonths(const QList<bool> &v)
{
    if (temperatureTrendMonths() == v) {
        return;
    }
    mTemperatureTrendMonths = v;
    emit temperatureTrendMonthsChanged();
}

QString BoundaryCondition::shortPropertyNameGenetive(const QString &propertyName)
{
    if (propertyName == "type") {
        //: In genetive case. Stands for type of boundary conditon.
        return tr("type");
    } else if (propertyName == "temperatures1") {
        return tr("T(I)");
    } else if (propertyName == "heatFlowDensities") {
        return tr("q");
    } else if (propertyName == "temperatures3") {
        return tr("T(III)");
    } else if (propertyName == "heatTransferFactors") {
        return tr("\316\261");
    } else if (propertyName == "hasTemperatureTrend") {
        return tr("trend usage");
    } else if (propertyName == "temperatureTrend") {
        return tr("trend value");
    } else if (propertyName == "temperatureTrendStartYear") {
        return tr("trend start year");
    } else if (propertyName == "temperatureTrendMonths") {
        return tr("trend months");
    } else {
        return Item::shortPropertyNameGenetive(propertyName);
    }
}
