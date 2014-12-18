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

#include <tools/rectangulartool.h>

#include <QtGui/QPainter>
#include <QtWidgets/QApplication>

#include <graphicsviews/scene.h>
#include <graphicsviews/view.h>
#include <tools/lug.h>
#include <tools/anchor.h>
#include <tools_panel/rectangulartoolsettings.h>

using namespace qfgui;

RectangularTool::RectangularTool(ToolSettings *settings,
                                 bool allowMovingCenter):
    Tool(),
    mSize(0, 0),
    mTopLeftLug(new Lug(this)),
    mTopRightLug(new Lug(this)),
    mBottomLeftLug(new Lug(this)),
    mBottomRightLug(new Lug(this)),
    mLeftLug(new Lug(this, Lug::ShowWhileHovered)),
    mRightLug(new Lug(this, Lug::ShowWhileHovered)),
    mTopLug(new Lug(this, Lug::ShowWhileHovered)),
    mBottomLug(new Lug(this, Lug::ShowWhileHovered)),
    mCenterLug(allowMovingCenter ? new Lug(this, Lug::NeverShow) : NULL),
    mActiveLug(NULL),
    mActiveLugLocation(bottomRight),
    mChangesType(notChangig),
    mChangesOrientations(0),
    mLugsAreInner(false),
    mLugsMaxSize(0),
    mLugsMinSize(0),
    mSettings(qobject_cast<RectangularToolSettings *>(settings))
{
    Q_ASSERT(mSettings != NULL);

    updateLugsCursors();

    foreach(QGraphicsItem * item, childItems()) {
        Lug *l = qgraphicsitem_cast<Lug *>(item);
        Q_ASSERT(l != NULL);
        connect(l, SIGNAL(startedChange(bool)), this, SLOT(startChange(bool)));
        connect(l, SIGNAL(stoppedChange()), this, SLOT(stopChange()));
        connect(l, SIGNAL(changedTo(QPointF)), this, SLOT(changeTo(QPointF)));
        connect(l, SIGNAL(hoverStateChanged(bool)),
                this, SLOT(onLugHoverChanged(bool)));
    }

    connect(mSettings, SIGNAL(sizeChanged(bool)), SLOT(getSizeFromSettings(bool)));
    connect(mSettings, SIGNAL(basepointPosChanged(bool)), SLOT(getTopLeftPosFromSettings(bool)));
}

void RectangularTool::updateLugsCursors()
{
    mLeftLug->setCursor(Qt::SizeHorCursor);
    mRightLug->setCursor(Qt::SizeHorCursor);
    mTopLug->setCursor(Qt::SizeVerCursor);
    mBottomLug->setCursor(Qt::SizeVerCursor);
    mTopLeftLug->setCursor(Qt::SizeFDiagCursor);
    mBottomRightLug->setCursor(Qt::SizeFDiagCursor);
    mTopRightLug->setCursor(Qt::SizeBDiagCursor);
    mBottomLeftLug->setCursor(Qt::SizeBDiagCursor);
    if (mCenterLug != NULL) {
        mCenterLug->setCursor(Qt::SizeAllCursor);
    }
}

void RectangularTool::onSceneHasChanged()
{
    if (scene() != NULL) {
        connect(scene(), SIGNAL(mainViewScaleChanged(qreal)),
                this, SLOT(updateLugsSizeLimits(qreal)));

        Q_ASSERT(!scene()->views().isEmpty());
        View *qfview = static_cast<View *>(scene()->views().first());
        updateLugsSizeLimits(qfview->transform().m11());

        // *** всё, что связано с растяжением сразу после создания ***
        // делаем его маленьким прямоугольником _внутри_ себя
        mBottomRightLug->setRect(mSize.width() - mLugsMinSize,
                                 mSize.height() - mLugsMinSize,
                                 mLugsMinSize, mLugsMinSize);
        mLugsAreInner = true;
        // после emulateMouseHover мы его показали (в слоте), надо спрятать,
        mBottomRightLug->setLugVisibility(false);
        // после добавления в сцену, она нам сразу передаст нажатие, которым
        // мы были созданы (левый клик), пусть его перехватит нужное ушко
        mBottomRightLug->grabMouse();

        mSettings->setEnabled(true);
    } else {
        mSettings->setEnabled(false);
    }
}

