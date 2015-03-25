/*
 * Copyright (C) 2010-2012  Denis Pesotsky
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

#include <computations/computationthread.h>

#include <cmath>

#include <QtCore/qmath.h>

#include <geometry/block_within_polygon.h>
#include <geometry/clip_polyline.h>
#include <mainwindow.h>

#include <computations/blockslogger.h>

#include <control_panel/computationcontrol.h>
using namespace qfgui;

ComputationThread::ComputationThread(Scene *scene,
                                     const ComputationSettings &settings)
    : QThread()
    , mDomain()
    , mStepsInDay(settings.stepsInDay)
    , mInitialDate(settings.initialDate)
    , mFinalDate(settings.finalDate)
    , mIsAxiallySymmetric(settings.isAxiallySymmetric)
    , mSceneIsReadyForRedraw(true)
    , mSceneWantsNewData(false /* сцена сама попросит первые данные */)
    , mComputationData()
    , mScene(scene)
    , mUsedBlocks()
    , mMustStop(false)
    , mNeedBlocksRedrawing(settings.needBlocksRedrawing)
    , mLoggingMode(settings.loggingMode)
    , mLoggerMaxY(settings.saveDataMaxY)
{
}

void ComputationThread::run()
{
    {
        QString errorText;
        errorText = setDataInDomain(mScene->blocks(),
                                    mScene->outerBoundaryPolygons());
        if (!errorText.isNull()) {
            emit badStart(errorText);
            return;
        }
        mScene->setBlocksInDomain(mUsedBlocks);
    }

    Q_ASSERT(mStepsInDay > 0);
    Q_ASSERT(mInitialDate.daysTo(mFinalDate) >= 1);
    qDebug("ComputationThread::run(): setting time step");
    mDomain.setTimeStep(86400.0 / mStepsInDay);

    qDebug("ComputationThread::run(): starting computations");

    BlocksLogger logger(mUsedBlocks, mLoggerMaxY, mScene->mainWindow());

    /// текущий день рассчётов
    QDate date;
    /// день, данные о котором были в сцене перед отправлением очередных данных
    QDate dateOfPreviousData = mInitialDate;

    // идём от начальной даты до предпоследней => переходим в начало последней
    for (date = mInitialDate; date != mFinalDate; date = date.addDays(1)) {
        if (mLoggingMode == ComputationSettings::DailyLogging) {
            // запоминаем все дни с первого по предпоследний...
            logger.addData(ComputationData(mDomain, date, dateOfPreviousData));
        }
        if (date.day() == 1) {
            if (mLoggingMode == ComputationSettings::MonthlyLogging) {
                logger.addData(ComputationData(mDomain, date, dateOfPreviousData));
            }
            // Каждый месяц пробуем выслать сцене новые данные...
            if (mNeedBlocksRedrawing && mSceneWantsNewData && mSceneIsReadyForRedraw) {
                mSceneIsReadyForRedraw = false;
                mSceneWantsNewData = false;
                mComputationData = ComputationData(mDomain,
                                                   date,
                                                   dateOfPreviousData);
                emit signalNewDataIsReady(mComputationData);
                dateOfPreviousData = date;
            }
        }
        mDomain.setDate(date.year(), date.month(), date.day());
        emit dateChanged(date);

        // Только теперь делаем шаги, что переведёт нас к началу завтрашнего дня
        mDomain.doSteps(mStepsInDay);

        if (mMustStop) {
            break;
        }
    }

    // Последняя итерация цикла перевела нас на начало последнего дня
    Q_ASSERT(date == mFinalDate);
    // (конечный результат не идёт в журнал - он пойдёт в начало следующего)

    if (mLoggingMode != ComputationSettings::NoLogging) {
        // и сохраняем всё
        emit loggerDataIsReady(logger);
    }

    while (!mSceneIsReadyForRedraw) {
        // ждём...
        msleep(100);
    }
    mComputationData = ComputationData(mDomain, date, dateOfPreviousData);
    emit dateChanged(mFinalDate);

    qDebug("ComputationThread::run(): computations finished");
    emit finished(mComputationData);
}

