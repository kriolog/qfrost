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

#ifndef QFGUI_RECTANGULARTOOLSETTINGS_H
#define QFGUI_RECTANGULARTOOLSETTINGS_H

#include <tools_panel/toolsettings.h>

#include <QtCore/QRectF>

namespace qfgui
{

class RectangularToolSettings : public ToolSettings
{
    Q_OBJECT
public:
    RectangularToolSettings(QObject *parent): ToolSettings(parent),
        mSize(), mBasepointPos(), mBasepoint(Qt::TopLeftCorner) {}

    inline QRectF rect() const;
    const Qt::Corner &basePoint() const {
        return mBasepoint;
    }
    const QPointF &basepointPos() const {
        return mBasepointPos;
    }
    const QSizeF &size() const {
        return mSize;
    }
    qreal x() const {
        return mBasepointPos.x();
    }
    qreal y() const {
        return mBasepointPos.y();
    }
    qreal width() const {
        return mSize.width();
    }
    qreal height() const {
        return mSize.height();
    }

    void setBasepoint(Qt::Corner basepoint);
    void setBasepointPos(const QPointF &basepointPos, bool isFromTool = true);
    void setSize(const QSizeF &size, bool isFromTool = true);
    /// Меняет @a mSize и/или @a mBasepointPos, чтобы соответствовать @p rect
    void setRect(const QRectF &rect);
    void setEnabled(bool enabled) {
        emit isNowEnabled(enabled);
    }

signals:
    void basepointChanged();
    void basepointPosChanged(bool senderIsTool);
    void sizeChanged(bool senderIsTool);
    /// Сигнал о том, что настройки нужны/не нужны (инструмент появился/исчез)
    void isNowEnabled(bool enabled);

private:
    /// Размеры прямоугольника инструмента.
    QSizeF mSize;
    /// Позиция опорной точки у прямоугольника инструмента.
    QPointF mBasepointPos;
    /// Опорная точка инструмента.
    Qt::Corner mBasepoint;
    /// Меняет позицию верхнего левого угла без изменения размера.
    void moveTopLeft(const QPointF &pos);
};

}

QRectF qfgui::RectangularToolSettings::rect() const
{
    QRectF result;
    result.setSize(mSize);
    switch (mBasepoint) {
    case Qt::TopLeftCorner:
        result.moveTopLeft(mBasepointPos);
        break;
    case Qt::TopRightCorner:
        result.moveTopRight(mBasepointPos);
        break;
    case Qt::BottomLeftCorner:
        result.moveBottomLeft(mBasepointPos);
        break;
    case Qt::BottomRightCorner:
        result.moveBottomRight(mBasepointPos);
        break;
    }
    return result;
}


#endif // QFGUI_RECTANGULARTOOLSETTINGS_H
