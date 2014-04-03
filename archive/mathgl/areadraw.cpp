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

#include "areadraw.h"
#include <mgl/mgl.h>

#include <QtCore/qmath.h>
#include <QtCore/QVector>
#include <QtCore/qpoint.h>

using namespace qfgui;

AreaDraw::AreaDraw(const QVector<QPointF> &points,
                   const QVector<float> &temperatures,
                   const QVector<float> &transitionTemperatures,
                   const QVector<float> &thawedParts,
                   QObject *parent)
    : QObject(parent)
    , mMustPlotTemperatures(true)
    , mPoints()
    , mTemperatures()
    , mTransitionTemperatures()
    , mThawedParts()
    , mMinT(-10)
    , mMaxT(0)
    , mMinX(0)
    , mMaxX(10)
    , mMinY(0)
    , mMaxY(0)
{
    Q_ASSERT(points.size() == temperatures.size());
    Q_ASSERT(temperatures.size() == transitionTemperatures.size());
    Q_ASSERT(transitionTemperatures.size() == thawedParts.size());
    if (points.isEmpty()) {
        // TMP mathgl FIX
        return;
    }

    QVector<qreal> xCoords;
    QVector<qreal> yCoords;
    foreach(QPointF point, points) {
        float x = point.x();
        float y = point.y();

        int xFloor = qFloor(x);
        int yFloor = qFloor(y);
        int xCeil = qCeil(x);
        int yCeil = qCeil(y);

        if (xCeil < mMinX) {
            mMinX = xCeil;
        }
        if (xFloor > mMaxX) {
            mMaxX = xFloor;
        }
        if (yCeil < mMinY) {
            mMinX = yCeil;
        }
        if (yFloor > mMaxY) {
            mMaxY = yFloor;
        }

        xCoords << x;
        yCoords << y;
    }
    mPoints.Set(&xCoords.front(), points.size(), 1);
    mPoints.Set(&xCoords.front(), points.size(), 2);

    mXCoords.Set(xCoords.toStdVector());
    mYCoords.Set(yCoords.toStdVector());

    mTemperatures.Set(temperatures.toStdVector());
    mThawedParts.Set(thawedParts.toStdVector());
    mTransitionTemperatures.Set(transitionTemperatures.toStdVector());

    mMaxT = qCeil(mTemperatures.Maximal());
    mMinT = qFloor(mTemperatures.Minimal());
}

int AreaDraw::Draw(mglGraph *gr)
{
    if (mPoints.nx == 1) {
        return 1;
    }

    gr->Rotate(40, 60);

    //  gr->SetRanges(mMinX, mMaxX, mMinY, mMaxY,
    //                mMustPlotTemperatures ? mMinT : 0,
    //                mMustPlotTemperatures ? mMaxT : 1);

    gr->XRange(mXCoords);
    gr->YRange(mYCoords);
    gr->ZRange(mTemperatures);
    gr->AdjustTicks();

    /* gr->Cont(mMustPlotTemperatures
      ? mTemperatures
      : mThawedParts,
      mPoints);*/
    //  gr->ContF(mTemperatures, mXCoords, mYCoords);
    // gr->ContF(mTemperatures, mPoints);
    // gr->Dens(mXCoords, mYCoords, mTemperatures);
    //gr->TriCont(mXCoords, mYCoords, mTemperatures);

    gr->Label('y', tr("z, m").toAscii());
    gr->Label('x', tr(mMustPlotTemperatures
                      ? "t, \302\260C"
                      : "v_{th}").toAscii());

    gr->Axis("xyz");
    gr->Grid("xyz", "h;");
    return 0;
};

#include "areadraw.moc"