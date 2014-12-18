/*
 * Copyright (C) 2010-2014  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_BOUNDARYPOLYGONCALC_H
#define QFGUI_BOUNDARYPOLYGONCALC_H

#include <QtCore/QtGlobal>

#include <qfrost.h>

QT_FORWARD_DECLARE_CLASS(QPainterPath)
QT_FORWARD_DECLARE_CLASS(QLineF)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(BoundaryPolygon)
QT_FORWARD_DECLARE_CLASS(Vertex)
QT_FORWARD_DECLARE_CLASS(BoundaryPolyline)
QT_FORWARD_DECLARE_CLASS(BoundaryCondition)
QT_FORWARD_DECLARE_CLASS(Scene)

class BoundaryPolygonCalc
{
public:
    /**
     * Инициализирует экземпляр этого класса для сцены @p scene.
     */
    BoundaryPolygonCalc(const Scene *scene): mScene(scene) {
    }

    /**
     * Операция прибавления полигона.
     * Метод пытается объединить данный полигон с каждым из внешних
     * (тех, внутри которых существуют другие полигоны граничных условий,
     * называемые внутренними), или внутренних (если нет внешних, с которыми он
     * пересекается) полигонов граничных условий. После объединения получившиеся
     * полигоны получают условия старых, где это возможно. Если данный
     * полигон полностью лежит внутри какого-либо внешнего, то становится его
     * внутренним полигоном.
     *
     * @param polygon должен быть симплифицированым средствами Qt, то есть он
     * должен быть замкнутым, непересекающимся, не должен содержать смежные
     * стороны, принадлежащие одной прямой.
     * @param ignoreInnerPolygons надо ли изменять внутренние полигоны
     *
     * @return Пару списков: полигоны граничных условий к добавлению в сцену и
     * к удалению из неё.
     */
    QPair<BoundaryPolygonList, BoundaryPolygonList> uniteOperation(const QPolygonF &polygon,
            bool mustChangeInnerPolygons = true);


    /**
     * Операция вычитания полигона.
     * Метод пытается вычесть данный полигон из каждого из внешних (тех, внутри
     * которых существуют другие полигоны граничных условий, называемые
     * внутренними), или внутренних (если нет внешних, с которыми он
     * пересекается) полигонов граничных условий.
     * После вычитания получившиеся полигоны получают условия старых, где
     * это возможно.
     *
     * @param polygon должен быть симплифицированым средствами Qt, то есть он
     * должен быть замкнутым, непересекающимся, не должен содержать смежные
     * стороны, принадлежащие одной прямой.
     *
     * @return Пару списков: полигоны граничных условий к добавлению в сцену и
     * к удалению из неё.
     */
    QPair<BoundaryPolygonList, BoundaryPolygonList> subtractOperation(const QPolygonF &polygon);

    /**
     * Операция удаления граничных условий.
     * Возвращает список полигонов, которые имеют в себе хотя бы одно
     * из граничных условий @p conditions
     * и соответствующие списки углов, где оно заменено на пустое.
     */
    QList<QPair<BoundaryPolygon *, QList<Vertex> > > removeConditionsOperation
    (const QList<const BoundaryCondition *> &conditions) const;

    /**
     * Возвращает точку, лежащую  на отрезке на заданном расстоянии от его
     * начала.
     *
     * @param point1 начальная точка отрезка
     *
     * @param point2 конечная точка отрезка
     *
     * @param distance расстояние от начальной точки отрезка до искомой точки.
     */
    static QPointF pointOnSegment(const QPointF &point_1,
                                  const QPointF &point_2,
                                  const qreal distance);

    /**
     * Возвращает точку, лежащую  на отрезке на заданном расстоянии от его
     * начала.
     *
     * @param segment отрезок
     *
     * @param distance расстояние от начальной точки отрезка до искомой точки.
     */
    static QPointF pointOnSegment(const QLineF &segment,
                                  const qreal distance);

    /**
     * Является ли полигон самопересекающимся. Если нет, то количество сторон не
     * может быть менне трёх; противонаправленные смежные стороны допускаются
     */
    static bool polygonIsNotSelfIntersecting(const QPolygonF &polygon);

    /**
     * Принадлежат ли два отрезка одной и той же прямой
     *
     * @param line_1 первый отрезок
     *
     * @param line_2 вторый отрезок
     *
     */
    static bool belongToOneLine(const QLineF &line_1, const QLineF &line_2);

    /**
     * Объединяет один path с другим. Возвращает список получившихся
     * полигонов.
     *
     * @param path path, к которому прибавляют.
     *
     * @param unitingPath path, который прибавляют.
     *
     * @return список получившихся полигонов.
     */
    static QList<QPolygonF> united(const QPainterPath &path, const QPainterPath &unitingPath);

    /**
     * Пересекает один полигон с другим. Возвращает список получившихся
     * полигонов.
     *
     * @param polygon содержащий полигон.
     *
     * @param intersectingPolygon содержащийся полигон.
     */
    static bool isContained(const QPolygonF &polygon, const QPolygonF &potentialContainedPolygon);

    /**
     * Метод проверяет,являются ли вектора, построенные на данных отрезках,
     * сонаправленными.
     */
    static bool areCodirectionalLines(const QLineF &line_1,
                                      const QLineF &line_2);

    /**
     * Метод проверяет,являются ли вектора, построенные на данных отрезках,
     * коллинеарными.
     */
    static bool areCollinearLines(const QLineF &line_1,
                                  const QLineF &line_2);

    /**
     * Удаляет все нулевые углы полигона, то есть те, в которых смежные отрезки
     * противонаправлены. Полигон не должен самопересекаться в смысле Qt и
     * должен быть замкнутым.
     */
    static QPolygonF removeZeroAndFlatAngles(const QPolygonF &polygon);

    /**
     * Операция придания условий полигону граничных условий на заданной полилини.
     * Если @a prepareCorners == true, то в углы списка, не имеющие точек смены
     * условия с расстоянием, равным нулю, такие точки добавляются.
     * @return новый список углов полигона.
     */
    static QList<Vertex> addConditionFromPolyline(const QList<Vertex> &corners,
            const BoundaryPolyline &polyline,
            const bool prepareCornersAndSimplify = true);

    /**
     * Присваевает полигону заданное свойство.
     * Возвращает список углов полигона граничных условий. При этом
     * информация о свойствах хранится в точке смены условия с нулевым расстоянием
     * нулевого угла.
     */
    static QList<Vertex> setCondition(const QPolygonF &polygon,
                                      const BoundaryCondition *condition);

    /**
     * Возвращает точку, лежащую на отрезке, по известной её координате x.
     * Если получившаяся точка лежит вне внутренней части отрезка, возвращается
     * QFrost::noPoint.
     */
    static QPointF pointByXOnSegment(const qreal x,
                                     const QLineF &segment);

    /**
     * Точку, лежащую на отрезке, по известной её координате y.
     * Если получившаяся точка лежит вне внутренней части отрезка, возвращается
     * QFrost::noPoint.
     */
    static QPointF pointByYOnSegment(const qreal y,
                                     const QLineF &segment);

    /**
     * Расстояние от начальной точки данного
     * отрезка до данной точки. Расстояние равно нулю, если эти две точки
     * примерно совпадают; меньше нуля, если точка лежит
     * в левом полупространстве, образованном прямой, нормальной к данному
     * отрезку и проходящей через его начальную точку; меньше нуля в других
     * случаях. Если расстояние примерно совпадает с длинной отрезка,
     * оно приравнивается к ней.
     *
     * @param point точка
     *
     * @param line отрезок
     */
    static qreal signedDistanceFromOrigin(const QPointF &p,
                                          const QLineF &segment);
    // (!) sic
    /**
     * Расстояние от начальной точки данного отрезка до проекции данной точки на
     * него. Расстояние равно нулю, если эти две точки
     * примерно совпадают; меньше нуля, если проекция
     * в левом полупространстве, образованном прямой, нормальной к данному
     * отрезку и проходящей через его начальную точку; меньше нуля в других
     * случаях. Если расстояние примерно совпадает с длинной отрезка,
     * оно приравнивается к ней.
     *
     * @param point точка
     *
     * @param line отрезок
     */
    static qreal signedProjectionDistance(const QPointF &p,
                                          const QLineF &segment);

    /**
     * Полигон, вписанный в единичную окружность с центром в (0,0), имеющий
     * @p numberOfAngles углов.
     */
    static QPolygonF unitRoundPolygon(uint numberOfAngles = 120);
    
    /**
     * Полигон, вписанный в эллипс @p ellipse, имеющий @p numberOfAngles углов.
     */
    static QPolygonF ellipseShapedPolygon(const QRectF &ellipse,
                                   uint numberOfAngles = 120);

