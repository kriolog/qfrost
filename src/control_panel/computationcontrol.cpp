/*
 * Copyright (C) 2010-2015  Denis Pesotsky, Maxim Torgonsky
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

#include <control_panel/computationcontrol.h>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QUndoStack>
#include <QtWidgets/QComboBox>

#include <smartdoublespinbox.h>
#include <qfrost.h>
#include <mainwindow.h>
#include <control_panel/dateeditwithundo.h>
#include <units/units.h>
#include <application.h>
#include <graphicsviews/scene.h>

#include <undo/binders/undobinderqcombobox.h>

using namespace qfgui;

ComputationControl::ComputationControl(QWidget *parent)
    : QWidget(parent)
    , mInitialDateEdit(new DateEditWithUndo(QDate(2000, 1, 1),
                                            //: Full undo command text. Nominative case.
                                            QUndoStack::tr("set initial date", "full text") + "\n"
                                            //: Short action name. Accusative case.
                                            + QUndoStack::tr("set initial date", "action text"),
                                            this))
    , mFinalDateEdit(new DateEditWithUndo(QDate(2010, 1, 1),
                                          //: Full undo command text. Nominative case.
                                          QUndoStack::tr("set final date", "full text") + "\n"
                                          //: Short action name. Accusative case.
                                          + QUndoStack::tr("set final date", "action text"),
                                          this))
    , mNumOfStepsSpinBox(new QSpinBox(this))
    , mStatus(new QStackedLayout)
    , mStepLabel(new QLabel(this))
    , mComputationsProgressBar(new QProgressBar(this))
    , mBeginOfComputation()
    , mLoggingMode(new QComboBox(this))
    , mSaveDataMaxY(new SmartDoubleSpinBox(this))
    , mSaveDataMaxYLabel(NULL)
    , mProblemType(new QComboBox(this))
    , mControlButtons(new QStackedLayout)
    , mStartButton(new QPushButton(QIcon::fromTheme("media-playback-start"),
                                   tr("&Start"), this))
    , mStopButton(new QPushButton(QIcon::fromTheme("media-playback-stop"),
                                  tr("&Stop"), this))
    , mNeedToRedrawBlocks(new QCheckBox(tr("&Redraw blocks"), this))
    , mHasBlocks(false)
    , mDatesAreOk(true)
{
    mNumOfStepsSpinBox->setRange(1, 999999);
    mNumOfStepsSpinBox->setValue(32);
    connect(mNumOfStepsSpinBox, SIGNAL(valueChanged(int)),
            SLOT(updateStepInfo(int)));


    mInitialDateEdit->setDisplayFormat(QFrost::dateFormat());
    mFinalDateEdit->setDisplayFormat(QFrost::dateFormat());
    connect(mInitialDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDates()));
    connect(mFinalDateEdit, SIGNAL(dateChanged(QDate)), SLOT(checkDates()));

    connect(mStartButton, SIGNAL(clicked()), SLOT(slotStartComputation()));
    connect(mStopButton, SIGNAL(clicked()), SLOT(slotStopComputation()));

    connect(mNeedToRedrawBlocks, SIGNAL(stateChanged(int)),
            this, SLOT(slotNeedBlocksRedrawing(int)));

    connect(Application::findMainWindow(this)->qfScene(), SIGNAL(blocksCountChanged(int)),
            this, SLOT(onBlocksCountChanged(int)));

    mControlButtons->addWidget(mStartButton);
    mControlButtons->addWidget(mStopButton);

    mStatus->addWidget(mStepLabel);
    mStatus->addWidget(mComputationsProgressBar);

    QGroupBox *computationBox = new QGroupBox(tr("Computations Control"), this);
    QFormLayout *formLayout = new QFormLayout(computationBox);
    ////////////////////////////////////////////////////////////////////////////
    // сначала те виджеты, которые активны только до рассчётов
    formLayout->setRowWrapPolicy(QFormLayout::WrapLongRows);
    formLayout->addRow(tr("&Initial date:"), mInitialDateEdit);
    formLayout->addRow(tr("Fina&l date:"), mFinalDateEdit);
    formLayout->addRow(mProblemType);
    formLayout->addRow(tr("S&teps per day:"), mNumOfStepsSpinBox);
    formLayout->addRow(mStatus);
    formLayout->addRow(mLoggingMode);
    //: Log and save data...
    formLayout->addRow(tr("… &down to:"), mSaveDataMaxY);

    mSaveDataMaxYLabel = formLayout->labelForField(mSaveDataMaxY);
    Q_ASSERT(mSaveDataMaxYLabel != NULL);

    mLoggingMode->addItem(tr("Logging Off"));
    mLoggingMode->addItem(tr("Monthly Logging"));
    mLoggingMode->addItem(tr("Daily Logging"));
    connect(mLoggingMode, SIGNAL(currentIndexChanged(int)),
            SLOT(updateSaveDataMaxYVisibility(int)));
    updateSaveDataMaxYVisibility(mLoggingMode->currentIndex());

    mProblemType->addItem(tr("Flat Problem"));
    mProblemType->addItem(tr("Axisymmetric Problem"));
    mProblemType->setItemData(0,
                              //: Full undo command text. Nominative case.
                              QUndoStack::tr("disabling axisymmetry", "full text") + "\n"
                              //: Short action name. Accusative case.
                              + QUndoStack::tr("disabling axisymmetry", "action text"),
                              QFrost::UndoTextRole);
    mProblemType->setItemData(1,
                              //: Full undo command text. Nominative case.
                              QUndoStack::tr("enabling axisymmetry", "full text") + "\n"
                              //: Short action name. Accusative case.
                              + QUndoStack::tr("enabling axisymmetry", "action text"),
                              QFrost::UndoTextRole);
    new UndoBinderQComboBox(Application::findMainWindow(parent)->undoStack(), mProblemType);

    // а теперь те, которые активны всегда
    formLayout->addRow(mControlButtons);
    formLayout->addRow(mNeedToRedrawBlocks);
    ////////////////////////////////////////////////////////////////////////////
    mSaveDataMaxY->setMinimum(-QFrost::sceneHalfSizeInMeters);
    mSaveDataMaxY->setMaximum(QFrost::sceneHalfSizeInMeters);
    mSaveDataMaxY->setValue(5);
    mSaveDataMaxY->setSuffix(Units::meterSuffix());
    ////////////////////////////////////////////////////////////////////////////
    QGroupBox *stylesBox = new QGroupBox(tr("Blocks Style"), this);

    QButtonGroup *styles = new QButtonGroup(this);
    QAbstractButton *styleTemp = new QRadioButton(tr("&Temperature"), stylesBox);
    QAbstractButton *styleTempDiff = new QRadioButton(tr("Temperature &minus Tbf"), stylesBox);
    QAbstractButton *styleMixed = new QRadioButton(tr("&Mixed"), stylesBox);
    QAbstractButton *styleWater = new QRadioButton(tr("T&hawed volume fraction"), stylesBox);

    styles->addButton(styleTemp, QFrost::blockShowsTemperatureField);
    styles->addButton(styleTempDiff, QFrost::blockShowsTemperatureDiffField);
    styles->addButton(styleMixed, QFrost::blockShowsConditionField);
    styles->addButton(styleWater, QFrost::blockShowsThawedPartField);

    styleTemp->setToolTip(tr("Color of block represents temperature"));
    styleTempDiff->setToolTip(tr("Color of block represents temperature relative to phase transition"));
    styleMixed->setToolTip(tr("If block is at phase transition,<br/>"
                              "it's color repesents thawed volume fraction;<br/>"
                              "else it represents temperature"));
    styleWater->setToolTip(tr("Color of block represents thawed volume fraction"));

    connect(styles, SIGNAL(buttonClicked(int)),
            parent, SLOT(changeBlocksStyleForComputation(int)));

    styleTemp->click();

    QVBoxLayout *stylesLayout = new QVBoxLayout(stylesBox);
    //stylesLayout->setSizeConstraint(QLayout::SetMinimumSize);
    stylesLayout->addWidget(styleTemp);
    stylesLayout->addWidget(styleTempDiff);
    stylesLayout->addWidget(styleWater);
    stylesLayout->addWidget(styleMixed);

    ////////////////////////////////////////////////////////////////////////////

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addStretch();
    mainLayout->addWidget(computationBox);
    mainLayout->addStretch();
    mainLayout->addWidget(stylesBox);
    mainLayout->addStretch();

    onComputationStateChanged(false);
    onBlocksCountChanged(0);
    updateStepInfo(mNumOfStepsSpinBox->value());
    mNeedToRedrawBlocks->setChecked(true);

    mNumOfStepsSpinBox->setToolTip(tr("Number of steps to do in each astronomical day"));
    mInitialDateEdit->setToolTip(tr("Initial date for computation.<br/>"
                                    "When computation is active, it shows represented date."));
    mFinalDateEdit->setToolTip(tr("Final date for computation"));
    mStartButton->setToolTip(tr("Start computation"));
    mStopButton->setToolTip(tr("Stop computation"));
    mNeedToRedrawBlocks->setToolTip(tr("Continuously redraw blocks when computation is active"));
}

void ComputationControl::slotStartComputation()
{
    mStartButton->setDisabled(true);
    ComputationSettings settings;
    settings.initialDate = mInitialDateEdit->date();
    settings.finalDate = mFinalDateEdit->date();
    settings.stepsInDay = mNumOfStepsSpinBox->value();
    settings.needBlocksRedrawing = mNeedToRedrawBlocks->isChecked();
    settings.loggingMode = static_cast<ComputationSettings::LoggingMode>(mLoggingMode->currentIndex());
    settings.saveDataMaxY = mSaveDataMaxY->value();
    settings.isAxiallySymmetric = (mProblemType->currentIndex() == 1);
    emit signalStartComputation(settings);
}

void ComputationControl::onBlocksCountChanged(int blocksCount)
{
    Q_ASSERT(mInitialDateEdit->isEnabled()); // расчёты сейчас не могут идти
    mHasBlocks = (blocksCount != 0);
    updateStartButton();
}

void ComputationControl::slotStopComputation()
{
    mStopButton->setDisabled(true);
    emit signalStopComputation();
}

void ComputationControl::onComputationStateChanged(bool computationIsNowOn)
{
    mInitialDateEdit->setDisabled(computationIsNowOn);
    mFinalDateEdit->setDisabled(computationIsNowOn);
    mNumOfStepsSpinBox->setDisabled(computationIsNowOn);
    mLoggingMode->setDisabled(computationIsNowOn);
    mSaveDataMaxY->setDisabled(computationIsNowOn);
    mProblemType->setDisabled(computationIsNowOn);

    // включаем обе кнопки, ибо они выключаются при нажатии на них
    mStopButton->setEnabled(true);
    mStartButton->setEnabled(true);

    mControlButtons->setCurrentIndex(computationIsNowOn);
    mStatus->setCurrentIndex(computationIsNowOn);

    if (computationIsNowOn) {
        mBeginOfComputation = mInitialDateEdit->date();
        // изначально показываем, что просто заняты (а когда поймаем первый
        // сигнал, там установится нужный максимум, см. соответствующий слот)
        mComputationsProgressBar->setRange(0, 0);
    } else {
        mComputationsProgressBar->reset();
        checkDates();
    }
}

void ComputationControl::checkDates()
{
    mDatesAreOk = (mInitialDateEdit->date() < mFinalDateEdit->date());
    updateStartButton();
}

void ComputationControl::updateStartButton()
{
    mStartButton->setEnabled(mDatesAreOk && mHasBlocks);
}

void ComputationControl::updateStepInfo(int numOfStepsInDay)
{
    if (mStepLabel->isHidden()) {
        return;
    }
    Q_ASSERT(numOfStepsInDay > 0);
    QString s;
    s += "<i>";

    const QPair<QString, bool> singleStep = QFrost::singleStepInfo(numOfStepsInDay);
    //: %1 is equality symbol (= or \342\211\210), %2 is time (HH:mm:ss)
    s += tr("Single step %1 %2")
         .arg(singleStep.second ? "=" : "\342\211\210")
         .arg(singleStep.first);
    /*s += "<br/>";
    int stepsTotal;
    stepsTotal = mInitialDateEdit->date().daysTo(mFinalDateEdit->date());
    stepsTotal *= numOfStepsInDay;
    if(stepsTotal > 0) {
        s += tr("%n step(s) will be done", "", stepsTotal);
    }*/
    s += "</i>";
    mStepLabel->setText(s);
}

