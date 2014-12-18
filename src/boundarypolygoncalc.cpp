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

#include <boundarypolygoncalc.h>

#include <QtCore/QPair>
#include <QtCore/QLineF>
#include <QtMath>

#include <graphicsviews/boundarypolygon.h>
#include <graphicsviews/scene.h>

using namespace qfgui;

QList<Vertex> BoundaryPolygonCalc::conditionsFromPolylines(const QPolygonF &polygon,
        const QList<BoundaryPolyline> &donorPolylines)
{
    Q_ASSERT(!polygon.isEmpty());
    Q_ASSERT(polygon.isClosed());

    QList<Vertex> result;

    if (donorPolylines.isEmpty()) {
        // Если не из чего придавать условия полигону, возвращаем пустой.
        Vertex voidCorner;
        foreach(QPointF point, polygon) {
            voidCorner.point = point;
            result.append(voidCorner);
        }
        result[0].conditionPoints.append(ConditionPoint());
        return result;
    }

    //заполняем полигон граничных условий "пустыми" углами
    Vertex voidCorner;
    voidCorner.conditionPoints.append(ConditionPoint());
    for (int i = 0; i < polygon.size() - 1; ++i) {
        voidCorner.point = polygon.at(i);
        result.append(voidCorner);
    }
    //а последний угол -- без смены условия (т.к. полигон замкнут)
    voidCorner.conditionPoints.clear();
    voidCorner.point = polygon.last();
    result.append(voidCorner);

    //пребираем донорские полилинии
    foreach(BoundaryPolyline donorPolyline, donorPolylines) {
        result = addConditionFromPolyline(result, donorPolyline, false);
    }

    simplifyConditions(result);

    return result;
}

QList<Vertex> BoundaryPolygonCalc::addConditionFromPolyline(const QList<Vertex> &corners,
        const qfgui::BoundaryPolyline &polyline,
        const bool prepareCornersAndSimplify)
{
    Q_ASSERT(!corners.isEmpty());
    Q_ASSERT(corners.first().point == corners.last().point);
    Q_ASSERT(corners.last().conditionPoints.isEmpty());

    QList<Vertex> result;
    result = corners;

    if (prepareCornersAndSimplify) {
        // Добавляем в начало каждого угла предварительно найденное условие.
        result = addZeroDistanceConditionPoints(result);
    }
    Q_ASSERT(!zeroDistanceConditionPointAbsent(result));

    for (int i = 0; i < result.size() - 1; ++i) {
        QLineF currentSide;
        currentSide = QLineF(result.at(i).point, result.at(i + 1).point);

        qreal sideLength;
        sideLength = currentSide.length();

        Vertex currentVertex;
        currentVertex = result.at(i);

        //для каждого отрезка текущей донорской полилинии
        for (int j = 0; j < polyline.polygon.size() - 1; ++j) {
            QLineF polylineSegment;
            polylineSegment = QLineF(polyline.polygon.at(j), polyline.polygon.at(j + 1));
            //если текущий отрезок нашего полигона и текущая донорская линия
            //лежат на одной прямой
            if (belongToOneLine(currentSide, polylineSegment)) {
                //Делаем их сонаправленными
                if (!qFuzzyCompare(currentSide.dx(), 0)) {
                    if (currentSide.dx()*polylineSegment.dx() < 0) {
                        reverseLine(polylineSegment);
                    }
                } else {
                    if (currentSide.dy()*polylineSegment.dy() < 0) {
                        reverseLine(polylineSegment);
                    }
                }
                //номер точки, начиная с которой нужно заменять условие
                int originPointNumber;
                ConditionPoint currentConditionPoint;
                /* условие, которое будет в конечном итоге идти после конечной
                 * точки донорского отрезка. Определяется условие, которое было
                 * в этой точке изначально. Проще говоря, нужно это условие сохранить.*/
                const BoundaryCondition *previousCondition;

                qreal distanceToOrigin;
                qreal distanceToEnd;

                /* перевод координат донорского отрезка в расстояние от начальной точки
                 * обрезание по границам текущего отрезка полигона */
                distanceToOrigin = qMax<qreal>(0, signedDistanceFromOrigin(polylineSegment.p1(), currentSide));
                distanceToEnd = qMin<qreal>(sideLength, signedDistanceFromOrigin(polylineSegment.p2(), currentSide));
                if (!(distanceToOrigin > sideLength || qFuzzyCompare(distanceToOrigin, sideLength) || distanceToEnd < 0)) {
                    //преобразованные начало и конец донорского отрезка валидны

                    ///текущая точка - начало преобразованного донорского отрезка
                    currentConditionPoint.condition = polyline.condition;
                    currentConditionPoint.distance = distanceToOrigin;
                    bool conditionPointWasFound = false;
                    //для всех точек смены условия текущего отрезка полигона
                    //нужно найти ту из них, перед которой вставим новую
                    //точку смены условия (начальную точку донорского отрезка)
                    for (int k = 0; k < currentVertex.conditionPoints.size(); ++k) {
                        if (qFuzzyCompare(distanceToOrigin,  currentVertex.conditionPoints.at(k).distance)) {
                            /* если начало донорского отрезка совпадает с текущей
                             * точкой смены условия (по расстоянию), меняем в ней
                             * условие на донорское */

                            // запоминаем условие текущей точки
                            previousCondition = currentVertex.conditionPoints.at(k).condition;
                            // Изменяем условие текущий точки смены условия.
                            currentVertex.conditionPoints[k].condition = currentConditionPoint.condition;
                            // 1я точка после новой имеет следующий за k номер
                            originPointNumber = k + 1;
                            /* точка смены условия, вместо которой вставляем
                             * начало донорского отрезка, нашлась. Она будет
                             * следующей за той, в которой поменяли условие. Если
                             * след. точки не существует, мы не войдём в
                             * дальнейшем в циклы вставки конца донорского
                             * отрезка и  удаления точек */
                            conditionPointWasFound = true;
                            break;
                            /* если точка начала донорского отрезка лежит левее текущей
                             * точки смены условия, вставляем перед последней первую */
                        } else if (distanceToOrigin < currentVertex.conditionPoints.at(k).distance) {
                            /* если точка смены условия не первая, запоминаем
                             * условие предыдущей точки смены условия */
                            if (k >= 1) {
                                previousCondition = currentVertex.conditionPoints.at(k - 1).condition;
                                /* первая точка смены условия имеет distance = 0,
                                 * поэтому в эту ветку if мы никогда не попадём */
                            } else {
                                //запоминаем условие первой точки
                                Q_ASSERT(!currentVertex.conditionPoints.isEmpty());
                                previousCondition = currentVertex.conditionPoints.first().condition;
                            }
                            currentVertex.conditionPoints.insert(k, currentConditionPoint);
                            //список точек (и номер 1й точки после ..) вырос на 1, учтём это
                            originPointNumber = k + 1;
                            //точка смены условия, прерд которой вставляем
                            //начало донорского отрезка, нашлась
                            conditionPointWasFound = true;
                            break;
                        }
                    }
                    /* в этом случае начальная точка донорского отрезка
                     * находится правее всех точек смены условия. Добавляем
                     * её в конец, не забывая сохранить условие. */
                    if (!conditionPointWasFound) {
                        Q_ASSERT(!currentVertex.conditionPoints.isEmpty());
                        previousCondition = currentVertex.conditionPoints.last().condition;
                        currentVertex.conditionPoints.append(currentConditionPoint);
                        /* задаём значение на 1 больше номера последнего
                         * элемента списка. По формальной логике так и должно
                         * быть. В цикл удаления точек смены условия не попадём. */
                        originPointNumber = currentVertex.conditionPoints.size();
                    }


                    //текущая точка теперь - конец преобразованного донорского отрезка
                    currentConditionPoint.distance = distanceToEnd;
                    //номер точки, до которой включительно нужно заменять условие
                    int endPointNumber;
                    /* если конечная точка донорского отрезка меньше длины полигона,
                     * то добавляем её: */
                    if (!qFuzzyCompare(distanceToEnd, sideLength)) {
                        // остались ли ещё точки смены условия, которые нужно проверять
                        if (originPointNumber < currentVertex.conditionPoints.size()) {
                            conditionPointWasFound = false;
                            /* для всех точек смены условия нужно найти ту из
                             * них, после которой вставим новую точку смены
                             * условия (конечную точку донорского отрезка) */
                            for (int k = originPointNumber; k < currentVertex.conditionPoints.size(); ++k) {
                                /* если конец донорского отрезка совпадает с
                                 * текущей точкой смены условия (по расстоянию),
                                 * ничего не делаем */
                                if (qFuzzyCompare(distanceToEnd,  currentVertex.conditionPoints.at(k).distance)) {
                                    //удаляться будут дочки до текущей, не включая её
                                    endPointNumber = k - 1;
                                    conditionPointWasFound = true;
                                    break;
                                    /* если точка конца донорского отрезка
                                     * лежит левее текущей точки смены условия,
                                     * вставляем перед последней первую */
                                } else if (distanceToEnd < currentVertex.conditionPoints.at(k).distance) {
                                    /* вот зачем нам нужно был предыдущее
                                     * условие: если в донорском отрезке не
                                     * содержится ни одной точки смены условия,
                                     * информаця о условии, которое было
                                     * изначально на месте точки конца отрезка,
                                     * без этого параметра потеряласть бы */
                                    if (k == originPointNumber) {
                                        currentConditionPoint.condition = previousCondition;
                                    } else {
                                        currentConditionPoint.condition = currentVertex.conditionPoints.at(k - 1).condition;
                                    }
                                    currentVertex.conditionPoints.insert(k, currentConditionPoint);
                                    /* удаляться будут дочки до новой, не
                                     * включая её. Если в донорском отрезке
                                     * не содержится ни одной точки смены
                                     * условия, мы не сможем войти в цикл
                                     * удаления точек смены условия */
                                    endPointNumber = k - 1;
                                    conditionPointWasFound = true;
                                    break;
                                }
                            }
                            /* в этом случае конечная точка донорского
                             * отрезка находится правее всех точек смены
                             * условия. При этом гарантированно в донорском
                             * отрезке содержится хотя бы одна точка смены
                             * условия, иначе мы не попали бы во внешний if */
                            if (!conditionPointWasFound) {
                                Q_ASSERT(!currentVertex.conditionPoints.isEmpty());
                                currentConditionPoint.condition = currentVertex.conditionPoints.last().condition;
                                /* удаляться будут точки до конечной точки донорского отрезка,
                                 * исключая её. Нужно бы посчитать номер последней из них
                                 * до того, как выполнится append(), иначе пришлось бы
                                 * вычитать 2 */
                                endPointNumber = currentVertex.conditionPoints.size() - 1;
                                currentVertex.conditionPoints.append(currentConditionPoint);
                            }
                            /* начальная и конечная точки донорского отрезка лежат между FIXME: что тут написанно?
                             * последней смены условия и длиной отрезка. При этом, с учётом условия
                             * внешнего if'а, точка донорского отрезка меньше длины полигона.
                             * Добавляем точку конца донорского отрезка в конец списка,
                             * здесь нам снова нужно предыдущее условие. */
                        } else {
                            currentConditionPoint.condition = previousCondition;
                            // формально нужно вычесть 2, хотя это ничего не меняет.
                            endPointNumber = currentVertex.conditionPoints.size() - 2;
                            currentVertex.conditionPoints.append(currentConditionPoint);
                        }
                        /* Конечная точка донорского отрезка равна длине полигона.
                         * Ничего не делаем */
                    } else {
                        /* будут удалены все точки смены условия, лежащие
                         * после начальной точки донорского отрезка */
                        endPointNumber = currentVertex.conditionPoints.size() - 1;
                    }

                    /* теперь мы знаем, какие точки перекрываются донорским отрезком,
                     * не забываем удалять их */
                    while (originPointNumber <= endPointNumber) {
                        currentVertex.conditionPoints.removeAt(originPointNumber);
                        --endPointNumber;
                    }
                    result.replace(i, currentVertex);
                }
            }
        }
    }

    if (prepareCornersAndSimplify) {
        simplifyConditions(result);
    }

    return result;
}

