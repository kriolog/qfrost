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

#ifndef QFGUI_COMPUTATIONTHREAD_H
#define QFGUI_COMPUTATIONTHREAD_H

#include <QtCore/QThread>
#include <QtCore/QMetaType>
#include <QtCore/QDate>

#include <computations/blockslogger.h>
#include <core/domain.h>
#include <graphicsviews/block.h>
#include <graphicsviews/scene.h>

namespace qfgui
{

class ComputationThread : public QThread
{
    Q_OBJECT
public:
    ComputationThread(Scene *scene, const ComputationSettings &settings);
    virtual ~ComputationThread() {
        qDebug("Thread deleted");
    }
    /**
     * Вызывать, когда сцена МОЖЕТ принять актуальные данные -- то есть
     * отрисовала предыдущие данные.
     */
    void onSceneReadyForRedraw() {
        mSceneIsReadyForRedraw = true;
    }

    /**
     * Вызывать, когда сцена ХОЧЕТ принять актуальные данные -- то есть прошло
     * заданное время от начала предыдущей отрисовки
     */
    void onSceneWantsNewData() {
        mSceneWantsNewData = true;
    }

    void setNeedBlocksRedrawing(bool b) {
        mNeedBlocksRedrawing = b;
    }

    /// Останавливает рассчёты и ждёт, пока они пройдут
    void stop();

protected:
    void run();

private:
    qfcore::Domain mDomain;

    const int mStepsInDay;
    const QDate mInitialDate;
    const QDate mFinalDate;
    const bool mIsAxiallySymmetric;

    /// Сцена отрисовала предыдущую порцию данных и готова к новой отрисовке.
    bool mSceneIsReadyForRedraw;
    /// В сцене прошло 3 секунды от начала предыдущей отрисовки.
    bool mSceneWantsNewData;
    /// Информация о сцене. Нельзя трогать, пока сцена не считала всё!
    ComputationData mComputationData;

    Scene *const mScene;

    QList<Block *> mUsedBlocks;

    /**
     * Заполняет расчётную область из сцены.
     * @return сообщение об ошибке, если что-то не так, QString("") если нас
     * попросили остановиться, или QString(), если ошибки нет.
     */
    QString setDataInDomain(QList<Block *> blocks,
                            const QList<BoundaryPolygon *> outerPolygons);

    bool mMustStop;

    bool mNeedBlocksRedrawing;

    const int mLoggingMode;
    const double mLoggerMaxY;

signals:
    void signalNewDataIsReady(const qfgui::ComputationData &data);
    void finished(const qfgui::ComputationData &data);
    void dateChanged(QDate date);
    void badStart(QString errorText);
    void loggerDataIsReady(qfgui::BlocksLogger logger);
};

}

#endif // QFGUI_COMPUTATIONTHREAD_H
