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
#include <units/physicalpropertyspinbox.h>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QSlider>
#include <QFormLayout>
#include <QPushButton>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMouseEvent>

#include <QDebug>

using namespace qfgui;

BackgroundDialog::BackgroundDialog(const QPixmap &pixmap, QWidget *parent) :
    QDialog(parent),
    mView(new ViewBase(new QGraphicsScene(pixmap.rect(), this), this)),
    mButtons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel)),
    mPixmapItem(new QGraphicsPixmapItem(pixmap)),
    mCross1(new Cross(mPixmapItem)),
    mCross2(new Cross(mPixmapItem)),
    mCross1PixmapX(new QSpinBox(this)),
    mCross1PixmapY(new QSpinBox(this)),
    mCross2PixmapX(new QSpinBox(this)),
    mCross2PixmapY(new QSpinBox(this)),
    mCross1SceneX(PhysicalPropertySpinBox::createSceneCoordinateSpinBox()),
    mCross1SceneY(PhysicalPropertySpinBox::createSceneCoordinateSpinBox()),
    mCross2SceneX(PhysicalPropertySpinBox::createSceneCoordinateSpinBox()),
    mCross2SceneY(PhysicalPropertySpinBox::createSceneCoordinateSpinBox()),
    mPlaceCross1Button(new QPushButton(tr("&Place"))),
    mPlaceCross2Button(new QPushButton(tr("P&lace"))),
    mIsPlacingCross1(false),
    mIsPlacingCross2(false),
    mViewPressTimer()
{
    mViewPressTimer.start();
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    mCross1PixmapX->setMinimum(0);
    mCross1PixmapX->setMaximum(pixmap.width());
    mCross2PixmapX->setMinimum(0);
    mCross2PixmapX->setMaximum(pixmap.width());
    
    mCross1PixmapY->setMinimum(0);
    mCross1PixmapY->setMaximum(pixmap.height());
    mCross2PixmapY->setMinimum(0);
    mCross2PixmapY->setMaximum(pixmap.height());
    
    connect(mCross1PixmapX, SIGNAL(valueChanged(int)), SLOT(updateCross1Pos()));
    connect(mCross1PixmapY, SIGNAL(valueChanged(int)), SLOT(updateCross1Pos()));
    connect(mCross2PixmapX, SIGNAL(valueChanged(int)), SLOT(updateCross2Pos()));
    connect(mCross2PixmapY, SIGNAL(valueChanged(int)), SLOT(updateCross2Pos()));
    
    connect(mCross1PixmapX, SIGNAL(valueChanged(int)), SLOT(checkCrossesPos()));
    connect(mCross1PixmapY, SIGNAL(valueChanged(int)), SLOT(checkCrossesPos()));
    connect(mCross2PixmapX, SIGNAL(valueChanged(int)), SLOT(checkCrossesPos()));
    connect(mCross2PixmapY, SIGNAL(valueChanged(int)), SLOT(checkCrossesPos()));
    connect(mCross1SceneX, SIGNAL(valueChanged(double)), SLOT(checkCrossesPos()));
    connect(mCross1SceneY, SIGNAL(valueChanged(double)), SLOT(checkCrossesPos()));
    connect(mCross2SceneX, SIGNAL(valueChanged(double)), SLOT(checkCrossesPos()));
    connect(mCross2SceneY, SIGNAL(valueChanged(double)), SLOT(checkCrossesPos()));
    
    connect(mPlaceCross1Button, SIGNAL(clicked()), SLOT(startPlacingCross1()));
    connect(mPlaceCross2Button, SIGNAL(clicked()), SLOT(startPlacingCross2()));
    
    mCross2PixmapX->setValue(pixmap.width());
    mCross2PixmapY->setValue(pixmap.height());
    
    QHBoxLayout *cross1PixmapPos = new QHBoxLayout();
    cross1PixmapPos->addWidget(mCross1PixmapX);
    cross1PixmapPos->addWidget(mCross1PixmapY);
    cross1PixmapPos->addWidget(mPlaceCross1Button);
    QHBoxLayout *cross2PixmapPos = new QHBoxLayout();
    cross2PixmapPos->addWidget(mCross2PixmapX);
    cross2PixmapPos->addWidget(mCross2PixmapY);
    cross2PixmapPos->addWidget(mPlaceCross2Button);
    
    QHBoxLayout *cross1ScenePos = new QHBoxLayout();
    cross1ScenePos->addWidget(mCross1SceneX);
    cross1ScenePos->addWidget(mCross1SceneY);
    QHBoxLayout *cross2ScenePos = new QHBoxLayout();
    cross2ScenePos->addWidget(mCross2SceneX);
    cross2ScenePos->addWidget(mCross2SceneY);
    
    QFormLayout *imagePosLayout = new QFormLayout();
    imagePosLayout->addRow("Image pos 1:", cross1PixmapPos);
    imagePosLayout->addRow("Image pos 2:", cross2PixmapPos);
    
    QFormLayout *scenePosLayout = new QFormLayout();
    scenePosLayout->addRow("Scene pos 1:", cross1ScenePos);
    scenePosLayout->addRow("Scene pos 2:", cross2ScenePos);
    
    mView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mView->setBackgroundBrush(Qt::lightGray);
    
    mView->viewport()->installEventFilter(this);
    mView->viewport()->setMouseTracking(true);
    
    mCross2->setPos(pixmap.width(), pixmap.height());
    
    mPixmapItem->setTransformationMode(Qt::SmoothTransformation);
    mView->scene()->addItem(mPixmapItem);
    
    QSlider *slider = mView->createScaleSlider(Qt::Horizontal, this);
    Q_ASSERT(slider->minimum() == -slider->maximum());
    
    QHBoxLayout *posLayout = new QHBoxLayout();
    posLayout->addLayout(imagePosLayout);
    posLayout->addLayout(scenePosLayout);
    
    mainLayout->addLayout(posLayout);
    mainLayout->addWidget(mView);
    mainLayout->addWidget(slider);
    mainLayout->addWidget(mButtons);
    
    connect(mButtons, SIGNAL(accepted()), SLOT(acceptAndSendResult()));
    connect(mButtons, SIGNAL(rejected()), SLOT(reject()));
    
    mCross1->setCursor(Qt::OpenHandCursor);
    mCross2->setCursor(Qt::OpenHandCursor);
}

