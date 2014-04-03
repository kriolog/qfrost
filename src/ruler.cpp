/*
 * Copyright (C) 2012  Denis Pesotsky
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

#include "ruler.h"

#include <QtWidgets/QScrollBar>
#include <QtWidgets/QGridLayout>

#include "qfrost.h"
#include "view.h"
#include "scene.h"
#include "tools/anchor.h"

using namespace qfgui;

const int Ruler::kBreadth = 18;
const int Ruler::kAnchorTickSize = 4;

QWidget *Ruler::createRulers(View *view)
{
    QWidget *widget = new QWidget();
    QGridLayout *gridLayout = new QGridLayout(widget);
    gridLayout->setSpacing(0);
    gridLayout->setContentsMargins(QMargins());

    Ruler *horizonalRuler = new Ruler(Qt::Horizontal, view);
    Ruler *verticalRuler = new Ruler(Qt::Vertical, view);

    QWidget *corner = new QWidget();
    //corner->setBackgroundRole(QPalette::Window);
    corner->setFixedSize(kBreadth, kBreadth);
    gridLayout->addWidget(corner, 0, 0);
    gridLayout->addWidget(horizonalRuler, 0, 1);
    gridLayout->addWidget(verticalRuler, 1, 0);
    gridLayout->addWidget(view, 1, 1);

    return widget;
}

void Ruler::updateChildrenOffsets(QWidget *parent, bool hideTicks)
{
    foreach(QObject * object, parent->children()) {
        Ruler *ruler = qobject_cast<Ruler *>(object);
        if (ruler == NULL) {
            continue;
        }
        if (hideTicks) {
            ruler->hideCursorTick();
        }
        ruler->updateOffsets();
    }
}

Ruler::Ruler(Qt::Orientation orientation, View *parent)
    : QWidget(parent)
    , mIsHorizontal(orientation == Qt::Horizontal)
    , mOrigin(0.)
    , mUnit(1.)
    , mZoom(1.)
    , mShowCursorTick(false)
    , mShowAnchorTick(false)
    , mCursorPos()
    , mAnchorPos()
    , mCursorScenePos()
    , mAnchorScenePos()
    , mRect()
    , mView(parent)
    , mOffset1()
    , mOffset2()
    , mIsFullyZoomed(false)
{
    QFont txtFont("Sans", 8);
    txtFont.setLetterSpacing(QFont::AbsoluteSpacing, 0.1);
    txtFont.setStretch(QFont::SemiCondensed);
    setFont(txtFont);

    mView->viewport()->installEventFilter(this);

    connect(parent, SIGNAL(scaleChanged(qreal)), this, SLOT(setRulerZoom(qreal)));
    connect(parent, SIGNAL(gridSpanChanged(int)), this, SLOT(setRulerUnit(int)));
    connect(scrollbarForValue(), SIGNAL(valueChanged(int)), this, SLOT(setOrigin(int)));
    connect(parent, SIGNAL(mouseMoved(QPointF)), this, SLOT(setCursorPos(QPointF)));
    connect(parent->qfScene()->anchor(), SIGNAL(signalPositionChanged(QPointF)), this, SLOT(setAnchorPos(QPointF)));
}

QScrollBar *Ruler::scrollbarForValue()
{
    return mIsHorizontal
           ? mView->horizontalScrollBar()
           : mView->verticalScrollBar();
}

void Ruler::setOrigin(const int origin)
{
    if (origin == 0) {
        QScrollBar *s = scrollbarForValue();
        int min = s->minimum();
        if (min == 0 && min == s->maximum()) {
            mIsFullyZoomed = true;
            setOrigin();
            return;
        }
    }
    mIsFullyZoomed = false;
    mOrigin = -origin + mOffset1;
    updateCursorPos();
    updateAnchorPos();
    update();
}

void Ruler::setOrigin()
{
    Q_ASSERT(mIsFullyZoomed);
    // Раз мы сюда попали, значит информация о скроллбарах ничего не даст, но
    // зато заведомо известно, что ноль сцены ровно посередине.
    mOrigin = (mIsHorizontal
               ? mView->mapFromScene(QPointF(0, 0)).x()
               : mView->mapFromScene(QPointF(0, 0)).y())
              + mOffset1;
    updateCursorPos();
    updateAnchorPos();
    update();
}

void Ruler::setRulerUnit(const int rulerUnitInt)
{
    qreal rulerUnit = qreal(rulerUnitInt) / 10.0;
    if (mUnit != rulerUnit) {
        mUnit = rulerUnit;
        update();
    }
}

void Ruler::setRulerZoom(const qreal rulerZoom)
{
    if (mZoom != rulerZoom) {
        mZoom = rulerZoom;
        if (mIsFullyZoomed) {
            setOrigin();
        }
        update();
    }
}

void Ruler::drawAScaleMeter(QPainter *painter, qreal scaleMeter,
                            qreal startPosition, bool drawText)
{
    scaleMeter  = scaleMeter * mUnit * mZoom;

    // Ruler rectangle starting mark
    qreal rulerStartMark = mIsHorizontal ? mRect.left() : mRect.top();
    // Ruler rectangle ending mark
    qreal rulerEndMark = mIsHorizontal ? mRect.right() : mRect.bottom();

    /* Размер, на который надо продлевать линейку слева/сверху, чтобы
     * показывался текст тех отметок, которые не попадают в видимую часть */
    static const qreal additionalSize = 100;
    // Condition A # If origin point is between the start & end mark,
    //we have to draw both from origin to left mark & origin to right mark.
    // Condition B # If origin point is left of the start mark, we have to draw
    // from origin to end mark.
    // Condition C # If origin point is right of the end mark, we have to draw
    // from origin to start mark.
    if (mOrigin >= rulerStartMark && mOrigin <= rulerEndMark) {
        drawFromOriginTo(painter, mOrigin, rulerEndMark, 0, scaleMeter, startPosition, drawText);
        drawFromOriginTo(painter, mOrigin, rulerStartMark - (drawText ? additionalSize : 0),
                         0, -scaleMeter, startPosition, drawText);
    } else if (mOrigin < rulerStartMark) {
        int tickNo = int((rulerStartMark - mOrigin) / scaleMeter);
        drawFromOriginTo(painter, mOrigin + scaleMeter * tickNo,
                         rulerEndMark, tickNo, scaleMeter, startPosition, drawText);
    } else if (mOrigin > rulerEndMark) {
        int tickNo = int((mOrigin - rulerEndMark) / scaleMeter);
        drawFromOriginTo(painter, mOrigin - scaleMeter * tickNo,
                         rulerStartMark - (drawText ? additionalSize : 0),
                         tickNo, -scaleMeter, startPosition, drawText);
    }
}

