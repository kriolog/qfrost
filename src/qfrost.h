/*
 * Copyright (C) 2010-2016  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_QFROST_H
#define QFGUI_QFROST_H

#include <QtCore/QRect>
#include <QtCore/QPair>
#include <QtWidgets/QGraphicsItem>

/// Сравнение двух точек с погрешностью 1e-12
inline bool qFuzzyCompare(const QPointF &point1, const QPointF &point2)
{
    return qFuzzyCompare(point1.x(), point2.x()) &&
           qFuzzyCompare(point1.y(), point2.y());
}

/// Сравнение двух точек с определённой заданной погрешностью
inline bool qFuzzyCompare(const QPointF &point1,
                          const QPointF &point2,
                          const qreal fuzziness)
{
    return qAbs(point2.x() - point1.x()) < fuzziness
           && qAbs(point2.y() - point1.y()) < fuzziness;
}

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(BoundaryPolygon)

typedef QList<BoundaryPolygon *> BoundaryPolygonList;
typedef QMap<QString, QPair< QVariant, QVariant > > ItemChanges;

enum PhysicalProperty {
    NoProperty,
    Temperature,
    HeatFlowDensity,
    HeatTransferFactor,
    Conductivity,
    Capacity,
    TransitionHeat,
    Moisture,
    Density,
    Energy,
    Power,
    PowerDensity
};

struct QFrost {
public:
    /**
     * Расстояние в единицах сцены между узлами сетки,
     * ставится в соответствие одной единице чертежа (безотносительно реальных
     * единиц измерения).
     * Этому числу, должны быть кратны размеры итемов в сцене. Потому, эту
     * константу, делённую на N>2 (напр., на 10), удобно использовать как размер
     * размер вспомогательного прямоугольника или крестап ри различных действиях.
     * @warning не следует путать сетку с видимой сеткой!
     */
    static const int unitsInGridStep = 10;

    /**
     * Размер, который всегда по крайней мере в два раза меньше минимального
     * размера блока.
     */
    static const int microSize = unitsInGridStep / 10;

    static const qreal microSizeF;

    /**
     * Малое число, которое по крайней мере в 100 раз больше принятой погрешности
     * сравнения (1e-12).
     */
    static const qreal accuracy;

    /// Количество знаков после запятой для значений координат сцены (в метрах).
    static const int meterDecimals = 3;
    /// Сколько метров в одной единице чертежа. Равняется 10^-metersDecimal.
    static const double metersInUnit;

    /// Количество знаков после запятой для размеров блока (в метрах).
    static const int meterDecimalsBlockSize = 2;
    /// Минимальный размер блока (в метрах). Равен 10^-meterDecimalsBlockSize.
    static const double minBlockSizeMeters;
    /// Минимальный размер блока (в координатах сцены).
    static const double minBlockSizeScene;

    /**
     * Сцена ограничена этим числом со всех четырёх сторон.
     * @note Сама сцена чуть больше (ведь для наглядности нужен запас),
     * но именно так ограничен анкор, инструменты и прочие вещи.
     */
    static const int sceneHalfSize;

    /**
     * Сцена ограничена этим числом (в метрах) со всех четырёх сторон.
     */
    static const double sceneHalfSizeInMeters;

    /// Несуществующая точка (для отражения того факта, что привязка не найдена)
    static const QPointF noPoint;

    /// Прямоугольник, ограничивающий сцену.
    static const QRect boundRect;

    /// Прямоугольник, ограничивающий сцену (приведённый к QRectF).
    static const QRectF boundRectF;

    /// Прямоугольник, ограничивающий сцену (в метрах).
    static const QRectF boundRectInMeters;

    /// Форма записи даты, содержащая день, месяц и год. Год всегда в 4 цифры.
    /// Представляет из себя изменённый (при необходимости) QLocale::dateFormat.
    static QString dateFormat();

    /// Перечисление типов инструментов
    enum ToolType {
        /// Нет инструмента == "обычный курсор"
        noTool,
        /// Полигональное выделение
        polygonalSelection,
        /// Прямоугольное выделение
        rectangularSelection,
        /// Создавалка полигонов граничных условий
        boundaryPolygonCreator,
        /// Создавалка эллиптических граничных условий
        boundaryEllipseCreator,
        /// Раскрашивалка граничных условий
        boundaryConditionsCreator,
        /// Создавалка блоков
        blockCreator,
        /// Эллиптическое выделение
        ellipseSelection,
        /// Построение температурных кривых
        curvePlot
    };

    /// Свойство, которое должно быть true, чтобы UndoBinder'ы создавали команды
    static const char *const UndoBinderIsEnabled;

    /// Свойство, которое установлено true, если введены некорректные данные
    static const char *const InvalidInputPropertyName;

    /// Изменяет свойство InvalidInputPropertyName у @p widget на @p isValid.
    /// Перерисовывает, если поменялось (ибо используются таблицы стилей Qt).
    static void setInputValidity(QWidget *widget, bool isValid);

    /// Физический смысл величины
    static const int PhysicalPropertyRole;
    /// Дополнительное (к Unit) ограничение на минимальное допустимое значение
    static const int MinimumRole;
    /// Дополнительное (к Unit) ограничение на максимальное допустимое значение
    static const int MaximumRole;
    /// Прямое редактирование величины (для использования в undo-коммандах)
    static const int DirectEditRole;
    /// Текст для undo-команды (для QComboBox и пр.)
    static const int UndoTextRole;

    /// Что отображают блоки в сцене
    enum BlockStyle {
        blockShowsSoil,                 ///< Грунт (и рамку)
        blockShowsTemperature,          ///< Температуру (и рамку)
        blockShowsBoundaryConditions,   ///< Ничего не показывает (только рамку)
        blockShowsTemperatureField,     ///< Температуру (без рамки)
        blockShowsTemperatureDiffField, ///< Разницу между T и T_bf (без рамки)
        blockShowsThawedPartField,      ///< Отн. объём талой фазы (без рамки)
        blockShowsConditionField        /**< Температуру, если фазовых переходов
                                             нет: блок 100% талый/мёрзлый; иначе
                                             отн. объём талой фазы (без рамки)*/
    };

    /// Метры из единиц сцены
    static double meters(int sceneUnits) {
        return k * sceneUnits;
    }

    /// Метры из единиц сцены
    static double meters(double sceneUnits) {
        return k * sceneUnits;
    }

    static QSizeF meters(const QSizeF &sceneUnits) {
        return k * sceneUnits;
    }

    static QPointF meters(const QPointF &sceneUnits) {
        return k * sceneUnits;
    }

    static QRectF meters(const QRectF &sceneUnits) {
        QRectF result;
        result.setSize(meters(sceneUnits.size()));
        result.moveTopLeft(meters(sceneUnits.topLeft()));
        return result;
    }

    /**
     * Единицы сцены из метров
     */
    static int sceneUnits(double meters) {
        return qRound(meters / QFrost::k);
    }

    static QPoint sceneUnits(const QPointF &meters) {
        return QPoint(sceneUnits(meters.x()), sceneUnits(meters.y()));
    }

    static QSize sceneUnits(const QSizeF &meters) {
        return QSize(sceneUnits(meters.width()), sceneUnits(meters.height()));
    }

    /**
     * ZValue для различных итемов (для удобства обращения)
     */
    enum ZValues {
        AnchorZValue = 1002,
        ToolZValue = 1001,
        BackgroundZValue = 1000,
        BoundaryPolygonZValue = 999
    };

    /// Типы пересечения полигона граничных условий с данным полигоном
    enum IntersectionType {
        //результатом пересечения может быть точка, отрезок, фигура или их
        //комбинация
        allVariants,
        //результатом пересечения не может быть только набор изолированных
        //точек
        excludingOnlyPoints,
        //результатом пересечения может быть одна или несколько фигур
        //комбинация
        excludingBorder
    };

    /// Type для различных итемов