QList<Vertex> BoundaryPolygonCalc::setCondition(const QPolygonF &polygon, const BoundaryCondition *condition)
{
    Q_ASSERT(polygon.size() >= 3);
    Q_ASSERT(polygon.isClosed());

    QList<Vertex> result;

    foreach(QPointF point, polygon) {
        Vertex newVertex;
        newVertex.point = point;
        result.append(newVertex);
    }
    // Добавляем точку смены условия в начало полигона.
    ConditionPoint onlyConditionPoint;
    onlyConditionPoint.distance = 0;
    onlyConditionPoint.condition = condition;
    Q_ASSERT(!result.isEmpty());
    result.first().conditionPoints.append(onlyConditionPoint);

    return result;
}

QPointF BoundaryPolygonCalc::pointOnSegment(const QPointF &point_1,
        const QPointF &point_2,
        const qreal distance)
{
    Q_ASSERT(distance >= 0);
    Q_ASSERT(QLineF(point_1, point_2).length() > distance);
    QPointF result;
    qreal length = QLineF(point_1 , point_2).length();

    if (qFuzzyIsNull(distance) || qFuzzyIsNull(length)) {
        result = point_1;
        return result;
    } else if (qFuzzyCompare(distance, length)) {
        return point_2;
    }

    qreal k = distance / length;
    result.setX(point_1.x() + (point_2.x() - point_1.x())*k);
    result.setY(point_1.y() + (point_2.y() - point_1.y())*k);
    return result;
}


QPointF BoundaryPolygonCalc::pointOnSegment(const QLineF &segment, const qreal distance)
{
    return pointOnSegment(segment.p1(), segment.p2(), distance);
}

QPointF BoundaryPolygonCalc::pointByXOnSegment(const qreal x, const QLineF &segment)
{
    QPointF result;
    result.setX(x);
    result.setY(segment.y1() + (x - segment.x1()) * segment.dy() / segment.dx());

    qreal resultDistance;
    resultDistance = signedDistanceFromOrigin(result, segment);
    if (!(resultDistance > 0 && resultDistance < segment.length())) {
        result = QFrost::noPoint;
    }
    return result;
}

QPointF BoundaryPolygonCalc::pointByYOnSegment(const qreal y, const QLineF &segment)
{
    QPointF inversedPoint;
    inversedPoint = pointByXOnSegment(y, QLineF(segment.y1(), segment.x1(),
                                      segment.y2(), segment.x2()));
    return QPointF(inversedPoint.y(), inversedPoint.x());
}

qreal BoundaryPolygonCalc::signedDistanceFromOrigin(const QPointF &p, const QLineF &segment)
{
    // Если возникнет необходимость, можно убрать.
    Q_ASSERT(!qFuzzyIsNull(segment.length()));

    if (qFuzzyCompare(p, segment.p1())) {
        return 0;
    }

    QLineF lineToPoint;
    lineToPoint = QLineF(segment.p1(), p);
    qreal result;
    result = lineToPoint.length();

    //точка лежит левее начала отрезка
    if (dotProduct(segment, lineToPoint) < 0) {
        result *= -1;
    }

    if (qFuzzyCompare(result, segment.length())) {
        result = segment.length();
    }

    return result;
}

qreal BoundaryPolygonCalc::signedProjectionDistance(const QPointF &p, const QLineF &segment)
{
    // Если возникнет нкобходимость, можно убрать.
    Q_ASSERT(!qFuzzyIsNull(segment.length()));

    if (qFuzzyCompare(p, segment.p1())) {
        return 0;
    }

    qreal result;
    result =  dotProduct(segment, QLineF(segment.p1(), p))
              / segment.length();

    if (qFuzzyCompare(result, segment.length())) {
        result = segment.length();
    }

    return result;
}

