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

#ifndef QFGUI_CURVEDRAW_H
#define QFGUI_CURVEDRAW_H

#include <QtCore/QObject>
#include <mgl/mgl_define.h>
#include <mgl/mgl_data.h>

namespace qfgui
{

class CurveDraw: public QObject, public mglDraw
{
    Q_OBJECT
public:
    CurveDraw(const QVector<float> &coords,
              const QVector<float> &temperatures,
              const QVector<float> &transitionTemperatures,
              const QVector<float> &thawedParts,
              QObject *parent);

    virtual int Draw(mglGraph *gr);

    void autoLimitsT();
    void autoLimitsZ();

public slots:
    void setPlotTemperatures(bool b) {
        mMustPlotTemperatures = b;
        emit updated();
    }
    void setMinT(int f) {
        Q_ASSERT(f < mMaxT);
        mMinT = f;
        emit updated();
    }
    void setMaxT(int f) {
        Q_ASSERT(f > mMinT);
        mMaxT = f;
        emit updated();
    }
    void setMinCoord(int f) {
        Q_ASSERT(f < mMaxCoord);
        mMinCoord = f;
        emit updated();
    }
    void setMaxCoord(int f) {
        Q_ASSERT(f > mMinCoord);
        mMaxCoord = f;
        emit updated();
    }

    int minT() const {
        return mMinT;
    }
    int maxT() const {
        return mMaxT;
    }
    int minCoord() const {
        return mMinCoord;
    }
    int maxCoord() const {
        return mMaxCoord;
    }

signals:
    void updated();

private:
    bool mMustPlotTemperatures;
    mglData mCoords;
    mglData mTemperatures;
    mglData mTransitionTemperatures;
    mglData mThawedParts;
    int mMinT;
    int mMaxT;
    int mMinCoord;
    int mMaxCoord;
};

}

#endif // QFGUI_CURVEDRAW_H
