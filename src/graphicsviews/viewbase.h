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

#ifndef QFGUI_VIEWBASE_H
#define QFGUI_VIEWBASE_H

#include <QtWidgets/QGraphicsView>

namespace qfgui {

QT_FORWARD_DECLARE_CLASS(ZoomSlider)

class ViewBase : public QGraphicsView
{
    Q_OBJECT
public:
    ViewBase(QGraphicsScene *scene, QWidget* parent = NULL,
             double minScale = 0.1, double maxScale = 10.0);

    ZoomSlider *createZoomSlider(QWidget * parent = NULL);

signals:
    /**
     * Сигнал о том, что позиция курсора изменилась.
     * @param newPosition новая позиция в координатах сцены. Если курсор вышел
     * за пределы, то QFrost::noPoint.
     */
    void mouseMoved(const QPointF &newPosition);

    /**
     * Сигнал о том, что курсора резко перепрыгнул (зашёл за край экрана).
     * @param newPosition новая позиция в координатах сцены. Если курсор вышел
     * за пределы, то QFrost::noPoint.
     */
    void mouseJumped(const QPointF &newPosition);

    /**
     * Сигнал о том, что масштаб изменился.
     * @param newScale новый масштаб.
     */
    void scaleChanged(qreal newScale);

    void slidersValuesChanged(int value);

    void startedHandScroll();
    void stoppedHandScroll();

protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    
    virtual Qt::Orientations sceneChangesOrientations() const {
        return 0;
    }
    
    static const int kAutoScrollViewMargin = 4;
    
    /// Устанавливает масштаб примерно равным @p factor (в соответствие с шагом
    /// масштабного слайдера).
    void setScale(double factor);

private slots:
    /**
    * Метод осуществляет один шаг автоматической прокрутки. Он проверяет,
    * в каких направлениях возможно изменение инструмента в сцене, затем
    * проверяет, находится ли курсор достаточно близко к краям поля и затем
    * осуществляет шаг прокрутки. Если шаг прокрутки не был осуществлён, он
    * вызывает метод stopHandScroll().
    */
   void doAutoScroll();

   /// Отправляет сигнал о позиции курсора (если он менялся с прошлого сигнала).
   void sendMousePos();

   /// Устанавливает наш масштаб в соответствие со значением на слайдере.
   void setScaleFromSliderValue(int value);

private:
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
    
    /// Позиция курсора (в координатах сцены)
    QPointF mMousePos;
    
    /// Изменилась ли позиция курсора с предыдущего отправления сигнала
    bool mMousePosChanged;
    
    const double mMinimumScale;
    const double mMaximumScale;
    
    static const double kScaleStep;
    
    const int mMinimumZoomSliderValue;
    const int mMaximumZoomSliderValue;
    
    int mZoomSliderValue;
};
}

#endif // QFGUI_VIEWBASE_H
