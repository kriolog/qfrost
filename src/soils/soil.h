/*
 * Copyright (C) 2010-2012  Denis Pesotsky
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

#ifndef QFGUI_SOIL_H
#define QFGUI_SOIL_H

#include <itemviews/item.h>

#include <core/soilblock.h>
#include <qfrost.h>

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(Block)

class Soil: public Item
{
    Q_OBJECT
    Q_PROPERTY(double conductivityThawed
               READ conductivityThawed
               WRITE setConductivityThawed
               NOTIFY conductivityThawedChanged)
    Q_PROPERTY(double conductivityFrozen
               READ conductivityFrozen
               WRITE setConductivityFrozen
               NOTIFY conductivityFrozenChanged)
    Q_PROPERTY(double capacityThawed
               READ capacityThawed
               WRITE setCapacityThawed
               NOTIFY capacityThawedChanged)
    Q_PROPERTY(double capacityFrozen
               READ capacityFrozen
               WRITE setCapacityFrozen
               NOTIFY capacityFrozenChanged)
    Q_PROPERTY(double transitionTemperature
               READ transitionTemperature
               WRITE setTransitionTemperature
               NOTIFY transitionTemperatureChanged)
    Q_PROPERTY(double transitionHeat
               READ transitionHeat
               WRITE setTransitionHeat
               NOTIFY transitionHeatChanged)
    Q_PROPERTY(bool usesUnfrozenWaterCurve
               READ usesUnfrozenWaterCurve
               WRITE setUsesUnfrozenWaterCurve
               NOTIFY usesUnfrozenWaterCurveChanged)
    Q_PROPERTY(DoubleMap unfrozenWaterCurve
               READ unfrozenWaterCurve
               WRITE setUnfrozenWaterCurve
               NOTIFY unfrozenWaterCurveChanged)
    Q_PROPERTY(double moistureTotal
               READ moistureTotal
               WRITE setMoistureTotal
               NOTIFY moistureTotalChanged)
    Q_PROPERTY(double dryDensity
               READ dryDensity
               WRITE setDryDensity
               NOTIFY dryDensityChanged)
    Q_PROPERTY(double internalHeatSourcePowerDensity
               READ internalHeatSourcePowerDensity
               WRITE setInternalHeatSourcePowerDensity
               NOTIFY internalHeatSourcePowerDensityChanged)

public:
    /// Конструктор для нового грунта с именем @p name и цветом @p color.
    Q_INVOKABLE Soil(const QString &name, const QColor &color);

    /// Конструктор для нового грунта с именем @p name, цветом @p color и всеми
    /// прочими свойствами (кроме родителя) из @p other.
    Q_INVOKABLE Soil(const Item *other, const QString &name, const QColor &color);

    /// Делает полную копию @p other (копирует оттуда всё, кроме родителя)
    Q_INVOKABLE Soil(const Item *other);

    /// Конструктор для случая, если в дальнейшем планируется делать load()
    Q_INVOKABLE Soil();

    inline const qfcore::SoilBlock &block() const {
        return mBlock;
    }

    /// Добавляет @p block в список блоков в сцене, грунтом которых мы являемся
    void addBlock(Block *block) {
        Q_ASSERT(!mBlocks.contains(block));
        mBlocks.append(block);
    }

    /// Добавляет @p block из списка блоков в сцене, грунтом которых мы являемся
    void removeBlock(Block *block) {
        Q_ASSERT(mBlocks.count(block) == 1);
        mBlocks.removeOne(block);
    }

    /// Список блоков в сцене, грунтом которых мы являемся
    const QList<Block *> &blocks() const {
        return mBlocks;
    }

    inline double conductivityThawed() const {
        return mBlock.mConductivity.thawed;
    }
    inline double conductivityFrozen() const {
        return mBlock.mConductivity.frozen;
    }
    inline double capacityThawed() const {
        return mBlock.mCapacity.thawed;
    }
    inline double capacityFrozen() const {
        return mBlock.mCapacity.frozen;
    }
    inline double transitionTemperature() const {
        return mBlock.mTransitionTemperature;
    }
    inline double transitionHeat() const {
        return mBlock.mTransitionHeat;
    }
    inline bool usesUnfrozenWaterCurve() const {
        return mBlock.mUsesUnfrozenWaterCurve;
    }
    inline DoubleMap unfrozenWaterCurve() const {
        return DoubleMap(mBlock.mUnfrozenWaterCurve);
    }
    inline double moistureTotal() const {
        return mBlock.mMoistureTotal;
    }
    inline double dryDensity() const {
        return mBlock.dryDensity();
    }
    inline double internalHeatSourcePowerDensity() const {
        return mBlock.internalHeatSourcePowerDensity();
    }

    double moistureTotalMinimum() const;
    double moistureTotalMaximum() const;

    QString shortPropertyNameGenetive(const QString &propertyName);

public slots:
    void setConductivityThawed(double d);
    void setConductivityFrozen(double d);
    void setCapacityThawed(double d);
    void setCapacityFrozen(double d);
    void setTransitionTemperature(double d);
    void setTransitionHeat(double d);
    void setUsesUnfrozenWaterCurve(bool b);
    void setUnfrozenWaterCurve(const DoubleMap &m);
    void setMoistureTotal(double d);
    void setDryDensity(double d);
    void setInternalHeatSourcePowerDensity(double d);

signals:
    void conductivityThawedChanged(double d);
    void conductivityFrozenChanged(double d);
    void capacityThawedChanged(double d);
    void capacityFrozenChanged(double d);
    void transitionTemperatureChanged(double d);
    void transitionHeatChanged(double d);
    void usesUnfrozenWaterCurveChanged(bool b);
    void unfrozenWaterCurveChanged(const DoubleMap &m);
    void moistureTotalChanged(double d);
    void dryDensityChanged(double d);
    void internalHeatSourcePowerDensityChanged(double d);

    void moistureTotalMinimumChanged(double d);
    void moistureTotalMaximumChanged(double d);

protected:
    QList<QString> priorityProperties();

private:
    qfcore::SoilBlock mBlock;

    /// Блоки в сцене, грунтом которых мы являемся
    QList<Block *> mBlocks;

    double mOldMoistureTotalMinimum;
    double mOldMoistureTotalMaximum;

    /// Изменяет влажность у @m mBlock, если она недопустима, делает в блоке
    /// updateFromWaterCurve и испускает соответствующие сигналы,
    /// если он изменил температуру и/или теплоту фазового перехода.
    void updateFromWaterCurve();

    friend class ChangeSoilPropertyCommand;
    friend class RemoveSoilsCommand;
    friend class BlockWithOldSoil;
    friend class BlockPortable;
    friend class SoilsModelBase;
};

}

#endif // QFGUI_SOIL_H
