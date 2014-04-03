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

#ifndef QFGUI_CURVEPLOTDIALOG_H
#define QFGUI_CURVEPLOTDIALOG_H

#include <plot/plotdialog.h>

QT_FORWARD_DECLARE_CLASS(QMathGL)
QT_FORWARD_DECLARE_CLASS(mglDraw)
QT_FORWARD_DECLARE_CLASS(QSpinBox)

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(Scene)
QT_FORWARD_DECLARE_CLASS(CurveDraw)

class CurvePlotDialog: public PlotDialog
{
    Q_OBJECT
public:
    CurvePlotDialog(const Scene *scene, QWidget *parent);

private:
    QSpinBox *mMinT;
    QSpinBox *mMaxT;
    QSpinBox *mMinCoord;
    QSpinBox *mMaxCoord;

    CurveDraw *mDraw;

private slots:
    void updateLimits();
    void autoLimitsT();
    void autoLimitsZ();

private:
    void createDraw(const Scene *scene);
};

}

#endif // QFGUI_CURVEPLOTDIALOG_H