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

#include <tools/boundaryconditionsapplicator.h>

#include <QtGui/QPainter>
#include <QtWidgets/QMenu>
#include <QtWidgets/QStyle>

#include <boundarypolygon.h>
#include <boundarypolygoncalc.h>
#include <scene.h>
#include <tools/anchor.h>
#include <polyline.h>
#include <boundary_conditions/boundarycondition.h>
#include <mainwindow.h>

using namespace qfgui;

BoundaryConditionsApplicator::BoundaryConditionsApplicator() : Tool(),
    mIsStickedToPolygon(false),
    mPolylineOrder(undefined),
    mOriginPoint(),
    mEndPoint(),
    mPolyline(new Polyline(this)),
    mAllowApplyConditionToWhole(true)
{
    setFlag(ItemHasNoContents);
}

void BoundaryConditionsApplicator::onSceneHasChanged()
{
    if (scene() != NULL) {
        connect(scene(), SIGNAL(mousePressed(QPointF)),
                SLOT(tryToApplyPolyline()));
        connect(scene(), SIGNAL(mousePressed(PointOnBoundaryPolygon)),
                SLOT(tryToApplyPolyline()));
        connect(qfScene()->anchor(), SIGNAL(signalPositionChanged(PointOnBoundaryPolygon)),
                SLOT(updatePolyline(PointOnBoundaryPolygon)));
    }
}

QPolygonF BoundaryConditionsApplicator::straightOrderedPolyline() const
{
    Q_ASSERT(!mOriginPoint.isNull());
    Q_ASSERT(mOriginPoint.polygon()->polygon().isClosed());
    Q_ASSERT(mOriginPoint.polygon()->polygon().size() >= 3);

    // Провека на наличие полигона.
    if (mOriginPoint.isNull()) {
        return QPolygonF();
    }

    // Для удобства пользования и во избежание лишних вызовов.
    QPolygonF polygon = mOriginPoint.polygon()->polygon();

    /* Проверка валидности точекпо номеру (- 2, т.к. последняя точка полигона
     * равна первой) */
    if (mOriginPoint.index() > polygon.size() - 2
            || mEndPoint.index() > polygon.size() - 2) {
        return QPolygonF();
    }

    // Проверка валидности точек по расстоянию.
    qreal length_1;
    length_1 = mOriginPoint.polygon()->segment(mOriginPoint.index()).length();
    qreal length_2;
    length_2 = mOriginPoint.polygon()->segment(mEndPoint.index()).length();
    if (qFuzzyCompare(mOriginPoint.distance(), length_1)
            || qFuzzyCompare(mEndPoint.distance(), length_2)
            || mOriginPoint.distance() > length_1
            || mEndPoint.distance() > length_2) {
        return QPolygonF();
    }

    if (mOriginPoint.index() == mEndPoint.index()
            && qFuzzyCompare(mOriginPoint.distance(), mEndPoint.distance())) {
        /* Если точки совпадают, возвращаем список из единственного исходного
         * полигона. */
        return polygon;
    }

    //запоминаем начальную и конечную точки будущих полилиний
    QPointF firstPolylinePoint;
    QPointF lastPolylinePoint;
    firstPolylinePoint = mOriginPoint.toPoint();
    lastPolylinePoint = mEndPoint.toPoint();

    QPolygonF straightPolyline;

    /* Получаем первую полилинию: прямой порядок
     * Если номера точек полигона совпадают и расстояние у второй больше, чем
     * расстояние у первой, возвращаем отрезок, построенный на этих точках. В
     * противном случае обходим все угловые точки полигона, лежащие между ними.
     * если запихнуть нижеслежуещее в один алгоритм, перевернуть полигон и
     * пересчитать исходные точки, получится криво. */

    //добавляем первую точку
    straightPolyline.append(firstPolylinePoint);

    if (mOriginPoint.index() == mEndPoint.index()
            && mOriginPoint.distance() < mEndPoint.distance()) {
        /* Точки лежат на одном отрезке, первая ближе. Полилиния состоит из
         * двух этих точек */
        straightPolyline.append(lastPolylinePoint);
    } else {
        /* Наращиваем полилинию точками полигона. Обходим в прямом порядке
         * все угловые точки полигона, лежащие между начальной и конечной
         * точками. */
        int i;
        if (mOriginPoint.index() < polygon.size() - 2) {
            i = mOriginPoint.index() + 1;
        } else {
            // Перескакиваем через конец = начало
            i = 0;
        }
        while (i != mEndPoint.index()) {
            straightPolyline.append(polygon.at(i));
            ++i;
            if (i == polygon.size() - 1) {
                // Перескакиваем через конец = начало
                i = 0;
            }
        }
        straightPolyline.append(polygon.at(mEndPoint.index()));
        if (!qFuzzyCompare(mEndPoint.distance(), 0)) {
            // Добавляем конечную точку
            straightPolyline.append(lastPolylinePoint);
        }
    }

    return straightPolyline;
}

