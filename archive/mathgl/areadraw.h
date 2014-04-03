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

#ifndef QFGUI_AREADRAW_H
#define QFGUI_AREADRAW_H

#include <QtCore/QObject>
#include <mgl/mgl_define.h>
#include <mgl/mgl_data.h>

QT_FORWARD_DECLARE_CLASS(QPointF)

namespace qfgui
{

class AreaDraw: public QObject, public mglDraw
{
    Q_OBJECT
public:
    AreaDraw(const QVector<QPointF> &points,
             const QVector<float> &temperatures,
             const QVector<float> &transitionTemperatures,
             const QVector<float> &thawedParts,
             QObject *parent);

    virtual int Draw(mglGraph *gr);

private:
    bool mMustPlotTemperatures;
    mglData mPoints;
    mglData mXCoords;
    mglData mYCoords;
    mglData mTemperatures;
    mglData mTransitionTemperatures;
    mglData mThawedParts;
    int mMinT;
    int mMaxT;
    int mMinX;
    int mMaxX;
    int mMinY;
    int mMaxY;
};

}

#endif // QFGUI_AREADRAW_H
