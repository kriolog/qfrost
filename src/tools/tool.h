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

#ifndef QFGUI_TOOL_H
#define QFGUI_TOOL_H

#include <QtWidgets/QGraphicsObject>

#include <qfrost.h>

namespace qfgui
{

class Tool : public QGraphicsObject
{
public:
    Tool() : QGraphicsObject() {
        setZValue(QFrost::ToolZValue);
    }

    /**
     * В каких направлениях на данный момент происходят изменения в инструменте.
     * То есть в каких направлениях может понадобиться автоматическая прокрутка.
     */
    virtual Qt::Orientations changesOrientations() const {
        return 0;
    }

    /**
     * Применить инструмент.
     * @param alt - если true, применить альтернативным способом.
     */
    virtual void apply(bool alt) = 0;

    /**
     * Возвращает пустой прямоугольник.
     */
    virtual QRectF boundingRect() const {
        return QRectF();
    }

    /**
     * Ничего не делает.
     */
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget = 0) {
        Q_UNUSED(painter)
        Q_UNUSED(option)
        Q_UNUSED(widget)
    }

    /**
     * Отмена последней модификации инструмента (например, отмена добавления
     * последней точки в полилинию). По умолчанию ничего не делает.
     */
    virtual void cancelLastChange() {

    }

    /**
     * Текущая точка визуального центра (в координатах сцены).
     * Если визуальный центр отсутствует, QFrost::noPoint.
     * По умолчанию возвращается центр sceneBoundingRect().
     */
    virtual QPointF visualCenter() const {
        return sceneBoundingRect().center();
    }

protected:
    /// Метод вызывается после смены сцены.
    virtual void onSceneHasChanged() = 0;

    /// Метод вызывается непосредственно перед сменой сцены.
    virtual void beforeSceneChange() { }

private:
    /**
     * Для onSceneHasChanged() и beforeSceneChange().
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

}

#endif // QFGUI_TOOL_H
