/*
 * Copyright (C) 2011-2013  Denis Pesotsky
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

#include <computations/blockslogger.h>

#include <QtWidgets/QFileDialog>
#include <QtWidgets/QApplication>
#include <QtCore/QTextStream>

#include <mainwindow.h>

using namespace qfgui;

BlocksLogger::BlocksLogger(const QList<Block *> &blocksToLog, double maxY,
                           MainWindow *mainWindow)
    : mCentersAndNums()
    , mData()
    , mThawedParts()
    , mMainWindow(mainWindow)
{
    QList<Block *> sortedBlocks = blocksToLog;
    qSort(sortedBlocks.begin(), sortedBlocks.end(), xzLessThan);
    QList<Block *>::ConstIterator it;
    for (it = sortedBlocks.constBegin(); it != sortedBlocks.constEnd(); ++it) {
        QPointF center = QFrost::meters((*it)->rect().center());
        if (center.y() > maxY) {
            continue;
        }
        mCentersAndNums << QPair<QPointF, std::size_t>(center, (*it)->numInDomain());
        mThawedParts << (*it)->soilBlock()->transitionTemperature();
    }
}

void BlocksLogger::addData(const ComputationData &data)
{
    mData.append(data);
}

/// Количество лет (не целое) даты @p date от начала года. Всегда < 1.0.
static const double yearsFromAnnumStart(const QDate &date)
{
    const QDate firstDateInYear(date.year(), 1, 1);

    return firstDateInYear.daysTo(date) / (QDate::isLeapYear(date.year()) ? 366.0 : 355.0);
}

/// Разница в годах (не целая) между @p date1 и @p date2
static const double yearsPassed(const QDate &date1, const QDate &date2)
{
    Q_ASSERT(date1 <= date2);
    return yearsFromAnnumStart(date1) + (date2.year() - date1.year()) + yearsFromAnnumStart(date2);
}

void BlocksLogger::exportData(QTextStream &out, BlocksLogger::ExportFormat format) const
{
    Q_ASSERT(!mData.isEmpty());
    Q_ASSERT(!mData.first().isEmpty());

    if (format == LastBlockTXT) {
        out << "date\tyears_passed\tT\tV\n";
        const QDate firstDate = mData.first().date();
        for (QList<ComputationData>::ConstIterator it = mData.constBegin(); it != mData.constEnd(); ++it) {
            const BlockData &bdata = it->soilBlockDataAt(mCentersAndNums.last().second);
            out << it->date().toString("yyyy-MM-dd\t")
                << yearsPassed(firstDate, it->date()) << "\t"
                << bdata.temperature() << "\t" << bdata.thawedPart() << "\n";
        }
    } else if (format == CSV) {
        out << "x,y";
        for (QList<ComputationData>::ConstIterator it = mData.constBegin(); it != mData.constEnd(); ++it) {
            out << "," << it->date().toString("yyyy-MM-dd");
        }
        out << "\n";
        for (QList<QPair<QPointF, std::size_t> >::ConstIterator j = mCentersAndNums.constBegin(); j != mCentersAndNums.constEnd(); ++j) {
            out << j->first.y() << "," << j->first.y();
            for (QList<ComputationData>::ConstIterator it = mData.constBegin(); it != mData.constEnd(); ++it) {
                const BlockData &bdata = it->soilBlockDataAt(j->second);
                out << "," << bdata.temperature();
            }
            out << "\n";
        }
    } else {
        QList<ComputationData>::ConstIterator i;
        for (i = mData.constBegin(); i != mData.constEnd(); ++i) {
            out << i->date().toString("yyyyMMdd") << "\n";
        }
        out << "\n";

        QList<QPair<QPointF, std::size_t> >::ConstIterator j;
        for (j = mCentersAndNums.constBegin(); j != mCentersAndNums.constEnd(); ++j) {
            out << j->first.y() << "\n";
        }
        out << "\n";

        for (i = mData.constBegin(); i != mData.constEnd(); ++i) {
            for (j = mCentersAndNums.constBegin(); j != mCentersAndNums.constEnd(); ++j) {
                out << i->soilBlockDataAt(j->second).temperature() << "\n";
            }
        }
        out << "\n";

        for (i = mData.constBegin(); i != mData.constEnd(); ++i) {
            for (j = mCentersAndNums.constBegin(); j != mCentersAndNums.constEnd(); ++j) {
                out << i->soilBlockDataAt(j->second).thawedPart() << "\n";
            }
        }
        out << "\n";

        QList<double>::ConstIterator k;
        for (k = mThawedParts.constBegin(); k != mThawedParts.constEnd(); ++k) {
            out << *k << "\n";
        }
    }

    /* Формат gnuplot (сильно избыточный):
    for(i = mData.constBegin(); i != mData.constEnd(); ++i) {
        QList<QPair<QPointF, std::size_t> >::ConstIterator j;
        for(j = mCentersAndNums.constBegin(); j != mCentersAndNums.constEnd(); ++j) {
            const BlockData *bdata = &(i->soilBlockDataAt(j->second));
            out << i->date().toString("yyyyMMdd") << " "
                << j->first.y() << " "
                << bdata->temperature() << " "
                << bdata->thawedPart() << " " << "\n";
        }
        out << "\n";
    }
    */
}
