/*
 * Copyright (C) 2011-2015  Denis Pesotsky
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

#ifndef QFGUI_BLOCKCREATORSETTINGS_H
#define QFGUI_BLOCKCREATORSETTINGS_H

#include <tools_panel/rectangulartoolsettings.h>

namespace qfgui
{

class BlockCreatorSettings: public RectangularToolSettings
{
    Q_OBJECT
public:
    BlockCreatorSettings(QObject *parent)
        : RectangularToolSettings(parent)
        , mSize()
        , mQ()
        , mMustChangePolygons() { }

    void setBlocksSize(QSizeF size) {
        mSize = size;
        emit blocksChanged();
    }
    void setBlocksQ(const QSizeF &q) {
        mQ = q;
        emit blocksChanged();
    }
    void setMustChangePolygons(bool b) {
        mMustChangePolygons = b;
    }

    /**
     * Размер первого блока (в метрах).
     * Ширина и/или высота могут быть равны 0, что соответствует бесконечному
     * размеру по соответствующей оси, -- другими словами, одномерной задаче
     */
    const QSizeF &blocksSize() const {
        Q_ASSERT(mSize.width() >= 0);
        Q_ASSERT(mSize.height() >= 0);
        return mSize;
    }
    const QSizeF &blocksQ() const {
        Q_ASSERT(mQ.width() >= 1);
        Q_ASSERT(mQ.height() >= 1);
        return mQ;
    }
    bool mustChangePolygons() const {
        return mMustChangePolygons;
    }
signals:
    void blocksChanged();
private:
    QSizeF mSize;
    QSizeF mQ;
    bool mMustChangePolygons;
};

}

#endif // QFGUI_BLOCKCREATORSETTINGS_H
