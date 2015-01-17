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

#include <tools/blockcreator.h>

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QMessageBox>
#include <QtGui/QPainter>

#include <graphicsviews/block.h>
#include <graphicsviews/scene.h>
#include <tools_panel/blockcreatorsettings.h>

using namespace qfgui;

BlockCreator::BlockCreator(ToolSettings *settings): RectangularTool(settings, false),
    mColumns(),
    mRows(),
    mInitialPoint(),
    mLastPoint(),
    mPrimaryLastPoint(),
    mTetrisInPrimary(),
    mTetrisInComplementary()
{
    BlockCreatorSettings *s = qobject_cast<BlockCreatorSettings *>(settings);
    Q_ASSERT(s != NULL);
    connect(s, SIGNAL(blocksChanged()), this, SLOT(recalculate()));
    connect(s, SIGNAL(basepointChanged()), this, SLOT(recalculate()));
}

void BlockCreator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    if (isChanging()) {
        // коричневый -- "среднее" между зелёным и красным
        painter->setBrush(QBrush(QColor(150, 110, 65, 100), Qt::SolidPattern));
        painter->setPen(QPen(QColor(150, 110, 65), 0));
        painter->drawRect(boundingRect());
    } else {
        drawBlocks(painter);
    }
}

void BlockCreator::drawBlocks(QPainter *painter)
{
    /* Основной (primary) прямоугольник */
    painter->setPen(QPen(Qt::magenta, 0));
    if (mTetrisInPrimary) {
        painter->setBrush(QBrush(QColor(255, 20, 80, 80) , Qt::SolidPattern));
        painter->drawRect(primaryRect());
        if (!mTetrisInComplementary) {
            QBrush brush;
            brush.setColor(Qt::darkGreen);
            brush.setStyle(Qt::FDiagPattern);
            brush.setMatrix(painter->matrix().inverted());
            painter->setBrush(brush);
            painter->drawRect(primaryRect());
        }
    } else {
        painter->setBrush(QBrush(QColor(50, 200, 50, 80) , Qt::SolidPattern));
        painter->drawRect(primaryRect());
        if (mTetrisInComplementary) {
            QBrush brush;
            brush.setColor(Qt::darkRed);
            brush.setStyle(Qt::FDiagPattern);
            brush.setMatrix(painter->matrix().inverted());
            painter->setBrush(brush);
            painter->drawRect(primaryRect());
        }
    }

    /* Внешняя область */
    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(mTetrisInComplementary
                             ? QColor(255, 20, 80, 80)
                             : QColor(50, 200, 50, 80),
                             Qt::SolidPattern));

    // Вычитаем из всего прямоугольника уже нарисованный и рисуем результат
    QPainterPath sidePath;
    sidePath.addRect(boundingRect());
    sidePath.addRect(primaryRect());
    painter->drawPath(sidePath);

    /* Блоки основного (primary) прямоугольника */
    QVector<QLineF> primaryLinesInScene;
    QPointF point1;
    QPointF point2;
    /* Горизонтальные линии */
    for (int j = 0; j < mRows.size(); ++j) {
        point1 = QPointF(mInitialPoint.x(), mInitialPoint.y() + mRows.at(j));
        point2 = QPointF(mPrimaryLastPoint.x(), mInitialPoint.y() + mRows.at(j));
        primaryLinesInScene.append(QLineF(point1, point2));
    }
    /* Вертикальные линии */
    for (int i = 0; i < mColumns.size(); ++i) {
        point1 = QPointF(mInitialPoint.x() + mColumns.at(i) , mInitialPoint.y());
        point2 = QPointF(mInitialPoint.x() + mColumns.at(i) , mPrimaryLastPoint.y());
        primaryLinesInScene.append(QLineF(point1, point2));
    }

    painter->setPen(QPen(Qt::darkGreen, 0, Qt::DotLine));
    painter->drawLines(primaryLinesInScene);

    /* Дополняющие блоки */
    QVector<QLineF> complementaryLinesInScene;
    /* Горизонтальные линии */
    for (int j = 0; j < mRows.size(); ++j) {
        point1 = QPointF(mLastPoint.x(), mInitialPoint.y() + mRows.at(j));
        point2 = QPointF(mPrimaryLastPoint.x(), mInitialPoint.y() + mRows.at(j));
        complementaryLinesInScene.append(QLineF(point1, point2));
    }
    /* Вертикальные линии */
    for (int i = 0; i < mColumns.size(); ++i) {
        point1 = QPointF(mInitialPoint.x() + mColumns.at(i) , mLastPoint.y());
        point2 = QPointF(mInitialPoint.x() + mColumns.at(i) , mPrimaryLastPoint.y());
        complementaryLinesInScene.append(QLineF(point1, point2));
    }

    painter->setPen(QPen(Qt::darkGreen, 0, Qt::DotLine));
    painter->drawLines(complementaryLinesInScene);

    /* Общая рамка */
    painter->setPen(QPen(mTetrisInPrimary ? Qt::red : Qt::darkGreen, 0));
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());
}

