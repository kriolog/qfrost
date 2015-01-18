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

#ifndef QFGUI_BLOCKSCOUNTLABEL_H
#define QFGUI_BLOCKSCOUNTLABEL_H

#include <QtWidgets/QFrame>

QT_FORWARD_DECLARE_CLASS(QLabel)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Scene)

class BlocksCountLabel: public QFrame
{
    Q_OBJECT
public:
    BlocksCountLabel(Scene *scene, QWidget *parent = NULL);

private slots:
    void setBlocksCount(int blocksCount);

private:
    QLabel *mMainLabel;
};

}

#endif // QFGUI_BLOCKSCOUNTLABEL_H
