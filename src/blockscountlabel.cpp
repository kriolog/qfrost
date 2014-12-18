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

#include "blockscountlabel.h"

#include <graphicsviews/scene.h>

using namespace qfgui;

BlocksCountLabel::BlocksCountLabel(Scene *scene, QWidget *parent)
    : QLabel(parent)
{
    setBlocksCount(0);
    connect(scene, SIGNAL(blocksCountChanged(int)), SLOT(setBlocksCount(int)));
}

void BlocksCountLabel::setBlocksCount(int blocksCount)
{
    setText(tr("%n blocks", "", blocksCount));
}