void RectangularTool::updateLugsSizeLimits(qreal scale)
{
    mLugsMaxSize = 50 / scale;
    mLugsMinSize = 15 / scale;
    updateLugsGeometry();
}

QRectF RectangularTool::boundingRect() const
{
    return QRectF(QPointF(), mSize);
}

void RectangularTool::updateLugsGeometry()
{
    /// Текущая ширина инструмента.
    qreal width = mSize.width();
    /// Текущая высота инструмента.
    qreal height = mSize.height();

    if (qMin(width, height) >= mLugsMinSize * 3) {
        // Делаем язычки внутри инструмента.
        mLugsAreInner = true;
        /// Ширина угловых язычков
        qreal cornerLugWidth;
        /// Ширина верхнего и нижнего боковых язычков
        qreal sideLugWidth;
        /// Горизонтальные отступы между угловыми и боковыми язычками
        qreal widthMargin;
        if (width < mLugsMinSize * 4) {
            // Ширина меньше четырёх минимальных размеров язычка. Изменяются
            // отступы между угловыми и боковыми язычками, ширина всех язычков
            // равна минимальной.
            cornerLugWidth = mLugsMinSize;
            sideLugWidth = cornerLugWidth;
            widthMargin = (width - 3 * mLugsMinSize) / 2;
        } else if (width < mLugsMaxSize * 4) {
            // Ширина находится в интервале от четырёх минимальных до четырёх
            // максимальных размеров язычка. Изменяются
            // отступы и язычки.
            cornerLugWidth = width / 4;
            sideLugWidth = cornerLugWidth;
            widthMargin = width / 8;
        } else {
            // Ширина больше четырёх максимальных размеров язычка. Изменяются
            // отступы и боковые язычки.
            // Ширина угловых язычков равна максимальной.
            cornerLugWidth = mLugsMaxSize;
            widthMargin = cornerLugWidth / 2;
            sideLugWidth = width - 3 * mLugsMaxSize;
        }

        /// Высота угловых язычков
        qreal cornerLugHeight;
        /// Высота левого и правого боковых язычков
        qreal sideLugHeight;
        /// Вертикальные отступы между угловыми и боковыми язычками
        qreal heightMargin;
        if (height < mLugsMinSize * 4) {
            // Высота меньше четырёх минимальных размеров язычка. Изменяются
            // отступы между угловыми и боковыми язычками, высота всех язычков
            // равна минимальной.
            cornerLugHeight = mLugsMinSize;
            sideLugHeight = cornerLugHeight;
            heightMargin = (height - 3 * mLugsMinSize) / 2;
        } else if (height < mLugsMaxSize * 4) {
            // Высота находится в интервале от четырёх минимальных до четырёх
            // максимальных размеров язычка. Изменяются
            // отступы и язычки.
            cornerLugHeight = height / 4;
            sideLugHeight = cornerLugHeight;
            heightMargin = height / 8;
        } else {
            // Высота больше четырёх максимальных размеров язычка. Изменяются
            // отступы и боковые язычки.
            // Высота угловых язычков равна максимальной.
            cornerLugHeight = mLugsMaxSize;
            heightMargin = cornerLugHeight / 2;
            sideLugHeight = height - 3 * mLugsMaxSize;
        }

        mTopLeftLug->setRect(0, 0, cornerLugWidth, cornerLugHeight);
        mTopRightLug->setRect(width - cornerLugWidth, 0,
                              cornerLugWidth, cornerLugHeight);
        mBottomLeftLug->setRect(0, height - cornerLugHeight, cornerLugWidth,
                                cornerLugHeight);
        mBottomRightLug->setRect(width - cornerLugWidth, height - cornerLugHeight,
                                 cornerLugWidth, cornerLugHeight);

        mLeftLug->setRect(0, cornerLugHeight + heightMargin,
                          cornerLugWidth, sideLugHeight);
        mRightLug->setRect(width - cornerLugWidth, cornerLugHeight + heightMargin,
                           cornerLugWidth, sideLugHeight);
        mTopLug->setRect(cornerLugWidth + widthMargin, 0,
                         sideLugWidth, cornerLugHeight);
        mBottomLug->setRect(cornerLugWidth + widthMargin, height - cornerLugHeight,
                            sideLugWidth, cornerLugHeight);

        if (mCenterLug != NULL) {
            mCenterLug->setRect(cornerLugWidth, cornerLugHeight,
                                sideLugWidth + 2 * widthMargin,
                                sideLugHeight + 2 * heightMargin);
        }

    } else {
        // Делаем язычки снаружи инструмента
        mLugsAreInner = false;
        mTopLeftLug->setRect(-mLugsMinSize, -mLugsMinSize, mLugsMinSize, mLugsMinSize);
        mTopRightLug->setRect(width, -mLugsMinSize, mLugsMinSize, mLugsMinSize);
        mBottomLeftLug->setRect(-mLugsMinSize, height, mLugsMinSize, mLugsMinSize);
        mBottomRightLug->setRect(width, height, mLugsMinSize, mLugsMinSize);

        mLeftLug->setRect(-mLugsMinSize, 0, mLugsMinSize, height);
        mRightLug->setRect(width, 0, mLugsMinSize, height);
        mTopLug->setRect(0, -mLugsMinSize, width, mLugsMinSize);
        mBottomLug->setRect(0, height, width, mLugsMinSize);

        if (mCenterLug != NULL) {
            mCenterLug->setRect(0, 0, width, height);
        }
    }
}

