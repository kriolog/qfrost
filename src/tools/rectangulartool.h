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

#ifndef QFGUI_RECTANGULARTOOL_H
#define QFGUI_RECTANGULARTOOL_H

#include <tools/tool.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(ToolSettings)
QT_FORWARD_DECLARE_CLASS(RectangularToolSettings)
QT_FORWARD_DECLARE_CLASS(Lug)

class RectangularTool : public Tool
{
    Q_OBJECT
public:
    RectangularTool(ToolSettings *settings,
                    bool allowMovingCenter = true);

    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget = 0) = 0;
    virtual void apply(bool alt) = 0;
    QRectF boundingRect() const;

    Qt::Orientations changesOrientations() const {
        return mChangesOrientations;
    }

private:
    /// Размеры инструмента.
    QSizeF mSize;

    Lug *mTopLeftLug;
    Lug *mTopRightLug;
    Lug *mBottomLeftLug;
    Lug *mBottomRightLug;
    Lug *mLeftLug;
    Lug *mRightLug;
    Lug *mTopLug;
    Lug *mBottomLug;
    Lug *mCenterLug;
    Lug *mActiveLug;

    /**
     * Расположение активного ушка. Если активного ушка нет, его последнее
     * актуальное расположение.
     */
    enum {
        topLeft,
        topRight,
        bottomLeft,
        bottomRight,
        left,
        right,
        top,
        bottom,
        center
    } mActiveLugLocation;

    enum ChangesType {
        notChangig,
        moving,
        resizing
    };

    ChangesType mChangesType;

    Qt::Orientations mChangesOrientations;

    /**
     * Если true, ушки на данный момент находятся внутри инструменты.
     * То есть внутри инструмента достаточно места, чтобы туда влезки внутренние
     * ушки. Если false, то ушки рисуются снаружи.
     */
    bool mLugsAreInner;

    /// Минимальные размеры ушек.
    qreal mLugsMaxSize;

    /// Максимальные размеры ушек.
    qreal mLugsMinSize;

    RectangularToolSettings *mSettings;

    /**
     * Обновляет геометрию всех ушек (размеры и позицию) исходя из
     * mLugsMinSize и mLugsMaxSize.
     */
    void updateLugsGeometry();

    /**
     * Частично обновляет геометрию активного ушка (только позицию).
     */
    void updateActiveLugGeometry();

    /**
     * Прячет все дочерние ушки, кроме @p object.
     */
    void hideAllLugsExcept(const QGraphicsObject *object);

    /**
     * Показывает все дочерние ушки.
     */
    void showAllLugs();

    /**
     * Изменяет геометрию инструмента, а также нормализует его размеры и меняет
     * местами указатели на язычки, mActiveLug и mActiveLugLocation, если
     * нормализация произошла.
     */
    void updateGeometryAndLugPointers(const QRectF &rect);

    /**
     * Даёт ушкам курсоры в зависимости от их расположения.
     */
    void updateLugsCursors();

    /**
     * Заменяет mActiveLug на @p lug, обновляет mActiveLugLocation и
     * mChangesOrientations.
     */
    void setActiveLug(Lug *lug);

    /**
     * Передвигает курсор в ключевую точку активного ушка.
     */
    void moveCursorToActiveLug() const;

    /**
     * Обрезает прямоуольник @p rect по прямоугольнику сцены.
     */
    static void cutBySceneRect(QRectF &rect);

    /**
     * Передвигает @p rect так, чтобы он влезал в прямоугольник сцены.
     */
    static void moveBySceneRect(QRectF &rect);

    /**
     * Ближайшая к @p point точка, лежащая внутри прямоугольника сцены.
     */
    static QPointF pointBoundedByBoundRect(const QPointF &point);

protected:
    /**
     * Прямоугольник инструмента в координатах сцены.
     */
    QRectF rectInScene() const {
        return boundingRect().translated(pos());;
    }

    bool isChanging() const {
        return mChangesType != notChangig;
    }

    /**
     * Этот метод запускается после каждого изменения.
     * По умолчанию ничего не делает.
     */
    virtual void onSizeChange() { }

    /// Этот метод запускается после начала изменений.
    virtual void onStartChange() { }

    /// Этот метод запускается после остановки изменений.
    virtual void onStopChange() = 0;

    virtual void onSceneHasChanged();

    RectangularToolSettings *settings() {
        return mSettings;
    }

    const RectangularToolSettings *settings() const {
        return mSettings;
    }

private slots:
    /**
     * Начать реагирование на привязку.
     *  @param alternateChange если true - изменяться будут размеры,
     *                          если false - позиция)
     */
    void startChange(bool alternateChange);

    /**
     * Метод вызывается при остановке изменений.
     */
    void stopChange();

    /**
     * Среагировать на изменение позиции, высланное ушком.
     * @param point новая позиция (возможно, с заменной координатой
     *              относительно реальной позиции привязки).
     */
    void changeTo(const QPointF &point);

    /**
     * Прячет все ушки кроме того, на который навели мышку, если надо.
     */
    void onLugHoverChanged(bool lugIsHovered);

    /**
     * Обновляет mLugsMaxSize, mLugsMinSize и геометрию всех ушек.
     */
    void updateLugsSizeLimits(qreal scale);

    void getSizeFromSettings(bool dontNeedIt);
    void getTopLeftPosFromSettings(bool dontNeedIt);
};

}

#endif // QFGUI_RECTANGULARTOOL_H
