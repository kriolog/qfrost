/*
 * Copyright (C) 2011-2012  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_NONSCALABLEITEM_H
#define QFGUI_NONSCALABLEITEM_H

#include <QtWidgets/QGraphicsObject>

namespace qfgui
{

/**
 * Итем, имеющий масштабируемый базовый прямоугольник немасштабируемую границу.
 * То есть boundaryRect() зависит от масштаба отображения и представляет из себя
 * базовый прямоугольник с добавленной рамкой - такой, что рамка эта имеет
 * видимую постоянную толщину при масштабировании.
 */
class NonScalableItem : public QGraphicsObject
{
    Q_OBJECT
public:
    /**
     * Стандартный конструктор.
     * @p border - толщина немасштабируемой рамки в пикселах отображения.
     */
    NonScalableItem(QGraphicsItem *parent = NULL, qreal border = 5);

    QRectF boundingRect() const;

private:
    /// Толщина немасштабируемой рамки в пикселах отображения.
    const qreal mBorder;

    /**
     * Толщина рамки в единицах сцены (для текущего масштаба отображения).
     */
    qreal mScaledBorder;

    /// Базовый прямоугольник (к нему со всех сторон прибавляется рамка).
    virtual QRectF baseRect() const {
        return QRectF();
    }

protected:
    /**
     * Толщина рамки в единицах сцены (для текущего масштаба отображения).
     */
    qreal scaledBorder() {
        return mScaledBorder;
    }

    /**
     * Толщина рамки в пикселах отображения.
     */
    qreal border() {
        return mBorder;
    }


    /**
     * Тут мы отслеживаем смену сцены, чтобы установить соединения.
     */
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private slots:
    /**
     * Обновляет mScaledBorder и boundingRect из нового масштаба отображения.
     */
    void onViewScaleChanged(qreal viewScale);
};

}

#endif // QFGUI_NONSCALABLEITEM_H
