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

#include "zoomslider.h"

#include <QtWidgets/QAction>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QToolButton>
#include <QtGui/QIcon>

using namespace qfgui;

ZoomSlider::ZoomSlider(QWidget *parent)
    : QWidget(parent)
    , mSlider(new QSlider(Qt::Horizontal, this))
    , mZoomInButton(new QToolButton(this))
    , mZoomOutButton(new QToolButton(this))
    , mZoomInAction(NULL)
    , mZoomOutAction(NULL)
{
    mZoomInButton->setIcon(QIcon::fromTheme("zoom-in"));
    mZoomInButton->setAutoRaise(true);
    mZoomInButton->setAutoRepeat(true);

    mZoomOutButton->setIcon(QIcon::fromTheme("zoom-out"));
    mZoomOutButton->setAutoRaise(true);
    mZoomOutButton->setAutoRepeat(true);
 
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(mZoomOutButton);
    layout->addWidget(mSlider, 1);
    layout->addWidget(mZoomInButton);

    connect(mSlider, SIGNAL(actionTriggered(int)), SLOT(updateButtons()));
    connect(mSlider, SIGNAL(valueChanged(int)), SIGNAL(valueChanged(int)));
 
    connect(mZoomOutButton, SIGNAL(clicked()), SLOT(zoomOut()));
    connect(mZoomInButton, SIGNAL(clicked()), SLOT(zoomIn()));
}

int ZoomSlider::value() const
{
    return mSlider->value();
}

void ZoomSlider::setValue(int value)
{
    mSlider->setValue(value);
    updateButtons();
}

void ZoomSlider::setMinimum(int value)
{
    mSlider->setMinimum(value);
    updateButtons();
}

void ZoomSlider::setMaximum(int value)
{
    mSlider->setMaximum(value);
    updateButtons();
}

void ZoomSlider::setZoomInAction(QAction *action)
{
    mZoomInAction = action;
}

void ZoomSlider::setZoomOutAction(QAction *action)
{
    mZoomOutAction = action;
}

void ZoomSlider::zoomOut()
{
    if (mZoomOutAction) {
        mZoomOutAction->trigger();
    } else {
        mSlider->triggerAction(QAbstractSlider::SliderSingleStepSub);
    }
}

void ZoomSlider::zoomIn()
{
    if (mZoomInAction) {
        mZoomInAction->trigger();
    } else {
        mSlider->triggerAction(QAbstractSlider::SliderSingleStepAdd);
    }
}

void ZoomSlider::updateButtons()
{
    mZoomOutButton->setEnabled(mSlider->value() > mSlider->minimum());
    mZoomInButton->setEnabled(mSlider->value() < mSlider->maximum());
}
