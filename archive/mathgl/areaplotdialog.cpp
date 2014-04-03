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

#include "areaplotdialog.h"

#include <qfrost.h>
#include <block.h>
#include <plot/areadraw.h>

using namespace qfgui;

AreaPlotDialog::AreaPlotDialog(const Scene *scene, QWidget *parent)
    : PlotDialog(parent)
    , mDraw()
{
    createDraw(scene);
    setDraw(mDraw);

    resize(400, 500);
}

bool xzLessThan2(const Block *b1, const Block *b2)
{
    QPointF center1 = b1->rect().center();
    QPointF center2 = b2->rect().center();
    if (center1.x() == center2.x()) {
        return center1.y() < center2.y();
    } else {
        return center1.x() < center2.x();
    }
}

void AreaPlotDialog::createDraw(const Scene *scene)
{
    QList<Block *> blocks = scene->blocks();
    QVector<QPointF> points;
    QVector<float> temperatures;
    QVector<float> transitionTemperatures;
    QVector<float> thawedParts;
    qSort(blocks.begin(), blocks.end(), xzLessThan2);
    foreach(Block * block, blocks) {
        const qfcore::SoilBlock *soilBlock = block->soilBlock();
        Q_ASSERT(block->soilBlock()->thawedPartIsOk());
        points << QFrost::meters(block->rect().center());
        temperatures << soilBlock->temperature();
        transitionTemperatures << soilBlock->transitionTemperature();
        thawedParts << soilBlock->thawedPart();
    }
    mDraw = new AreaDraw(points,
                         temperatures,
                         transitionTemperatures,
                         thawedParts,
                         this);
}


#include "areaplotdialog.moc"