QPolygonF BoundaryPolygonCalc::unitRoundPolygon(uint numberOfAngles)
{
    Q_ASSERT(360 % numberOfAngles == 0);
    // Укладывается ли шаг целое число раз в 90 градусов.
    Q_ASSERT(numberOfAngles % 4 == 0);
    
    QPolygonF result;
    
    //Заполняем превую четверть окружности.
    result << QPointF(1, 0); //для сохранения точности.
    for (uint i = 1; i < numberOfAngles / 4; ++i) {
        const double angle = double(i) * (2.0 * boost::math::constants::pi<double>() / double(numberOfAngles));
        result << QPointF(qCos(angle), qSin(angle));
    }
    
    //Заполняем вторую четверть окружности.
    result << QPointF(0, 1); //для сохранения точности.
    for (uint i = 1; i < numberOfAngles / 4 + 1; ++i) {
        QPointF currentPoint;
        currentPoint = result.at(numberOfAngles / 4 - i);
        result << QPointF(-currentPoint.x(), currentPoint.y());
    }
    
    //Заполняем вторую половину окружности.
    for (uint i = 1; i < numberOfAngles / 2; ++i) {
        QPointF currentPoint;
        currentPoint = result.at(numberOfAngles / 2 - i);
        result << QPointF(currentPoint.x(), -currentPoint.y());
    }
    
    return result;
}

QPolygonF BoundaryPolygonCalc::ellipseShapedPolygon(const QRectF &ellipse, uint numberOfAngles)
{
    /// Полуширина эллипса
    qreal a;
    /// Полувысота эллипса
    qreal b;
    /// Координаты середины эллипса
    QPointF shiftPoint;
    
    a = ellipse.width() / 2;
    b = ellipse.height() / 2;
    shiftPoint = ellipse.center();
    
    QPolygonF result;
    foreach(const QPointF &point, unitRoundPolygon(numberOfAngles)) {
        result.append(QPointF(point.x() * a, point.y() * b) + shiftPoint);
    }
    //Замыкаем полигон
    result.append(result.first());
    return result;
}

