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


#ifndef QFGUI_COLORGENERATOR_H
#define QFGUI_COLORGENERATOR_H

#include <QtCore/QObject>
#include <QtCore/QList>

QT_FORWARD_DECLARE_CLASS(QPainter)
QT_FORWARD_DECLARE_CLASS(QRect)
QT_FORWARD_DECLARE_CLASS(QRectF)
QT_FORWARD_DECLARE_CLASS(QColor)

namespace qfcore
{
QT_FORWARD_DECLARE_CLASS(SoilBlock)
}

namespace qfgui
{

/// Генератор цвета из некоторой величины
class ColorGenerator: public QObject
{
    Q_OBJECT
public:
    ColorGenerator(QObject *parent);

    QColor colorFromTemperature(const qfcore::SoilBlock &soilBlock) const;
    QColor colorFromTemperatureDiff(const qfcore::SoilBlock &soilBlock) const;
    QColor colorFromThawedPart(const qfcore::SoilBlock &soilBlock) const;

    bool discretizesColors() const {
        return mDiscretizeColors;
    }
    int drawTemperatureLegend(QPainter *painter,
                              const QRect &rect,
                              bool useDarkPen,
                              const QString &zeroLabel = "0") const;
    void drawThawedPartLegend(QPainter *painter,
                              const QRectF &rect,
                              bool useDarkPen) const;
public slots:
    void setDiscretizeColors(bool discretizeColors);

signals:
    /**
     * Сигнал о том, что цветовые шкалы изменились.
     * @param justDiscreteness изменилась ли только дискретность или сама шкала
     */
    void changed(bool justDiscreteness = false);

private:
    QColor colorFromTemperature(double t) const;

    bool mDiscretizeColors;

    double mT1;
    double mT2_1;
    double mT2;
    double mT3;
};

}

#endif // QFGUI_COLORGENERATOR_H