void Ruler::drawFromOriginTo(QPainter *painter, qreal startMark, qreal endMark,
                             int startTickNo, qreal step, qreal startPosition,
                             bool drawText)
{
    for (qreal current = startMark;
            (step < 0 ? current >= endMark : current <= endMark);
            current += step) {
        qreal x1 = mIsHorizontal ? current : mRect.left() + startPosition;
        qreal y1 = mIsHorizontal ? mRect.top() + startPosition : current;
        qreal x2 = mIsHorizontal ? current : mRect.right();
        qreal y2 = mIsHorizontal ? mRect.bottom() : current;
        painter->drawLine(QLineF(x1, y1, x2, y2));
        if (drawText) {
            QString text = QString::number(QFrost::meters(step * qreal(startTickNo++) / mZoom));
            static const QChar minus = 0x2212;
            text.replace('-', minus);
            if (mIsHorizontal) {
                painter->drawText(x1 + 3, y1 + 9, text);
            } else {
                QRectF boundingRect(x1 + 1, y1 + 2, 7, 1000);
                QRectF lastBoundingRect;
                for (QString::ConstIterator i = text.constBegin();
                        i != text.constEnd(); ++i) {
                    bool isMinus = (*i == minus);
                    if (*i == '.' || isMinus) {
                        boundingRect.adjust(0, -5, 0, 0);
                    }
                    painter->drawText(boundingRect, Qt::AlignHCenter, *i, &lastBoundingRect);
                    boundingRect.setTop(lastBoundingRect.bottom() - (isMinus ? 6 : 3));
                }
            }
        }
    }
}

bool Ruler::eventFilter(QObject *watched, QEvent *event)
{
    // Сначала обрабатываем событие
    bool result = QWidget::eventFilter(watched, event);
    // Если меняет стиль, значит отступы могли измениться, а если только
    // показывается, значит мы ещё не измеряли отступы
    if (event->type() == QEvent::StyleChange
            || event->type() == QEvent::Show) {
        updateOffsets();
    }
    return result;
}

void Ruler::resizeEvent(QResizeEvent *event)
{
    updateRect(event->size());
    QWidget::resizeEvent(event);
}

void Ruler::updateOffsets()
{
    QWidget *viewport = mView->viewport();
    QRect viewRect(mapFromGlobal(viewport->mapToGlobal(QPoint(0, 0))),
                   viewport->size());
    int newOffset1;
    int newOffset2;
    if (mIsHorizontal) {
        newOffset1 = viewRect.left();
        newOffset2 = width() - viewRect.right();
    } else {
        newOffset1 = viewRect.top();
        newOffset2 = height() - viewRect.bottom();
    }
    bool mustUpdateRect = false;
    if (newOffset1 != mOffset1) {
        mOffset1 = newOffset1;
        if (mIsFullyZoomed) {
            setOrigin();
        } else {
            setOrigin(scrollbarForValue()->value());
        }
        mustUpdateRect = true;
    }
    if (newOffset2 != mOffset2) {
        mOffset2 = newOffset2;
        mustUpdateRect = true;
    }
    if (mustUpdateRect) {
        updateRect(size());
    }
}