void RectangularTool::updateActiveLugGeometry()
{
    QRectF toolRect = boundingRect();

    if (mLugsAreInner) {
        if (mActiveLugLocation != center) {
            if (mActiveLug->boundingRect().width() > mSize.width() ||
                    mActiveLug->boundingRect().height() > mSize.height()) {
                mActiveLug->setLugVisibility(false);
            } else {
                mActiveLug->setLugVisibility(true);
            }
        }
        switch (mActiveLugLocation) {
        case topLeft:
            mActiveLug->moveTopLeft(toolRect.topLeft());
            break;
        case topRight:
            mActiveLug->moveTopRight(toolRect.topRight());
            break;
        case bottomLeft:
            mActiveLug->moveBottomLeft(toolRect.bottomLeft());
            break;
        case bottomRight:
            mActiveLug->moveBottomRight(toolRect.bottomRight());
            break;
        case left:
            mActiveLug->setHeight(toolRect.height());
            mActiveLug->moveTopLeft(toolRect.topLeft());
            break;
        case right:
            mActiveLug->setHeight(toolRect.height());
            mActiveLug->moveTopRight(toolRect.topRight());
            break;
        case top:
            mActiveLug->setWidth(toolRect.width());
            mActiveLug->moveTopLeft(toolRect.topLeft());
            break;
        case bottom:
            mActiveLug->setWidth(toolRect.width());
            mActiveLug->moveBottomLeft(toolRect.bottomLeft());
            break;
        case center:
            mActiveLug->moveCenter(toolRect.center());
            break;
        }
    } else {
        switch (mActiveLugLocation) {
        case topLeft:
            mActiveLug->moveBottomRight(toolRect.topLeft());
            break;
        case topRight:
            mActiveLug->moveBottomLeft(toolRect.topRight());
            break;
        case bottomLeft:
            mActiveLug->moveTopRight(toolRect.bottomLeft());
            break;
        case bottomRight:
            mActiveLug->moveTopLeft(toolRect.bottomRight());
            break;
        case left:
            mActiveLug->moveTopRight(toolRect.topLeft());
            break;
        case right:
            mActiveLug->moveTopLeft(toolRect.topRight());
            break;
        case top:
            mActiveLug->moveBottomLeft(toolRect.topLeft());
            break;
        case bottom:
            mActiveLug->moveTopLeft(toolRect.bottomLeft());
            break;
        case center:
            mActiveLug->moveCenter(toolRect.center());
            break;
        }
    }
}

