/*
 * Copyright (C) 2010-2012  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_LUG_H
#define QFGUI_LUG_H

#include <QtWidgets/QGraphicsObject>

#include <qfrost.h>

namespace qfgui
{

/**
 * Прямоугольное "ушко".
 * Когда пользователь по нему нажимает, перемещает курсор во View куда надо,
 * испускает сигнал startedChange(bool) и начинает дублировать сигналы анкора в
 * виде changedTo(QPointF) (или не совсем дублировать: возможна замена одной из
 * координат, см. метод slotAnchorPosition).
 * Когда отпускают зажатую кнопку, перестаёт дублировать сигналы анкора и
 * испускает сигнал stoppedChange().
 * При всём этом сама она никак не меняется внешне, но имеется метод setRect, а
 * для изменения видимости следует использовать sitVisible(bool) и связанные
 * методы (hide() и show()) из QGraphicsItem.
 */
class Lug : public QGraphicsObject
{
    Q_OBJECT
public:
    /// Режим показывания ушка.
    enum ShowPolicy {
        /// Показывается всегда
        AlwaysShow,
        /// Никогда не показывается
        NeverShow,
        /// Показывается, только на него пока наведён курсор
        ShowWhileHovered
    };

    Lug(QGraphicsItem *parent = NULL, ShowPolicy policy = AlwaysShow);
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = NULL);
    QRectF boundingRect() const;

    void setRect(qreal x, qreal y, qreal width, qreal height);

    /**
     * Спрятать или показать ушко. Введено вместо setVisibility() из-за
     * того, что тот делает нехорошие вещи, если итем в одном event loop'е
     * сначала показать, а потом спрятать.
     * Сделано вместо методов setVisible, show и hide, которые не подходят.
     */
    void setLugVisibility(bool isVisible);

    void moveTopLeft(const QPointF &point);
    void moveTopRight(const QPointF &point);
    void moveBottomLeft(const QPointF &point);
    void moveBottomRight(const QPointF &point);
    void moveCenter(const QPointF &point);
    void setWidth(qreal width);
    void setHeight(qreal height);

    DEFINETYPE(LUGTYPE)

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    /// Прямоугольник, задающий геометрию ушка в координатах сцены
    QRectF mRect;

    /**
     * Направление, по которому должно "скользить" ушко. 0, если ушко не
     * ограничено в передвижениях. Оба направления, если может скользить в
     * обоих направлениях, то есть должно выбирать то, что ближе).
     */
    Qt::Orientations mRestraintOrientations;

    /**
     * На ушко наведён курсор. Это поле используется при рисовании.
     */
    bool mIsHovered;

    /**
     * Политика рисования.
     */
    ShowPolicy mShowPolicy;

    /**
     * true, если на данный момент не надо это ушко рисовать по внутренним
     * причинам. То есть по связанным с mShowPolicy вещам.
     * HACK: можно бы было использовать флаг ItemHasNoContents, но он глючный, а
     *       именно иногда всё равно делает paint, что приводит к артифактам.
     */
    bool mDontPaint;

    /**
     * true, если на данный момент ушко надо спрятать по внешним причинам.
     * То есть если родитель нас "просит" спрятаться/показаться.
     */
    bool mIsVisible;

    /// Кнопка мыши, с нажатия которой начались изменения
    Qt::MouseButton mChangesButton;

signals:
    /**
     * Пока ушко реагирует на привязку, этот сигнал высылается при каждой смене
     * позиции привязки.
     * @param point новая позиция (возможно, с заменной координатой
     *              относительно реальной позиции привязки).
     */
    void changedTo(const QPointF &point);

    /**
     * Начато реагирование на привязку.
     * @param parentMustResize родитель должен изменять позицию (если false,
     *                           то размеры)
     */
    void startedChange(bool alternateChange);

    /**
     * Остановлено реагирование на привязку.
     */
    void stoppedChange();

    /**
     * Курсор введён или выведен за пределы ушка.
     */
    void hoverStateChanged(bool isHovered);

private slots:
    /**
     * Слот, который реагирует на смену позицию привязки.
     * При нажатии кнопки мыши, нужно подключать к этому слоту соответствующий
     * сигнал привязки, а когда кнопка отпущена, нужно производить отключение.
     * @param point новая позиция привязки.
     */
    void slotAnchorPosition(const QPointF &point);
};

}

#endif // QFGUI_LUG_H
