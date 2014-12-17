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

#include "viewbase.h"

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(Block)
QT_FORWARD_DECLARE_CLASS(MainWindow)
QT_FORWARD_DECLARE_CLASS(Scene)

class View : public ViewBase
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

    QPointF visibleTopLeft() const;

    bool isLight() const {
        return mIsLight;
    }

    void save(QDataStream &out);
    void load(QDataStream &in);

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);

    Qt::Orientations sceneChangesOrientations() const;

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

    /// Точка, на которую нужно отцентроваться, когда мы покажемся
    QPointF mPointToCenterOn;

    /**
     * Устанавливает цвет фона и сетки исходя из mIsLight
     */
    void updateColorScheme();

public slots:
    void setLightColorScheme(bool b);

    /**
     * Изменение промежутка сетки
     */
    void updateGridSpan();

signals:
    /**
     * Сигнал о том, что изменилось расстояние между видимыми узлами сетки
     */
    void gridSpanChanged(int gridSpan);

    void colorSchemeChanged();
};

}

#endif // QFGUI_VIEW_H
