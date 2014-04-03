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

#ifndef QFGUI_REMOVEBLOCKSCOMMAND_H
#define QFGUI_REMOVEBLOCKSCOMMAND_H

#include <QtWidgets/QUndoCommand>

QT_FORWARD_DECLARE_CLASS(QGraphicsItem)
namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Block)
QT_FORWARD_DECLARE_CLASS(Scene)

class RemoveBlocksCommand : public QUndoCommand
{
public:
    RemoveBlocksCommand(Scene *scene, const QList<Block *> &blocks,
                        QUndoCommand *parent = 0);
    ~RemoveBlocksCommand();
    void undo();
    void redo();
private:
    Scene *mScene;
    QList<Block *> mBlocks;
};

}

#endif // QFGUI_REMOVEBLOCKSCOMMAND_H
