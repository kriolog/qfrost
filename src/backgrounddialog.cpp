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

#include "backgrounddialog.h"

#include <graphicsviews/cross.h>
#include <graphicsviews/viewbase.h>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QSlider>

using namespace qfgui;

BackgroundDialog::BackgroundDialog(const QPixmap &pixmap, QWidget *parent) :
    QDialog(parent),
    mButtons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)),
    mPixmapItem(new QGraphicsPixmapItem(pixmap)),
    mCross1(new Cross(mPixmapItem)),
    mCross2(new Cross(mPixmapItem))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    QGraphicsScene *scene = new QGraphicsScene(pixmap.rect(), this);
    ViewBase *view = new ViewBase(scene, this);
    
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    view->setBackgroundBrush(Qt::lightGray);
    
    mCross2->setPos(pixmap.width(), pixmap.height());
    
    mPixmapItem->setTransformationMode(Qt::SmoothTransformation);
    scene->addItem(mPixmapItem);
    
    mainLayout->addWidget(view);
    mainLayout->addWidget(view->createScaleSlider(Qt::Horizontal, this));
    mainLayout->addWidget(mButtons);
    
    connect(mButtons, SIGNAL(accepted()), SLOT(accept()));
    connect(mButtons, SIGNAL(rejected()), SLOT(reject()));
}
