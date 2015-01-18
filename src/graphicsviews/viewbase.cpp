/*
 * Copyright (C) 2014-2015  Denis Pesotsky
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
 *
 */

#include "viewbase.h"

#include "qfrost.h"
#include "zoomslider.h"

#include <QApplication>
#include <QTimer>
#include <QScrollBar>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QtMath>

#include <cmath>

using namespace qfgui;

const double ViewBase::kScaleStep = 1.25;

ViewBase::ViewBase(QGraphicsScene *scene, QWidget *parent,
                   double minScale, double maxScale):
    QGraphicsView(scene, parent),
    mIsHandScrolling(false),
    mHandScrollingPrevCurpos(),
    mAutoScrollTimer(),
    mAutoScrollCount(),
    mMousePos(QFrost::noPoint),
    mMousePosChanged(false),
    mMinimumScale(minScale),
    mMaximumScale(maxScale),
    mMinimumZoomSliderValue(qCeil(qLn(mMinimumScale)/qLn(kScaleStep))),
    mMaximumZoomSliderValue(qFloor(qLn(mMaximumScale)/qLn(kScaleStep))),
    mZoomSliderValue()
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setMouseTracking(true);

    setDragMode(QGraphicsView::NoDrag);

    // у нас обновления, как правило, происходят прямоугольниками
    setViewportUpdateMode(BoundingRectViewportUpdate);

    mAutoScrollTimer = new QTimer(this);
    connect(mAutoScrollTimer, SIGNAL(timeout()), SLOT(doAutoScroll()));

    /* HACK: контектное меню нам не нужно и передавать его дальше не нужно:
     * без этого свойства по непонятным причинам (возможно, баг Qt)
     * может косячить функция setSceneCursorPos -- если обозримую область
     * передвинуло очень далеко, может открыться контекстное меню главного окна.
     */
    setContextMenuPolicy(Qt::PreventContextMenu);

    //TODO потестить setViewportUpdateMode(BoundingRectViewportUpdate);

    setOptimizationFlags(DontSavePainterState);

    //translate(256, 213);
    //rotate(15);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(sendMousePos()));
    // Период 40 мс, т.е. 25 кадров в секунду, как раз для человеского глаза
    timer->start(40);

    setScaleFromSliderValue(mZoomSliderValue);
}

ZoomSlider *ViewBase::createZoomSlider(QWidget *parent)
{
    ZoomSlider *slider = new ZoomSlider(parent);

    slider->setMinimum(mMinimumZoomSliderValue);
    slider->setMaximum(mMaximumZoomSliderValue);

    slider->setValue(mZoomSliderValue);

    connect(slider, SIGNAL(valueChanged(int)), SLOT(setScaleFromSliderValue(int)));

    connect(this, SIGNAL(slidersValuesChanged(int)), slider, SLOT(setValue(int)));

    return slider;
}

void ViewBase::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        if (!event->isAutoRepeat()) {
            viewport()->grabMouse();
            startHandScroll(QCursor::pos());
        }
        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void ViewBase::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !(event->isAutoRepeat())) {
        viewport()->releaseMouse();
        stopHandScroll();
        return;
    }
    QGraphicsView::keyReleaseEvent(event);
}

void ViewBase::leaveEvent(QEvent *event)
{
    sendMousePos();
    emit mouseMoved(QFrost::noPoint);
    QGraphicsView::leaveEvent(event);
}

void ViewBase::mousePressEvent(QMouseEvent *event)
{
    // передаём клик в стандартный обработчик, чтобы тот передал его в сцену
    QGraphicsView::mousePressEvent(event);

    if (event->isAccepted()) {
        // если сцена приняла событие, оно нам ни к чему уже
        return;
    }

    if (event->button() == Qt::RightButton) {
        startHandScroll(event->globalPos());
        return;
    }
}

void ViewBase::mouseMoveEvent(QMouseEvent *event)
{
    if (!mAutoScrollTimer->isActive()) {
        mMousePosChanged = true;
        mMousePos = mapToScene(event->pos());
        if (mIsHandScrolling) {
            doHandScroll(event->globalPos());
        } else {
            tryToStartAutoScroll(event->pos());
        }
    }
    
    /* это нужно как минимум для корректной работы прианкоривания к
     * позиции курсора при трансформациях, то есть после
     * setTransformationAnchor(QGraphicsView::AnchorUnderMouse) */
    QGraphicsView::mouseMoveEvent(event);
}

