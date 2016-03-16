/*
 * Copyright (C) 2014-2016  Denis Pesotsky
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
#include <graphicsviews/zoomslider.h>

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QSlider>
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
#include <qmath.h>
#include <QLabel>

using namespace qfgui;

const QString BackgroundDialog::kReferenceFileExtension = ".qfref";

static QLayout *xyLayout(QWidget *xWidget, QWidget *yWidget)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    
    QHBoxLayout *xLayout = new QHBoxLayout();
    xLayout->addWidget(new QLabel("X:"));
    xLayout->addWidget(xWidget, 1);
    
    QHBoxLayout *yLayout = new QHBoxLayout();
    yLayout->addWidget(new QLabel("Y:"));
    yLayout->addWidget(yWidget, 1);
    
    mainLayout->addStretch(1);
    mainLayout->addLayout(xLayout);
    mainLayout->addStretch(1);
    mainLayout->addLayout(yLayout);
    mainLayout->addStretch(1);
    
    return mainLayout;
}

static QLayout *xyLayout(QWidget *xWidget1, QWidget *xWidget2,
                         QWidget *yWidget1, QWidget *yWidget2)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    
    QHBoxLayout *xLayout = new QHBoxLayout();
    xLayout->addWidget(new QLabel("X:"));
    xLayout->addWidget(xWidget1, 1);
    xLayout->addWidget(xWidget2);
    
    QHBoxLayout *yLayout = new QHBoxLayout();
    yLayout->addWidget(new QLabel("Y:"));
    yLayout->addWidget(yWidget1, 1);
    yLayout->addWidget(yWidget2);
    
    mainLayout->addStretch(1);
    mainLayout->addLayout(xLayout);
    mainLayout->addStretch(1);
    mainLayout->addLayout(yLayout);
    mainLayout->addStretch(1);
    
    return mainLayout;
}

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
    mCross1PixmapX->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    mCross2PixmapX->setMinimum(0);
    mCross2PixmapX->setMaximum(pixmap.width());
    mCross2PixmapX->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    mCross1PixmapY->setMinimum(0);
    mCross1PixmapY->setMaximum(pixmap.height());
    mCross1PixmapY->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    mCross2PixmapY->setMinimum(0);
    mCross2PixmapY->setMaximum(pixmap.height());
    mCross2PixmapY->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

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

    const QString setAutoSceneCoordTip = tr("Set this coordinate automatically for scaling with given factor ratio.\n"
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
    //imagePos1Box->setFlat(true);
    QHBoxLayout *imagePos1Layout = new QHBoxLayout(imagePos1Box);
    imagePos1Layout->addLayout(xyLayout(mCross1PixmapX, mCross1PixmapY), 1);
    imagePos1Layout->addWidget(mPlaceCross1Button);

    QGroupBox *imagePos2Box = new QGroupBox(tr("Image Position 2"));
    //imagePos2Box->setFlat(true);
    QHBoxLayout *imagePos2Layout = new QHBoxLayout(imagePos2Box);
    imagePos2Layout->addLayout(xyLayout(mCross2PixmapX, mCross2PixmapY), 1);
    imagePos2Layout->addWidget(mPlaceCross2Button);

    QGroupBox *scenePos1Box = new QGroupBox(tr("Domain Position 1"));
    //scenePos1Box->setFlat(true);
    scenePos1Box->setLayout(xyLayout(mCross1SceneX, autoSetCross1SceneX,
                                     mCross1SceneY, autoSetCross1SceneY));

    QGroupBox *scenePos2Box = new QGroupBox(tr("Domain Position 2"));
    //scenePos2Box->setFlat(true);
    scenePos2Box->setLayout(xyLayout(mCross2SceneX, autoSetCross2SceneX,
                                     mCross2SceneY, autoSetCross2SceneY));

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

    ZoomSlider *slider = mView->createZoomSlider(this);
    Q_ASSERT(slider->slider()->minimum() == -slider->slider()->maximum());

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

    if (qAbs(qLn((dxScene/dyScene)/(dxImage/dyImage))) > qLn(100.5)) {
        // соотношение сторон изменяется более чем в 100.5...
        if (QMessageBox::question(this,
                                  tr("Scale Factor Ratio Changes too Much"),
                                  tr("Background aspect factor ratio (X:Y) will too much differ from original. "
                                     "It can lead to fuzzy lines on it.\n\nIf you have vector version of image, it is recommended "
                                     "to export bitmap with uniform scaling (1:1) before loading it in %1.\n\n"
                                     "Are you sure to continue and use flattened background?")
                                  .arg(QCoreApplication::applicationName()))
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

/// Открывает диалог выбора соотношения сторон x:y и возвращает результат.
/// Соотношение задаётся двумя целыми числами. 
/// Если диалог был отменён, возвращается отрицательное число.
static double getAxesScaleFactorRatio(int crossNum, Qt::Orientation axeOrientation, QWidget *parent = 0)
{
    const QChar axeLetter = axeOrientation == Qt::Horizontal ? 'x' : 'y';

    QDialog *dialog = new QDialog(parent);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dialog);
    QObject::connect(buttons, SIGNAL(accepted()), dialog, SLOT(accept()));

    QSpinBox *xVal = new QSpinBox(dialog);
    QSpinBox *yVal = new QSpinBox(dialog);
    xVal->setMinimum(1);
    yVal->setMinimum(1);
    static const int maxVal = 99999;
    xVal->setMaximum(maxVal);
    yVal->setMaximum(maxVal);

    QHBoxLayout *valLayout = new QHBoxLayout();
    valLayout->addWidget(xVal, 1);
    valLayout->addWidget(new QLabel(":"));
    valLayout->addWidget(yVal, 1);
    valLayout->setContentsMargins(QMargins());

    QLabel *l = new QLabel(QObject::tr("You requested auto setting %1 coordinate (meters) value for cross %2."
                                       "Please input background scale factor ratio (X:Y). It will be used besides "
                                       "all other coordinates you've entered in main dialog window.<br><br>"
                                       "<b>Warning!</b> If you have vector version of background, it is recommended "
                                       "to export it with uniform scaling (and input 1:1 here).")
                           .arg(axeLetter).arg(crossNum));
    l->setWordWrap(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
    mainLayout->addWidget(l);
    mainLayout->addLayout(valLayout);
    mainLayout->addWidget(buttons);

    if (dialog->exec() == QDialog::Accepted) {
        return double(xVal->value())/double(yVal->value());
    } else {
        return -420;
    }
}

void BackgroundDialog::autoSetCross1SceneX()
{
    const double d = getAxesScaleFactorRatio(1, Qt::Horizontal);
    if (d > 0.0) {
        autoSetCross1SceneX(d);
    }
}

void BackgroundDialog::autoSetCross1SceneY()
{
    const double d = getAxesScaleFactorRatio(1, Qt::Vertical);
    if (d > 0.0) {
        autoSetCross1SceneY(d);
    }
}

void BackgroundDialog::autoSetCross2SceneX()
{
    const double d = getAxesScaleFactorRatio(2, Qt::Horizontal);
    if (d > 0.0) {
        autoSetCross2SceneX(d);
    }
}

void BackgroundDialog::autoSetCross2SceneY()
{
    const double d = getAxesScaleFactorRatio(2, Qt::Vertical);
    if (d > 0.0) {
        autoSetCross2SceneY(d);
    }
}

void BackgroundDialog::autoSetCross1SceneX(double d)
{
    const double r = double(mCross1PixmapX->value() - mCross2PixmapX->value()) /
                     double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dx = r * d * (mCross1SceneY->value() - mCross2SceneY->value());

    mCross1SceneX->setValue(mCross2SceneX->value() + dx);
}

void BackgroundDialog::autoSetCross1SceneY(double d)
{
    const double r = double(mCross1PixmapX->value() - mCross2PixmapX->value()) /
                     double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dy = (mCross1SceneX->value() - mCross2SceneX->value()) / r / d;

    mCross1SceneY->setValue(mCross2SceneY->value() + dy);
}

void BackgroundDialog::autoSetCross2SceneX(double d)
{
    const double r = double(mCross1PixmapX->value() - mCross2PixmapX->value()) /
                     double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dx = r * d * (mCross1SceneY->value() - mCross2SceneY->value());

    mCross2SceneX->setValue(mCross1SceneX->value() - dx);
}

void BackgroundDialog::autoSetCross2SceneY(double d)
{
    const double r = double(mCross1PixmapX->value() - mCross2PixmapX->value()) /
                     double(mCross1PixmapY->value() - mCross2PixmapY->value());

    const double dy = (mCross1SceneX->value() - mCross2SceneX->value()) / r / d;

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
