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

#ifndef QFGUI_RULER_H
#define QFGUI_RULER_H

#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QScrollBar>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(View)

class Ruler: public QWidget
{
    Q_OBJECT
public:
    Ruler(Qt::Orientation orientation, View *parent);

    QSize minimumSizeHint() const {
        return QSize(kBreadth, kBreadth);
    }

    qreal origin() const {
        return mOrigin;
    }

    qreal rulerUnit() const {
        return mUnit;
    }

    qreal rulerZoom() const {
        return mZoom;
    }

    /// Создаёт виджет из view с линейками сверху и слева и возвращает указатель
    /// на созданный виджет.
    static QWidget *createRulers(View *view);

    static void updateChildrenOffsets(QWidget *parent, bool hideTicks);

public slots:
    /// Устанавливает начало отсчёта равным origin (с учётом отступа), где
    /// origin - новое значение на соответствующем нам скроллбаре.
    void setOrigin(const int origin);

    void setRulerUnit(const int rulerUnitInt);

    void setRulerZoom(const qreal rulerZoom);

    void setCursorPos(const QPointF &cursorPos);
    void setAnchorPos(const QPointF &pos);

    void hideCursorTick();
    void hideAnchorTick();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    /// Устанавливает начало отсчёта исходя из расчётов по mapFromScene.
    /// Этот метод следует использовать, только если информация по скроллбарам
    /// нам ничего не даёт (см. mIsFullyZoomed).
    void setOrigin();

    void drawAScaleMeter(QPainter *painter, qreal scaleMeter,
                         qreal startPosition, bool drawText = false);

    void drawFromOriginTo(QPainter *painter, qreal startMark,
                          qreal endMark, int startTickNo, qreal step,
                          qreal startPosition, bool drawText);

    void drawMousePosTick(QPainter *painter);
    void drawAnchorPosTick(QPainter *painter);

    /// Получает mAnchorPos из mAnchorScenePos
    void updateAnchorPos();
    /// Получает mCursorPos из mCursorScenePos
    void updateCursorPos();
    /// Пересчитывает mRect для нового размера @p size
    void updateRect(const QSize &size);
    /// Пересчитывает отступы исходя из viewport
    void updateOffsets();

    QPoint mapFromScene(const QPointF &point);

    /// Скроллбар, влияющий на наш отображаемый диапазон
    QScrollBar *scrollbarForValue();
private:
    bool mIsHorizontal;
    qreal mOrigin;
    qreal mUnit;
    qreal mZoom;

    /// Размер линейки вширь (высота/ширина горизонтальной/вертикальной)
    static const int kBreadth;
    /// Высота отметки с позицией курсора. Вширь её размер вдвое больше.
    static const int kAnchorTickSize;

    /// Надо ли рисовать отметку с позицией курсора
    bool mShowCursorTick;
    /// Надо ли рисовать отметку с позицией привязки
    bool mShowAnchorTick;
    /// Координаты курсора в наших координатах
    QPoint mCursorPos;
    /// Координаты привязки в наших координатах
    QPoint mAnchorPos;
    /// Координаты курсора в координатах сцены
    QPointF mCursorScenePos;
    /// Координаты привязки в координатах сцены
    QPointF mAnchorScenePos;
    /// Прямоугольник, в котором помещается отметка с позицией курсора
    QRect mCursorRect;
    /// Прямоугольник, в котором помещается отметка с позицией привязки
    QRect mAnchorRect;

    /** Прямоугольник, для которого мы рисуем себя. Он уменьшен относительно
     * видимого прямоугольника на mOffset1 и mOffset2, чтобы ширина линейки
     * соответствовала видимой ширине в mView */
    QRectF mRect;

    View *mView;

    /// Отступ слева/сверху
    int mOffset1;
    /// Отступ справа/снизу
    int mOffset2;

    /// Достигнуто ли максимальное увеличение. Оно достигнуто, если у
    /// соответствующего нам скроллбара значение, минимум и максимум равны нулю.
    bool mIsFullyZoomed;
};

}

#endif // QFGUI_RULER_H