QPolygonF BoundaryConditionsApplicator::reverseOrderedPolyline() const
{
    // Провека на наличие полигона.
    if (mOriginPoint.isNull()) {
        return QPolygonF();
    }

    Q_ASSERT(mOriginPoint.polygon()->polygon().isClosed());
    Q_ASSERT(mOriginPoint.polygon()->polygon().size() >= 3);

    // Для удобства пользования и во избежание лишних вызовов.
    QPolygonF polygon;
    polygon = mOriginPoint.polygon()->polygon();

    /* Проверка валидности точекпо номеру (- 2, т.к. последняя точка полигона
     * равна первой) */
    if (mOriginPoint.index() > polygon.size() - 2
            || mEndPoint.index() > polygon.size() - 2) {
        return QPolygonF();
    }

    // Проверка валидности точек по расстоянию.
    qreal length_1;
    length_1 = mOriginPoint.polygon()->segment(mOriginPoint.index()).length();
    qreal length_2;
    length_2 = mOriginPoint.polygon()->segment(mEndPoint.index()).length();
    if (qFuzzyCompare(mOriginPoint.distance(), length_1)
            || qFuzzyCompare(mEndPoint.distance(), length_2)
            || mOriginPoint.distance() > length_1
            || mEndPoint.distance() > length_2) {
        return QPolygonF();
    }

    if (mOriginPoint.index() == mEndPoint.index()
            && qFuzzyCompare(mOriginPoint.distance(), mEndPoint.distance())) {
        /* Если точки совпадают, возвращаем список из единственного исходного
         * полигона. */
        return polygon;
    }

    //запоминаем начальную и конечную точки будущих полилиний
    QPointF firstPolylinePoint;
    QPointF lastPolylinePoint;
    firstPolylinePoint = mOriginPoint.toPoint();
    lastPolylinePoint = mEndPoint.toPoint();

    QPolygonF reversePolyline;

    /* получаем вторую полилинию: обратный порядок
     * если номера точек полигона совпадают и расстояние у второй меньше, чем
     * расстояние у первой, возвращаем отрезок, построенный на этих точках. В
     * противном случае обходим все угловые точки полигона, лежащие между
     * ними. */

    //добавляем первую точку
    reversePolyline.append(firstPolylinePoint);

    if (mOriginPoint.index() == mEndPoint.index()
            && mOriginPoint.distance() > mEndPoint.distance()) {
        /* Точки лежат на одном отрезке, первая дальше. Полилиния состоит из
         * двух этих точек */
        reversePolyline.append(lastPolylinePoint);
    } else {
        /* Наращиваем полилинию точками полигона. Обходим в обратном порядке
         * все угловые точки полигона, лежащие между начальной и конечной
         * точками. */
        if (!qFuzzyCompare(mOriginPoint.distance(), 0)) {
            reversePolyline.append(polygon.at(mOriginPoint.index()));
        }
        int i;
        if (mOriginPoint.index() > 0) {
            i = mOriginPoint.index() - 1;
        } else {
            // Перескакиваем через конец = начало
            i = polygon.size() - 2;
        }
        while (i != mEndPoint.index()) {
            reversePolyline.append(polygon.at(i));
            --i;
            if (i == -1) {
                // Перескакиваем через конец = начало
                i = polygon.size() - 2;
            }
        }
        // Добавляем конечную точку
        reversePolyline.append(lastPolylinePoint);
    }

    return reversePolyline;
}

