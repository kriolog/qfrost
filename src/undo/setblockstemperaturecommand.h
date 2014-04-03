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

#ifndef QFGUI_SETBLOCKSTEMPERATURECOMMAND_H
#define QFGUI_SETBLOCKSTEMPERATURECOMMAND_H

#include <QtWidgets/QUndoCommand>

#include <undo/blockwitholdparameters.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Block)

class SetBlocksTemperatureCommand : public QUndoCommand
{
public:
    static SetBlocksTemperatureCommand *createCommand(const QList<Block *> &blocks,
            double temperature,
            QUndoCommand *parent = 0);

    static SetBlocksTemperatureCommand *createCommand(const QList<Block *> &blocks,
            double t1, double t2,
            double depth1, double depth2,
            QUndoCommand *parent = 0);
    void undo();
    void redo();

private:
    /// Конструктор для применения одной температуры
    SetBlocksTemperatureCommand(const QList<Block *> &blocks,
                                double temperature,
                                QUndoCommand *parent = 0);
    /// Конструктор для применения градиента температур
    SetBlocksTemperatureCommand(const QList<Block *> &blocks,
                                double t1, double t2,
                                double depth1, double depth2,
                                QUndoCommand *parent = 0);

    /// Температура, которую получит блок на глубине @p depth
    double temperatureFromGradient(double depth);
    /// Температура, которую получит блок @p block
    double temperatureFromGradient(const Block *block);
    /// Температура, которую получит блок @p block
    double temperatureFromGradient(const BlockWithOldTemperature &block);

    QList<BlockWithOldTemperature> mBlocks;
    double mTemperature;
    double mTemperature2;
    double mDepth1;
    double mDepth2;
    double mIsGradient;
};

}

#endif // QFGUI_SETBLOCKSTEMPERATURECOMMAND_H
