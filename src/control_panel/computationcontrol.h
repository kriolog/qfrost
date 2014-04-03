/*
 * Copyright (C) 2010-2012  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_COMPUTATIONCONTROL_H
#define QFGUI_COMPUTATIONCONTROL_H

#include <QtWidgets/QWidget>
#include <QtCore/QDate>

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QStackedLayout)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QDateEdit)
QT_FORWARD_DECLARE_CLASS(QProgressBar)
QT_FORWARD_DECLARE_CLASS(QComboBox)

namespace qfgui
{

/**
 * Параметры, необходимые для старта рассчётов.
 */
struct ComputationSettings {
    enum LoggingMode {
        NoLogging,
        MonthlyLogging,
        DailyLogging
    };
    QDate initialDate;
    QDate finalDate;
    int stepsInDay;
    bool needBlocksRedrawing;
    LoggingMode loggingMode;
    bool isAxiallySymmetric;
    double saveDataMaxY;
};

QT_FORWARD_DECLARE_CLASS(DateEditWithUndo)
QT_FORWARD_DECLARE_CLASS(SmartDoubleSpinBox)
/**
 * Виджет для старта рассчётов и для управления ими.
 */
class ComputationControl : public QWidget
{
    Q_OBJECT
public:
    ComputationControl(QWidget *parent = NULL);

    void onComputationStateChanged(bool computationIsNowOn);

    void save(QDataStream &out);

    void load(QDataStream &in);

    QComboBox *problemTypeComboBox() const {
        return mProblemType;
    }

private:
    DateEditWithUndo *mInitialDateEdit;
    DateEditWithUndo *mFinalDateEdit;
    QSpinBox *mNumOfStepsSpinBox;
    QStackedLayout *mStatus;
    QLabel *mStepLabel;
    QProgressBar *mComputationsProgressBar;
    /// Дата, когда начались рассчёты (нужно для прогресса)
    QDate mBeginOfComputation;
    QComboBox *mLoggingMode;
    SmartDoubleSpinBox *mSaveDataMaxY;
    QWidget *mSaveDataMaxYLabel;

    QComboBox *mProblemType;

    QStackedLayout *mControlButtons;
    QPushButton *mStartButton;
    QPushButton *mStopButton;

    QCheckBox *mNeedToRedrawBlocks;

    bool mHasBlocks;
    bool mDatesAreOk;

    void updateStartButton();

private slots:
    void slotStartComputation();
    void slotStopComputation();
    /// Если дата окончания больше даты начала, блокирует кнопку старта
    void checkDates();
    /// Обновляет текстовую информацию о шагах
    void updateStepInfo(int numOfStepsInDay);
    /// Обновляет прогрессбар
    void slotComputationDateChanged(const QDate &date);
    void slotNeedBlocksRedrawing(int state);

    void onBlocksCountChanged(int blocksCount);

    /// Обновляет видимость строки с глубиной журналирования
    void updateSaveDataMaxYVisibility(int index);

signals:
    void signalStartComputation(const ComputationSettings &settings);
    void signalStopComputation();
    void signalNeedBlocksRedrawing(bool needRedrawing);

    friend class ReadFromComputationDataCommand;
};

}

#endif // QFGUI_COMPUTATIONCONTROL_H