qreal BoundaryConditionsApplicator::length(const QPolygonF &polygon)
{
    QPainterPath polygonPath;
    polygonPath.addPolygon(polygon);
    return polygonPath.length();
}

void BoundaryConditionsApplicator::updatePolyline(const PointOnBoundaryPolygon &p)
{
    if (scene() == NULL) {
        // HACK: иногда мы успеем получить в этот слот сигнал уже тогда, когда
        //       удалились из сцены, так что просто выйдем отсюда.
        //       Повторить можно так: быстро водим курсором и вдруг тыкаем ESC.
        return;
    }
    Q_ASSERT(!p.isNull());
    Q_ASSERT(qfScene() != NULL);

    if (mEndPoint.polygon() != NULL && mEndPoint.polygon()->scene() == NULL) {
        // Это на случай, если полигон, на котором мы рисуем, вдруг удалился
        // из сцены (например, если юзер сделал Undo). Обнуляем нас.
        mIsStickedToPolygon = false;
        mOriginPoint = PointOnBoundaryPolygon();
        mEndPoint = PointOnBoundaryPolygon();
        mPolyline->updatePolyline(QPolygonF());
        return;
    }

    //Если в данный момент привязка происходит по полигону
    if (!mIsStickedToPolygon) {
        /* Если ещё нет полигона, которому будем задавать свойства, рисуем
         * точку на потенциальном. */

        // Обновляем начальную точку.
        mOriginPoint = p;
        // Рисуем полилинию из единственной точки.
        mPolyline->updatePolyline(mOriginPoint.toPoint());
    } else if (mOriginPoint.polygon() == p.polygon()) {
        /* Если уже есть полигон, к которому, к тому же, только что
         * произошла привязка, то у него теперь есть начальная и конечная
         * точки. */

        // Обновляем конечную точку.
        mEndPoint = p;

        // Рисуем одну из полилиний (или весь полигон).
        if (qFuzzyCompare(mOriginPoint.toPoint(),
                          mEndPoint.toPoint(),
                          qfScene()->anchor()->stickRadius() / 2)) {
            /* Если начальная и конечная точки совпадают с погрешностью в
                * stickRadius привязки, приравниваем конечную к начальной */
            mEndPoint = mOriginPoint;

            if (mAllowApplyConditionToWhole) {
                // Если разрешено, рисуем весь полигон.
                mPolyline->updatePolyline(mOriginPoint.polygon()->polygon());
            }
            return; //sic
        }
        /* Если мы вышли за область примерного совпадения начальной и
            * конечной точек, разрешаем применять "цвет" ко всему полигону. */
        mAllowApplyConditionToWhole = true;

        /* Рисуем одну из полилиний в уже найденной ориентации полилинии,
            * или из условия отношения длин полилиний. */
        switch (mPolylineOrder) {
        case straight:
            mPolyline->updatePolyline(straightOrderedPolyline());
            break;
        case reverse:
            mPolyline->updatePolyline(reverseOrderedPolyline());
            break;
        case undefined:
            QPolygonF straightPolyline;
            QPolygonF reversePolyline;
            straightPolyline = straightOrderedPolyline();
            reversePolyline = reverseOrderedPolyline();
            if (length(straightPolyline) < length(reversePolyline)) {
                mPolyline->updatePolyline(straightPolyline);
            } else {
                mPolyline->updatePolyline(reversePolyline);
            }
            break;
        }
    } else {
        // Сюда мы не должны попасть
        Q_ASSERT(false);
    }
}

