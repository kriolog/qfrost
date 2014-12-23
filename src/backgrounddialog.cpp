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
#include <QCheckBox>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMessageBox>
#include <QCoreApplication>

using namespace qfgui;

const QString BackgroundDialog::kReferenceFileExtension = ".qfref";

BackgroundDialog::BackgroundDialog(const QString &imageFileName,
                                   const QPixmap &pixmap,
                                   QWidget *parent) :
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
    mViewPressTimer(),
    mReferenceFileName(QFileInfo(imageFileName + kReferenceFileExtension)
                       .absoluteFilePath()),
    mSaveReferenceFile(new QCheckBox(tr("&Save input data to reference file %1")
                                     .arg(locale().quoteString(mReferenceFileName))))
{
    setWindowTitle(tr("Background Reference"));
    
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
    
    const QIcon autoSetCoordIcon = QIcon::fromTheme("transform-scale");
    QPushButton *autoSetCross1SceneX = new QPushButton(autoSetCoordIcon, "");
    QPushButton *autoSetCross1SceneY = new QPushButton(autoSetCoordIcon, "");
    QPushButton *autoSetCross2SceneX = new QPushButton(autoSetCoordIcon, "");
    QPushButton *autoSetCross2SceneY = new QPushButton(autoSetCoordIcon, "");
    
    const QString autoSetSceneCoordHelp = tr("Set this coordinate automatically for uniform image scaling.\n"
                                             "All pixmap and remaining scene coordinates must be set.");
    autoSetCross1SceneX->setToolTip(autoSetSceneCoordHelp);
    autoSetCross1SceneY->setToolTip(autoSetSceneCoordHelp);
    autoSetCross2SceneX->setToolTip(autoSetSceneCoordHelp);
    autoSetCross2SceneY->setToolTip(autoSetSceneCoordHelp);
    
    autoSetCross1SceneX->setFlat(true);
    autoSetCross1SceneY->setFlat(true);
    autoSetCross2SceneX->setFlat(true);
    autoSetCross2SceneY->setFlat(true);
    
    connect(autoSetCross1SceneX, SIGNAL(clicked()), SLOT(autoSetCross1SceneX()));
    connect(autoSetCross1SceneY, SIGNAL(clicked()), SLOT(autoSetCross1SceneY()));
    connect(autoSetCross2SceneX, SIGNAL(clicked()), SLOT(autoSetCross2SceneX()));
    connect(autoSetCross2SceneY, SIGNAL(clicked()), SLOT(autoSetCross2SceneY()));
    
    QHBoxLayout *cross1PixmapPos = new QHBoxLayout();
    cross1PixmapPos->addWidget(mCross1PixmapX, 1);
    cross1PixmapPos->addWidget(mCross1PixmapY, 1);
    cross1PixmapPos->addWidget(mPlaceCross1Button);
    QHBoxLayout *cross2PixmapPos = new QHBoxLayout();
    cross2PixmapPos->addWidget(mCross2PixmapX, 1);
    cross2PixmapPos->addWidget(mCross2PixmapY, 1);
    cross2PixmapPos->addWidget(mPlaceCross2Button);
    
    QHBoxLayout *cross1ScenePos = new QHBoxLayout();
    cross1ScenePos->addWidget(mCross1SceneX, 1);
    cross1ScenePos->addWidget(autoSetCross1SceneX);
    cross1ScenePos->addWidget(mCross1SceneY, 1);
    cross1ScenePos->addWidget(autoSetCross1SceneY);
    QHBoxLayout *cross2ScenePos = new QHBoxLayout();
    cross2ScenePos->addWidget(mCross2SceneX, 1);
    cross2ScenePos->addWidget(autoSetCross2SceneX);
    cross2ScenePos->addWidget(mCross2SceneY, 1);
    cross2ScenePos->addWidget(autoSetCross2SceneY);
    
    QFormLayout *imagePosLayout = new QFormLayout();
    imagePosLayout->addRow(tr("First image pos:"), cross1PixmapPos);
    imagePosLayout->addRow(tr("Second image pos:"), cross2PixmapPos);
    
    QFormLayout *scenePosLayout = new QFormLayout();
    scenePosLayout->addRow(tr("First scene pos:"), cross1ScenePos);
    scenePosLayout->addRow(tr("Second scene pos:"), cross2ScenePos);
    
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

    mSaveReferenceFile->setChecked(true);
    mSaveReferenceFile->setToolTip(tr("Save input data to reference file (*.%1) in the same folder with image.\n"
                                      "It will be automatically loaded when you open this image again.")
                                   .arg(kReferenceFileExtension));
    
    mainLayout->addLayout(posLayout);
    mainLayout->addWidget(mView);
    mainLayout->addWidget(slider);
    mainLayout->addWidget(mSaveReferenceFile);
    mainLayout->addWidget(mButtons);
    
    connect(mButtons, SIGNAL(accepted()), SLOT(acceptAndSendResult()));
    connect(mButtons, SIGNAL(rejected()), SLOT(reject()));
    
    mCross1->setCursor(Qt::OpenHandCursor);
    mCross2->setCursor(Qt::OpenHandCursor);
    
    tryLoadReferenceFile();
}