bool BoundaryPolygonCalc::polygonIsNotSelfIntersecting(const QPolygonF &polygon)
{
    if (polygon.size() < 3) {
        return false;
    }

    QPolygonF changingPolygon;
    changingPolygon = polygon;

    if (!changingPolygon.isClosed()) {
        changingPolygon.append(changingPolygon.first());
    }

    QPointF intersectionPoint ;
    int size = changingPolygon.size();
    // Проверяем для каждого отрезка все, следующие за смежным к нему.
    for (int i = 0; i < size - 3; ++i) {
        QLineF line_1(changingPolygon.at(i), changingPolygon.at(i + 1));
        for (int j = i + 2; j < size - 1; ++j) {
            QLineF line_2(changingPolygon.at(j), changingPolygon.at(j + 1));
            if (line_1.intersect(line_2, &intersectionPoint)
                    == QLineF::BoundedIntersection) {
                // Нельзя проверять пару (первый отрезок, последний отрезок)
                if (!(i == 0 && j == size - 2)) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool BoundaryPolygonCalc::belongToOneLine(const QLineF &line1, const QLineF &line2)
{
    //разность начальных точек прямых
    QLineF firstPointsSegment;
    firstPointsSegment = QLineF(line1.p1(), line2.p1());
    QLineF lastPointsSegment;
    lastPointsSegment = QLineF(line1.p2(), line2.p2());

    //НДУ: Вектора, образованные на каждом из трёх отрезков коллинеарны.
    /* HACK: используем избыточное количество условий для большей стабильности */
    return qFuzzyCompare(line1.dx() * line2.dy(),
                         line2.dx() * line1.dy())
           && (qFuzzyCompare(line1.dx() * firstPointsSegment.dy(),
                             firstPointsSegment.dx() * line1.dy())
               || qFuzzyCompare(line1.dx() * lastPointsSegment.dy(),
                                lastPointsSegment.dx() * line1.dy()));
}

void BoundaryPolygonCalc::simplifyConditions(QList<Vertex> &corners)
{
    bool firstConditionPointWasFound;
    ConditionPoint currentConditionPoint;
    Vertex currentVertex;
    const BoundaryCondition *previousCondition = NULL;

    int firstI = 0;
    int firstJ = 0;
    int lastI = 0;
    int lastJ = 0;

    firstConditionPointWasFound = false;
    //для всех углов полигона
    for (int i = 0; i < corners.size() - 1; ++i) {
        currentVertex = corners.at(i);
        //для всех точек смены условия каждого угла
        for (int j = 0; j < corners.at(i).conditionPoints.size(); ++j) {
            currentConditionPoint = currentVertex.conditionPoints.at(j);
            if (firstConditionPointWasFound) {
                //обработка второй и следующих точек смены условия

                if (currentConditionPoint.condition == previousCondition) {
                    // если условие в ней не изменилось, удаляем
                    currentVertex.conditionPoints.removeAt(j);
                    corners.replace(i, currentVertex);
                    // учитываем, что размер списка точек уменьшился на 1 после удаления
                    --j;
                } else {
                    /* если условие в ней изменилось, дальнейший поиск ведём
                     * относительно её условия; делаем эту точку последней */
                    previousCondition = currentConditionPoint.condition;
                    lastI = i;
                    lastJ = j;
                }
            } else {
                // обработка первой точки смены условия
                previousCondition = currentConditionPoint.condition;
                firstConditionPointWasFound = true;
                firstI = i;
                firstJ = j;
                lastI = i;
                lastJ = j;
            }
        }
    }
    Q_ASSERT(previousCondition != NULL);
    if (firstConditionPointWasFound
            && !(lastI == firstI && lastJ == firstJ)
            && (corners.at(firstI).conditionPoints.at(firstJ).condition
                == corners.at(lastI).conditionPoints.at(lastJ).condition)) {
        //удаляем первую точку смены условия, если последняя имеет то же условие.
        currentVertex = corners.at(firstI);
        currentVertex.conditionPoints.removeAt(firstJ);
        corners.replace(firstI, currentVertex);
    }
}

QList<Vertex> BoundaryPolygonCalc::addZeroDistanceConditionPoints(const QList<Vertex> &corners)
{
    Q_ASSERT(!corners.isEmpty());
    Q_ASSERT(corners.first().point ==  corners.last().point);
    Q_ASSERT(corners.last().conditionPoints.isEmpty());

    /// Условие первой из найденных точек смены условия.
    const BoundaryCondition *firstCondition = NULL;
    /// Индекс угла, в котором найдена первая точка смены условия.
    int firstI = -1;

    QList<Vertex> result;
    result = corners;

    // Ищем угол, в котором находится первая точка смены условия.
    // Последний угол всегда пустой.
    for (int i = 0; i < result.size() - 1; ++i) {
        if (!result.at(i).conditionPoints.isEmpty()) {
            firstCondition = result.at(i).conditionPoints.last().condition;
            firstI = i;
            break;
        }
    }
    Q_ASSERT(firstI >= 0);
    Q_ASSERT(firstCondition != NULL);

    ConditionPoint zeroConditionPoint;
    zeroConditionPoint = ConditionPoint(0, firstCondition);

    /* Добавляем, если это требуется, нулевые точки смены условия во все углы,
     * начиная со следующего после того, в котором нашлась первая точка смены
     * условия. */
    for (int i = firstI + 1; i < result.size() - 1; ++i) {
        if (result.at(i).conditionPoints.isEmpty()) {
            // В текущем углу нет точек смены условия.
            // Добавляем точку смены условия с нулевым расстоянием.
            result[i].conditionPoints.append(zeroConditionPoint);
        } else {
            // В текущем углу есть точка смены условия.
            Q_ASSERT(result.at(i).conditionPoints.first().distance >= 0);
            if (result.at(i).conditionPoints.first().distance != 0) { //sic
                // Добавляем точку смены условия с нулевым расстоянием.
                result[i].conditionPoints.prepend(zeroConditionPoint);
            }
            // Запоминаем условие последней точки смены условия.
            zeroConditionPoint.condition = result.at(i).conditionPoints.last().condition;
        }
    }

    /* Добавляем нулевые точки смены условия во все углы, начиная с нулевого и
     * заканчивая тем, в котором нашлась первая точка смены условия. */
    for (int i = 0; i < firstI; ++i) {
        result[i].conditionPoints.append(zeroConditionPoint);
    }

    /* При необходимости добавляем нулевую точку смены условия в угол с первой
     * точкой смены условия. */
    Q_ASSERT(!result.at(firstI).conditionPoints.isEmpty());
    if (result.at(firstI).conditionPoints.first().distance != 0) { //sic
        // Добавляем точку смены условия с нулевым расстоянием.
        result[firstI].conditionPoints.prepend(zeroConditionPoint);
    }

    return result;
}

bool BoundaryPolygonCalc::zeroDistanceConditionPointAbsent(const QList<Vertex> &corners)
{
    for (int i = 0; i < corners.size() - 1; ++i) {
        Vertex corner;
        corner = corners.at(i);
        if (corner.conditionPoints.isEmpty()
                || corner.conditionPoints.first().distance != 0) {
            return true;
        }
    }
    return false;
}

QPair<BoundaryPolygonList, BoundaryPolygonList> BoundaryPolygonCalc::uniteOperation(const QPolygonF &modifyingPolygon,
        bool mustChangeInnerPolygons)
{
    Q_ASSERT(polygonIsSimple(modifyingPolygon));
    Q_ASSERT(modifyingPolygon.isClosed());

    /// Внешние полигоны, которые нужно добавить в сцену
    BoundaryPolygonList addedOuterPolygons;
    /// Внешние полигоны, которые нужно изъять из сцены
    BoundaryPolygonList removedOuterPolygons;

    // полигоны не могут пересекаться только в изолированных точках
    BoundaryPolygonList outerPolygons;
    outerPolygons = outerBoundaryPolygons(modifyingPolygon, QFrost::excludingOnlyPoints);

    /**
     * Внешний полигон, который получится в результате объединения внешних
     * полигонов с модифицирущем, если они пересекаются
     */
    QPolygonF unitedPolygon;
    unitedPolygon = modifyingPolygon;

    /**
     * Список полигонов, родителем которых станет объединённый
     * внешний полигон, который нужно будет добавить в сцену
     */
    BoundaryPolygonList addedInnerPolygons;

    /// Условия внешних и "вычтенных" полигонов
    QList<BoundaryPolyline> outerAndSubtractedPolylines;
    outerAndSubtractedPolylines = initialAndSubtractedBoundaryPolylines(outerPolygons,
                                  modifyingPolygon);

    bool modifedPolygonIsInner = false;
    foreach(BoundaryPolygon * outerPolygon, outerPolygons) {
        // для всех найденных внешних полигонов граничных условий
        if (outerPolygon->contains(modifyingPolygon)) {
            BoundaryPolygon *newOuterPolygon;
            newOuterPolygon = newPolygonWithSubtractedInnerPolygons(outerPolygon,
                              modifyingPolygon,
                              mustChangeInnerPolygons);
            if (newOuterPolygon != NULL) {
                addedOuterPolygons << newOuterPolygon;
                removedOuterPolygons << outerPolygon;
            }
            modifedPolygonIsInner = true;
            /* модифцирующий полигон может лежать внутри только
              * одного внешнего полигона, так что выходим из цикла. */
            break;
        } else {
            /* Если полигон граничных условий пересекается с
              * модифицирующим (но только не в точках),
              * но последний не содержится в первом, объединяем их
              * и запоминаем объединение в объединённом полигоне.
              * Получившиеся при объединении новые внутренние полигоны делаем
              * полигонами граничных условий и кладём их в контейнер, элементы
              * которого потом станут дочерними по отношению к суммарному
              * объединению. Внутренние полигоны пересекаем с
              * модифицирующим и кладём в новый общий полигон
              * граничных условий. (Условие пересечения учтено выше) */
            Q_ASSERT(outerPolygon->intersectsExcludingOnlyPoints(modifyingPolygon));

            /**
              * Первый член списка -- внешний полигон, получившийся при
              * объединении, последующие -- внутренние.
              */
            QList<QPolygonF> outerAndListOfInnerPolygons;
            outerAndListOfInnerPolygons = united(unitedPolygon, outerPolygon->polygon());
            outerAndListOfInnerPolygons = sortOuterAndInnerPolygons(outerAndListOfInnerPolygons);

            Q_ASSERT(!outerAndListOfInnerPolygons.isEmpty());
            // наращиваем объединённый полигон
            unitedPolygon = outerAndListOfInnerPolygons.first();

            for (int i = 1 /* sic! */; i < outerAndListOfInnerPolygons.size(); ++i) {
                // создаём новые внутренние полигоны и кладём их в список
                BoundaryPolygon *newInnerPolygon = new BoundaryPolygon(conditionsFromPolylines(outerAndListOfInnerPolygons.at(i),
                        outerAndSubtractedPolylines));
                addedInnerPolygons.append(newInnerPolygon);
            }

            foreach(BoundaryPolygon * innerPolygon, outerPolygon->childBoundaryPolygonItems()) {
                if (mustChangeInnerPolygons && innerPolygon->intersectsExcludingBorder(modifyingPolygon)) {
                    /* если текущий внутренний полигон пересекается с
                      * модифицирующим, обрезаем его и кладём в список. */

                    /// список полилиний условий текущего внутреннего полигона
                    QList<BoundaryPolyline> innerPolylines;
                    innerPolylines = innerPolygon->boundaryPolylines();
                    /**
                      * Результат пересечения текущего внутреннего
                      * полигона с модифицирующим
                      */
                    QList<QPolygonF> intrersectionPolygons;
                    intrersectionPolygons = intersected(innerPolygon->polygon(), modifyingPolygon);
                    foreach(QPolygonF intrersectionPolygon, intrersectionPolygons) {
                        //добавляем дополнительные полилинии
                        tryExtraConditionPolylines(intrersectionPolygon, innerPolylines);
                    }
                    /**
                      * Cписок новых внутренних полигонов, полученных из
                      * исходного вычитанием модифицирующего
                      */
                    QList<QPolygonF> subtractedInnerPolygons;
                    subtractedInnerPolygons = subtracted(innerPolygon->polygon(), modifyingPolygon);
                    foreach(QPolygonF currentPolygon, subtractedInnerPolygons) {
                        /* Создаём новые полигоны граничных условий и
                          * кладём их в список. */
                        BoundaryPolygon *newInnerPolygon = new BoundaryPolygon(conditionsFromPolylines(currentPolygon, innerPolylines));
                        addedInnerPolygons << newInnerPolygon;
                    }
                } else {
                    /* Создаём копию внутреннего полигона граничных условий и
                      * кладём её в список. */
                    BoundaryPolygon *newInnerPolygon = new BoundaryPolygon(innerPolygon);
                    addedInnerPolygons << newInnerPolygon;
                }
            }
            // Кладём текущий внешний полигон в список удаляемых из сцены.
            removedOuterPolygons << outerPolygon;
        }
    }
    if (!modifedPolygonIsInner) {
        BoundaryPolygon *newOuterPolygon = new BoundaryPolygon(conditionsFromPolylines(unitedPolygon, outerAndSubtractedPolylines));
        addedOuterPolygons << newOuterPolygon;

        foreach(BoundaryPolygon * innerPolygon, addedInnerPolygons) {
            innerPolygon->setParentItem(newOuterPolygon);
        }
    }
    return QPair<BoundaryPolygonList, BoundaryPolygonList>(addedOuterPolygons, removedOuterPolygons);
}

BoundaryPolygonList BoundaryPolygonCalc::outerBoundaryPolygons(const QPolygonF &polygon,
        const QFrost::IntersectionType &intersectionType) const
{
    BoundaryPolygonList boundaryPolygons;
    boundaryPolygons = mScene->boundaryPolygons(polygon, intersectionType);

    BoundaryPolygonList result;
    foreach(BoundaryPolygon * boundaryPolygon, boundaryPolygons) {
        if (boundaryPolygon->parentItem() == NULL) {
            // полигон граничных условий является внешним
            result << boundaryPolygon;
        }
    }
    return result;
}

QList<BoundaryPolyline> BoundaryPolygonCalc::initialAndSubtractedBoundaryPolylines(const qfgui::BoundaryPolygonList polygons,
        const QPolygonF &modifyingPolygon)
{
    QList<BoundaryPolyline> polylines;
    QList<QPolygonF> subtractedPolygons;
    subtractedPolygons << modifyingPolygon;

    foreach(BoundaryPolygon * polygon, polygons) {
        polylines << polygon->boundaryPolylines();
        changeToSubtracted(subtractedPolygons, polygon->polygon());
    }

    foreach(QPolygonF subtractedPolygon, subtractedPolygons) {
        tryExtraConditionPolylines(subtractedPolygon, polylines);
    }

    return polylines;
}

BoundaryPolygon *BoundaryPolygonCalc::newPolygonWithSubtractedInnerPolygons(BoundaryPolygon *outerPolygon,
        const QPolygonF &polygon,
        bool mustChangeInnerPolygons)
{
    /* пытаемся вычесть polygon из каждого из внутренних
     * полигонов; если получилось - придаём условия новым полигонам,
     * копируем родителя, кладём их в эту копию и возвращаем её */

    /// Список изменённых внутренних полигонов для добавления в нового родителя
    BoundaryPolygonList addedInnerPolygons;

    /// Список старых внутренних полигонов, не пересекающихся с модифицирующим
    BoundaryPolygonList nonIntersectingInnerPolygons;

    /// Пересекается ли хотя бы один внутренний полигон с модифицирующим.
    bool intersectionWasFound;
    intersectionWasFound = false ;

    foreach(BoundaryPolygon * innerPolygon, outerPolygon->childBoundaryPolygonItems()) {
        //Проверяем каждый внутренний полигон
        if (mustChangeInnerPolygons && innerPolygon->intersectsExcludingBorder(polygon)) {
            /* Если текущий внутренний полигон пересекается (с
             * образованием фигур) с модифицирующим, обрезаем его,
             * придаём ему условия и добавляем в список добавляемых
             * внутренних полигонов. */
            intersectionWasFound = true;
            if (innerPolygon->contains(polygon)) {
                /* Если модифцирующий полигон содержится в
                 * каком-либо из внутренних, то он не валиден. */
                return NULL;
            }
            QList<BoundaryPolyline> innerPolylines;
            innerPolylines = innerPolygon->boundaryPolylines();
            /**
             * Результат пересечения текущего внутреннего
             * полигона с модифицирующим
             */
            QList<QPolygonF> intrersectionPolygons;
            intrersectionPolygons = intersected(innerPolygon->polygon(), polygon);
            foreach(QPolygonF intrersectionPolygon, intrersectionPolygons) {
                //добавляем дополнительные полилинии
                tryExtraConditionPolylines(intrersectionPolygon, innerPolylines);
            }
            /**
             * Список новых внутренних полигонов, полученных из
             * исходного вычитанием модифицирующего
             */
            QList<QPolygonF> newInnerPolygons;
            newInnerPolygons = subtracted(innerPolygon->polygon(), polygon);
            foreach(QPolygonF newInnerPolygon, newInnerPolygons) {
                BoundaryPolygon *newInnerBoundaryPolygon;
                // Создаём новый внутренний полигон и кладём его в список
                newInnerBoundaryPolygon = new BoundaryPolygon(conditionsFromPolylines(newInnerPolygon, innerPolylines));
                addedInnerPolygons.append(newInnerBoundaryPolygon);
            }
        } else {
            /* Если текущий внутренний полигон не пересекается
             * с модифицирующим (или нам не надо их трогать),
             * добавляем его в соответствующий список */
            nonIntersectingInnerPolygons.append(innerPolygon);
        }
    }
    if (intersectionWasFound) {
        /* Если нашлись внутренние полигоны, пересекающиеся с
         * модифицирующим, заполняем списки добавляемых в
         * сцену и удаляемых из неё полигонов */

        /// Новый внешний полигон
        BoundaryPolygon *newOuterPolygon;
        newOuterPolygon = new BoundaryPolygon(outerPolygon);

        // Копируем все не изменённые внутреннние полигоны.
        foreach(BoundaryPolygon * polygon, nonIntersectingInnerPolygons) {
            BoundaryPolygon *newInnerPolygon;
            newInnerPolygon = new BoundaryPolygon(polygon);
            newInnerPolygon->setParentItem(newOuterPolygon);
        }

        // Делаем все новые внутренние полигоны детьми нового внешнего.
        foreach(BoundaryPolygon * polygon, addedInnerPolygons) {
            polygon->setParentItem(newOuterPolygon);
        }

        return newOuterPolygon;
    }

    return NULL;
}

QPair<BoundaryPolygonList, BoundaryPolygonList> BoundaryPolygonCalc::subtractOperation(const QPolygonF &modifyingPolygon)
{
    Q_ASSERT(polygonIsSimple(modifyingPolygon));
    Q_ASSERT(modifyingPolygon.isClosed());

    /// Возвращаемые внешние полигоны, которые нужно добавить в сцену
    BoundaryPolygonList addedOuterPolygons;
    /// Возвращаемые внешние полигоны, которые нужно изъять из сцены
    BoundaryPolygonList removedOuterPolygons;

    BoundaryPolygonList boundaryPolygons;
    // полигоны не могут пересекаться только в изолированных точках
    boundaryPolygons = mScene->boundaryPolygons(modifyingPolygon, QFrost::excludingBorder);

    foreach(BoundaryPolygon * boundaryPolygon, boundaryPolygons) {
        if (boundaryPolygon->parentItem() == NULL) {
            //полигон граничных условий является внешним
            if (boundaryPolygon->contains(modifyingPolygon)) {
                BoundaryPolygon *newOuterPolygon;
                newOuterPolygon = newPolygonWithUnitedInnerPolygons(boundaryPolygon, modifyingPolygon);
                if (newOuterPolygon != NULL) {
                    addedOuterPolygons << newOuterPolygon;
                    removedOuterPolygons << boundaryPolygon;
                }
                /* модифцирующий полигон может лежать внутри только
                 * одного внешнего полигона, так что выходим из цикла. */
                break;

            } else {    //if(boundaryPolygon->intersectsExcludingBorder(modifyingPolygon)) {
                /* если внешний полигон граничных условий пересекается с
                * модифицирующим, (с образованием фигур) но не содержит
                * его, вычитаем из полигона граничных
                * условий модифцирующий. (условие пересечения учтено выше).
                */

                /**
                 * Список новых внешних полигонов, полученных в результате
                 * вычитания.
                 */
                QList<QPolygonF> newOuterPolygons;
                newOuterPolygons = subtracted(boundaryPolygon->polygon(), modifyingPolygon);
                /**
                 * Список полилиний полигонов, пересекающихся с модифицирующим:
                 * внешнего и пересекающихся внутренних
                 */
                QList<BoundaryPolyline> polylines;
                polylines.append(boundaryPolygon->boundaryPolylines());

                /// Список внутренних полигонов, которые вычитаются из новых внешних.
                QList<QPolygonF> polygonsToSubtract;

                /**
                 * Cписок внутренних полигонов граничных условий,
                 * не пересекающихся с модифицирующим.
                 */
                BoundaryPolygonList nonIntersectingInnerPolygons;

                /**
                 * Список полигонов, получающийся последовательным вычитанием из
                 * модифицирующего полигона каждого внутреннего полигона, который
                 * пересекается с ним.
                 * Из этих полигонов добудем дополнительные полилинии
                 */
                QList<QPolygonF> subtractedPolygons;
                subtractedPolygons.append(intersected(modifyingPolygon, boundaryPolygon->polygon()));

                foreach(BoundaryPolygon * innerPolygon, boundaryPolygon->childBoundaryPolygonItems()) {
                    // Для всех внутренних полигонов.
                    if (innerPolygon->intersects(modifyingPolygon)) {
                        // Внутренний полигон пересекается с модифицирующи.

                        // добавляем текущий внутренний полигон в список
                        polygonsToSubtract.append(innerPolygon->polygon());
                        // вычитаем текущий внутренний полигон из списка
                        changeToSubtracted(subtractedPolygons, innerPolygon->polygon());
                        //сохраняем информацию о условиях
                        polylines.append(innerPolygon->boundaryPolylines());
                    } else {
                        /* Если внутренний полигон не пересекается с
                         * модифицирующим, то добавляем его в соотв. список */
                        nonIntersectingInnerPolygons.append(innerPolygon);
                    }
                }
                foreach(QPolygonF subtractedPolygon, subtractedPolygons) {
                    // добавляем дополнительные полилинии
                    tryExtraConditionPolylines(subtractedPolygon, polylines);
                }

                // список новых внешние полигоны граничных условий
                BoundaryPolygonList newOuterBoundaryPolygons;
                foreach(QPolygonF newOuterPolygon, newOuterPolygons) {
                    /*  вычитаем из новых внешних полигонов пересекающиеся с
                     * модифицирующим внутренние */
                    foreach(QPolygonF polygonToSubtract, polygonsToSubtract) {
                        newOuterPolygon = newOuterPolygon.subtracted(polygonToSubtract);
                    }
                    // создаём новые внешние полигоны граничных условий.
                    BoundaryPolygon *newOuterBoundaryPolygon = new BoundaryPolygon(conditionsFromPolylines(newOuterPolygon, polylines));
                    // кладём новые внешние полигоны граничных условий в список
                    newOuterBoundaryPolygons.append(newOuterBoundaryPolygon);
                }

                foreach(BoundaryPolygon * nonIntersectingInnerPolygon, nonIntersectingInnerPolygons) {
                    // Кладём неперескающиеся внутренние полигоны в новые внешние
                    foreach(BoundaryPolygon * newOuterBoundaryPolygon, newOuterBoundaryPolygons) {
                        if (newOuterBoundaryPolygon->contains(nonIntersectingInnerPolygon->polygon())) {
                            /* если внутренний полигон содержится в текущем
                             * новом внешнем, то кладём его туда */
                            nonIntersectingInnerPolygon->setParentItem(newOuterBoundaryPolygon);
                            break;
                        }
                    }
                }
                addedOuterPolygons << newOuterBoundaryPolygons;
                removedOuterPolygons << boundaryPolygon;
            }
        }
    }
    return QPair<BoundaryPolygonList, BoundaryPolygonList>(addedOuterPolygons, removedOuterPolygons);
}

BoundaryPolygon *BoundaryPolygonCalc::newPolygonWithUnitedInnerPolygons(BoundaryPolygon *outerPolygon, const QPolygonF &modifyingPolygon)
{
    /// новый объединённый внутренний полигон
    QPolygonF unitedInnerPolygon;
    /* объединённый внутренний полигон так и останется равным
     * модифицирующему, если не найдётся внутренних полигонов,
     * пересекающихся с последним */
    unitedInnerPolygon = modifyingPolygon;

    /// Список старых внутренних полигонов, не пересекающихся с модифицирующим
    BoundaryPolygonList nonIntersectingInnerPolygons;

    /**
     * Полигоны, которые будут объединяться с модифицирующим
     * для образования нового объединённого полигона граничных условий
     */
    QList<QPolygonF> polygonsToUnion;

    /**
     * Список полилиний условий, которыми будем придавать усвлоия
     * новым внутренним полигонам
     */
    QList<BoundaryPolyline> innerPolygonsPolylines;

    /**
     * Список полигонов, получающийся последовательным вычитанием из
     * модифицирующего полигона каждого внутреннего полигона, который
     * пересекается с ним.
     * Из этих полигонов добудем дополнительные полилинии
     */
    QList<QPolygonF> subtractedPolygons;
    subtractedPolygons.append(modifyingPolygon);
    foreach(BoundaryPolygon * innerPolygon, outerPolygon->childBoundaryPolygonItems()) {
        /* для всех внутренних полигонов ищем пересекающиеся
         * (но только не в точках) с текущим модифицирующим */
        if (innerPolygon->intersectsExcludingOnlyPoints(modifyingPolygon)) {
            /* если внутренний полигон пересекается с
             * модифицирующим, запоминаем его параметры */
            if (innerPolygon->contains(modifyingPolygon)) {
                /*  Если модифцирующий полигон содержится в
                 * каком-либо из внутренних, то он не валиден. */
                return NULL;
            }
            // вычитаем текущий внутренний полигон из списка
            changeToSubtracted(subtractedPolygons, innerPolygon->polygon());
            //добавляем информацию о форме
            polygonsToUnion.append(innerPolygon->polygon());
            //добавляем информацию о условиях
            innerPolygonsPolylines.append(innerPolygon->boundaryPolylines());
        } else {
            nonIntersectingInnerPolygons.append(innerPolygon);
        }
    }
    if (!polygonsToUnion.isEmpty()) {
        /* если существуют внутренние полигоны, пересекающиеся с
         * объединённым, подправляем форму последнего */
        QList<QPolygonF> modifedPolygonCondidates;
        modifedPolygonCondidates = united(modifyingPolygon, polygonsToUnion);
        /* Из всех кондидатов нам нужен внешний (тот, в котором
         * лежат все остальные). */
        unitedInnerPolygon = getOuterPolygon(modifedPolygonCondidates);
    }
    foreach(QPolygonF subtractedPolygon, subtractedPolygons) {
        // Добавляем дополнительную информацию о условиях
        tryExtraConditionPolylines(subtractedPolygon, innerPolygonsPolylines);
    }

    // Копируем новый внешний полигон
    BoundaryPolygon *newOuterPolygon = new BoundaryPolygon(outerPolygon);

    // Делаем новый объединённый внутренний полигон ребёнком нового внешнего.
    BoundaryPolygon *newInnerUnitedPolygon = new BoundaryPolygon(conditionsFromPolylines(unitedInnerPolygon, innerPolygonsPolylines));
    newInnerUnitedPolygon->setParentItem(newOuterPolygon);

    // Копируем все не изменённые внутреннние полигоны.
    foreach(BoundaryPolygon * polygon, nonIntersectingInnerPolygons) {
        BoundaryPolygon *newInnerPolygon;
        newInnerPolygon = new BoundaryPolygon(polygon);
        newInnerPolygon->setParentItem(newOuterPolygon);
    }

    return newOuterPolygon;
}

QPolygonF BoundaryPolygonCalc::getOuterPolygon(const QList< QPolygonF> &polygons)
{
    if (polygons.size() == 1) {
        return polygons.first();
    }

    if (polygons.size() == 0) {
        QPolygonF voidPolygon;
        voidPolygon << QPointF(0, 0);
        return voidPolygon;
    }

    for (int i = 0; i < polygons.size(); ++i) {
        int j;
        i != polygons.size() - 1 ? j = i + 1 : j = 0;
        //TODO:посмотреть, не будет ли в них мусора при создании
        QPainterPath iPath;
        QPainterPath jPath;
        iPath.addPolygon(polygons.at(i));
        jPath.addPolygon(polygons.at(j));
        if (iPath.contains(jPath)) {
            return polygons.at(i);
        }
    }

    QPolygonF voidPolygon;
    voidPolygon << QPointF(0, 0);
    return voidPolygon;
}

QList<QPolygonF> BoundaryPolygonCalc::sortOuterAndInnerPolygons(QList<QPolygonF> &polygons)
{
    if (polygons.size() == 1) {
        return polygons;
    }

    QList<QPolygonF> result;
    if (polygons.size() == 0) {
        QPolygonF voidPolygon;
        voidPolygon << QPointF(0, 0);
        result.append(voidPolygon);
        return result;
    }

    int i;
    int j;
    bool outerWasFound = false;
    for (i = 0; i < polygons.size(); ++i) {
        i != polygons.size() - 1 ? j = i + 1 : j = 0;
        // TODO:посмотреть, не будет ли в них мусора при создании
        QPainterPath iPath;
        QPainterPath jPath;
        iPath.addPolygon(polygons.at(i));
        jPath.addPolygon(polygons.at(j));
        if (iPath.contains(jPath)) {
            outerWasFound = true;
            break;
        }
    }
    if (outerWasFound) {
        result.append(polygons.at(i));
        polygons.removeAt(i);
        foreach(QPolygonF currentPolygon, polygons) {
            result.append(currentPolygon);
        }
        return result;
    }

    QPolygonF voidPolygon;
    voidPolygon << QPointF(0, 0);
    result.append(voidPolygon);
    return result;
}

QList<QPolygonF> BoundaryPolygonCalc::united(const QPolygonF &polygon, const QPolygonF &unitingPolygon) const
{
    QPainterPath path;
    path.addPolygon(polygon);
    QPainterPath unitingPath;
    unitingPath.addPolygon(unitingPolygon);
    return united(path, unitingPath);
}

QList<QPolygonF> BoundaryPolygonCalc::united(const QPolygonF &polygon, const QList< QPolygonF > &unitingPolygons) const
{
    QPainterPath path;
    path.addPolygon(polygon);
    QPainterPath unitingPath;
    foreach(QPolygonF unitingPolygon, unitingPolygons) {
        unitingPath.addPolygon(unitingPolygon);
    }
    return united(path, unitingPath);
}

QList<QPolygonF> BoundaryPolygonCalc::united(const QList< QPolygonF > &polygons,
        const QList< QPolygonF > &unitingPolygons) const
{
    QPainterPath path;
    foreach(QPolygonF polygon, polygons) {
        path.addPolygon(polygon);
    }
    QPainterPath unitingPath;
    foreach(QPolygonF unitingPolygon, unitingPolygons) {
        unitingPath.addPolygon(unitingPolygon);
    }
    return united(path, unitingPath);
}

QList<QPolygonF> BoundaryPolygonCalc::united(const QPainterPath &path,
        const QPainterPath &unitingPath)
{
    QList<QPolygonF> result;
    result = path.united(unitingPath).simplified().toSubpathPolygons();

    // Избавляемся от противонаправленых смежных сторон.
    //result = removeZeroAndFlatAngles(result); FIXME FIXME FIXME ??? раскомментить?
    Q_ASSERT(!oppositeDirectionConsecutiveSidesWereFound(result));
    return result;
}

QList<QPolygonF> BoundaryPolygonCalc::subtracted(const QPolygonF &polygon,
        const QPolygonF &subtractingPolygon) const
{
    QPainterPath path;
    path.addPolygon(polygon);
    QPainterPath subtractingPath;
    subtractingPath.addPolygon(subtractingPolygon);

    QList<QPolygonF> result;
    result = path.subtracted(subtractingPath).simplified().toSubpathPolygons();

    // Избавляемся от противонаправленых смежных сторон.
    result = removeZeroAndFlatAngles(result);
    Q_ASSERT(!oppositeDirectionConsecutiveSidesWereFound(result));
    return result;
}

QList<QPolygonF> BoundaryPolygonCalc::intersected(const QPolygonF &polygon,
        const QPolygonF &subtractingPolygon) const
{
    QPainterPath path;
    path.addPolygon(polygon);
    QPainterPath intersectingPath;
    intersectingPath.addPolygon(subtractingPolygon);

    QList<QPolygonF> result;
    result = path.intersected(intersectingPath).simplified().toSubpathPolygons();

    // Избавляемся от противонаправленых смежных сторон.
    result = removeZeroAndFlatAngles(result);
    Q_ASSERT(!oppositeDirectionConsecutiveSidesWereFound(result));
    return result;
}

bool BoundaryPolygonCalc::isContained(const QPolygonF &polygon,
                                      const QPolygonF &potentialContainedPolygon)
{
    QPainterPath polygonPath;
    QPainterPath potentialContainedPolygonPath;
    polygonPath.addPolygon(polygon);
    potentialContainedPolygonPath.addPolygon(potentialContainedPolygon);

    if (polygonPath.contains(potentialContainedPolygonPath)) {
        return true;
    }
    return false;
}

bool BoundaryPolygonCalc::tryExtraConditionPolylines(const QPolygonF &polygon,
        QList<BoundaryPolyline> &conditionPolylines)
{
    QList<BoundaryPolyline> newConditionPolylines;
    newConditionPolylines =  BoundaryPolygon::boundaryPolylines(conditionsFromPolylines(polygon, conditionPolylines));
    const BoundaryCondition *previousPolylineCondition;
    previousPolylineCondition = NULL;
    int conditionCounter;
    //счётчик количества условий, отличных от пустого
    conditionCounter = 0;
    //подсчёт количества условий, отличных от пустого
    foreach(BoundaryPolyline newConditionPolyline, newConditionPolylines) {
        if (!newConditionPolyline.condition->isVoid()) {
            if (newConditionPolyline.condition != previousPolylineCondition) {
                ++conditionCounter;
                if (conditionCounter > 1) {
                    break;
                }
                previousPolylineCondition = newConditionPolyline.condition;
            }
        }
    }
    /* Если есть только одно условие, отличное от пустого, применяем его к
     * новым полилиниям и добавляем их к исходным */
    if (conditionCounter == 1) {
        foreach(BoundaryPolyline newConditionPolyline, newConditionPolylines) {
            Q_ASSERT(previousPolylineCondition != NULL);
            newConditionPolyline.condition = previousPolylineCondition;
            conditionPolylines.append(newConditionPolyline);
        }
        return true;
    }
    return false;
}

void BoundaryPolygonCalc::changeToSubtracted(QList<QPolygonF> &polygons, const QPolygonF &polygon)
{
    QList<QPolygonF> currentPolygons;
    foreach(QPolygonF currentPolygon, polygons) {
        currentPolygons.append(subtracted(currentPolygon, polygon));
    }
    polygons.clear();
    polygons.append(currentPolygons);
}

bool BoundaryPolygonCalc::polygonIsSimple(const QPolygonF &polygon) const
{
    return polygonIsNotSelfIntersecting(polygon)
           && !collinearConsecutiveSidesWereFound(polygon);
}

QPolygonF BoundaryPolygonCalc::removeZeroAndFlatAngles(const QPolygonF &polygon)
{
    QPolygonF result;
    result = removeEqualConsecutiveCorners(polygon);

    Q_ASSERT(result.isClosed());
    Q_ASSERT(result.size() >= 3);

    for (int i = 0; i < result.size() - 2; ++i) {
        QLineF line_1;
        QLineF line_2;
        line_1 = QLineF(result.at(i), result.at(i + 1));
        line_2 = QLineF(result.at(i + 1), result.at(i + 2));
        if (qFuzzyCompare(line_1.p1(), line_2.p2())) {
            // удаляем точки второго отрезка
            result.remove(i + 1, 2);
            if (i > 0) {
                /* Если итерация не нулевая, нужно сделать шаг назад, чтобы
                 * проверить вновь образовавшуюся пару, первый отрезок которой
                 * будет равен первому отрезку на прошлой итерации. Второй
                 * отрезок новый, он может быть любым. */
                --(--i);
            } else {
                // отступать некуда, сделаем проверку в конце.
                --i;
            }

        } else if (areCollinearLines(line_1, line_2)) {
            /* Cонаправленные отрезки могут элегантно превратиться в
             * противонаправленые на другой итерации. Пример:
             * QPolygonF(QPointF(-1, -1) QPointF(0, 0) QPointF(0, 1)
             *           QPointF(1, 0) QPointF(0, 0) QPointF(0.5, 0)
             *           QPointF(0, 0) QPointF(0.6, 0) QPointF(-0.5, -0.5)
             *           QPointF(-1, -1) ) */
            // удаляем общую точку отрезков.
            result.remove(i + 1);
            // чтобы на следующей итерации i осталась прежней
            --i;
        }
    }

    // Проверяем пару (конечный отрезок, начальный отрезок)
    Q_ASSERT(result.isClosed());
    Q_ASSERT(result.size() >= 3);

    QLineF line_1;
    QLineF line_2;
    Q_ASSERT(!result.isEmpty());
    line_1 = QLineF(result.first(), result.at(1));
    line_2 = QLineF(result.at(result.size() - 2), result.last());
    if (areCollinearLines(line_1, line_2)) {
        // удаляем начальную и конечную точки
        result.remove(0);
        result.remove(result.size() - 1);
        // замыкаем полигон, если необходимо:
        if (!result.isClosed()) {
            result.append(result.first());
        }
    }

    Q_ASSERT(result.isClosed());
    Q_ASSERT(result.size() >= 3);

    result = removeFlatAngles(result);

    Q_ASSERT(result.isClosed());
    Q_ASSERT(result.size() >= 3);

    return result;
}

QList<QPolygonF> BoundaryPolygonCalc::removeZeroAndFlatAngles(const QList<QPolygonF> &polygons)
{
    QList<QPolygonF> result;
    foreach(QPolygonF polygon, polygons) {
        QPolygonF modifyedPolygon;
        modifyedPolygon = removeFlatAngles(polygon);
        if (modifyedPolygon.size() >= 3) {
            result << removeZeroAndFlatAngles(polygon);
        }
    }
    return result;
}

QPolygonF BoundaryPolygonCalc::removeFlatAngles(const QPolygonF &polygon)
{
    QPolygonF result;
    result = removeEqualConsecutiveCorners(polygon);

    for (int i = 0; i < result.size() - 2; ++i) {
        QLineF line_1;
        QLineF line_2;
        line_1 = QLineF(result.at(i), result.at(i + 1));
        line_2 = QLineF(result.at(i + 1), result.at(i + 2));
        if (areCodirectionalLines(line_1, line_2)) {
            // удаляем общую точку отрезков.
            result.remove(i + 1);
            // чтобы на следующей итерации i осталась прежней
            --i;
        }
    }

    // Проверяем пару (конечный отрезок, начальный отрезок)
    QLineF line_1;
    QLineF line_2;
    line_1 = QLineF(result.first(), result.at(1));
    line_2 = QLineF(result.at(result.size() - 2), result.last());
    if (areCodirectionalLines(line_1, line_2)) {
        // удаляем начальную и конечную точки
        result.remove(0);
        result.remove(result.size() - 1);
        Q_ASSERT(!result.isEmpty());
        // замыкаем полигон, если необходимо:
        if (!result.isClosed()) {
            result.append(result.first());
        }
    }

    return result;
}

QPolygonF BoundaryPolygonCalc::removeEqualConsecutiveCorners(const QPolygonF &polygon)
{
    QPolygonF result;
    result = polygon;

    for (int i = 0; i < result.size() - 1; ++i) {
        if (qFuzzyCompare(result.at(i), result.at(i + 1))) {
            // удаляем повторяющийся угол и проверяем отрезок из той же точки
            result.remove(i + 1);
            --i;
        }
    }

    return result;
}

bool BoundaryPolygonCalc::areOppositeDirectionLines(const QLineF &line_1,
        const QLineF &line_2)
{
    if (qFuzzyCompare(line_1.p1(), line_1.p2()) || qFuzzyCompare(line_2.p1(), line_2.p2())) {
        // Если любой из векторов нулевой, возвращаем true
        return true;
    }
    /// Скалярное произведение
    qreal scalarProduct = dotProduct(line_1, line_2);

    if (qFuzzyCompare(line_1.dx()*line_2.dy(), line_2.dx()*line_1.dy())
            && scalarProduct < 0
            && !qFuzzyIsNull(scalarProduct)) {
        /* Вектора коллинеарны + скалярное произведение < 0 = они
         * противонаправлены. */
        return true;
    }
    return false;
}

bool BoundaryPolygonCalc::areCodirectionalLines(const QLineF &line_1, const QLineF &line_2)
{
    if (qFuzzyCompare(line_1.p1(), line_1.p2()) || qFuzzyCompare(line_2.p1(), line_2.p2())) {
        //Если любой из векторов нулевой, возвращаем true
        return true;
    }
    /// Скалярное произведение
    qreal scalarProduct = dotProduct(line_1, line_2);

    if (qFuzzyCompare(line_1.dx()*line_2.dy(), line_2.dx()*line_1.dy())
            && scalarProduct > 0
            && !qFuzzyIsNull(scalarProduct)) {
        // Вектора коллинеарны и скалярное произведение > 0 => они сонаправлены.
        return true;
    }
    return false;
}

bool BoundaryPolygonCalc::areCollinearLines(const QLineF &line_1, const QLineF &line_2)
{
    if (qFuzzyCompare(line_1.p1(), line_1.p2())
            || qFuzzyCompare(line_2.p1(), line_2.p2())) {
        //Если любой из векторов нулевой, возвращаем true
        return true;
    }

    if (qFuzzyCompare(line_1.dx()*line_2.dy(), line_2.dx()*line_1.dy())) {
        // Вектора коллинеарны
        return true;
    }
    return false;
}

bool BoundaryPolygonCalc::collinearConsecutiveSidesWereFound(const QPolygonF &polygon)
{
    Q_ASSERT(polygon.isClosed());
    Q_ASSERT(polygon.size() >= 3);

    QPolygonF changingPolygon;
    changingPolygon = polygon;

    //чтобы проверить пару (конечный отрезок, начальный отрезок)
    changingPolygon.append(changingPolygon.at(1));

    QLineF line_1;
    QLineF line_2;
    line_2 = QLineF(changingPolygon.at(0), changingPolygon.at(1));
    for (int i = 0; i < changingPolygon.size() - 2; ++i) {
        //аналогично следующему: line_1 = QLineF(polygon.at(i), polygon.at(i + 1));
        line_1 = line_2;
        line_2 = QLineF(changingPolygon.at(i + 1), changingPolygon.at(i + 2));
        if (areCollinearLines(line_1, line_2)) {
            return true;
        }
    }
    return false;
}

bool BoundaryPolygonCalc::oppositeDirectionConsecutiveSidesWereFound(const QPolygonF &polygon)
{
    Q_ASSERT(polygon.isClosed());
    Q_ASSERT(polygon.size() >= 3);

    QPolygonF changingPolygon;
    changingPolygon = polygon;

    //чтобы проверить пару (конечный отрезок, начальный отрезок)
    changingPolygon.append(changingPolygon.at(1));

    QLineF line_1;
    QLineF line_2;
    line_2 = QLineF(changingPolygon.at(0), changingPolygon.at(1));
    for (int i = 0; i < changingPolygon.size() - 2; ++i) {
        //аналогично следующему: line_1 = QLineF(polygon.at(i), polygon.at(i + 1));
        line_1 = line_2;
        line_2 = QLineF(changingPolygon.at(i + 1), changingPolygon.at(i + 2));
        if (areOppositeDirectionLines(line_1, line_2)) {
            return true;
        }
    }
    return false;
}

bool BoundaryPolygonCalc::oppositeDirectionConsecutiveSidesWereFound(const QList< QPolygonF > &polygons)
{
    foreach(QPolygonF polygon, polygons) {
        if (oppositeDirectionConsecutiveSidesWereFound(polygon)) {
            return true;
        }
    }
    return false;
}

QList<QPair<BoundaryPolygon *, QList<Vertex> > >
BoundaryPolygonCalc::removeConditionsOperation(const QList< const BoundaryCondition *> &conditions) const
{
    QList<QPair<BoundaryPolygon *, QList<Vertex> > > result;
    foreach(BoundaryPolygon * p, mScene->boundaryPolygons()) {
        QList<Vertex> newCorners = p->corners();
        QList<Vertex>::Iterator i;
        bool thisPolygonIsAffected = false;
        for (i = newCorners.begin(); i != newCorners.end(); ++i) {
            QList <ConditionPoint >::Iterator j;
            for (j = i->conditionPoints.begin(); j != i->conditionPoints.end(); ++j) {
                if (conditions.contains(j->condition)) {
                    j->condition = BoundaryCondition::voidCondition();
                    thisPolygonIsAffected = true;
                }
            }
        }
        simplifyConditions(newCorners);
        if (thisPolygonIsAffected) {
            result.append(QPair<BoundaryPolygon *, QList<Vertex> >(p, newCorners));
        }
    }
    return result;
}
