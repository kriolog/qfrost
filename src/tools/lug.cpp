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

#include <tools/lug.h>

#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <graphicsviews/scene.h>
#include <tools/anchor.h>

using namespace qfgui;

Lug::Lug(QGraphicsItem *parent, ShowPolicy showPolicy):
    QGraphicsObject(parent),
    mRect(),
    mRestraintOrientations(0),
    mIsHovered(false),
    mShowPolicy(showPolicy),
    mDontPaint(mShowPolicy == AlwaysShow ? false : true),
    mIsVisible(false),
    mChangesButton(Qt::NoButton)
{
    setAcceptHoverEvents(true);
}

void Lug::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Qt::MouseButton button = event->button();

    // на случай, если мы получили событие искуственно
    if (!mIsHovered) {
        // эмулируем вход курсора
        hoverEnterEvent(NULL);
    }

    if ((button != Qt::LeftButton && button != Qt::RightButton) || mChangesButton != Qt::NoButton) {
        // нам нужны только левые и правые клики, остальное игнорируем
        // + нажатие какой-нибудь кнопки при уже идущих изменениях нам не нужно
        event->ignore();
        return;
    }

    mChangesButton = button;

    emit startedChange(event->button() == Qt::RightButton);

    Scene *tmpScene = qobject_cast<Scene *>(scene());
    Q_ASSERT(tmpScene != NULL);
    connect(tmpScene->anchor(),
            SIGNAL(signalPositionChanged(const QPointF &)),
            SLOT(slotAnchorPosition(const QPointF &)));
}

void Lug::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    if (event->button() != mChangesButton) {
        event->ignore();
        return;
    }

    mChangesButton = Qt::NoButton;

    ungrabMouse();
    Scene *tmpScene = qobject_cast<Scene *>(scene());
    Q_ASSERT(tmpScene != NULL);
    disconnect(tmpScene->anchor(),
               SIGNAL(signalPositionChanged(const QPointF &)),
               this, SLOT(slotAnchorPosition(const QPointF &)));

    emit stoppedChange();

    // на случай, если мы начали изменения искуственно
    if (mIsHovered && !boundingRect().contains(event->pos())) {
        hoverLeaveEvent(NULL);
    }
}

void Lug::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    mIsHovered = true;
    if (mShowPolicy == ShowWhileHovered) {
        mDontPaint = false;
    }

    update();
    emit hoverStateChanged(true);
}

void Lug::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)
    if (mChangesButton != Qt::NoButton) {
        // Если ещё нажата кнопка, считаем, что на нас курсор ещё наведён.
        // Это HACK изменившегося в Qt5 поведения: мы теперь получаем это
        // событие, если курсор вышел за пределы окна.
        return;
    }
    mIsHovered = false;
    if (mShowPolicy == ShowWhileHovered) {
        mDontPaint = true;
    }
    /* Просто вызов нашего метода update() не катит, ибо это ничего
     * не сделает при наличии флага ItemHasNoContents.
     */
    update();
    emit hoverStateChanged(false);
}

void Lug::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (mDontPaint || !mIsVisible) {
        return;
    }
    painter->setBrush(QBrush(QColor(40, 40, 40, mIsHovered ? 60 : 30)));
    painter->setPen(QPen(Qt::darkCyan, 0, Qt::SolidLine));
    painter->drawRect(mRect);
}

QRectF Lug::boundingRect() const
{
    return mRect;
}

void Lug::slotAnchorPosition(const QPointF &point)
{
    if (point == QFrost::noPoint) {
        return;
    }
    emit changedTo(point);
}

void Lug::setRect(qreal x, qreal y, qreal width, qreal height)
{
    prepareGeometryChange();
    mRect = QRectF(x, y, width, height);
}

void Lug::setLugVisibility(bool isVisible)
{
    if (mIsVisible == isVisible) {
        return;
    }
    mIsVisible = isVisible;
    if (!mDontPaint) {
        update();
    }
}

void Lug::moveTopLeft(const QPointF &point)
{
    prepareGeometryChange();
    mRect.moveTopLeft(point);
}

void Lug::moveTopRight(const QPointF &point)
{
    prepareGeometryChange();
    mRect.moveTopRight(point);
}

void Lug::moveBottomLeft(const QPointF &point)
{
    prepareGeometryChange();
    mRect.moveBottomLeft(point);
}

void Lug::moveBottomRight(const QPointF &point)
{
    prepareGeometryChange();
    mRect.moveBottomRight(point);
}

void Lug::moveCenter(const QPointF &point)
{
    prepareGeometryChange();
    mRect.moveCenter(point);
}

void Lug::setWidth(qreal width)
{
    prepareGeometryChange();
    mRect.setWidth(width);
}

void Lug::setHeight(qreal height)
{
    prepareGeometryChange();
    mRect.setHeight(height);
}