private:
    const Scene *mScene;

    /**
     * Внешние полигоны граничных условий, пересекающиеся с данным полигоном.
     *
     * @param polygon полигон для которого ведётся поиск.
     *
     * @param intersectionType тип пересечения:
     *            allVariants -- результатом пересечения может быть точка,
     *                             отрезок, фигура или их комбинация
     *            excludingOnlyPoints -- результатом пересечения не может быть
     *                               множество изолированных точек
     *            excludingBorder -- результатом пересечения должен включать
     *                               хотя бы одну фигуру.
     * @return список полигонов граничных условий, пересекающихся с @a polygon
     */
    BoundaryPolygonList outerBoundaryPolygons(const QPolygonF &polygon,
            const QFrost::IntersectionType &intersectionType) const;


    /**
     * Полилинии граничных условий, полученные из исходных полигонов и из списка
     * "вычтенных". "Вычтенные" полигоны создаются при вычитании исходных из
     * модифицирующего. Полилинии, им соответствующие - это дополнительные
     * полилинии, получающиеся при наложении полилиний исходных полигонов на
     * "вычтенный"
     *
     * @param polygons список исходных полигонов
     *
     * @param modifyingPolygon модифицирующий полигон

     */
    QList<BoundaryPolyline> initialAndSubtractedBoundaryPolylines(const BoundaryPolygonList polygons,
            const QPolygonF &modifyingPolygon);

    /**
     * Пытается вычесть @p polygon из каждого дочернего полигона
     * @p outerPolygon.
     *
     * @param mustChangeInnerPolygons надо ли менять внутренние полигоны
     *                                   (если false -- просто скопируем их)
     *
     * @warning @p polygon должен целиком содержаться в @p outerPolygon.
     *
     * @return указатель на новый полигон - копию @p outerPolygon и заполненный
     *         новыми детьми. Или, если действие требуется, то есть операция
     *         ничего не изменила бы, то NULL.
     */
    BoundaryPolygon *newPolygonWithSubtractedInnerPolygons(BoundaryPolygon *outerPolygon,
            const QPolygonF &modifyingPolygon,
            bool mustChangeInnerPolygons);

    /**
     * Пытается объединить @p polygon с каждым из дочерних полигонов
     * @p outerPolygon.
     *
     * @warning @p polygon должен целиком содержаться в @p outerPolygon.
     *
     * @return указатель на новый полигон - копию @p outerPolygon и заполненный
     *         новыми детьми. Или, если @p polygon содержится в одном из
     *         дочерних полигонов @p outerPolygon, то NULL.
     */
    BoundaryPolygon *newPolygonWithUnitedInnerPolygons(BoundaryPolygon *outerPolygon,
            const QPolygonF &modifyingPolygon);

    /**
     * Список углов, которые получатся у полигона @p polygon, если задать ему
     * граничные условия из полигонов @p donorPolylines по совпадающим частям.
     *
     * Наложение полилиний с условиями друг на друга происходит в порядке их
     * следования в подаваемом списке.
     * @warning Полигон и полилинии должны быть симплифицированы (не содержать
     * одинаковых соседних точек)
     */
    static QList<Vertex> conditionsFromPolylines(const QPolygonF &polygon,
            const QList<BoundaryPolyline> &donorPolylines);

    /**
     * Удаляет точки смены условия, если условия на них на самом деле не меняются
     * (у рассматриваемой и предыдущей точек смены условия совпадают).
     */
    static void simplifyConditions(QList<Vertex> &corners);

    /**
     * Добавляет точки смены условия с нулевым расстоянием в те углы списка,
     * в которых их нет.
     */
    static QList<Vertex> addZeroDistanceConditionPoints(const QList<Vertex> &corners);

    /**
     * Метод нужен для отладки. Возвращает true, если хотя бы у одного
     * угла (кроме последнего), нет точки смены условия с нулевым расстоянием.
     */
    static bool zeroDistanceConditionPointAbsent(const QList<Vertex> &corners);

    /**
     * Предполагается, что на вход подаётся полигон (внешний), внутри которого
     * лежат другие (внутренние), причём во внутренних полигонов нет. Метод
     * возвращает внешний полигон.
     *
     * @param polygons список полигонов, в котором ваполняется поиск.
     *
     * @return внешний полигон, если находит его, полигон с единственной нулевой
     * точкой в обратном случае. Если размер списка = 1, то единственный полигон
     * в нём - внешний.
     */
    QPolygonF getOuterPolygon(const QList<QPolygonF> &polygons);

    /**
     * Предполагается, что на вход подаётся полигон (внешний), внутри которого
     * лежат другие (внутренние), причём во внутренних полигонов нет. Метод
     * сортирует полигоны так, что внешний оказывается первым в списке.
     *
     * @param список полигонов, в котором ваполняется поиск.
     *
     * @return список из внешнего и внутренних полигонов, если находится внешний
     * полигон, полигон с единственной нулевой точкой в обратном случае. Если
     * размер списка = 1, то единственный полигон в нём - внешний.
     */
    QList<QPolygonF> sortOuterAndInnerPolygons(QList<QPolygonF> &polygons);

    /**
     * Объединяет один полигон с другим. Возвращает список получившихся
     * полигонов.
     *
     * @param polygon полигон, к которому прибавляют.
     *
     * @param unitingPolygon полигон, который прибавляют.
     *
     * @return список получившихся полигонов.
     */
    QList<QPolygonF> united(const QPolygonF &polygon, const QPolygonF &unitingPolygon) const;

    /**
     * Объединяет один полигон с другими. Возвращает список получившихся
     * полигонов.
     *
     * @param polygon полигон, к которому прибавляют.
     *
     * @param unitingPolygons полигоны, которые прибавляют.
     *
     * @return список получившихся полигонов.
     */
    QList<QPolygonF> united(const QPolygonF &polygon, const QList<QPolygonF> &unitingPolygons) const;

    /**
     * Объединяет одни полигоны с другими. Возвращает список получившихся
     * полигонов.
     *
     * @param polygons полигоны, к которым прибавляют.
     *
     * @param unitingPolygons полигоны, которые прибавляют.
     *
     * @return список получившихся полигонов.
     */
    QList<QPolygonF> united(const QList<QPolygonF> &polygons, const QList<QPolygonF> &unitingPolygons) const;

    /**
     * Вычитает второй полигон из первого. Возвращает список получившихся
     * полигонов.
     *
     * @param polygon полигон, из которого вычитают.
     *
     * @param subtractingPolygon полигон, который вычитают.
     *
     * @return список получившихся полигонов.
     */
    QList<QPolygonF> subtracted(const QPolygonF &polygon, const QPolygonF &subtractingPolygon) const;

    /**
     * Пересекает один полигон с другим. Возвращает список получившихся
     * полигонов.
     *
     * @param polygon один полигон.
     *
     * @param intersectingPolygon другой полигон.
     *
     * @return список получившихся полигонов.
     */
    QList<QPolygonF> intersected(const QPolygonF &polygon, const QPolygonF &intersectingPolygon) const;

    /**
     * Метод пытается добавить в подаваемый на вход список полилиний с условиями
     * дополнительные, получаемые из полигона, получившего условия из тех же исходных
     * полилинияй. При этом, если у получаемых из полигона линий существует
     * одно и только одно условие, отличный от пустого, все они получают это
     * условия и добавляются к исходным. В противном случае ничего не происходит.
     *
     * @param polygon полигон.
     *
     * @param conditionPolylines список исходных полилиний с условиями.
     *
     * @return добавились ли полилинии с условиями.
     */
    bool tryExtraConditionPolylines(const QPolygonF &polygon, QList<BoundaryPolyline> &conditionPolylines);

    /**
     * Метод заменяет список полигонов на список этих полигонов, из которых
     * вычтен данный полигон
     *
     * @param polygons заменяемый список полигонов.
     *
     * @param polygon полигон, с которым пересекается список.
     */
    void changeToSubtracted(QList<QPolygonF> &polygons, const QPolygonF &polygon);

    /**
     * Меняет местами точки в отрезке @p line
     */
    static void reverseLine(QLineF &line) {
        QPointF p1;
        p1 = line.p2();
        line.setP2(line.p1());
        line.setP1(p1);
    }

    /**
     * Скалярное произведение векторов, построенных на данных отрезках.
     */
    static qreal dotProduct(const QLineF &line_1, const QLineF &line_2) {
        return line_1.dx() * line_2.dx() + line_1.dy() * line_2.dy();
    }

    /**
     * Полигон является простым, то есть он не должен самопересекаться и содержать
     * противонаправленные смежные стороны.
     */
    bool polygonIsSimple(const QPolygonF &polygon) const;

    /**
     * Удаляет все нулевые углы каждого полигона, то есть те, в которых смежные
     * отрезки противонаправлены. Полигоны не должны самопересекаться в смысле
     * Qt и должны быть замкнутыми.
     */
    static QList< QPolygonF > removeZeroAndFlatAngles(const QList<QPolygonF> &polygons);

    /**
     * Удаляет все развёрнутые углы полигона, то есть те, в которых смежные
     * отрезки сонаправлены. Полигон не должен иметь повторяющихся точек.
     */
    static QPolygonF removeFlatAngles(const QPolygonF &polygon);

    /**
     * После преобразования у полигона не существует равных последовательных
     * углов.
     */
    static QPolygonF removeEqualConsecutiveCorners(const QPolygonF &polygon);

    /**
     * Метод проверяет,являются ли вектора, построенные на данных отрезках,
     * противонаправленными.
     */
    static bool areOppositeDirectionLines(const QLineF &line_1,
                                          const QLineF &line_2);

    /**
     * Метод нужен для отладки. Проверяет, нет ли в полигоне противонаправленных
     * смежных сторон.
     */
    static bool collinearConsecutiveSidesWereFound(const QPolygonF &polygon);

    /**
     * Метод нужен для отладки. Проверяет, нет ли в полигоне противонаправленных
     * смежных сторон.
     */
    static bool oppositeDirectionConsecutiveSidesWereFound(const QPolygonF &polygon);

    /**
     * Метод нужен для отладки. Проверяет, нет ли в каком - либо из полигонов
     * противонаправленных смежных сторон.
     */
    static bool oppositeDirectionConsecutiveSidesWereFound(const QList<QPolygonF> &polygons);
};


}

#endif // QFGUI_BOUNDARYPOLYGONCALC_H
