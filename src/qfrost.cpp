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

#include <qfrost.h>

#include <QtCore/Qt>
#include <QtCore/QtMath>
#include <QtCore/QLocale>
#include <QtCore/QTime>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>
#include <QtWidgets/QStyle>

using namespace qfgui;

// Если эта величина равна 0.01, то блоки размером 0.01м имеют некрасивый текст.
// Т.е. оно должно быть хотя бы в 10 раз быть больше минимального размера блока.
const double QFrost::metersInUnit = qPow(10.0, -QFrost::meterDecimals);

const double QFrost::k = metersInUnit / unitsInGridStep;

const double QFrost::minBlockSizeMeters = qPow(10.0, -meterDecimalsBlockSize);
const double QFrost::minBlockSizeScene = minBlockSizeMeters / k;

const double QFrost::sceneHalfSizeInMeters = 1000;

const qreal QFrost::microSizeF = QFrost::microSize;

const qreal QFrost::accuracy = 1e-8;

const int QFrost::sceneHalfSize = sceneUnits(sceneHalfSizeInMeters);

const int QFrost::PhysicalPropertyRole = Qt::UserRole + 2;
const int QFrost::MinimumRole = Qt::UserRole + 3;
const int QFrost::MaximumRole = Qt::UserRole + 4;
const int QFrost::DirectEditRole = Qt::UserRole + 5;
const int QFrost::UndoTextRole = Qt::UserRole + 6;

const QPointF QFrost::noPoint = QPointF(sceneHalfSize * 10, sceneHalfSize * 10);

const QRect QFrost::boundRect(-sceneHalfSize, -sceneHalfSize,
                              2 * sceneHalfSize, 2 * sceneHalfSize);

const QRectF QFrost::boundRectF(boundRect);

const QRectF QFrost::boundRectInMeters(-sceneHalfSizeInMeters,
                                       -sceneHalfSizeInMeters,
                                       2 * sceneHalfSizeInMeters,
                                       2 * sceneHalfSizeInMeters);

const char *const QFrost::UndoBinderIsEnabled = "undoBinderIsEnabled";
const char *const QFrost::InvalidInputPropertyName = "hasInvalidInput";

void QFrost::setInputValidity(QWidget *widget, bool isValid)
{
    const QVariant wasInvalidVariant = widget->property(QFrost::InvalidInputPropertyName);

    const bool wasInvalid = wasInvalidVariant.isValid() && wasInvalidVariant.toBool();

    if (isValid == wasInvalid) {
        return;
    }

    widget->setProperty(QFrost::InvalidInputPropertyName, isValid);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

QString QFrost::dateFormat()
{
    QString result = QLocale().dateFormat(QLocale::ShortFormat);
    if (!result.contains("yyyy")) {
        Q_ASSERT(result.contains("yy"));
        result.replace("yy", "yyyy");
    }
    return result;
}

QPair< QString, bool > QFrost::singleStepInfo(int numStepsInDay)
{
    Q_ASSERT(numStepsInDay > 0);
    if (numStepsInDay == 1) {
        return qMakePair(QString("24:00:00"), true);
    } else {
        QTime time = QTime(0, 0, 0).addMSecs(qRound(86400000.0 / numStepsInDay));
        const bool isPrecise = (86400 % numStepsInDay == 0);
        return qMakePair(time.toString("HH:mm:ss"), isPrecise);
    }
}

QSize QFrost::upperBoundIconSize(const QIcon &icon, int maxHeight)
{
    const QSize maxSize(maxHeight, maxHeight);
    QSize result;
    foreach (const QSize &iconSize, icon.availableSizes()) {
        const int height = iconSize.height();
        if (height > result.height() && height <= maxHeight) {
            result = iconSize;
        }
    }
    return result;
}

QString QFrost::romanNumeral(int number, int markCount)
{
    typedef QPair<int, QString> valueMapping;
    static QVector<valueMapping> importantNumbers = {
        {1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"},
        {100,  "C"}, { 90, "XC"}, { 50, "L"}, { 40, "XL"},
        {10,   "X"}, {  9, "IX"}, {  5, "V"}, {  4, "IV"},
        {1,    "I"},
    };

    QString result;
    bool needMark = false;
    QString marks(markCount, '\'');
    for (auto mapping : importantNumbers) {
        int value = mapping.first;
        const QString &digits = mapping.second;
        while (number >= value) {
            result += digits;
            number -= value;
            needMark = true;
        }
        if ((value == 1000 || value == 100 || value == 10 || value == 1) && needMark) {
            result += marks;
            needMark = false;
        }
    }
    return result;
}