void Ruler::updateRect(const QSize &size)
{
    mRect = QRectF(QPointF(0, 0), size);
    if (mIsHorizontal) {
        mRect.adjust(mOffset1, 0, -mOffset2, 0);
    } else {
        mRect.adjust(0, mOffset1, 0, -mOffset2);
    }
}

void Ruler::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::TextAntialiasing, true);
    painter.setClipRect(mRect);

    /*painter.fillRect(mRect, Qt::white);
      painter.setPen(Qt::black); */
    painter.setPen(palette().color(QPalette::WindowText));

    drawAScaleMeter(&painter, 10, qreal(kBreadth) / 1.5);
    drawAScaleMeter(&painter, 20, qreal(kBreadth) / 1.8);
    drawAScaleMeter(&painter, 100, 0, true);

    // Сперва привязка, потом курсор: полоска должна быть поверх треугольника
    drawAnchorPosTick(&painter);
    drawMousePosTick(&painter);
}

void Ruler::setCursorPos(const QPointF &cursorPos)
{
    if (cursorPos == QFrost::noPoint) {
        hideCursorTick();
        return;
    }
    QRegion changedRegion;
    if (mShowCursorTick) {
        changedRegion += mCursorRect;
    } else {
        mShowCursorTick = true;
    }
    mCursorScenePos = cursorPos;
    updateCursorPos();
    changedRegion += mCursorRect;
    update(changedRegion);
}

void Ruler::setAnchorPos(const QPointF &pos)
{
    if (pos == QFrost::noPoint) {
        mShowAnchorTick = false;
        update(mAnchorRect);
    } else {
        QRegion changedRegion;
        if (mShowAnchorTick) {
            changedRegion += mAnchorRect;
        } else {
            mShowAnchorTick = true;
        }
        mAnchorScenePos = pos;
        updateAnchorPos();
        changedRegion += mAnchorRect;
        update(changedRegion);
    }
}

void Ruler::hideCursorTick()
{
    mShowCursorTick = false;
    update(mCursorRect);
}

void Ruler::hideAnchorTick()
{
    mShowAnchorTick = false;
    update(mAnchorRect);
}

void Ruler::updateCursorPos()
{
    mCursorPos = mapFromScene(mCursorScenePos);
    mCursorRect = mIsHorizontal
                  ? QRect(mCursorPos.x(), rect().top(), 1, kBreadth)
                  : QRect(rect().left(), mCursorPos.y(), kBreadth, 1);
}

void Ruler::updateAnchorPos()
{
    mAnchorPos = mapFromScene(mAnchorScenePos);
    // В ширину/высоту оно на 1 пиксель шире, т.к. иначе не влезает, проверенно
    mAnchorRect = mIsHorizontal
                  ? QRect(mAnchorPos.x() - kAnchorTickSize,
                          rect().bottom() - kAnchorTickSize,
                          kAnchorTickSize * 2 + 1, kBreadth)
                  : QRect(rect().right() - kAnchorTickSize,
                          mAnchorPos.y() - kAnchorTickSize,
                          kBreadth, kAnchorTickSize * 2 + 1);
}

QPoint Ruler::mapFromScene(const QPointF &point)
{
    return mapFromGlobal(mView->viewport()->mapToGlobal(mView->mapFromScene(point)));
}

void Ruler::drawMousePosTick(QPainter *painter)
{
    if (!mShowCursorTick) {
        return;
    }
    QPoint starPt = mCursorPos;
    QPoint endPt;
    if (mIsHorizontal) {
        starPt.setY(this->rect().top());
        endPt.setX(starPt.x());
        endPt.setY(this->rect().bottom());
    } else {
        starPt.setX(this->rect().left());
        endPt.setX(this->rect().right());
        endPt.setY(starPt.y());
    }
    painter->setOpacity(0.4);
    painter->drawLine(starPt, endPt);
    painter->setOpacity(1);
}

void Ruler::drawAnchorPosTick(QPainter *painter)
{
    if (!mShowAnchorTick) {
        return;
    }
    QPoint pt1 = mAnchorPos;
    QPoint pt2 = pt1;
    QPoint pt3 = pt1;

    if (mIsHorizontal) {
        pt1.setY(this->rect().bottom());
        pt2.setY(this->rect().bottom() - kAnchorTickSize);
        pt3.setY(pt2.y());

        pt2.setX(pt2.x() - kAnchorTickSize);
        pt3.setX(pt3.x() + kAnchorTickSize);
    } else {
        pt1.setX(this->rect().right());
        pt2.setX(this->rect().right() - kAnchorTickSize);
        pt3.setX(pt2.x());

        pt2.setY(pt2.y() - kAnchorTickSize);
        pt3.setY(pt3.y() + kAnchorTickSize);
    }
    //painter->drawLine(pt1,pt2);

    QPolygon polygon;
    polygon << pt1 << pt2 << pt3;

    painter->save();
    painter->setBrush(Qt::magenta);
    painter->drawPolygon(polygon);
    painter->restore();
}