void ViewBase::mouseReleaseEvent(QMouseEvent *event)
{
    if (mIsHandScrolling) {
        stopHandScroll();
        mMousePosChanged = true;
        mMousePos = mapToScene(event->pos());
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void ViewBase::wheelEvent(QWheelEvent *event)
{
    static const double scaleStep = 1.25;

    qreal tempScaleStep;
    bool signumOfDelta = std::signbit(event->delta());
    if (!signumOfDelta) {
        tempScaleStep = scaleStep;
    } else {
        tempScaleStep = 1.0 / scaleStep;
    }
    /********************************** ZOOM **********************************/
    //TODO: зуммировать пошагово (типа как SmartDoubleSpinBox)
    //transfrom().m11() соответствует масштабу
    if ((!signumOfDelta && (transform().m11() <= mMaximumScale)) ||
        (signumOfDelta && (transform().m11() >= mMinimumScale))) {
        if (transformationAnchor() != AnchorUnderMouse) {
            setTransformationAnchor(AnchorUnderMouse);
        }
        scale(tempScaleStep, tempScaleStep);

        mMousePosChanged = true;
        mMousePos = mapToScene(event->pos());

        if (!signumOfDelta) {
            ++mZoomSliderValue;
        } else {
            --mZoomSliderValue;
        }
        emit slidersValuesChanged(mZoomSliderValue);
        emit scaleChanged(transform().m11());
    }

    // QGraphicsView::wheelEvent(event);
    // -- не должен включаться стандартный скролл, нужен только зум
}

void ViewBase::setScaleFromSliderValue(int value)
{
    if (mZoomSliderValue == value) {
        return;
    }

    mZoomSliderValue = value;

    const double newScaleFactor = qPow(kScaleStep, value);
    QTransform t(newScaleFactor, transform().m12(), transform().m13(),
                 transform().m21(), newScaleFactor, transform().m23(),
                 transform().m31(), transform().m32(), transform().m33());

    const bool mustAnchorCenter = (qobject_cast<ZoomSlider*>(sender()) != NULL);
    if (mustAnchorCenter != (transformationAnchor() == AnchorViewCenter)) {
        setTransformationAnchor(mustAnchorCenter
                                ? AnchorViewCenter
                                : AnchorUnderMouse);
    }
    setTransform(t);

    emit scaleChanged(newScaleFactor);
    emit slidersValuesChanged(mZoomSliderValue);
}

void ViewBase::setScale(double factor)
{
    const int newZoomSliderValue = qRound(qLn(factor)/qLn(kScaleStep));

    setScaleFromSliderValue(newZoomSliderValue);
}

void ViewBase::startHandScroll(const QPoint &pos)
{
    emit startedHandScroll();
    mIsHandScrolling = true;
    mHandScrollingPrevCurpos = pos;
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}

void ViewBase::stopHandScroll()
{
    emit stoppedHandScroll();
    mIsHandScrolling = false;
    QApplication::restoreOverrideCursor();
}

void ViewBase::doHandScroll(const QPoint &pos)
{
    QPoint mousePos = pos;
    QPoint delta = mHandScrollingPrevCurpos - mousePos;
    const QRect screenContainer(QApplication::desktop()->screenGeometry(this));

    QScrollBar *v = verticalScrollBar();
    QScrollBar *h = horizontalScrollBar();

    /** При приближении курсора к краю экрана ближе, чем на это значение, он
     *  будет перемещён на противоположный край */
    static const int margin = 5;

    // If the delta is huge it probably means we just wrapped in that direction
    // and Qt sent bad event
    const QPoint absDelta(abs(delta.x()), abs(delta.y()));
    if (absDelta.y() > screenContainer.height() / 2) {
        delta.setY(mHandScrollingPrevCurpos.y() - QCursor::pos().y());
    }
    if (absDelta.x() > screenContainer.width() / 2) {
        delta.setX(mHandScrollingPrevCurpos.x() - QCursor::pos().x());
    }

    if (mousePos.y() < screenContainer.top() + margin
            && v->value() < v->maximum() - 2 * margin) {
        // перемещаем курсор сверху вниз
        mousePos.setY(screenContainer.bottom() - margin);
    }  else if (mousePos.y() > screenContainer.bottom() - margin
                && v->value() > v->minimum() + 2 * margin) {
        // перемещаем курсор снизу вверх
        mousePos.setY(screenContainer.top() + margin);
    }

    if (mousePos.x() < screenContainer.left() + margin
            && h->value() < h->maximum() - 2 * margin) {
        // перемещаем курсор слева направо
        mousePos.setX(screenContainer.right() - margin);
    }  else if (mousePos.x() > screenContainer.right() - margin
                && h->value() > h->minimum() + 2 * margin) {
        // перемещаем курсор справа налево
        mousePos.setX(screenContainer.left() + margin);
    }

    h->setValue(h->value() + delta.x());
    v->setValue(v->value() + delta.y());

    if (mousePos != pos) {
        QCursor::setPos(mousePos);
        emit mouseJumped(mapToScene(viewport()->mapFromGlobal(mousePos)));
    }
    //запоминаем новую точку отсчёта
    mHandScrollingPrevCurpos = mousePos;
}

void ViewBase::doAutoScroll()
{
    int verticalStep = verticalScrollBar()->pageStep();
    int horizontalStep = horizontalScrollBar()->pageStep();
    if (mAutoScrollCount < qMax(verticalStep, horizontalStep)) {
        // за 1 шаг прокрутки нельзя скакать более, чем на экран
        ++mAutoScrollCount;
    }

    int verticalValue = verticalScrollBar()->value();
    int horizontalValue = horizontalScrollBar()->value();

    QPoint pos = viewport()->mapFromGlobal(QCursor::pos());
    QRect area = viewport()->rect();

    // Мнимая позиция курсора (чтобы он был внутри видимого прямоугольника)
    QPoint imaginaryPos = pos;

    if (sceneChangesOrientations() & Qt::Horizontal) {
        if (pos.x() - area.left() < kAutoScrollViewMargin) {
            horizontalScrollBar()->setValue(horizontalValue - mAutoScrollCount);
            imaginaryPos.setX(area.left());
        } else if (area.right() - pos.x() < kAutoScrollViewMargin) {
            horizontalScrollBar()->setValue(horizontalValue + mAutoScrollCount);
            imaginaryPos.setX(area.right());
        }
    }

    if (sceneChangesOrientations() & Qt::Vertical) {
        if (pos.y() - area.top() < kAutoScrollViewMargin) {
            verticalScrollBar()->setValue(verticalValue - mAutoScrollCount);
            imaginaryPos.setY(area.top());
        } else if (area.bottom() - pos.y() < kAutoScrollViewMargin) {
            verticalScrollBar()->setValue(verticalValue + mAutoScrollCount);
            imaginaryPos.setY(area.bottom());
        }
    }

    bool verticalUnchanged = (verticalValue == verticalScrollBar()->value());
    bool horizontalUnchanged = (horizontalValue == horizontalScrollBar()->value());
    if (verticalUnchanged && horizontalUnchanged) {
        stopAutoScroll();
    }

    mMousePosChanged = true;
    mMousePos = mapToScene(imaginaryPos);
}

void ViewBase::sendMousePos()
{
    if (mMousePosChanged) {
        mMousePosChanged = false;
        emit mouseMoved(mMousePos);
    }
}

void ViewBase::tryToStartAutoScroll(const QPoint &pos)
{
    QRect area = viewport()->rect();

    bool go;
    // нужен скролл по горизонтали
    go = (sceneChangesOrientations() & Qt::Horizontal
          && (pos.x() - area.left() < kAutoScrollViewMargin
              || area.right() - pos.x() < kAutoScrollViewMargin));

    // нужен скролл по вертикали
    go |= (sceneChangesOrientations() & Qt::Vertical
           && (pos.y() - area.top() < kAutoScrollViewMargin
               || area.bottom() - pos.y() < kAutoScrollViewMargin));

    if (go) {
        startAutoScroll();
    }
}

void ViewBase::startAutoScroll()
{
    mAutoScrollTimer->start(40);
    mAutoScrollCount = 0;
}

void ViewBase::stopAutoScroll()
{
    mAutoScrollTimer->stop();
    mAutoScrollCount = 0;
}