#define LUGTYPE (QGraphicsItem::UserType + 1)
#define BOUNDARYPOLYGONTYPE (QGraphicsItem::UserType + 2)
#define BLOCKTYPE (QGraphicsItem::UserType + 3)

#define DEFINETYPE(tname) enum { Type = tname }; int type() const { return Type; }

    /// Информация, необходимая для отображения времени одного шага,
    /// если совершается @p numStepsInDay шагов в сутки.
    /// Первый элемент возвращаемой пары -- текст в формате HH:mm:ss.
    /// Второй элемент -- является ли полученный текст точным (или примерным).
    static QPair<QString, bool> singleStepInfo(int numStepsInDay);

    /// Наибольший размер иконки @p icon с высотой, не превышающий @p maxHeight.
    /// В отличие от actualSize, этот размер всегда содержится в availableSizes.
    static QSize upperBoundIconSize(const QIcon &icon, int maxHeight);

    /// Строка с римской записью числа @p number
    static QString romanNumeral(int number, int markCount = 0);
    
    /// Данные для заголовка таблицы с названием месяца (летние шрифты курсивом)
    static QVariant monthHeaderData(int section, int role,
                                    bool useShortMonthFormat = true);

private:
    /**
     * Коэффициент для получения метров из единиц сцены.
     * На него нужно умножать: double meters = sceneUnits * QFrost::k.
     * @warning Для обратного преобразования (метры -> единицы сцены) нельзя
     *          просто делить на него -- результат может обрезаться.
     *          Нужно использовать sceneUnits().
     */
    static const double k;
};

}

#endif // QFGUI_QFROST_H
