/*
 * Copyright (C) 2015  Denis Pesotsky
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
 * 
 */

#ifndef QFGUI_SORTEDPOINTSPLOT_H
#define QFGUI_SORTEDPOINTSPLOT_H

#include <QFrame>

QT_FORWARD_DECLARE_CLASS(QCustomPlot)
QT_FORWARD_DECLARE_CLASS(QCPGraph)
QT_FORWARD_DECLARE_CLASS(QCPItemLine)
QT_FORWARD_DECLARE_CLASS(QStackedLayout)

namespace qfgui {

QT_FORWARD_DECLARE_CLASS(SortedPointsModel)

class SortedPointsPlot : public QFrame
{
    Q_OBJECT
public:
    SortedPointsPlot(SortedPointsModel *model, QWidget *parent = NULL);

protected:
    void changeEvent(QEvent *event);

private slots:
    void updateGraph();

private:
    SortedPointsModel *mModel;

    QCustomPlot *mPlot;
    QCPGraph *mGraph;

    QStackedLayout *mLayout;

    QCPItemLine *mGraphExtension;
};

}

#endif // QFGUI_SORTEDPOINTSPLOT_H