#include <iostream>
QString ComputationThread::setDataInDomain(QList<Block *> blocks,
        const QList<BoundaryPolygon *> outerPolygons)
{
    if (blocks.isEmpty()) {
        return tr("Domain is empty!\n"
                  "Please create blocks in the domain.");
    }

    QList<Block *> blocksInDomain;
    /***************** заполняем список необходимых блоков ********************/
    QList<Block *>::Iterator b;
    QList<BoundaryPolygon *>::ConstIterator p;
    qDebug("ComputationThread: determining needed blocks");
    for (p = outerPolygons.begin(); p != outerPolygons.end(); ++p) {
        for (b = blocks.begin(); b != blocks.end(); ++b) {
            if (BlockWithinPolygon::blockIntersectsPolygon(*b, *p)) {
                if (!(*b)->isReady()) {
                    return tr("Block without parameters detected!\n"
                              "You must set soil of all blocks inside domain "
                              "before starting computation.");
                }
                blocksInDomain.append(*b);
                // если блок попал в рассчётную область, проверять его больше не надо
                b = blocks.erase(b) - 1;

                if (mMustStop) {
                    return QString("");
                }
            }
        }
    }

    /* теперь blocks соответствует блокам вне рассчётной области,
     * а blocksInDomain -- блокам, попавшим в неё */

    if (blocksInDomain.isEmpty()) {
        return tr("Domain is empty!\n"
                  "There are no blocks inside boundary polygons.");
    }

    /******************* заполняем список всех полигонов **********************/
    QList<BoundaryPolygon *> allPolygons = outerPolygons;
    for (p = outerPolygons.begin(); p != outerPolygons.end(); ++p) {
        Q_ASSERT((*p)->isOuter());
        allPolygons.append((*p)->childBoundaryPolygonItems());
    }

    /*************** помечаем не попавшие в рассчётную область блоки **********/
    qDebug("ComputationThread: flagging block not in domain");
    for (b = blocks.begin(); b != blocks.end(); ++b) {
        (*b)->unsetInDomainFlag();
        if (mMustStop) {
            return QString("");
        }
    }

    /******************* создаём в рассчётной области блоки *******************/
    qDebug("ComputationThread: moving blocks to domain");
    for (b = blocksInDomain.begin(); b != blocksInDomain.end(); ++b) {
        (*b)->moveDataToDomain(&mDomain, mIsAxiallySymmetric);
        mUsedBlocks.append(*b);

        if (mMustStop) {
            return QString("");
        }
    }

    /* TODO:
     * Конвертировать, где это возможно, двухмерные области в одномерные.
     * Это возможно, если для области выполнены все следующие условия:
     * 1) она ограничена прямоугольным полигоном (без дырок);
     * 2) свойства блоков (грунт и н.у.) в ней не меняются по горизонтали;
     * 3) граничные условия по "бокам" области пустые (void condition);
     * 4) граничные условия сверху и снизу от неё не меняются по вертикали.
     * Расчёты для такой области можно проводить как для одномерной задачи,
     * использовалав только один её столбец и распространяя результат на прочие
     * блоки исходя из их положения по оси Y.
     */

    /***** создаём в рассчётной области площадки теплопотока между блоками ****/
    qDebug("ComputationThread: creating heat surfaces for blocks");
    for (b = mUsedBlocks.begin(); b != mUsedBlocks.end(); ++b) {
        (*b)->createTopAndLeftHeatSurfaces(&mDomain, mIsAxiallySymmetric);
        if (mMustStop) {
            return QString("");
        }
    }

    /****************** проверяем, что не нужна регуляризация *****************/
    {
        const double maxStepSecs = mDomain.maximalStep();
        // наибольший допустимый шаг в часах
        const QTime maxStepTime = QTime(0, 0).addMSecs(maxStepSecs * 1000.0);
        qDebug("ComputationThread: maximal step for domain: %s",
               qPrintable(maxStepTime.toString("HH:mm:ss")));
        int minNeededStepsInDay = qCeil(86400.0 / maxStepSecs);
        if (minNeededStepsInDay > mStepsInDay) {
            return tr("Time step is not enought, it must not be bigger than %1!\n"
                      "At least %n steps per day is needed.", "",
                      minNeededStepsInDay)
                   .arg(maxStepTime.toString("HH:mm:ss"));
        }
    }

    /************* создаём в рассчётной области граничные условия ************/
    qDebug("ComputationThread: moving boundary conditions to domain");
    QList<BoundaryCondition *> boundaryConditions;
    {
        MainWindow *m = qobject_cast<MainWindow *>(mScene->parent());
        Q_ASSERT(m != NULL);
        boundaryConditions = m->boundaryConditions();

        if (mMustStop) {
            return QString("");
        }
    }
    foreach(BoundaryCondition * condition, boundaryConditions) {
        condition->moveDataToDomain(&mDomain);

        if (mMustStop) {
            return QString("");
        }
    }

    /***************** создаём в рассчётной области площадки *******************
     *************** теплопотока от блоков к граничным условиям ***************/
    qDebug("ComputationThread: creating heat surfaces for boundary conditions");
    for (p = allPolygons.begin(); p != allPolygons.end(); ++p) {
        foreach(BoundaryPolyline polyline, (*p)->boundaryPolylines()) {
            if (polyline.condition->isVoid()) {
                continue;
            }
            for (b = mUsedBlocks.begin(); b != mUsedBlocks.end(); ++b) {
                QPair<qreal, qreal > p = ClipPolyline::clippedParams(*b,
                                         polyline.polygon,
                                         mIsAxiallySymmetric);
                if (p.first != 0) {
                    Q_ASSERT(p.first > 0);
                    Q_ASSERT(mIsAxiallySymmetric || p.second <= (*b)->rect().width());
                    Q_ASSERT(mIsAxiallySymmetric || p.second <= (*b)->rect().height());
                    Q_ASSERT(!qFuzzyIsNull(p.first));
                    mDomain.addHeatSurface(p.second, p.first,
                                           (*b)->numInDomain(),
                                           polyline.condition->numInDomain());
                }

                if (mMustStop) {
                    return QString("");
                }
            }
        }
    }

    qDebug("ComputationThread: shuffling heat surfaces");
    try {
        mDomain.shuffleHeatSurfaces();
    } catch (std::exception &e) {
        std::cerr << "exception caught: " << e.what() << std::endl;
    }

    return QString();
}

void ComputationThread::stop()
{
    mMustStop = true;
    while (isRunning()) {
        // ждём полной остановки, прежде закончить этот метод
        msleep(100);
    }
}
