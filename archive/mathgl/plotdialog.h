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

#ifndef QFGUI_PLOTDIALOG_H
#define QFGUI_PLOTDIALOG_H

#include <QtGui/QDialog>

QT_FORWARD_DECLARE_CLASS(QMathGL)
QT_FORWARD_DECLARE_CLASS(QVBoxLayout)
QT_FORWARD_DECLARE_CLASS(mglDraw)

namespace qfgui
{

class PlotDialog: public QDialog
{
    Q_OBJECT
public:
    PlotDialog(QWidget *parent);

protected:
    void addWidget(QWidget *widget);
    void addLayout(QLayout *layout);

protected slots:
    void save();
    void redrawData();
    void setDraw(mglDraw *draw);

private:
    QMathGL *mMglWidget;
    QVBoxLayout *mMainLayout;
};

}

#endif // QFGUI_PLOTDIALOG_H