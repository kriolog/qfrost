/*
 * Copyright (C) 2010-2013  Denis Pesotsky
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

#ifndef QFGUI_COMPUTATIONDATA_H
#define QFGUI_COMPUTATIONDATA_H

#include <QtCore/QDate>
#include <QtCore/QList>

#include <core/domain.h>
#include <graphicsviews/block.h>
#include <graphicsviews/scene.h>

namespace qfgui
{

/// Информация об одном блоке (температура, кол-во незамёрзшей воды)
class BlockData
{
public:
    BlockData()
        : mTemperature(0)
        , mThawedPart(0) {
    }
    BlockData(const qfcore::SoilBlock &soilBlock)
        : mTemperature(soilBlock.temperature())
        , mThawedPart(soilBlock.thawedPart()) {
    }
    void getData(const Block *block) {
        mTemperature = block->soilBlock()->temperature();
        mThawedPart = block->soilBlock()->thawedPart();
    }
    double temperature() const {
        return mTemperature;
    }
    double thawedPart() const {
        return mThawedPart;
    }
private:
    double mTemperature;
    double mThawedPart;
};

/// Информация о состоянии сцены (состоянии всех блоков и т.п.)
class ComputationData
{
public:
    ComputationData() {}
    ComputationData(const qfcore::Domain &domain,
                    const QDate &date, const QDate &initialDate)
        : mSoilBlocksData()
        , mDate(date)
        , mInitialDate(initialDate) {
        std::vector<qfcore::SoilBlock>::const_iterator it;
        for (it = domain.blocks().begin(); it != domain.blocks().end(); ++it) {
            mSoilBlocksData.append(BlockData(*it));
        }
    }
    ComputationData(const Scene *scene) {
        QList<Block *> blocks = scene->blocksInDomain();
        for (int i = 0; i < blocks.size(); ++i) {
            mSoilBlocksData.append(BlockData());
        }
        foreach(Block * block, blocks) {
            mSoilBlocksData[block->numInDomain()].getData(block);
        }
    }
    const BlockData &soilBlockDataAt(std::size_t num) const {
        return mSoilBlocksData.at(num);
    }
    int numOfBlocks() const {
        return mSoilBlocksData.size();
    }
    const QDate &date() const {
        return mDate;
    }
    const QDate &initialDate() const {
        return mInitialDate;
    }
    bool isEmpty() const {
        return mSoilBlocksData.isEmpty();
    }
private:
    QList<BlockData> mSoilBlocksData;
    /// Дата, информация о которой тут сейчас
    QDate mDate;
    /// Дата, информация о которой была до начала вычислений
    QDate mInitialDate;
};

}

Q_DECLARE_METATYPE(qfgui::ComputationData)

#endif // QFGUI_COMPUTATIONDATA_H
