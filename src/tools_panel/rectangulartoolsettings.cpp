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

#include <tools_panel/rectangulartoolsettings.h>

using namespace qfgui;

void RectangularToolSettings::setBasepoint(Qt::Corner basepoint)
{
    if (mBasepoint == basepoint) {
        return;
    }
    QPointF oldTopLeft = rect().topLeft();
    mBasepoint = basepoint;
    // делаем так, чтоб прямоугольник стал равен старому, метод известит панель
    moveTopLeft(oldTopLeft);

    emit basepointChanged();
}
void RectangularToolSettings::setBasepointPos(const QPointF &basepointPos, bool isFromTool)
{
    if (mBasepointPos == basepointPos) {
        return;
    }
    mBasepointPos = basepointPos;
    emit basepointPosChanged(isFromTool);
}

void RectangularToolSettings::setSize(const QSizeF &size, bool isFromTool)
{
    if (mSize == size) {
        return;
    }
    mSize = size;
    emit sizeChanged(isFromTool);
}

void RectangularToolSettings::moveTopLeft(const QPointF &pos)
{
    if (rect().topLeft() == pos) {
        return;
    }
    QPointF delta;
    switch (mBasepoint) {
    case Qt::TopLeftCorner:
        delta = QPointF(0, 0);
        break;
    case Qt::TopRightCorner:
        delta = QPointF(mSize.width(), 0);
        break;
    case Qt::BottomLeftCorner:
        delta = QPointF(0, mSize.height());
        break;
    case Qt::BottomRightCorner:
        delta = QPointF(mSize.width(), mSize.height());
        break;
    }
    QPointF newBasepointPos = pos + delta;
    if (newBasepointPos != mBasepointPos) {
        mBasepointPos = newBasepointPos;
        emit basepointPosChanged(true);
    }
}

void RectangularToolSettings::setRect(const QRectF &rect)
{
    setSize(rect.size());
    moveTopLeft(rect.topLeft());
}
