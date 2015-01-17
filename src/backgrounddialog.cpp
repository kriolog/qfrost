/*
 * Copyright (C) 2014-2015  Denis Pesotsky
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
#include <QGroupBox>
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
#include <QTimer>
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
    mCross1SceneX(PhysicalPropertySpinBox::createSceneCoordinateSpinBox(this)),
    mCross1SceneY(PhysicalPropertySpinBox::createSceneCoordinateSpinBox(this)),
    mCross2SceneX(PhysicalPropertySpinBox::createSceneCoordinateSpinBox(this)),
    mCross2SceneY(PhysicalPropertySpinBox::createSceneCoordinateSpinBox(this)),
    mPlaceCross1Button(new QPushButton()),
    mPlaceCross2Button(new QPushButton()),
    mIsPlacingCross1(false),
    mIsPlacingCross2(false),
    mViewPressTimer(),
    mReferenceFileName(QFileInfo(imageFileName + kReferenceFileExtension)
                       .absoluteFilePath()),
    mSaveReferenceFile(new QCheckBox(tr("&Save input data to reference file %1")
                                     .arg(locale().quoteString(mReferenceFileName)))),
    mNeedReferenceFileNotification(false)
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
    
    const QIcon setWithMouseIcon = QIcon::fromTheme("transform-move");
    mPlaceCross1Button->setIcon(setWithMouseIcon);
    mPlaceCross2Button->setIcon(setWithMouseIcon);
    
    // HACK Пришлось отнимать число > 12, чтобы не вышла слишком большая кнопка,
    //      превышающая по высоте соответствующую ей форму (с парой спинбоксов).
    //      Вероятно, это связано с CSS padding/margin кнопки, но как их узнать?
    const int maxSetWithMouseHeight = mCross1PixmapX->height() + mCross1PixmapY->height() - 14;
    const QSize setWithMouseIconSize = QFrost::upperBoundIconSize(setWithMouseIcon, maxSetWithMouseHeight);
    mPlaceCross1Button->setIconSize(setWithMouseIconSize);
    mPlaceCross2Button->setIconSize(setWithMouseIconSize);
    
    const QString setWithMouseTip = tr("Set both coordinates for cross by placing cursor over image.\n"
                                       "Cross will follow mouse in image area. Left click to finish.");
    mPlaceCross1Button->setToolTip(setWithMouseTip);
    mPlaceCross2Button->setToolTip(setWithMouseTip);
    
    const QIcon setAutoCoordIcon = QIcon::fromTheme("transform-scale");
    QPushButton *autoSetCross1SceneX = new QPushButton(setAutoCoordIcon, "");
    QPushButton *autoSetCross1SceneY = new QPushButton(setAutoCoordIcon, "");
    QPushButton *autoSetCross2SceneX = new QPushButton(setAutoCoordIcon, "");
    QPushButton *autoSetCross2SceneY = new QPushButton(setAutoCoordIcon, "");
    
    const QString setAutoSceneCoordTip = tr("Set this coordinate automatically for uniform image scaling.\n"
                                            "All the rest coordinates (4 pixmap & 3 domain) must be set.");
    autoSetCross1SceneX->setToolTip(setAutoSceneCoordTip);
    autoSetCross1SceneY->setToolTip(setAutoSceneCoordTip);
    autoSetCross2SceneX->setToolTip(setAutoSceneCoordTip);
    autoSetCross2SceneY->setToolTip(setAutoSceneCoordTip);
    
    autoSetCross1SceneX->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    autoSetCross1SceneY->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    autoSetCross2SceneX->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    autoSetCross2SceneY->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    
    connect(autoSetCross1SceneX, SIGNAL(clicked()), SLOT(autoSetCross1SceneX()));
    connect(autoSetCross1SceneY, SIGNAL(clicked()), SLOT(autoSetCross1SceneY()));
    connect(autoSetCross2SceneX, SIGNAL(clicked()), SLOT(autoSetCross2SceneX()));
    connect(autoSetCross2SceneY, SIGNAL(clicked()), SLOT(autoSetCross2SceneY()));
    
    QGroupBox *imagePos1Box = new QGroupBox(tr("Image Position 1"));
    imagePos1Box->setFlat(true);
    QFormLayout *imagePos1CoordsLayout = new QFormLayout();
    imagePos1CoordsLayout->addRow(tr("X:"), mCross1PixmapX);
    imagePos1CoordsLayout->addRow(tr("Y:"), mCross1PixmapY);
    QHBoxLayout *imagePos1Layout = new QHBoxLayout(imagePos1Box);
    imagePos1Layout->addLayout(imagePos1CoordsLayout, 1);
    imagePos1Layout->addWidget(mPlaceCross1Button);
    
    QGroupBox *imagePos2Box = new QGroupBox(tr("Image Position 2"));
    imagePos2Box->setFlat(true);
    QFormLayout *imagePos2CoordsLayout = new QFormLayout();
    imagePos2CoordsLayout->addRow(tr("X:"), mCross2PixmapX);
    imagePos2CoordsLayout->addRow(tr("Y:"), mCross2PixmapY);
    QHBoxLayout *imagePos2Layout = new QHBoxLayout(imagePos2Box);
    imagePos2Layout->addLayout(imagePos2CoordsLayout, 1);
    imagePos2Layout->addWidget(mPlaceCross2Button);
    mPlaceCross1Button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mPlaceCross2Button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    
    QHBoxLayout *scenePos1XLayout = new QHBoxLayout();
    scenePos1XLayout->setMargin(0);
    scenePos1XLayout->addWidget(mCross1SceneX, 1);
    scenePos1XLayout->addWidget(autoSetCross1SceneX);
    
    QHBoxLayout *scenePos1YLayout = new QHBoxLayout();
    scenePos1YLayout->setMargin(0);
    scenePos1YLayout->addWidget(mCross1SceneY, 1);
    scenePos1YLayout->addWidget(autoSetCross1SceneY);
    
    QHBoxLayout *scenePos2XLayout = new QHBoxLayout();
    scenePos2XLayout->setMargin(0);
    scenePos2XLayout->addWidget(mCross2SceneX, 1);
    scenePos2XLayout->addWidget(autoSetCross2SceneX);
    
    QHBoxLayout *scenePos2YLayout = new QHBoxLayout();
    scenePos2YLayout->setMargin(0);
    scenePos2YLayout->addWidget(mCross2SceneY, 1);
    scenePos2YLayout->addWidget(autoSetCross2SceneY);
    
    QGroupBox *scenePos1Box = new QGroupBox(tr("Domain Position 1"));
    scenePos1Box->setFlat(true);
    QFormLayout *scenePos1Layout = new QFormLayout(scenePos1Box);
    scenePos1Layout->addRow(tr("X:"), scenePos1XLayout);
    scenePos1Layout->addRow(tr("Y:"), scenePos1YLayout);
    
    QGroupBox *scenePos2Box = new QGroupBox(tr("Domain Position 2"));
    scenePos2Box->setFlat(true);
    QFormLayout *scenePos2Layout = new QFormLayout(scenePos2Box);
    scenePos2Layout->addRow(tr("X:"), scenePos2XLayout);
    scenePos2Layout->addRow(tr("Y:"), scenePos2YLayout);
    
    QGroupBox *imagePosBox = new QGroupBox(tr("Anchor Points on Image (Coordinates in Pixels)"));
    QHBoxLayout *imagePosLayout = new QHBoxLayout(imagePosBox);
    imagePosLayout->addWidget(imagePos1Box);
    imagePosLayout->addWidget(imagePos2Box);
    
    QGroupBox *scenePosBox = new QGroupBox(tr("Anchor Points on Domain (Coordinates in Meters)"));
    QHBoxLayout *scenePosLayout = new QHBoxLayout(scenePosBox);
    scenePosLayout->addWidget(scenePos1Box);
    scenePosLayout->addWidget(scenePos2Box);
    
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
    posLayout->addWidget(imagePosBox, 1);
    posLayout->addWidget(scenePosBox, 1);

    mSaveReferenceFile->setChecked(true);
    mSaveReferenceFile->setToolTip(tr("Save input data to reference file (*.%1) in the same folder with image.\n"
                                      "It will be automatically loaded when you open this image again.")
                                   .arg(kReferenceFileExtension));
    
    mainLayout->addLayout(posLayout);
    mainLayout->addWidget(mSaveReferenceFile);
    mainLayout->addWidget(mView);
    mainLayout->addWidget(slider);
    mainLayout->addWidget(mButtons);
    
    connect(mButtons, SIGNAL(accepted()), SLOT(acceptAndSendResult()));
    connect(mButtons, SIGNAL(rejected()), SLOT(reject()));
    
    mCross1->setCursor(Qt::OpenHandCursor);
    mCross2->setCursor(Qt::OpenHandCursor);
    
    tryLoadReferenceFile();
}

void BackgroundDialog::acceptAndSendResult()
{
    const double dxScene = mCross1SceneX->value() - mCross2SceneX->value();
    const double dyScene = mCross1SceneY->value() - mCross2SceneY->value();

    const double dxImage = mCross1PixmapX->value() - mCross2PixmapX->value();
    const double dyImage = mCross1PixmapY->value() - mCross2PixmapY->value();

    if (qAbs(dxScene/dyScene - dxImage/dyImage) > 0.01) {
        if (QMessageBox::question(this,
                                  tr("Non-Uniform Scale"),
                                  tr("Background aspect ratio (as it's shown in domain) will differ from original. "
                                     "For uniform scaling use any of four buttons next to domain pos input fields.\n\n"
                                     "Are you sure to continue and show non-uniformly scaled background?"))
            != QMessageBox::Yes) {
            return;
        }
    }
 
    const double xScale = double(QFrost::sceneUnits(dxScene)) / dxImage;
    const double yScale = double(QFrost::sceneUnits(dyScene)) / dyImage;

    const double xShift = -xScale * double(mCross1PixmapX->value()) + QFrost::sceneUnits(mCross1SceneX->value());
    const double yShift = -yScale * double(mCross1PixmapY->value()) + QFrost::sceneUnits(mCross1SceneY->value());

    QTransform t;
    t.translate(xShift, yShift);
    t.scale(xScale, yScale);

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

    /******************* Загрузка файла привязки успешна... *******************/
    // ... снимем галку сохранения файла привязки, чтобы не перезаписывать...
    mSaveReferenceFile->setChecked(false);
    // ... добавим к тексту этой галочки примечание о замене старого файла...
    mSaveReferenceFile->setText(mSaveReferenceFile->text() + " " + tr("(replace old one)"));
    // ... и уведомим пользователя о загрузке файла (или же запланируем это).
    if (isVisible()) {
        QTimer::singleShot(0, this, SLOT(showReferenceFileNotification()));
    } else {
        mNeedReferenceFileNotification = true;
    }

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

void BackgroundDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    if (mNeedReferenceFileNotification) {
        // Используем задержку (хотя бы 100 мс), чтобы окно хорошо расположилось
        QTimer::singleShot(250, this, SLOT(showReferenceFileNotification()));
    }
}

void BackgroundDialog::showReferenceFileNotification()
{
    Q_ASSERT(mNeedReferenceFileNotification);

    QMessageBox::information(this, tr("Loaded Reference File"),
                             tr("Loaded corresponding reference file from image directory.\n"
                                "It is recommended to check coordinates before proceeding."));
}
