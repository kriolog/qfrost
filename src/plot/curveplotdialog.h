/*
 * Copyright (C) 2014  Denis Pesotsky
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

#ifndef QFGUI_CURVEPLOTDIALOG_H
#define QFGUI_CURVEPLOTDIALOG_H

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QDoubleSpinBox)

namespace qfgui {

QT_FORWARD_DECLARE_CLASS(Block)

class CurvePlotDialog : public QDialog
{
    Q_OBJECT
public:
    CurvePlotDialog(Block *block,
                    Qt::Orientation orientation,
                    QWidget *parent = NULL);

private:
    QDoubleSpinBox *mMinT;
    QDoubleSpinBox *mMaxT;
    QDoubleSpinBox *mMinCoord;
    QDoubleSpinBox *mMaxCoord;

    QList<Block*> mSlice;
    QVector<double> mTemperatures;
    QVector<double> mThawedParts;
    QVector<double> mTransitionTemperatures;
    QVector<double> mCoordsMain;
    QVector<double> mCoordsNormal;

    QDialogButtonBox *mDialogButtons;
};
}

#endif // QFGUI_CURVEPLOTDIALOG_H