void BackgroundDialog::acceptAndSendResult()
{
    const double sx = double(QFrost::sceneUnits(mCross1SceneX->value() - mCross2SceneX->value())) /
                      double(mCross1PixmapX->value() - mCross2PixmapX->value());
    const double sy = double(QFrost::sceneUnits(mCross1SceneY->value() - mCross2SceneY->value())) /
                      double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dx = -sx*double(mCross1PixmapX->value()) + QFrost::sceneUnits(mCross1SceneX->value());
    const double dy = -sy*double(mCross1PixmapY->value()) + QFrost::sceneUnits(mCross1SceneY->value());

    QTransform t;
    t.translate(dx, dy);
    t.scale(sx, sy);

    emit accepted(mPixmapItem->pixmap(), t);

    if (mSaveReferenceFile->isChecked()) {
        saveReferenceFile();
    }

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

bool BackgroundDialog::saveReferenceFile()
{
    QFile file(mReferenceFileName);
    if (file.exists()) {
        if (QMessageBox::question(this, tr("Save Reference File"),
                                  tr("%1 already exists.\nDo you want to replace it?")
                                  .arg(locale().quoteString(mReferenceFileName)))
            != QMessageBox::Yes)
        {
            return false;
        }
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, tr("Save Reference File Failed"),
                             tr("Can not write file %1.")
                             .arg(locale().quoteString(mReferenceFileName))
                            + "\n\n" + file.errorString());
        return false;
    }

    QTextStream out(&file);

    out << mCross1PixmapX->value() << " " << mCross1PixmapY->value() << " "
        << mCross2PixmapX->value() << " " << mCross2PixmapY->value() << "\n"
        << mCross1SceneX->value() << " " << mCross1SceneY->value() << " "
        << mCross2SceneX->value() << " " << mCross2SceneY->value() << "\n";

    QMessageBox::information(this, tr("Saved Reference File"),
                             tr("Saved reference file %1.\n"
                                "It will be loaded when you open this image again.")
                                .arg(locale().quoteString(mReferenceFileName)));

    return true;
}

bool BackgroundDialog::tryLoadReferenceFile()
{
    QFile file(mReferenceFileName);

    if (!file.exists()) {
        return false;
    }

    QWidget *const messageBoxParent = isVisible() ? this : parentWidget();

    const QString loadFailedTitle = tr("Load Reference File Failed");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(messageBoxParent, loadFailedTitle,
                             tr("Reference file %1 exists but can not be opened.")
                             .arg(locale().quoteString(mReferenceFileName))
                             + "\n\n" + file.errorString());
        return false;
    }

    QTextStream in(&file);

    QString token;
    bool ok;
 
    in >> token;
    mCross1PixmapX->setValue(token.toInt(&ok));
    if (!ok || in.atEnd()) {
        onBadInput:
        QMessageBox::warning(messageBoxParent, loadFailedTitle,
                             tr("Reference file %1 has bad format. "
                                "Maybe it was created with other version of %2 or incorrectly modified.")
                             .arg(locale().quoteString(mReferenceFileName))
                             .arg(QCoreApplication::applicationName()));

        mCross1PixmapX->setValue(0);
        mCross1PixmapY->setValue(0);
        mCross2PixmapX->setValue(mPixmapItem->pixmap().width());
        mCross2PixmapY->setValue(mPixmapItem->pixmap().height());

        mCross1SceneX->setValue(0.0);
        mCross1SceneY->setValue(0.0);
        mCross2SceneX->setValue(0.0);
        mCross2SceneY->setValue(0.0);

        return false;
    }

    in >> token;
    mCross1PixmapY->setValue(token.toInt(&ok));
    if (!ok || in.atEnd()) {
        goto onBadInput;
    }

    in >> token;
    mCross2PixmapX->setValue(token.toInt(&ok));
    if (!ok || in.atEnd()) {
        goto onBadInput;
    }

    in >> token;
    mCross2PixmapY->setValue(token.toInt(&ok));
    if (!ok || in.atEnd()) {
        goto onBadInput;
    }

    in >> token;
    mCross1SceneX->setValue(token.toDouble(&ok));
    if (!ok || in.atEnd()) {
        goto onBadInput;
    }

    in >> token;
    mCross1SceneY->setValue(token.toDouble(&ok));
    if (!ok || in.atEnd()) {
        goto onBadInput;
    }

    in >> token;
    mCross2SceneX->setValue(token.toDouble(&ok));
    if (!ok || in.atEnd()) {
        goto onBadInput;
    }

    in >> token;
    mCross2SceneY->setValue(token.toDouble(&ok));
    if (!ok) {
        goto onBadInput;
    }

    // загрузка завершена
    mSaveReferenceFile->setChecked(false);
    QMessageBox::information(messageBoxParent, tr("Loaded Reference File"),
                             tr("Loaded reference data from file %1.")
                             .arg(locale().quoteString(mReferenceFileName)));

    return true;
}

void BackgroundDialog::autoSetCross1SceneX()
{
    const double r = double(mCross1PixmapX->value() - mCross2PixmapX->value()) /
                     double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dx = r * (mCross1SceneY->value() - mCross2SceneY->value());

    mCross1SceneX->setValue(mCross2SceneX->value() + dx);
}

void BackgroundDialog::autoSetCross1SceneY()
{
    const double r = double(mCross1PixmapX->value() - mCross2PixmapX->value()) /
                     double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dy = (mCross1SceneX->value() - mCross2SceneX->value()) / r;

    mCross1SceneY->setValue(mCross2SceneY->value() + dy);
}

void BackgroundDialog::autoSetCross2SceneX()
{
    const double r = double(mCross1PixmapX->value() - mCross2PixmapX->value()) /
                     double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dx = r * (mCross1SceneY->value() - mCross2SceneY->value());

    mCross2SceneX->setValue(mCross1SceneX->value() - dx);
}

void BackgroundDialog::autoSetCross2SceneY()
{
    const double r = double(mCross1PixmapX->value() - mCross2PixmapX->value()) /
                     double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dy = (mCross1SceneX->value() - mCross2SceneX->value()) / r;

    mCross2SceneY->setValue(mCross1SceneY->value() - dy);
}