void RectangularTool::startChange(bool alternateChange)
{
    Lug *senderLug = qobject_cast<Lug *>(sender());
    Q_ASSERT(senderLug != NULL);
    setActiveLug(senderLug);

    mChangesType = alternateChange ? moving : resizing;
    if (mActiveLug == mCenterLug) {
        mChangesType = moving;
        hideAllLugsExcept(mCenterLug);
    }

    switch (mActiveLugLocation) {
    case topLeft:
    case topRight:
    case bottomLeft:
    case bottomRight:
    case center:
        mChangesOrientations = Qt::Horizontal | Qt::Vertical;
        break;
    case left:
    case right:
        mChangesOrientations = Qt::Horizontal;
        break;
    case top:
    case bottom:
        mChangesOrientations = Qt::Vertical;
        break;
    }

    moveCursorToActiveLug();
    QApplication::setOverrideCursor(mActiveLug->cursor());

    // Дабы он спрятался, если изменения были искуственно вызвано и он не влазит
    updateActiveLugGeometry();

    // чтобы инструмент перерисовался, если его вид зависит от mChangesType
    update();

    onStartChange();
}

void RectangularTool::moveCursorToActiveLug() const
{
    Scene *tmpScene = qobject_cast<Scene *>(scene());
    Q_ASSERT(tmpScene != NULL);
    View *view = tmpScene->qfView();

    switch (mActiveLugLocation) {
    case topLeft:
        view->setSceneCursorPos(mapToScene(boundingRect().topLeft()));
        break;
    case topRight:
        view->setSceneCursorPos(mapToScene(boundingRect().topRight()));
        break;
    case bottomLeft:
        view->setSceneCursorPos(mapToScene(boundingRect().bottomLeft()));
        break;
    case bottomRight:
        view->setSceneCursorPos(mapToScene(boundingRect().bottomRight()));
        break;
    case left:
        view->setSceneCursorX(mapToScene(boundingRect().topLeft()).x());
        break;
    case right:
        view->setSceneCursorX(mapToScene(boundingRect().topRight()).x());
        break;
    case top:
        view->setSceneCursorY(mapToScene(boundingRect().topLeft()).y());
        break;
    case bottom:
        view->setSceneCursorY(mapToScene(boundingRect().bottomLeft()).y());
        break;
    case center:
        view->setSceneCursorPos(mapToScene(boundingRect().center()));
        break;
    }
}

void RectangularTool::onLugHoverChanged(bool lugIsHovered)
{
    if (sender() != mCenterLug) {
        if (lugIsHovered) {
            QGraphicsObject *senderGraphicsObject;
            senderGraphicsObject = qobject_cast<QGraphicsObject *>(sender());
            Q_ASSERT(senderGraphicsObject != NULL);
            hideAllLugsExcept(senderGraphicsObject);
        } else {
            showAllLugs();
        }
    }
}

void RectangularTool::stopChange()
{
    mChangesType = notChangig;
    if (mActiveLug == mCenterLug) {
        showAllLugs();
    }

    mActiveLug = NULL;
    mChangesOrientations = 0;
    updateLugsGeometry();

    QApplication::restoreOverrideCursor();

    onStopChange();
}

void RectangularTool::hideAllLugsExcept(const QGraphicsObject *object)
{
    foreach(QGraphicsItem * item, childItems()) {
        Lug *lug = qgraphicsitem_cast<Lug *>(item);
        if (lug != NULL) {
            lug->setLugVisibility(lug == object);
        }
    }
}

void RectangularTool::showAllLugs()
{
    foreach(QGraphicsItem * item, childItems()) {
        Lug *lug = qgraphicsitem_cast< Lug * >(item);
        if (lug != NULL) {
            lug->setLugVisibility(true);
        }
    }
}

