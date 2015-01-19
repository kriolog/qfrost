/*
 * Copyright (C) 2010-2015  Denis Pesotsky, Maxim Torgonsky
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

#include "view.h"

#include <cmath>

#include <QMouseEvent>
#include <QScrollBar>

#include <qfrost.h>
#include <graphicsviews/scene.h>
#include <tools/tool.h>
#include <tools/anchor.h>
#include <mainwindow.h>

//#include <QtOpenGL/QGLWidget>

using namespace qfgui;

View::View(Scene *scene, MainWindow *parent): ViewBase(scene,
                                                       parent,
                                                       0.075 * QFrost::metersInUnit,
                                                       800.0 * QFrost::metersInUnit),
    mGridSpan(QFrost::unitsInGridStep),
    mIsLight(false),
    mMainGridPen(),
    mAdditionalGridPen(),
    mPointToCenterOn(QFrost::noPoint)
{
    updateColorScheme();

    connect(this, SIGNAL(scaleChanged(qreal)), SLOT(updateGridSpan()));

    connect(this, SIGNAL(startedHandScroll()),
            qfScene()->anchor(), SLOT(freeze()));

    connect(this, SIGNAL(stoppedHandScroll()),
            qfScene()->anchor(), SLOT(unfreeze()));

    connect(this, SIGNAL(mouseJumped(QPointF)),
            qfScene()->anchor(), SLOT(findAnchorFreezed(QPointF)));
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

QPointF View::visibleTopLeft() const
{
    return mapToScene(QPoint(0, 0));
}

void View::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        event->accept();
        Tool *tool = qfScene()->activeTool();
        if (tool) {
            const QPointF toolVisualCenter = tool->visualCenter();
            if (toolVisualCenter != QFrost::noPoint) {
                centerOn(toolVisualCenter);
                return;
            }
        }
        centerOn(qfScene()->blocksBoundingRect().center());
    }
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

void View::setSceneCursorPos(const QPointF &scenePoint)
{
    ensureVisible(scenePoint.x(), scenePoint.y(), 0, 0,
                  2 * kAutoScrollViewMargin, 2 * kAutoScrollViewMargin);
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
    out << transform().m11()
        << mapToScene(viewport()->rect().center());
}

void View::load(QDataStream &in)
{
    Q_ASSERT(in.status() == QDataStream::Ok);
    double scaleFactor;
    in >> scaleFactor >> mPointToCenterOn;
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    setScale(scaleFactor);
    if (isVisible()) {
        centerOn(mPointToCenterOn);
        mPointToCenterOn = QFrost::noPoint;
    }
}

Qt::Orientations View::sceneChangesOrientations() const
{
    return qfScene()->toolChangesOrientations();
}