void BlockCreator::onStopChange()
{
    recalculate();
}

void BlockCreator::recalculate()
{
    switch (settings()->basePoint()) {
    case Qt::TopLeftCorner:
        mInitialPoint = boundingRect().topLeft();
        mLastPoint = boundingRect().bottomRight();
        break;
    case Qt::TopRightCorner:
        mInitialPoint = boundingRect().topRight();
        mLastPoint = boundingRect().bottomLeft();
        break;
    case Qt::BottomRightCorner:
        mInitialPoint = boundingRect().bottomRight();
        mLastPoint = boundingRect().topLeft();
        break;
    case Qt::BottomLeftCorner:
        mInitialPoint = boundingRect().bottomLeft();
        mLastPoint = boundingRect().topRight();
        break;
    }

    /* если прямоугольник, задающий блоки, инвертирован
     * по соответствующей координате, первый член отрицателен */
    mColumns = roundedGeometricSequence(
                   (mInitialPoint.x() <= mLastPoint.x()) ?
                   blocksSize().width() :
                   -blocksSize().width(),
                   blocksQ().width(),
                   boundingRect().width());


    /* если прямоугольник, задающий блоки, инвертирован
     * по соответствующей координате, первый член отрицателен */
    mRows = roundedGeometricSequence(
                (mInitialPoint.y() <= mLastPoint.y())
                ? blocksSize().height() : -blocksSize().height(),
                blocksQ().height(), boundingRect().height());

    mPrimaryLastPoint = QPointF(mInitialPoint.x() + (mColumns.isEmpty()
                                ? 0.0 : mColumns.last()),
                                mInitialPoint.y() + (mRows.isEmpty()
                                        ? 0.0 : mRows.last()));

    QRectF r = rectInScene();
    if (r.width() < QFrost::minBlockSizeScene || r.height() < QFrost::minBlockSizeScene) {
        // Если один из размеров инструмента меньше минимального размера
        // блока, то мы ничего не собираемся создавать
        mTetrisInComplementary = true;
        mTetrisInPrimary = true;
    } else {
        mTetrisInPrimary = thereIsTetris(primaryRectInScene());
        mTetrisInComplementary = thereIsTetris(r);
    }

    update();
}

QSizeF BlockCreator::blocksQ() const
{
    const BlockCreatorSettings *s = qobject_cast<const BlockCreatorSettings *>(settings());
    Q_ASSERT(s != NULL);
    return s->blocksQ();
}

QSize BlockCreator::blocksSize() const
{
    const BlockCreatorSettings *s = qobject_cast<const BlockCreatorSettings *>(settings());
    Q_ASSERT(s != NULL);

    // берём размер из настроек инструмента
    QSize result = QFrost::sceneUnits(s->blocksSize());

    Q_ASSERT(result.isValid());

    /// Размер boundingRect() с отброшенной дробной частью
    QSize boundingSize = QSize(boundingRect().width(), boundingRect().height());

    // обрезаем его по нашему размеру
    result = result.boundedTo(boundingSize);

    // нулевой размер блока означает бесконечность (одномерную задачу)
    if (result.width() == 0) {
        result.setWidth(boundingSize.width());
    }
    if (result.height() == 0) {
        result.setHeight(boundingSize.height());
    }

    return result;
}

QList<qreal> BlockCreator::roundedGeometricSequence(const qreal first,
        const qreal q,
        const qreal max)
{
    Q_ASSERT(q >= 1);
    if (first == 0.0) {
        return QList<qreal>();
    }

    QList<qreal> result;
    qreal currentMember = first;
    qreal sum = currentMember;

    /// следующий округлённый элемент
    qreal roundSum = qreal(QFrost::unitsInGridStep * qRound(sum / QFrost::unitsInGridStep));
    while (qAbs(roundSum) <= max) {
        result.append(roundSum);
        currentMember *= q;
        sum += currentMember;
        roundSum = qreal(QFrost::unitsInGridStep * qRound(sum / QFrost::unitsInGridStep));
    }
    return result;
}