void BackgroundDialog::acceptAndSendResult()
{
    const double sx = (mCross1SceneX->value() - mCross2SceneX->value()) /
                      double(mCross1PixmapX->value() - mCross2PixmapX->value());
  //  emit accepted();
    accept();
}

void BackgroundDialog::checkCrossesPos()
{
    const bool pixmapPointsAreSame = (mCross1PixmapX->value() == mCross2PixmapX->value())
                                      && (mCross1PixmapY->value() == mCross2PixmapY->value());

    const bool scenePointsAreSame = (qFuzzyCompare(mCross1SceneX->value(), mCross2SceneX->value())
                                     && qFuzzyCompare(mCross2SceneY->value(), mCross2SceneY->value()));

    mButtons->button(QDialogButtonBox::Ok)->setEnabled(!pixmapPointsAreSame
                                                       && !scenePointsAreSame);
}

void BackgroundDialog::updateCross1Pos()
{
    mCross1->setPos(mCross1PixmapX->value(), mCross1PixmapY->value());
}

void BackgroundDialog::updateCross2Pos()
{
    mCross2->setPos(mCross2PixmapX->value(), mCross2PixmapY->value());
}

bool BackgroundDialog::eventFilter(QObject *object, QEvent *event)
{
    qDebug() << event;
    const bool isPlacing = (mIsPlacingCross1 || mIsPlacingCross2);

    Q_ASSERT(!isPlacing || (mIsPlacingCross1 != mIsPlacingCross2));
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (isPlacing) {
                if (event->type() == QEvent::MouseButtonRelease && mViewPressTimer.elapsed() < 250) {
                    return false;
                }
                finishPlacingCross();
            } else {
                if (event->type() == QEvent::MouseButtonRelease) {
                    return false;
                }
                QGraphicsItem *item = mView->itemAt(mView->mapFromGlobal(mouseEvent->globalPos()));
                if (item) {
                    if (item == mCross1) {
                        startPlacingCross1();
                        return true;
                    } else if (item == mCross2) {
                        startPlacingCross2();
                        return true;
                    }
                    return false;
                }
            }
        }
    } else if (event->type() == QEvent::MouseMove) {
        if (!(mIsPlacingCross1 || mIsPlacingCross2)) {
            return QDialog::eventFilter(object, event);
        }
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        const QPointF scenePos = mView->mapToScene(mView->mapFromGlobal(mouseEvent->globalPos()));
        (mIsPlacingCross1 ? mCross1PixmapX : mCross2PixmapX)->setValue(scenePos.x());
        (mIsPlacingCross1 ? mCross1PixmapY : mCross2PixmapY)->setValue(scenePos.y());
    }
    return false;
}

void BackgroundDialog::finishPlacingCross()
{
    Q_ASSERT(mIsPlacingCross1 != mIsPlacingCross2);
    mCross1->setCursor(Qt::OpenHandCursor);
    mCross2->setCursor(Qt::OpenHandCursor);
    if (mIsPlacingCross1) {
        mIsPlacingCross1 = false;
    } else {
        mIsPlacingCross2 = false;
    }
    mView->viewport()->unsetCursor();
    
    mPlaceCross1Button->setEnabled(true);
    mPlaceCross2Button->setEnabled(true);
}

void BackgroundDialog::startPlacingCross1()
{
    if (mIsPlacingCross1 || mIsPlacingCross2) {
        return;
    }
    mIsPlacingCross1 = true;
    mCross1->setCursor(Qt::BlankCursor);
    mCross2->setCursor(Qt::BlankCursor);
    mPlaceCross1Button->setEnabled(false);
    mPlaceCross2Button->setEnabled(false);
    mViewPressTimer.start();
}

void BackgroundDialog::startPlacingCross2()
{
    if (mIsPlacingCross1 || mIsPlacingCross2) {
        return;
    }
    mIsPlacingCross2 = true;
    mCross1->setCursor(Qt::BlankCursor);
    mCross2->setCursor(Qt::BlankCursor);
    mPlaceCross1Button->setEnabled(false);
    mPlaceCross2Button->setEnabled(false);
    mViewPressTimer.start();
}
