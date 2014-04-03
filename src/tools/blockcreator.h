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

#ifndef QFGUI_BLOCKCREATOR_H
#define QFGUI_BLOCKCREATOR_H

#include <tools/rectangulartool.h>

namespace qfgui
{

class BlockCreator: public RectangularTool
{
    Q_OBJECT
public:
    BlockCreator(ToolSettings *settings);

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);

    void apply(bool alt);

protected:
    /**
     * Применяем выделение.
     */
    void onStopChange();

private:
    /**
     * Размер блоков, которые мы стремимся создать
     */
    QSize blocksSize() const;

    /**
     * Коэффициент геометрической прогрессии размеров блоков, которые мы
     * стремимся создать
     */
    QSizeF blocksQ() const;

    /// Координаты столбцов разбиения главного прямоугольника
    QList<qreal> mColumns;

    /// Координаты строк разбиения главного прямоугольника
    QList<qreal> mRows;

    /// Точка, от которой начинаются блоки (в координатах инструмента).
    QPointF mInitialPoint;

    /// Точка, в которой заканчиваются все блоки (в координатах инструмента).
    QPointF mLastPoint;

    /**
     * Точка, в которой заканчиваются блоки с заданным размером (и начинаются
     * блоки с автоматически подобранным размером) (в координатах инструмента).
     */
    QPointF mPrimaryLastPoint;

    /// Есть ли "тетрис" во внутреннем прямоугольнике.
    bool mTetrisInPrimary;
    /// Есть ли "тетрис" во внешней и/или внутренней области.
    bool mTetrisInComplementary;

    /**
     * Члены геометрической прогрессии, округлённые по шагу сетки.
     * @param first первый член геометрической прогресси
     * @param q коэффициент геометрической прогресси
     * @param max максимальная величина члена геометрической прогресси
     */
    static QList<qreal> roundedGeometricSequence(const qreal first,
            const qreal q,
            const qreal max);

    /**
     * Прямоугольники для блоков к созданию в сцене, нормализованные
     * и упорядоченные по возрастанию координат.
     */
    QList<QList<QRectF> > blockRects(bool onlyPrimary);

    /**
     * Есть ли "тетрис" в заданном прямоугольнике.
     * Тетрис - это неспособность полностью  заменить занимаемое замещающимеся
     * блоками место.
     * @param rect проверямый прямоугольник (в координатах сцены).
     */
    bool thereIsTetris(const QRectF &rect) const;

    /**
     * Отрисовать (потенциальные) блоки.
     */
    void drawBlocks(QPainter *painter);

    /**
     * Главный прямоугольник.
     * Охватывающий те блоки, который подойдут под требования пользователя.
     */
    QRectF primaryRect() const {
        return QRectF(mInitialPoint, mPrimaryLastPoint).normalized();
    }

    /**
     * Главный прямоугольник в координатах сцены.
     */
    QRectF primaryRectInScene() const {
        return primaryRect().translated(pos());
    }

private slots:
    /**
     * Пересчёт координат потенциальных блоков с обновлением сведений о
     * "тетрисе" и вызовом update().
     */
    void recalculate();
};
}


#endif // QFGUI_BLOCKCREATOR_H