bool BlockCreator::thereIsTetris(const QRectF &rect) const
{
    if (rect.isEmpty()) {
        return false;
    }

    Q_ASSERT(scene() != NULL);
    foreach(Block * block, static_cast<Scene *>(scene())->blocks(rect)) {
        QRectF blockRect = block->rect();
        if ((blockRect.left() < rect.left() && blockRect.right() > rect.left())
                || (blockRect.right() > rect.right() && blockRect.left() < rect.right())
                || (blockRect.top() < rect.top() && blockRect.bottom() > rect.top())
                || (blockRect.bottom() > rect.bottom() && blockRect.top() < rect.bottom())) {
            return true;
        }
    }
    return false;
}

QList<QList<QRectF> > BlockCreator::blockRects(bool onlyPrimary)
{
    QList<QList<QRectF> > result;

    /// порядок столбцов прямой
    bool isStraightByX;
    isStraightByX = (settings()->basePoint() == Qt::TopLeftCorner);
    isStraightByX |= (settings()->basePoint() == Qt::BottomLeftCorner);
    /// порядок строк прямой
    bool isStraightByY;
    isStraightByY = (settings()->basePoint() == Qt::TopLeftCorner);
    isStraightByY |= (settings()->basePoint() == Qt::TopRightCorner);

    /// нужен ли дополнительный столбец прямоугольников
    bool needComplementaryColumn = !onlyPrimary
                                   && mPrimaryLastPoint.x() != mLastPoint.x();
    /// нужна ли дополнительная строка прямоугольников
    bool needComplementaryRow = !onlyPrimary
                                && mPrimaryLastPoint.y() != mLastPoint.y();

    for (QList<qreal>::ConstIterator i = mRows.constBegin(); i != mRows.constEnd(); ++i) {
        QRectF currentRect;
        qreal oldHorizontalSide = (i == mRows.constBegin()) ? 0.0 : *(i - 1);
        if (isStraightByY) {
            currentRect.setTop(oldHorizontalSide);
            currentRect.setBottom(*i);
        } else {
            currentRect.setTop(*i);
            currentRect.setBottom(oldHorizontalSide);
        }
        QList<QRectF> rectsInCurrentRow;
        for (QList<qreal>::ConstIterator j = mColumns.constBegin(); j != mColumns.constEnd(); ++j) {
            qreal oldVerticalSide = (j == mColumns.constBegin()) ? 0.0 : *(j - 1);
            if (isStraightByX) {
                currentRect.setLeft(oldVerticalSide);
                currentRect.setRight(*j);
                Q_ASSERT(currentRect.isValid());
                rectsInCurrentRow.append(currentRect);
            } else {
                currentRect.setLeft(*j);
                currentRect.setRight(oldVerticalSide);
                Q_ASSERT(currentRect.isValid());
                rectsInCurrentRow.prepend(currentRect);
            }
        }
        // в конец строки (если надо) добавляем дополнительный прямоугольник
        if (needComplementaryColumn) {
            if (isStraightByX) {
                currentRect.setLeft(mPrimaryLastPoint.x() - mInitialPoint.x());
                currentRect.setRight(mLastPoint.x() - mInitialPoint.x());
                Q_ASSERT(currentRect.isValid());
                rectsInCurrentRow.append(currentRect);
            } else {
                currentRect.setLeft(mLastPoint.x() - mInitialPoint.x());
                currentRect.setRight(mPrimaryLastPoint.x() - mInitialPoint.x());
                Q_ASSERT(currentRect.isValid());
                rectsInCurrentRow.prepend(currentRect);
            }
        }
        if (isStraightByY) {
            result.append(rectsInCurrentRow);
        } else {
            result.prepend(rectsInCurrentRow);
        }
    }

    // Добавляем строку с дополнительными прямоугольниками, если надо
    if (needComplementaryRow) {
        QList<QRectF> lastRow;
        QRectF currentRect;
        if (isStraightByY) {
            currentRect.setTop(mPrimaryLastPoint.y() - mInitialPoint.y());
            currentRect.setBottom(mLastPoint.y() - mInitialPoint.y());
        } else {
            currentRect.setTop(mLastPoint.y() - mInitialPoint.y());
            currentRect.setBottom(mPrimaryLastPoint.y() - mInitialPoint.y());
        }
        for (QList<qreal>::ConstIterator i = mColumns.constBegin(); i != mColumns.constEnd(); ++i) {
            qreal lastHorizonalSide = (i == mColumns.constBegin()) ? 0.0 : *(i - 1);
            if (isStraightByX) {
                currentRect.setLeft(lastHorizonalSide);
                currentRect.setRight(*i);
                Q_ASSERT(currentRect.isValid());
                lastRow.append(currentRect);
            } else {
                currentRect.setLeft(*i);
                currentRect.setRight(lastHorizonalSide);
                Q_ASSERT(currentRect.isValid());
                lastRow.prepend(currentRect);
            }
        }
        // при необходимости добавляем угловой прямоугольник
        if (needComplementaryColumn) {
            if (isStraightByX) {
                currentRect.setLeft(mPrimaryLastPoint.x() - mInitialPoint.x());
                currentRect.setRight(mLastPoint.x() - mInitialPoint.x());
                Q_ASSERT(currentRect.isValid());
                lastRow.append(currentRect);
            } else {
                currentRect.setLeft(mLastPoint.x() - mInitialPoint.x());
                currentRect.setRight(mPrimaryLastPoint.x() - mInitialPoint.x());
                Q_ASSERT(currentRect.isValid());
                lastRow.prepend(currentRect);
            }
        }
        if (isStraightByY) {
            result.append(lastRow);
        } else {
            result.prepend(lastRow);
        }
    }

    for (QList<QList<QRectF> >::Iterator i = result.begin(); i != result.end(); ++i) {
        for (QList<QRectF>::Iterator j = (*i).begin(); j != (*i).end(); ++j) {
            j->translate(pos() + mInitialPoint);
        }
    }

#ifndef QT_NO_DEBUG
    if (!result.isEmpty()) {
        // Размеры всех столбцов должны быть равны
        for (QList<QList<QRectF> >::ConstIterator i = result.begin() + 1; i != result.end(); ++i) {
            Q_ASSERT(i->size() == (i - 1)->size());
        }
        if (!result.first().isEmpty()) {
            for (QList<QList<QRectF> >::ConstIterator i = result.begin() + 1; i != result.end(); ++i) {
                // Прямоугольники одного столбца должны стоять стобиком
                Q_ASSERT(i->first().top() > (i - 1)->first().top());
                Q_ASSERT(i->first().bottom() > (i - 1)->first().bottom());
                // И они должны идти по возрастанию координат.
                Q_ASSERT(i->first().left() == (i - 1)->first().left());
                Q_ASSERT(i->first().right() == (i - 1)->first().right());
                for (QList<QRectF>::ConstIterator j = (*i).begin() + 1; j != (*i).end(); ++j) {
                    // Аналогично для прямоугольников одной строки
                    Q_ASSERT(j->top() == (j - 1)->top());
                    Q_ASSERT(j->bottom() == (j - 1)->bottom());
                    Q_ASSERT(j->left() > (j - 1)->left());
                    Q_ASSERT(j->right() > (j - 1)->right());
                    // И они должны быть валидны
                    Q_ASSERT(j->isValid());
                }
            }
        }
    }
#endif

    return result;
}

void BlockCreator::apply(bool alt)
{
    if ((alt && mTetrisInComplementary) || (!alt && mTetrisInPrimary)) {
        return;
    }

    static const int kCriticalSize = 20000;
    if (mColumns.size() * mRows.size() > kCriticalSize) {
        if (QMessageBox::question(scene()->views().first(), tr("Too much blocks"),
                                  tr("You are trying to add more than %n block(s), are you sure?"
                                     , "", kCriticalSize),
                                  QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
            return;
        }
    }

    QList<QList<QRectF> > blocksRects = blockRects(!alt);
    if (blocksRects.isEmpty() || blocksRects.first().isEmpty()) {
        return;
    }

    Q_ASSERT(scene() != NULL);
    Scene *s = qobject_cast<Scene *>(scene());
    Q_ASSERT(s != NULL);

    s->addBlocks(blocksRects,
                 qobject_cast<BlockCreatorSettings *>(settings())->mustChangePolygons());
    s->setTool(QFrost::blockCreator);
    // пересчитываемся, только если сцена нас не удалила из себя
    if (scene() != NULL) {
        recalculate();
    }
}
