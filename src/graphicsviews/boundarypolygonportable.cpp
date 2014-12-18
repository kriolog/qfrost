/*
 * Copyright (C) 2011-2012  Denis Pesotsky
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

#include "boundarypolygonportable.h"

using namespace qfgui;

BoundaryPolygonPortable::BoundaryPolygonPortable(BoundaryPolygon *boundaryPolygon)
{
    QList<Vertex> polygonCorners = boundaryPolygon->corners();
    foreach(Vertex vertex, polygonCorners) {
        PortableVertex portableVertex;
        portableVertex.point = vertex.point;
        foreach(ConditionPoint conditionPoint, vertex.conditionPoints) {
            PortableConditionPoint portableConditionPoint;
            portableConditionPoint.distance = conditionPoint.distance;
            Q_ASSERT(conditionPoint.condition != NULL);
            portableConditionPoint.conditionID = conditionPoint.condition->id();

            portableVertex.conditionPoints << portableConditionPoint;
        }
        corners << portableVertex;
    }

    foreach(BoundaryPolygon * child, boundaryPolygon->childBoundaryPolygonItems()) {
        children << BoundaryPolygonPortable(child);
    }
}

QDataStream &operator<< (QDataStream &out, const PortableVertex &p)
{
    out << p.point
        << p.conditionPoints;
    return out;
}

QDataStream &operator>> (QDataStream &in, PortableVertex &p)
{
    // на случай, если мы что-то не смогли считать до этого (размер массива)
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    in >> p.point
       >> p.conditionPoints;
    // а это на случай, если мы что-то не смогли считать сейчас
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    return in;
}

QDataStream &operator<< (QDataStream &out, const PortableConditionPoint &p)
{
    out << p.conditionID
        << p.distance;
    return out;
}
QDataStream &operator>> (QDataStream &in, PortableConditionPoint &p)
{
    // на случай, если мы что-то не смогли считать до этого (размер массива)
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    in >> p.conditionID
       >> p.distance;
    // а это на случай, если мы что-то не смогли считать сейчас
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    return in;
}

QDataStream &operator<< (QDataStream &out, const BoundaryPolygonPortable &p)
{
    out << p.corners
        << p.children;
    return out;
}
QDataStream &operator>> (QDataStream &in, BoundaryPolygonPortable &p)
{
    // на случай, если мы что-то не смогли считать до этого (размер массива)
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    in >> p.corners
       >> p.children;
    // а это на случай, если мы что-то не смогли считать сейчас
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
    return in;
}

BoundaryPolygon *BoundaryPolygonPortable::createPolygon(const QList<const BoundaryCondition *> conditions)
{
    QList<Vertex> polygonCorners;
    foreach(PortableVertex portableVertex, corners) {
        Vertex vertex;
        vertex.point = portableVertex.point;
        foreach(PortableConditionPoint portableConditionPoint,
                portableVertex.conditionPoints) {
            ConditionPoint conditionPoint;
            conditionPoint.distance = portableConditionPoint.distance;
            if (portableConditionPoint.conditionID != -1) {
                Q_ASSERT(conditions.size() > portableConditionPoint.conditionID);
                conditionPoint.condition = conditions.at(portableConditionPoint.conditionID);
            }
            vertex.conditionPoints << conditionPoint;
        }
        polygonCorners << vertex;
    }

    BoundaryPolygon *result = new BoundaryPolygon(polygonCorners);
    foreach(BoundaryPolygonPortable portableChild, children) {
        BoundaryPolygon *child = portableChild.createPolygon(conditions);
        child->setParentItem(result);
    }
    return result;
}
