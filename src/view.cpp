/*
 * Copyright (C) 2010-2013  Denis Pesotsky, Maxim Torgonsky
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

#include <view.h>

#include <cmath>

#include <QtCore/QTimer>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QScrollBar>
#include <QtGui/QLinearGradient>

#include <qfrost.h>
#include <scene.h>
#include <tools/anchor.h>
#include <mainwindow.h>

//#include <QtOpenGL/QGLWidget>

using namespace qfgui;

View::View(Scene *scene, MainWindow *parent): QGraphicsView(scene, parent),
    mIsHandScrolling(false),
    mHandScrollingPrevCurpos(),
    mAutoScrollTimer(),
    mAutoScrollCount(),
    mGridSpan(QFrost::unitsInGridStep),
    mIsLight(false),
    mMainGridPen(),
    mAdditionalGridPen(),
    mMousePos(QFrost::noPoint),
    mMousePosChanged(false),
    mPointToCenterOn(QFrost::noPoint)
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    /* это делается в setTransformationAnchor(QGraphicsView::AnchorUnderMouse),
     * но мало ли что там поменяют разработчики Qt. */
    setMouseTracking(true);

    setDragMode(QGraphicsView::NoDrag);

    // у нас обновления, как правило, происходят прямоугольниками
    setViewportUpdateMode(BoundingRectViewportUpdate);

    mAutoScrollTimer = new QTimer(this);
    connect(mAutoScrollTimer, SIGNAL(timeout()), SLOT(doAutoScroll()));

    updateColorScheme();

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
}

void View::sendMousePos()
{
    if (mMousePosChanged) {
        mMousePosChanged = false;
        emit mouseMoved(mMousePos);
    }
}

void View::setLightColorScheme(bool b)
{
    mIsLight = b;
    updateColorScheme();
    emit colorSchemeChanged();
}

void View::updateColorScheme()
{
    mMainGridPen = QPen(mIsLight ? Qt::darkGray : Qt::white, 0);
    mAdditionalGridPen = QPen(mIsLight ? Qt::lightGray : Qt::darkGray, 0);
    if (mIsLight) {
        mMainGridPen.setStyle(Qt::DashLine);
        mAdditionalGridPen.setStyle(Qt::DotLine);
    }
    setBackgroundBrush(mIsLight ? QColor(240, 230, 250) : Qt::black);
}

Scene *View::qfScene() const
{
    Scene *result = qobject_cast<Scene *>(scene());
    Q_ASSERT(result != NULL);
    return result;
}

void View::showEvent(QShowEvent *event)
{
    QGraphicsView::showEvent(event);

    if (mPointToCenterOn == QFrost::noPoint) {
        // при первом появлении чуть прокручиваем всё, чтобы частично влезали оси
        horizontalScrollBar()->setValue(-20);
        verticalScrollBar()->setValue(-20);
    } else {
        // FIXME: если открывается на полный экран, то сначала оно показывается,
        //        а потом уже изменяется размер => неправильно центруется
        centerOn(mPointToCenterOn);
        mPointToCenterOn = QFrost::noPoint;
    }
}

void View::mousePressEvent(QMouseEvent *event)
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

void View::startHandScroll(const QPoint &pos)
{
    qfScene()->anchor()->freeze();
    mIsHandScrolling = true;
    mHandScrollingPrevCurpos = pos;
    QApplication::setOverrideCursor(Qt::ClosedHandCursor);
}

void View::mouseMoveEvent(QMouseEvent *event)
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

void View::doHandScroll(const QPoint &pos)
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
        qfScene()->anchor()->findAnchorFreezed(mapToScene(viewport()->mapFromGlobal(mousePos)));
    }
    //запоминаем новую точку отсчёта
    mHandScrollingPrevCurpos = mousePos;
}

