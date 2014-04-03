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

#ifndef QFGUI_BLOCKPORTABLE_H
#define QFGUI_BLOCKPORTABLE_H

#include <QtCore/QGlobalStatic>
#include <QtCore/QRectF>
#include <QtCore/QList>
#include <QtCore/QDataStream>

#include <block.h>

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(BlockPortable)
}

QDataStream &operator<< (QDataStream &out, const qfgui::BlockPortable &bp);
QDataStream &operator>> (QDataStream &in, qfgui::BlockPortable &bp);

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(ColorGenerator)

/// Вся необходимая для загрузки/сохранения информация о блоке
struct BlockPortable {
public:
    /// Создаёт переносную версию блока @p block
    BlockPortable(Block *block);

    /// Для QList
    BlockPortable();

    /// Создаёт блок из хранящейся в нас информации, опираясь на грунты @p soils
    Block *createBlock(const QList<const Soil *> &soils,
                       const ColorGenerator *colorGenerator);

    /// Заполняет блок, который мы создали, информацией о соседях
    void fillBlockNeighbors(const QList<Block *> &blocks);

    /// Номер блока, площадь контакта с ним и расстояние до Y
    struct NumAndContact {
        NumAndContact(const BlockContact &bs);
        NumAndContact() {
            // для QList
        }
        quint32 otherBlockNum;
        double square;
        double r;
    };

private:
    /// Блок, отображением которого мы являемся
    Block *mBlock;
    /// Температура блока
    double mTemperature;
    /// Кол-во незамёрзшей воды в блоке
    double mThawedPart;
    /// Прямоугольник блока
    QRectF mRect;
    /// Порядковые номера блоков справа от нас
    QList<NumAndContact> mBlocksRight;
    /// Порядковые номера блоков сверху от нас
    QList<NumAndContact> mBlocksTop;
    /// Номер грунта
    quint32 mSoilID;
    /// Температура фазового перехода. Храним её, чтобы не считать снова.
    // FIXME больше не нужна, при следующем изменении формата файлов удалить!
    double mTransitionTemperature;

    friend QDataStream &(::operator<<)(QDataStream &out, const BlockPortable &bp);
    friend QDataStream &(::operator>>)(QDataStream &in, BlockPortable &bp);
};

}

#endif // QFGUI_BLOCKPORTABLE_H