void RectangularTool::updateGeometryAndLugPointers(const QRectF &rect)
{
    bool hadToNormalize = false;
    if (rect.width() < 0.0) {
        // Отражаем относительно вертикальной оси
        qSwap(mTopLeftLug, mTopRightLug);
        qSwap(mLeftLug, mRightLug);
        qSwap(mBottomLeftLug, mBottomRightLug);
        switch (mActiveLugLocation) {
        case topLeft:
            mActiveLugLocation = topRight;
            break;
        case topRight:
            mActiveLugLocation = topLeft;
            break;
        case bottomLeft:
            mActiveLugLocation = bottomRight;
            break;
        case bottomRight:
            mActiveLugLocation = bottomLeft;
            break;
        case left:
            mActiveLugLocation = right;
            break;
        case right:
            mActiveLugLocation = left;
            break;
        case top:
        case bottom:
        case center:
            break;
        }
        hadToNormalize = true;
    }
    if (rect.height() < 0.0) {
        // Отражаем относительно горизонтальной оси
        qSwap(mTopLeftLug, mBottomLeftLug);
        qSwap(mTopLug, mBottomLug);
        qSwap(mTopRightLug, mBottomRightLug);
        switch (mActiveLugLocation) {
        case topLeft:
            mActiveLugLocation = bottomLeft;
            break;
        case topRight:
            mActiveLugLocation = bottomRight;
            break;
        case bottomLeft:
            mActiveLugLocation = topLeft;
            break;
        case bottomRight:
            mActiveLugLocation = topRight;
            break;
        case left:
        case right:
            break;
        case top:
            mActiveLugLocation = bottom;
            break;
        case bottom:
            mActiveLugLocation = top;
            break;
        case center:
            break;
        }
        hadToNormalize = true;
    }

    QRectF normalizedRect;

    if (hadToNormalize) {
        normalizedRect = rect.normalized();
        updateLugsCursors();
        QApplication::restoreOverrideCursor();
        QApplication::setOverrideCursor(mActiveLug->cursor());
    } else {
        normalizedRect = rect;
    }

    mSettings->setRect(QFrost::meters(normalizedRect));

    if (normalizedRect.size() != mSize) {
        prepareGeometryChange();
        mSize.setHeight(normalizedRect.height());
        mSize.setWidth(normalizedRect.width());
    }

    if (normalizedRect.topLeft() != pos()) {
        setPos(normalizedRect.topLeft());
    }
}

void RectangularTool::setActiveLug(Lug *lug)
{
    mActiveLug = lug;
    if (mActiveLug == mTopLeftLug) {
        mActiveLugLocation = topLeft;
    } else if (mActiveLug == mTopRightLug) {
        mActiveLugLocation = topRight;
    } else if (mActiveLug == mBottomLeftLug) {
        mActiveLugLocation = bottomLeft;
    } else if (mActiveLug == mBottomRightLug) {
        mActiveLugLocation = bottomRight;
    } else if (mActiveLug == mLeftLug) {
        mActiveLugLocation = left;
    } else if (mActiveLug == mRightLug) {
        mActiveLugLocation = right;
    } else if (mActiveLug == mTopLug) {
        mActiveLugLocation = top;
    } else if (mActiveLug == mBottomLug) {
        mActiveLugLocation = bottom;
    } else if (mActiveLug == mCenterLug) {
        mActiveLugLocation = center;
    }
}