void View::leaveEvent(QEvent *event)
{
    sendMousePos();
    emit mouseMoved();
    QGraphicsView::leaveEvent(event);
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    if (mIsHandScrolling) {
        stopHandScroll();
        mMousePosChanged = true;
        mMousePos = mapToScene(event->pos());
    }
    QGraphicsView::mouseReleaseEvent(event);
}

void View::stopHandScroll()
{
    qfScene()->anchor()->unfreeze();
    mIsHandScrolling = false;
    QApplication::restoreOverrideCursor();
}

QPointF View::visibleTopLeft() const
{
    return mapToScene(QPoint(0, 0));
}

void View::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        centerOn(qfScene()->blocksBoundingRect().center());
        event->accept();
        return;
    }
}

void View::wheelEvent(QWheelEvent *event)
{
    static const double scaleStep = 1.25;
    //static const double minScale = 0.0009;
    //static const double maxScale = 5;
    static const double minScale = 0.075 * QFrost::metersInUnit;
    static const double maxScale = 800.0 * QFrost::metersInUnit;
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
    if ((!signumOfDelta && (transform().m11() <= maxScale)) ||
            (signumOfDelta && (transform().m11() >= minScale))) {
        scale(tempScaleStep);
        mMousePosChanged = true;
        mMousePos = mapToScene(event->pos());
    }
    // QGraphicsView::wheelEvent(event);
    // -- не должен включаться стандартный скролл, нужен только зум
}

void View::scale(qreal s)
{
    if (s != 1.0) {
        QGraphicsView::scale(s, s);
        updateGridSpan();
        emit scaleChanged(transform().m11());
    }
}

void View::keyPressEvent(QKeyEvent *event)
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

void View::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !(event->isAutoRepeat())) {
        viewport()->releaseMouse();
        stopHandScroll();
        return;
    }
    QGraphicsView::keyReleaseEvent(event);
}

void View::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Расскрашиваем фон backgroundBrush
    QGraphicsView::drawBackground(painter, rect);
    // Рисуем сетку
    drawGrid(painter, rect);
}

void View::drawGrid(QPainter *painter, const QRectF &rect)
{
    // находим границы для рисования (через rect с запасом)
    int x1 = mGridSpan * int(qFloor(rect.left()  / mGridSpan));
    int y1 = mGridSpan * int(qFloor(rect.top()   / mGridSpan));
    int x2 = mGridSpan * int(qCeil(rect.right()  / mGridSpan));
    int y2 = mGridSpan * int(qCeil(rect.bottom() / mGridSpan));

    if (!mIsLight) {
        QVector<QPoint> tenthPoints;
        QVector<QPoint> otherPoints;
        for (int x = x1; x < x2; x += mGridSpan) {
            for (int y = y1; y < y2; y += mGridSpan) {
                QPoint point(x, y);
                if (x % (10 * mGridSpan) == 0 || y % (10 * mGridSpan) == 0) {
                    tenthPoints << point;
                } else {
                    otherPoints << point;
                }
            }
        }
        painter->setPen(mMainGridPen);
        painter->drawPoints(tenthPoints);
        painter->setPen(mAdditionalGridPen);
        painter->drawPoints(otherPoints);
    } else {
        // Горизонтальные линии
        QVector<QLine> xTenthLines;
        QVector<QLine> xOtherLines;
        // Вертикальные линии
        QVector<QLine> yTenthLines;
        QVector<QLine> yOtherLines;

        for (int x = x1; x < x2; x += mGridSpan) {
            QLine line(x, y1, x, y2);
            if (x % (10 * mGridSpan) == 0) {
                yTenthLines << line;
            } else {
                yOtherLines << line;
            }
        }
        for (int y = y1; y < y2; y += mGridSpan) {
            QLine line(x1, y, x2, y);
            if (y % (10 * mGridSpan) == 0) {
                xTenthLines << line;
            } else {
                xOtherLines << line;
            }
        }

        double zoom = transform().m11();
        double xOffset = rect.left() * zoom;
        double yOffset = rect.top() * zoom;

        mMainGridPen.setDashOffset(xOffset);
        mAdditionalGridPen.setDashOffset(xOffset);
        painter->setPen(mMainGridPen);
        painter->drawLines(xTenthLines);
        painter->setPen(mAdditionalGridPen);
        painter->drawLines(xOtherLines);

        mMainGridPen.setDashOffset(yOffset);
        mAdditionalGridPen.setDashOffset(yOffset);
        painter->setPen(mMainGridPen);
        painter->drawLines(yTenthLines);
        painter->setPen(mAdditionalGridPen);
        painter->drawLines(yOtherLines);
    }
}

