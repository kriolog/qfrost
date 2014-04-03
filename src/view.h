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

#ifndef QFGUI_VIEW_H
#define QFGUI_VIEW_H

#include <QtWidgets/QGraphicsView>

#include <qfrost.h>

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(Block)
QT_FORWARD_DECLARE_CLASS(MainWindow)
QT_FORWARD_DECLARE_CLASS(Scene)

class View : public QGraphicsView
{
    Q_OBJECT
public:
    View(Scene *scene, MainWindow *parent = 0);

    /**
     * Указатель на сцену, приведённый к типу qfgui::Scene
     */
    Scene *qfScene() const;

    /**
     * Расстояние между видимыми узлами сетки.
     */
    int gridSpan() const {
        return mGridSpan;
    }

    /**
     * Перемещает курсор в данную точку сцены. Если она находится вне видимой
     * области, предварительно смещает обзор так, чтобы она туда перешла.
     * @param scenePoint точка сцены, куда требуется переместить курсор.
     */
    void setSceneCursorPos(const QPointF &scenePoint);

    /**
     * Сдвигает курсор по горизонтали в данный x сцены. Если x вылезает за
     * пределы видимой области, предварительно смещает обзор так, чтобы он влез.
     * @param x координата x сцены, куда требуется сдвинуть курсор.
     */
    void setSceneCursorX(qreal x);

    /**
     * Сдвигает курсор по вертикали в данный y сцены. Если y вылезает за
     * пределы видимой области, предварительно смещает обзор так, чтобы он влез.
     * @param x координата y сцены, куда требуется сдвинуть курсор.
     */
    void setSceneCursorY(qreal y);

    /**
     * Изменить масштаб в @p scale раз.
     * Аналогично вызову QGraphicsView::scale(s, s), но также испускает сигнал
     * и делает прочие необходимые вещи (например, обновляет шаг сетки).
     */
    void scale(qreal s);

    QPointF visibleTopLeft() const;

    bool isLight() const {
        return mIsLight;
    }

    void save(QDataStream &out);
    void load(QDataStream &in);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void leaveEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);

    /******************* Рисование различных вещей *******************/
    /**
     * Заполнение заднего фона (переопределено от QGraphicsView)
     */
    void drawBackground(QPainter *painter, const QRectF &rect);

    /**
     * Рисование сетки на заднем фоне
     */
    void drawGrid(QPainter *painter, const QRectF &rect);

    /**
     * Заполнение переднего фона (переопределено от QGraphicsView)
     */
    void drawForeground(QPainter *painter, const QRectF &rect);

    /**
     * Рисование осей
     */
    void drawAxes(QPainter *painter);

private:
    /// Флаг для включения/отключения ручной прокрутки.
    bool mIsHandScrolling;

    /// Начальные координаты курсора в глобальных координатах при руч. прокрутке.
    QPoint mHandScrollingPrevCurpos;

    ///Время, прошедшее от начала ручной прокрутки
    QTimer *mAutoScrollTimer;

    /**
     * Количество пикселов, на которые двигается видимая область
     * (задаёт скорость прокрутки).
     */
    int mAutoScrollCount;

    static const int mAutoScrollViewMargin = 4;

    /**
     * Промежуток сетки (расстояние между видимыми
     * точками сетки в текущем масштабе)
     */
    int mGridSpan;

    /// Включён ли светлый режим
    bool mIsLight;

    /// Перо для главной сетки
    QPen mMainGridPen;
    /// Перо для вспомогательной сетки
    QPen mAdditionalGridPen;

    /// Позиция курсора (в координатах сцены)
    QPointF mMousePos;

    /// Изменилась ли позиция курсора с предыдущего отправления сигнала
    bool mMousePosChanged;

    /// Точка, на которую нужно отцентроваться, когда мы покажемся
    QPointF mPointToCenterOn;

    /**
     * Изменение промежутка сетки
     */
    void updateGridSpan();

    /**
     * Принцип работы автоматической прокрутки:
     * При каждом изменении позиции курсора вызывается tryToStartAutoScroll.
     * Этот метод смотрит, как изменяется инструмент и проверяет, находится ли
     * курсор достаточно близко к соответствующим краям. Если находится, он
     * начинает прокрутку методом startAutoScroll. Последний метод запускает
     * таймер. Этот таймер  по таймауту вызывает метод doAutoScroll, который
     * собственно осуществляет прокрутку. Если по каким-то причинам очередной
     * вызов этого метода не прокрутил поле, то вызывается метод stopAutoScroll.
     */

    /**
     * Начинает, если необходимо, автоматическую прокрутку.
     */
    void tryToStartAutoScroll(const QPoint &pos);

    /**
     * Запускает таймер автоматической прокрутки.
     */
    void startAutoScroll();

    /**
     * Останавливает таймер автоматической прокрутки.
     */
    void stopAutoScroll();

    /**
     * Начинает ручную прокруту, изменяет форму курсора.
     * @param pos начальная позиция курсора в глобальных координатах.
     */
    void startHandScroll(const QPoint &pos);

    /**
     * Делает 1 шаг ручной прокрутки.
     * @param pos новая позиция курсора в глобальных координатах.
     */
    void doHandScroll(const QPoint &pos);

    /**
     * Останавливает ручную прокрутку, возвращает форму курсора к первоночальной
     */
    void stopHandScroll();

    /**
     * Устанавливает цвет фона и сетки исходя из mIsLight
     */
    void updateColorScheme();

public slots:
    void setLightColorScheme(bool b);

private slots:
    /**
     * Метод осуществляет один шаг автоматической прокрутки. Он проверяет,
     * в каких направлениях возможно изменение инструмента в сцене, затем
     * проверяет, находится ли курсор достаточно близко к краям поля и затем
     * осуществляет шаг прокрутки. Если шаг прокрутки не был осуществлён, он
     * вызывает метод stopHandScroll().
     */
    void doAutoScroll();

    /// Отправляет сигнал о позиции курсора
    void sendMousePos();

signals:
    /**
     * Сигнал о том, что позиция курсора изменилась.
     * @param newPosition новая позиция в координатах сцены. Если курсор вышел
     * за пределы, то QFrost::noPoint.
     */
    void mouseMoved(const QPointF &newPosition = QFrost::noPoint);

    /**
     * Сигнал о том, что масштаб изменилась.
     * @param newScale новый масштаб.
     */
    void scaleChanged(qreal newScale);

    /**
     * Сигнал о том, что изменилось расстояние между видимыми узлами сетки
     */
    void gridSpanChanged(int gridSpan);

    void colorSchemeChanged();
};

}

#endif // QFGUI_VIEW_H
