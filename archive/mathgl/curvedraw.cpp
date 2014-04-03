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

#include "curvedraw.h"
#include <mgl/mgl.h>

#include <QtCore/qmath.h>
#include <QtCore/QVector>

using namespace qfgui;

CurveDraw::CurveDraw(const QVector<float> &coords,
                     const QVector<float> &temperatures,
                     const QVector<float> &transitionTemperatures,
                     const QVector<float> &thawedParts,
                     QObject *parent)
    : QObject(parent)
    , mMustPlotTemperatures()
    , mCoords()
    , mTemperatures()
    , mTransitionTemperatures()
    , mThawedParts()
    , mMinT(0)
    , mMaxT(1)
    , mMinCoord(0)
    , mMaxCoord(1)
{
    Q_ASSERT(coords.size() == temperatures.size());
    Q_ASSERT(temperatures.size() == transitionTemperatures.size());
    Q_ASSERT(transitionTemperatures.size() == thawedParts.size());
    if (coords.isEmpty()) {
        // TMP mathgl FIX
        return;
    }
    mCoords.Set(coords.toStdVector());
    mTemperatures.Set(temperatures.toStdVector());
    mThawedParts.Set(thawedParts.toStdVector());
    mTransitionTemperatures.Set(transitionTemperatures.toStdVector());
}

void CurveDraw::autoLimitsT()
{
    mMaxT = qCeil(mTemperatures.Maximal());
    mMinT = qFloor(mTemperatures.Minimal());
    emit updated();
}

void CurveDraw::autoLimitsZ()
{
    mMaxCoord = qCeil(mCoords.Maximal());
    mMinCoord = qFloor(mCoords.Minimal());
    emit updated();
}

int CurveDraw::Draw(mglGraph *gr)
{
    if (mCoords.nx == 1) {
        return 1;
    }

    gr->Rotate(180, 0);

    gr->SetRanges(mMustPlotTemperatures ? mMinT : 0,
                  mMustPlotTemperatures ? mMaxT : 1,
                  mMinCoord, mMaxCoord);
    gr->AdjustTicks();

    if (mMustPlotTemperatures) {
        gr->Plot(mTransitionTemperatures, mCoords, "1H-");
    }
    gr->Plot(mMustPlotTemperatures
             ? mTemperatures
             : mThawedParts,
             mCoords, "2k-");

    gr->Label('y', tr("z, m").toAscii());
    gr->Label('x', tr(mMustPlotTemperatures
                      ? "t, \302\260C"
                      : "v_{th}").toAscii());

    gr->Axis("xy");
    gr->Grid("xy", "h;");
    return 0;
};

#include "curvedraw.moc"