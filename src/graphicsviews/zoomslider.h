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
 */

#ifndef QFGUI_ZOOMSLIDER_H
#define QFGUI_ZOOMSLIDER_H

#include <QtWidgets/QWidget>

QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QToolButton)

namespace qfgui
{

class ZoomSlider : public QWidget
{
    Q_OBJECT

public:
    ZoomSlider(QWidget *parent = NULL);

    int value() const;

    QSlider *slider() const { return mSlider; }

    void setZoomInAction(QAction *action);
    void setZoomOutAction(QAction *action);

public slots:
    void setValue(int);
    void setMinimum(int);
    void setMaximum(int);

signals:
    void valueChanged(int);

private slots:
    void zoomOut();
    void zoomIn();

    void updateButtons();

private:
    QSlider *mSlider;

    QToolButton *mZoomOutButton;
    QToolButton *mZoomInButton;

    QAction *mZoomInAction;
    QAction *mZoomOutAction;

};

}

#endif // QFGUI_ZOOMSLIDER_H