void View::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect)
    drawAxes(painter);
}

void View::drawAxes(QPainter *painter)
{
    /// (полу)длина осей
    static const int axesLength = 60;

    /// длина "указателей" на стрелках
    static const int headsLength = 12;
    /// на сколько отходят указатели на стрелках от её тела (= половина ширины)
    static const int headsPadding = 4;

    /// отступ букв от концов осей
    static const int lmargin = 20;
    /// полувысота букв
    static const int lettersHheight = 8;
    /// полуширина букв
    static const int lettersHwidth = 5;
    /// толщина внутренних линий
    static const int penWidth = 2; // FIXME если нечёт, в windows глючит обводка
    /// корректировка для стрелок
    static const int headCorrection = (penWidth == 1) ? 1 : 0;

    painter->save();

    // Ось координат не машстабируется.
    {
        QTransform t = painter->transform();
        painter->scale(1.0 / t.m11(), 1.0 / t.m22());
    }

    QVector<QLine> axeLines;
    axeLines
            << QLine(-axesLength, 0, axesLength - 2 * penWidth, 0)
            << QLine(0, -axesLength, 0, axesLength - 2 * penWidth);
    QVector<QLine> cosmeticLines;

    cosmeticLines
    /*************************** стрелка X ********************************/
            << QLine(axesLength - headsLength, -headsPadding, axesLength, 0)
            << QLine(axesLength - headsLength, headsPadding + headCorrection,
                     axesLength, headCorrection)
            /**************************** буква X *********************************/
            << QLine(axesLength + lmargin - lettersHwidth, -lettersHheight,
                     axesLength + lmargin + lettersHwidth, lettersHheight)
            << QLine(axesLength + lmargin + lettersHwidth, -lettersHheight,
                     axesLength + lmargin - lettersHwidth, lettersHheight)
            /*************************** стрелка Y ********************************/
            << QLine(-headsPadding, axesLength - headsLength, 0, axesLength)
            << QLine(headsPadding + headCorrection, axesLength - headsLength,
                     headCorrection, axesLength)
            /**************************** буква Y *********************************/
            << QLine(0, axesLength + lmargin, -lettersHwidth,
                     axesLength + lmargin - lettersHheight)
            << QLine(0, axesLength + lmargin, lettersHwidth,
                     axesLength + lmargin - lettersHheight)
            << QLine(0, axesLength + lmargin, 0,
                     axesLength + lmargin + lettersHheight);

    QPen pen;
    pen.setCosmetic(true);
    // рисуем обводку
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(mIsLight ? QColor(0, 0, 0, 100) : QColor(255, 255, 255, 100));
    pen.setWidth(penWidth + 2);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawLines(cosmeticLines);
    painter->setRenderHint(QPainter::Antialiasing, false);
    pen.setCapStyle(Qt::SquareCap);
    painter->setPen(pen);
    painter->drawLines(axeLines);

    // и заполнение
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(mIsLight ? Qt::white : Qt::black);
    pen.setWidth(penWidth);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->drawLines(cosmeticLines);
    pen.setCapStyle(Qt::SquareCap);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->drawLines(axeLines);
    painter->restore();
}