void ComputationControl::slotComputationDateChanged(const QDate &date)
{
    Q_ASSERT(!date.isNull());
    Q_ASSERT(date.isValid());
    Q_ASSERT(mFinalDateEdit->date() >= date);
    if (mComputationsProgressBar->isVisible()) {
        // после первого пойманного сигнала установим нормальный максимум
        if (mComputationsProgressBar->maximum() == 0) {
            int m = mInitialDateEdit->date().daysTo(mFinalDateEdit->date());
            mComputationsProgressBar->setRange(0, m);
        }
        mComputationsProgressBar->setValue(mBeginOfComputation.daysTo(date));
    }
}

void ComputationControl::slotNeedBlocksRedrawing(int state)
{
    emit signalNeedBlocksRedrawing(state);
}

void ComputationControl::load(QDataStream &in)
{
    Q_ASSERT(in.status() == QDataStream::Ok);
    mInitialDateEdit->load(in);
    mFinalDateEdit->load(in);

    qint32 numSteps;
    in >> numSteps;

    mNumOfStepsSpinBox->setValue(numSteps);

    bool isAxiallySymmetric;
    in >> isAxiallySymmetric;
    mProblemType->setProperty(QFrost::UndoBinderIsEnabled, false);
    mProblemType->setCurrentIndex(isAxiallySymmetric ? 1 : 0);
    mProblemType->setProperty(QFrost::UndoBinderIsEnabled, true);

    if (in.status() != QDataStream::Ok) {
        qWarning("Load failed at ComputationControl::load()");
        throw false;
    }
}

void ComputationControl::save(QDataStream &out)
{
    mInitialDateEdit->save(out);
    mFinalDateEdit->save(out);
    out << static_cast<qint32>(mNumOfStepsSpinBox->value())
        << (mProblemType->currentIndex() == 1);
}

void ComputationControl::updateSaveDataMaxYVisibility(int index)
{
    bool mustBeVisible = (index != ComputationSettings::NoLogging);
    mSaveDataMaxY->setVisible(mustBeVisible);
    mSaveDataMaxYLabel->setVisible(mustBeVisible);
}

QDate ComputationControl::currentDate() const
{
    return mInitialDateEdit->date();
}
