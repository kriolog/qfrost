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

#include "backgroundwidget.h"

#include <QPainter>
#include <QResizeEvent>

using namespace qfgui;

BackgroundWidget::BackgroundWidget(const QPixmap &pixmap, QWidget *parent):
    QWidget(parent),
    mPixmap(pixmap),
    mPoint1(52, 60),
    mPoint2(pixmap.width()-50, pixmap.height()-50)
{
    setMinimumSize(100, 100);
}
