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

#ifndef QFGUI_READFROMCOMPUTATIONDATACOMMAND_H
#define QFGUI_READFROMCOMPUTATIONDATACOMMAND_H

#include <undo/setdatecommand.h>

#include <QtCore/QPair>

#include <computations/computationdata.h>

namespace qfgui
{

class ReadFromComputationDataCommand : public SetDateCommand
{
public:
    /**
     * Заполняет @a mNewData из @p data, а @a mOldData из @p scene.
     * Также изменяет время у виджета с начальным временем.
     * @param isFinal -- является ли команда последней в рассчёте.
     */
    ReadFromComputationDataCommand(Scene *scene,
                                   const ComputationData &data,
                                   bool isFinal = false,
                                   QUndoCommand *parent = 0);
    virtual void undo();
    virtual void redo();
    virtual int id() const {
        return 2;
    }
    /// Забираем у @p other @a mNewData, если та команда не финальная.
    virtual bool mergeWith(const QUndoCommand *other);
private:
    typedef QPair<Block *, std::size_t> BlockAndNum;
    /// Данные, которые были в сцене
    ComputationData mOldData;
    /// Данные, которые стали в сцене
    ComputationData mNewData;
    /// Список пар блок-номер (номер -- это номер информации о блоке в данных)
    QList<BlockAndNum> mBlocksAndNums;
    /// Является ли это команда финальной в череде команд одного рассчёта.
    bool mIsFinal;

    /// Обновляет текст исходя из дат
    void updateText();
};

}

#endif // QFGUI_READFROMCOMPUTATIONDATACOMMAND_H