void RectangularTool::changeTo(const QPointF &point)
{
    QRectF resultingRect(pos(), mSize);
    if (mChangesType == resizing) {
        //растягиваем инструмент
        switch (mActiveLugLocation) {
        case topLeft:
            resultingRect.setTopLeft(point);
            break;
        case topRight:
            resultingRect.setTopRight(point);
            break;
        case bottomLeft:
            resultingRect.setBottomLeft(point);
            break;
        case bottomRight:
            resultingRect.setBottomRight(point);
            break;
        case left:
            resultingRect.setLeft(point.x());
            break;
        case right:
            resultingRect.setRight(point.x());
            break;
        case top:
            resultingRect.setTop(point.y());
            break;
        case bottom:
            resultingRect.setBottom(point.y());
            break;
        case center:
            break;
        }
    } else {
        //Перемещаем инструмент
        switch (mActiveLugLocation) {
        case topLeft:
            resultingRect.moveTopLeft(point);
            break;
        case topRight:
            resultingRect.moveTopRight(point);
            break;
        case bottomLeft:
            resultingRect.moveBottomLeft(point);
            break;
        case bottomRight:
            resultingRect.moveBottomRight(point);
            break;
        case left:
            resultingRect.moveLeft(point.x());
            break;
        case right:
            resultingRect.moveRight(point.x());
            break;
        case top:
            resultingRect.moveTop(point.y());
            break;
        case bottom:
            resultingRect.moveBottom(point.y());
            break;
        case center:
            resultingRect.moveCenter(point);
            break;
        }
    }\

    if (mChangesType == resizing) {
        cutBySceneRect(resultingRect);
    } else {
        moveBySceneRect(resultingRect);
    }\

    updateGeometryAndLugPointers(resultingRect);
    updateActiveLugGeometry();
    onSizeChange();
}

void RectangularTool::getTopLeftPosFromSettings(bool dontNeedIt)
{
    if (dontNeedIt) {
        return;
    }
    Q_ASSERT(mChangesType == notChangig);
    //Q_ASSERT(QFrost::boundRectF.contains(mSettings->rect())); FIXME: QTBUG-18719
    setPos(QFrost::sceneUnits(mSettings->rect().topLeft()));
    updateLugsGeometry();
    onStopChange();
}

void RectangularTool::getSizeFromSettings(bool dontNeedIt)
{
    if (dontNeedIt) {
        return;
    }
    Q_ASSERT(mChangesType == notChangig);

    //Q_ASSERT(QFrost::boundRectF.contains(mSettings->rect())); FIXME: QTBUG-18719
    prepareGeometryChange();
    mSize = QFrost::sceneUnits(mSettings->size());
    updateLugsGeometry();
    onSizeChange();
    onStopChange();
}

void RectangularTool::cutBySceneRect(QRectF &rect)
{
    // QTBUG-30456
    //rect = rect.intersected(QFrost::boundRectF);
    if (rect.left() < QFrost::boundRectF.left()) {
        rect.setLeft(QFrost::boundRectF.left());
    }
    if (rect.right() < QFrost::boundRectF.left()) {
        rect.setRight(QFrost::boundRectF.left());
    }

    if (rect.right() > QFrost::boundRectF.right()) {
        rect.setRight(QFrost::boundRectF.right());
    }
    if (rect.left() > QFrost::boundRectF.right()) {
        rect.setLeft(QFrost::boundRectF.right());
    }

    if (rect.top() < QFrost::boundRectF.top()) {
        rect.setTop(QFrost::boundRectF.top());
    }
    if (rect.bottom() < QFrost::boundRectF.top()) {
        rect.setBottom(QFrost::boundRectF.top());
    }

    if (rect.bottom() > QFrost::boundRectF.bottom()) {
        rect.setBottom(QFrost::boundRectF.bottom());
    }
    if (rect.top() > QFrost::boundRectF.bottom()) {
        rect.setTop(QFrost::boundRectF.bottom());
    }
}

void RectangularTool::moveBySceneRect(QRectF &rect)
{
    rect.moveTopLeft(pointBoundedByBoundRect(rect.topLeft()));
    rect.moveBottomRight(pointBoundedByBoundRect(rect.bottomRight()));
}

QPointF RectangularTool::pointBoundedByBoundRect(const QPointF &point)
{
    int s = QFrost::sceneHalfSize;
    return QPointF(qBound<qreal>(-s, point.x(), s),
                   qBound<qreal>(-s, point.y(), s));
}
