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

#ifndef QFGUI_SETBLOCKSTHAWEDPARTCOMMAND_H
#define QFGUI_SETBLOCKSTHAWEDPARTCOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <undo/blockwitholdparameters.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Block)

class SetBlocksThawedPartCommand : public QUndoCommand
{
public:
    static SetBlocksThawedPartCommand *createCommand(const QList<Block *> &blocks,
            double thawedPart,
            QUndoCommand *parent = 0);
    void undo();
    void redo();
private:
    SetBlocksThawedPartCommand(const QList<Block *> &blocks,
                               double thawedPart,
                               QUndoCommand *parent = 0);

    QList<BlockWithOldThawedPart> mBlocks;
    const double mNewV;
};

}

#endif // QFGUI_SETBLOCKSTHAWEDPARTCOMMAND_H