void BoundaryConditionsApplicator::tryToApplyPolyline()
{
    Q_ASSERT(qfScene() != NULL);
    if (!mIsStickedToPolygon) {
        /* Если ещё нет полигона, у которого будем менять свойства, задаём
         * его (начальная точка и указатель на полигон должны быть
         * гарантированно найдена ранее)*/
        if (!mOriginPoint.isNull()) {
            // Привязка для этого должна была найтись хотя бы однажды.
            mIsStickedToPolygon = true;
            // Приравниваем конечную точку к начальной.
            mEndPoint = mOriginPoint;
            // Рисуем полилинию, равную полигону.
            mPolyline->updatePolyline(mOriginPoint.polygon()->polygon());
        }
    } else if (mAllowApplyConditionToWhole) {
        //Если применение новых граничных условий разрешено.
        BoundaryPolyline applyingPolyline;

        QMenu conditionMenu;
        conditionMenu.setTitle(tr("Boundary Condition"));
        foreach(BoundaryCondition * bc, boundaryConditions()) {
            QAction *a = conditionMenu.addAction(bc->name());
            a->setData(qVariantFromValue(static_cast<void *>(bc)));
            int iconSize = conditionMenu.style()->pixelMetric(QStyle::PM_SmallIconSize);
            QPixmap pixmap(iconSize, iconSize);
            pixmap.fill(bc->color());
            a->setIcon(QIcon(pixmap));
        }

        QAction *selectedAction = conditionMenu.exec(QCursor::pos());
        if (selectedAction != NULL) {
            QVariant v = selectedAction->data();
            Q_ASSERT(v.canConvert<void *>());
            applyingPolyline.condition = static_cast<BoundaryCondition *>(v.value<void *>());
        } else {
            return;
        }

        BoundaryPolygon *p = const_cast<BoundaryPolygon *>(mOriginPoint.polygon());
        QList<Vertex> newCorners;
        if (mOriginPoint.index() == mEndPoint.index()
                && qFuzzyCompare(mOriginPoint.distance(), mEndPoint.distance())) {
            /* Если уже есть полигон, у которого будем менять свойства, а начальная
             * и конечная точки равны, задаём свойства всему полигону и меняем
             * порядок следования на undefined. */
            newCorners = BoundaryPolygonCalc::setCondition(p->polygon(), applyingPolyline.condition);
            mPolylineOrder = undefined;
        } else {
            switch (mPolylineOrder) {
            case straight:
                applyingPolyline.polygon = straightOrderedPolyline();
                newCorners = BoundaryPolygonCalc::addConditionFromPolyline(p->corners(), applyingPolyline);
                break;
            case reverse:
                applyingPolyline.polygon = reverseOrderedPolyline();
                newCorners = BoundaryPolygonCalc::addConditionFromPolyline(p->corners(), applyingPolyline);
                break;
            case undefined:
                QPolygonF straightPolyline;
                QPolygonF reversePolyline;
                straightPolyline = straightOrderedPolyline();
                reversePolyline = reverseOrderedPolyline();
                // Выбираем порядок следования точек для этого и последующих применений.
                if (length(straightPolyline) < length(reversePolyline)) {
                    applyingPolyline.polygon = straightPolyline;
                    mPolylineOrder = straight;
                } else {
                    applyingPolyline.polygon = reversePolyline;
                    mPolylineOrder = reverse;
                }
                newCorners = BoundaryPolygonCalc::addConditionFromPolyline(p->corners(), applyingPolyline);
                break;
            }
            // Задаём новую начальную точку.
            mOriginPoint = mEndPoint;
        }
        // Применяем углы к полигону
        if (p->corners() != newCorners) {
            Q_ASSERT(qfScene() != NULL);
            qfScene()->setBoundaryConditions(p, newCorners);
        }
        /* Запрещаем повторное применение, следующее СРАЗУ за текущим. Теперь,
         * чтобы новое добавление граничных условий удалось, нужно выйти за
         * заданный круг с центром в начальной точке. */
        mAllowApplyConditionToWhole = false;

        // Рисуем полилинию из единственной точки.
        mPolyline->updatePolyline(mOriginPoint.toPoint());
    }
}

Scene *BoundaryConditionsApplicator::qfScene() const
{
    if (scene() == NULL) {
        return NULL;
    }
    Scene *result;
    result = qobject_cast<Scene *>(scene());
    return result;
}

QList<BoundaryCondition *> BoundaryConditionsApplicator::boundaryConditions() const
{
    Q_ASSERT(qfScene() != NULL);
    MainWindow *m = qobject_cast<MainWindow *>(qfScene()->parent());
    Q_ASSERT(m != NULL);
    return m->boundaryConditions();
}
