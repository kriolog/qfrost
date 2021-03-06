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

#include "blockscountlabel.h"

#include <graphicsviews/scene.h>

#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>

using namespace qfgui;

BlocksCountLabel::BlocksCountLabel(Scene *scene, QWidget *parent)
    : QFrame(parent)
    , mMainLabel(new QLabel(this))
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(QMargins());

    layout->addWidget(mMainLabel);
    mMainLabel->setAlignment(Qt::AlignCenter);

    setBlocksCount(0);
    connect(scene, SIGNAL(blocksCountChanged(int)), SLOT(setBlocksCount(int)));
}

void BlocksCountLabel::setBlocksCount(int blocksCount)
{
    mMainLabel->setText(blocksCount == 0
                        ? tr("No blocks")
                        : tr("%n blocks", "", blocksCount));
}
