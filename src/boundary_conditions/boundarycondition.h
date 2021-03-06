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

#ifndef QFGUI_BOUNDARYCONDITION_H
#define QFGUI_BOUNDARYCONDITION_H

#include <itemviews/item.h>

#include <qfrost.h>

#include <core/boundarycondition.h>

namespace qfcore
{
QT_FORWARD_DECLARE_CLASS(Domain)
}

namespace qfgui
{

/// Граничное условие в человеко-понятной форме
class BoundaryCondition: public Item
{
    Q_OBJECT

    Q_PROPERTY(int type
               READ type
               WRITE setType
               NOTIFY typeChanged)

    Q_PROPERTY(QList<double> temperatures1
               READ temperatures1
               WRITE setTemperatures1
               NOTIFY temperatures1Changed)
    Q_PROPERTY(QList<double> heatFlowDensities
               READ heatFlowDensities
               WRITE setHeatFlowDensities
               NOTIFY heatFlowDensitiesChanged)
    Q_PROPERTY(QList<double> temperatures3
               READ temperatures3
               WRITE setTemperatures3
               NOTIFY temperatures3Changed)
    Q_PROPERTY(QList<double> heatTransferFactors
               READ heatTransferFactors
               WRITE setHeatTransferFactors
               NOTIFY heatTransferFactorsChanged)

    Q_PROPERTY(bool hasTemperatureTrend
               READ hasTemperatureTrend
               WRITE setHasTemperatureTrend
               NOTIFY hasTemperatureTrendChanged)
    Q_PROPERTY(double temperatureTrend
               READ temperatureTrend
               WRITE setTemperatureTrend
               NOTIFY temperatureTrendChanged)
    Q_PROPERTY(int temperatureTrendStartYear
               READ temperatureTrendStartYear
               WRITE setTemperatureTrendStartYear
               NOTIFY temperatureTrendStartYearChanged)

    Q_PROPERTY(bool usesTemperatureSpline
               READ usesTemperatureSpline
               WRITE setUsesTemperatureSpline
               NOTIFY usesTemperatureSplineChanged)
    
    Q_PROPERTY(YearlyParams yearlyParams3
               READ yearlyParams3
               WRITE setYearlyParams3
               NOTIFY yearlyParams3Changed)

public:
    Q_INVOKABLE BoundaryCondition(const QString &name,
                                  const QColor &color);

    Q_INVOKABLE BoundaryCondition(const Item *other,
                                  const QString &name,
                                  const QColor &color);

    Q_INVOKABLE BoundaryCondition(const Item *other);

    Q_INVOKABLE BoundaryCondition();

    int type() const {
        return mType;
    }
    const QList<double> &temperatures1() const {
        return mTemperatures1;
    }
    const QList<double> &heatFlowDensities() const {
        return mHeatFlowDensities;
    }
    const QList<double> &temperatures3() const {
        return mTemperatures3;
    }
    const QList<double> &heatTransferFactors() const {
        return mHeatTransferFactors;
    }
    const YearlyParams &yearlyParams3() const {
        return mYearlyParams3;
    }

    bool hasTemperatureTrend() const {
        return mHasTemperatureTrend;
    }
    double temperatureTrend() const {
        return mTemperatureTrend;
    }
    int temperatureTrendStartYear() const {
        return mTemperatureTrendStartYear;
    }

    bool usesTemperatureSpline() const {
        return mUsesTemperatureSpline;
    }

    static const BoundaryCondition *voidCondition() {
        return kmVoidCondition;
    }

    bool isVoid() const {
        return this == kmVoidCondition;
    }

    /**
     * Создание в рассчётной области граничного условия.
     * Также запоминает наш номер в ней.
     */
    void moveDataToDomain(qfcore::Domain *domain);

    std::size_t numInDomain() const {
        return mNumInDomain;
    }

    /// Обновляет имя пустого граничного условия (нужно при смене локали)
    static void updateVoidConditionName();

    QString shortPropertyNameGenetive(const QString &propertyName);

public slots:
    void setType(int t);
    void setTemperatures1(const QList<double> &v);
    void setHeatFlowDensities(const QList<double> &v);
    void setTemperatures3(const QList<double> &v);
    void setHeatTransferFactors(const QList<double> &v);

    void setHasTemperatureTrend(bool v);
    void setTemperatureTrend(double v);
    void setTemperatureTrendStartYear(double v);

    void setUsesTemperatureSpline(bool b);
    
    void setYearlyParams3(const YearlyParams &vals);

signals:
    void typeChanged();
    void temperatures1Changed();
    void heatFlowDensitiesChanged();
    void temperatures3Changed();
    void heatTransferFactorsChanged();

    void hasTemperatureTrendChanged();
    void temperatureTrendChanged();
    void temperatureTrendStartYearChanged();

    void usesTemperatureSplineChanged();
    
    void yearlyParams3Changed();

protected:
    int propertiesLackCount(int version);

private:
    /// Тип граничного условия.
    qfcore::BoundaryCondition::Type mType;

    /// Среднемесячные температуры (для 1го рода)
    QList<double> mTemperatures1;
    /// Среднемесячные плотности теплопотока (для 2го рода)
    QList<double> mHeatFlowDensities;
    /// Среднемесячные температуры (для 3го рода)
    QList<double> mTemperatures3;
    /// Среднемесячные коэффициенты теплоотдачи (для 3го рода)
    QList<double> mHeatTransferFactors;
    
    /// Карта по годам, каждый элемент - массив из 12 пар температур и альф
    QMap<int, QList<QPair<double,double> > > mYearlyParams3;

    /// Включен ли тренд температуры
    bool mHasTemperatureTrend;
    /// Тренд температуры (градусов за год)
    double mTemperatureTrend;
    /// Год, от которого начинается тренд (для него вклад считается нулевым)
    int mTemperatureTrendStartYear;

    /// Используется ли сглаживание температур (сплайн по среднемесячным)
    bool mUsesTemperatureSpline;

    /// Заполняет @p list двенадцатью @p value
    static void fillList(QList<double> &list, double value = 0);

    /// Заполняет @p list двенадцатью стандартными температурами
    static void fillTemperaturesList(QList<double> &list);

    /// Заполняет @p list двенадцатью стандартными коэффициентами теплоотдачи
    static void fillHeatTransferFactorsList(QList<double> &list);

    /// Возвращает std::vector, полученный из @p list
    static std::vector<double> stdVector(const QList<double> &list);
    
    static std::map<int, std::vector<std::pair<double, double> > > stdMap(const QMap<int, QList<QPair<double,double> > > &map);

    static const BoundaryCondition *const kmVoidCondition;

    /// Номер представителя этого условия в рассчётной области.
    std::size_t mNumInDomain;

    /// Массив термических сопротивлений (1/коэф. теплоотдачи)
    QList<double> resistivities() const;

    /// Граничное условие в понятии рассчётной области
    qfcore::BoundaryCondition boundaryCondition() const;

    friend class ChangeBoundaryConditionPropertyCommand;
};

}

#endif // QFGUI_BOUNDARYCONDITION_H
