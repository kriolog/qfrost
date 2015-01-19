/*
 * Copyright (C) 2010-2015  Denis Pesotsky
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

#include <undo/blockwitholdparameters.h>
#include <graphicsviews/block.h>
#include <graphicsviews/scene.h>
#include <soils/soil.h>

using namespace qfgui;

BlockWithOldThawedPart::BlockWithOldThawedPart(Block *block)
    : mBlock(block)
    , mOldV(block->mSoilBlock.mThawedPart)
{
    Q_ASSERT(block->scene() != NULL);
}

void BlockWithOldThawedPart::restore()
{
    setThawedPart(mOldV);
}

void BlockWithOldThawedPart::setThawedPart(double newV)
{
    Q_ASSERT(newV >= 0.0 && newV <= 1.0);
    qfcore::SoilBlock *s = &mBlock->mSoilBlock;

    s->setThawedPart(newV);

    Q_ASSERT(s->thawedPartIsOk());

    // перерисовываем блок, только если на нём отображён текст
    if (mBlock->mHasText) {
        mBlock->update();
    }
    mBlock->updateFromThawedPart();
}

void BlockWithOldThawedPart::smartSetThawedPart(double newV)
{
    qfcore::SoilBlock *s = &mBlock->mSoilBlock;
    setThawedPart(!s->usesUnfrozenWaterCurve()
                  ? newV
                  : qMax(newV, s->minBFThawedPart()));
}

////////////////////////////////////////////////////////////////////////////////

BlockWithOldTemperature::BlockWithOldTemperature(Block *block)
    : BlockWithOldThawedPart(block)
    , mOldT(block->mSoilBlock.temperature())
{
    Q_ASSERT(block->scene() != NULL);
}

void BlockWithOldTemperature::setTemperature(double newT)
{
    if (newT != block()->mSoilBlock.temperature()) {
        block()->mSoilBlock.setTemperature(newT);
        block()->updateFromTemperature();
    }
}

double BlockWithOldTemperature::depth() const
{
    return block()->rect().center().y();
}

void BlockWithOldTemperature::restore()
{
    setTemperature(mOldT);
    BlockWithOldThawedPart::restore();
}

////////////////////////////////////////////////////////////////////////////////

BlockWithOldTransitionParameters::BlockWithOldTransitionParameters(Block *block)
    : BlockWithOldThawedPart(block)
{
    Q_ASSERT(block->scene() != NULL);
}

void BlockWithOldTransitionParameters::restore()
{
    getNewPropertiesFromSoil();
    BlockWithOldThawedPart::restore();
}

void BlockWithOldTransitionParameters::getNewPropertiesFromSoil()
{
    qfcore::SoilBlock *s = &block()->mSoilBlock;

    if (block()->mSoil != NULL) {
        s->setAllProperties(block()->mSoil->block());
    } else {
        s->resetTransitionParameters();
    }

    block()->updateFromThawedPart();

    /* Если на блоке отображён текст, перерисуем его
     * (ведь могло обновиться кол-во незамёзшей воды) */
    if (block()->mHasText) {
        block()->update();
    }
}

////////////////////////////////////////////////////////////////////////////////

BlockWithOldSoil::BlockWithOldSoil(Block *block)
    : BlockWithOldTransitionParameters(block)
    , mOldSoil(block->mSoil)
{
    Q_ASSERT(block->scene() != NULL);
}

void BlockWithOldSoil::setSoil(const Soil *soil)
{
    Q_ASSERT(block()->mSoil != soil);

    const Soil *oldSoil = block()->mSoil;
    block()->mSoil = soil;

    if (oldSoil != NULL) {
        const_cast<Soil *>(oldSoil)->removeBlock(block());
    }
    if (soil != NULL) {
        const_cast<Soil *>(soil)->addBlock(block());
    }

    getNewPropertiesFromSoil();

    block()->updateBrush();
    block()->update();
}

void BlockWithOldSoil::restore()
{
    setSoil(mOldSoil);
    BlockWithOldTransitionParameters::restore();
}