void View::updateGridSpan()
{
    /**
     * Коэффициент, влияющий на максимальную "скученность" (количество на
     * единицу длины вида) точек сетки.
     * Чем он меньше, тем они могут быть скученней.
     */
    static const double maxGridDensity = 6.5;
    mGridSpan = QFrost::unitsInGridStep;
    int i = 1;
    qreal scaleFactor = transform().m11();
    while (scaleFactor * QFrost::unitsInGridStep * i < maxGridDensity) {
        mGridSpan = mGridSpan * QFrost::unitsInGridStep;
        i *= QFrost::unitsInGridStep;
    }
    emit gridSpanChanged(mGridSpan);
}

void View::tryToStartAutoScroll(const QPoint &pos)
{
    QRect area = viewport()->rect();
    Qt::Orientations sceneChangesOrientations;
    sceneChangesOrientations = qfScene()->toolChangesOrientations();

    bool go;
    // нужен скролл по горизонтали
    go = (sceneChangesOrientations & Qt::Horizontal &&
          (pos.x() - area.left() < mAutoScrollViewMargin ||
           area.right() - pos.x() < mAutoScrollViewMargin));

    // нужен скролл по вертикали
    go |= (sceneChangesOrientations & Qt::Vertical &&
           (pos.y() - area.top() < mAutoScrollViewMargin ||
            area.bottom() - pos.y() < mAutoScrollViewMargin));

    if (go) {
        startAutoScroll();
    }
}

void View::startAutoScroll()
{
    mAutoScrollTimer->start(50);
    mAutoScrollCount = 0;
}

void View::stopAutoScroll()
{
    mAutoScrollTimer->stop();
    mAutoScrollCount = 0;
}

void View::doAutoScroll()
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

    Qt::Orientations sceneChangesOrientations;
    sceneChangesOrientations = qfScene()->toolChangesOrientations();

    if (sceneChangesOrientations & Qt::Horizontal) {
        if (pos.x() - area.left() < mAutoScrollViewMargin) {
            horizontalScrollBar()->setValue(horizontalValue - mAutoScrollCount);
            imaginaryPos.setX(area.left());
        } else if (area.right() - pos.x() < mAutoScrollViewMargin) {
            horizontalScrollBar()->setValue(horizontalValue + mAutoScrollCount);
            imaginaryPos.setX(area.right());
        }
    }

    if (sceneChangesOrientations & Qt::Vertical) {
        if (pos.y() - area.top() < mAutoScrollViewMargin) {
            verticalScrollBar()->setValue(verticalValue - mAutoScrollCount);
            imaginaryPos.setY(area.top());
        } else if (area.bottom() - pos.y() < mAutoScrollViewMargin) {
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

void View::setSceneCursorPos(const QPointF &scenePoint)
{
    ensureVisible(scenePoint.x(), scenePoint.y(), 0, 0,
                  2 * mAutoScrollViewMargin, 2 * mAutoScrollViewMargin);
    QCursor::setPos(mapToGlobal(mapFromScene(scenePoint)));
}

void View::setSceneCursorX(qreal x)
{
    QPointF scenePoint;
    scenePoint.setX(x);
    scenePoint.setY(mapToScene(mapFromGlobal(QPoint(0,
                               QCursor::pos().y()))).y());
    setSceneCursorPos(scenePoint);
}

void View::setSceneCursorY(qreal y)
{
    QPointF scenePoint;
    scenePoint.setX(mapToScene(mapFromGlobal(QPoint(QCursor::pos().x(), 0))).x());
    scenePoint.setY(y);
    setSceneCursorPos(scenePoint);
}

void View::save(QDataStream &out)
{
    out << transform().m11();
    out << mapToScene(viewport()->rect().center());
}

void View::load(QDataStream &in)
{
    Q_ASSERT(in.status() == QDataStream::Ok);
    double s;
    in >> s;
    in >> mPointToCenterOn;
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    scale(s / transform().m11());
    if (isVisible()) {
        centerOn(mPointToCenterOn);
        mPointToCenterOn = QFrost::noPoint;
    }
}
