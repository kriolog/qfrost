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

#ifndef QFGUI_ADDBLOCKSCOMMAND_H
#define QFGUI_ADDBLOCKSCOMMAND_H

#include <QtWidgets/QUndoCommand>

QT_FORWARD_DECLARE_CLASS(QGraphicsItem)
QT_FORWARD_DECLARE_CLASS(QRectF)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Block)
QT_FORWARD_DECLARE_CLASS(Scene)

class AddBlocksCommand : public QUndoCommand
{
public:
    AddBlocksCommand(Scene *scene,
                     const QList<QList<QRectF> > &blocksRects,
                     bool mustChangeOuterPolygons,
                     QUndoCommand *parent = 0);
    ~AddBlocksCommand();

    void undo();
    void redo();

private:
    Scene *mScene;

    /// Все добавленные блоки
    QList<Block *> mAddedBlocks;
    /// Добавленные блоки, граничащие с теми, что были и остались, сверху
    QList<Block *> mAddedBlocksTop;
    /// Добавленные блоки, граничащие с теми, что были и остались, снизу
    QList<Block *> mAddedBlocksBottom;
    /// Добавленные блоки, граничащие с теми, что были и остались, слева
    QList<Block *> mAddedBlocksLeft;
    /// Добавленные блоки, граничащие с теми, что были и остались, справа
    QList<Block *> mAddedBlocksRight;

    /// Все удалённые блоки
    QList<Block *> mRemovedBlocks;
    /// Удалённые блоки, граничащие с теми, что были и остались, сверху
    QList<Block *> mRemovedBlocksTop;
    /// Удалённые блоки, граничащие с теми, что были и остались, снизу
    QList<Block *> mRemovedBlocksBottom;
    /// Удалённые блоки, граничащие с теми, что были и остались, слева
    QList<Block *> mRemovedBlocksLeft;
    /// Удалённые блоки, граничащие с теми, что были и остались, справа
    QList<Block *> mRemovedBlocksRight;
};

}

#endif // QFGUI_ADDBLOCKSCOMMAND_H
