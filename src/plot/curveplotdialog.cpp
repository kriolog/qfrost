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

#include "curveplotdialog.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include <QtMath>

#include <application.h>
#include <units/physicalpropertyspinbox.h>
#include <graphicsviews/block.h>
#include <plot/curveplot.h>
#include <mainwindow.h>
#include <control_panel/controlpanel.h>
#include <control_panel/computationcontrol.h>

using namespace qfgui;

CurvePlotDialog::CurvePlotDialog(Block *block,
                                 Qt::Orientation orientation,
                                 QWidget *parent)
    : QDialog(parent)
    , mPlot(new CurvePlot(orientation, this))
    , mPlotTemperature(new QCheckBox(tr("&Temperature"), this))
    , mPlotThawedPard(new QCheckBox(tr("Thawed &part"), this))
    , mPlotTransitionTemperature(new QCheckBox(tr("T&ransition temperature"), this))
    , mShowModelDateText(new QCheckBox(tr("Show model &date in title")))
    , mMinTemperature(new PhysicalPropertySpinBox(Temperature, this))
    , mMaxTemperature(new PhysicalPropertySpinBox(Temperature, this))
    , mMinCoord(PhysicalPropertySpinBox::createSceneCoordinateSpinBox())
    , mMaxCoord(PhysicalPropertySpinBox::createSceneCoordinateSpinBox())
    , mSlice(block->slice(orientation))
    , mTemperatures()
    , mThawedParts()
    , mTransitionTemperatures()
    , mCoordsMain()
    , mCoordsNormal()
    , mDialogButtons(new QDialogButtonBox(QDialogButtonBox::Close, this))
    , mIsUpdatingAdditionalLimits(false)
{
    Q_ASSERT(!mSlice.isEmpty());

    mTemperatures.reserve(mSlice.size());
    mThawedParts.reserve(mSlice.size());
    mTransitionTemperatures.reserve(mSlice.size());
    mCoordsMain.reserve(mSlice.size());
    mCoordsNormal.reserve(mSlice.size());

    foreach (Block *block, mSlice) {
        mTemperatures.append(block->soilBlock()->temperature());
        mThawedParts.append(block->soilBlock()->thawedPart());
        mTransitionTemperatures.append(block->soilBlock()->transitionTemperature());

        const QPointF center = block->metersCenter();
        mCoordsMain.append(orientation == Qt::Horizontal
                           ? center.x() : center.y());
        mCoordsNormal.append(orientation == Qt::Horizontal
                             ? center.y() : center.x());

        block->showArrows(); // TMP
    }

    mMinCoord->setMinimum(mCoordsMain.first());
    if (mMinCoord->minimum() > 0.0 && mMinCoord->minimum() < 1.0) {
        mMinCoord->setMinimum(0.0);
    }
    mMaxCoord->setMaximum(mCoordsMain.last());

    connect(mMinCoord, SIGNAL(valueChanged(double)),
            SLOT(updateAdditionalLimits()));
    connect(mMaxCoord, SIGNAL(valueChanged(double)),
            SLOT(updateAdditionalLimits()));
    connect(mMinCoord, SIGNAL(valueChanged(double)),
            SLOT(setPlotRangeCoords()));
    connect(mMaxCoord, SIGNAL(valueChanged(double)),
            SLOT(setPlotRangeCoords()));

    connect(mMinTemperature, SIGNAL(valueChanged(double)),
            SLOT(updateAdditionalLimits()));
    connect(mMaxTemperature, SIGNAL(valueChanged(double)),
            SLOT(updateAdditionalLimits()));
    connect(mMinTemperature, SIGNAL(valueChanged(double)),
            SLOT(setPlotRangeTemperature()));
    connect(mMaxTemperature, SIGNAL(valueChanged(double)),
            SLOT(setPlotRangeTemperature()));

    //: automatically set slice coordinate limits
    QPushButton *autoLimitCoord = new QPushButton(tr("&Auto"));
    autoLimitCoord->setSizePolicy(QSizePolicy::Fixed,
                                  QSizePolicy::Preferred);
    connect(autoLimitCoord, SIGNAL(clicked()),
            SLOT(autoMinMaxCoord()));
    autoMinMaxTemperature();

    //: automatically set t limits
    QPushButton *autoLimitTemperature = new QPushButton(tr("A&uto"));
    autoLimitTemperature->setSizePolicy(QSizePolicy::Fixed,
                                        QSizePolicy::Preferred);
    connect(autoLimitTemperature, SIGNAL(clicked()),
            SLOT(autoMinMaxTemperature()));
    autoMinMaxCoord();

    static const QString minMaxDelimText = "\342\200\223";

    QHBoxLayout *tLimits = new QHBoxLayout;
    tLimits->addWidget(mMinTemperature, 1);
    tLimits->addWidget(new QLabel(minMaxDelimText));
    tLimits->addWidget(mMaxTemperature), 1;
    tLimits->addWidget(autoLimitTemperature);

    QHBoxLayout *zLimits = new QHBoxLayout;
    zLimits->addWidget(mMinCoord, 1);
    zLimits->addWidget(new QLabel(minMaxDelimText));
    zLimits->addWidget(mMaxCoord, 1);
    zLimits->addWidget(autoLimitCoord); 

    QGroupBox *plotElementsBox = new QGroupBox(tr("Plot Elements"), this);
    QVBoxLayout *plotElements = new QVBoxLayout(plotElementsBox);
    plotElements->addWidget(mPlotTemperature);
    plotElements->addWidget(mPlotThawedPard);
    plotElements->addWidget(mPlotTransitionTemperature);
    plotElements->addWidget(mShowModelDateText);

    QGroupBox *limitsBox = new QGroupBox(tr("Axis Ranges"), this);
    QFormLayout *limits = new QFormLayout(limitsBox);
    limits->addRow(tr("Temperature:"), tLimits);
    limits->addRow(tr("Coordinate:"), zLimits);

    connect(mDialogButtons, SIGNAL(rejected()), SLOT(reject()));

    QVBoxLayout *settingsLayout = new QVBoxLayout();
    settingsLayout->addWidget(plotElementsBox);
    settingsLayout->addWidget(limitsBox);

    QHBoxLayout *middleLayout = new QHBoxLayout();
    middleLayout->addLayout(settingsLayout);
    middleLayout->addWidget(mPlot, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(middleLayout);
    mainLayout->addWidget(mDialogButtons);

    mPlot->setCoords(mCoordsMain);
    mPlot->setTemperature(mTemperatures);
    mPlot->setThawedPart(mThawedParts);
    mPlot->setTransitionTemperature(mTransitionTemperatures);

    mPlot->setModelDate(Application::findMainWindow(parent)->controlPanel()
                        ->computationControl()->currentDate());
 
    connect(mPlotTemperature, SIGNAL(toggled(bool)),
            mPlot, SLOT(setTemperatureVisible(bool)));

    connect(mPlotThawedPard, SIGNAL(toggled(bool)),
            mPlot, SLOT(setThawedPartVisible(bool)));

    connect(mPlotTransitionTemperature, SIGNAL(toggled(bool)),
            mPlot, SLOT(setTransitionTemperatureVisible(bool)));

    connect(mShowModelDateText, SIGNAL(toggled(bool)),
            mPlot, SLOT(setModelDateVisible(bool)));

    mPlotTemperature->setChecked(true);
    mPlotThawedPard->setChecked(true);
    mPlotTransitionTemperature->setChecked(true);
    mShowModelDateText->setChecked(true);
}

void CurvePlotDialog::autoMinMaxCoord()
{
    mMinCoord->setValue(mMinCoord->minimum());
    mMaxCoord->setValue(mMaxCoord->maximum());
}

void CurvePlotDialog::autoMinMaxTemperature()
{
    double minT = std::numeric_limits<double>::infinity();
    double maxT = -std::numeric_limits<double>::infinity();

    foreach (double t, mTemperatures) {
        if (t < minT) {
            minT = t;
        }
        if (t > maxT) {
            maxT = t;
        }
    }

    mMinTemperature->setValue(qFloor(minT));
    mMaxTemperature->setValue(qCeil(maxT));

    // Если полученный диапазон слишком мал, выставим его по average(minT, maxT)
    static const double minTemperatureRange = 5.0;
    if (mMaxTemperature->value() - mMinTemperature->value() < 5.0) {
        const double avgT = (maxT - minT)/2.0;
        static const double deltaT = minTemperatureRange / 2.0;

        mMinTemperature->setValue(qFloor(avgT - deltaT));
        mMaxTemperature->setValue(qCeil(avgT + deltaT));
    }
}

void CurvePlotDialog::updateAdditionalLimits()
{
    if (mIsUpdatingAdditionalLimits) {
        return;
    }

    mIsUpdatingAdditionalLimits = true;

    const double minT = mMinTemperature->value();
    const double maxT = mMaxTemperature->value();

    mMinTemperature->setMaximum(maxT);
    mMaxTemperature->setMinimum(minT);

    const double minCoord = mMinCoord->value();
    const double maxCoord = mMaxCoord->value();

    mMinCoord->setMaximum(maxCoord);
    mMaxCoord->setMinimum(minCoord);

    mIsUpdatingAdditionalLimits = false;
}

void CurvePlotDialog::setPlotRangeCoords()
{
    mPlot->setCoordsAxisRange(mMinCoord->value(),
                              mMaxCoord->value());
}

void CurvePlotDialog::setPlotRangeTemperature()
{
    mPlot->setTemperatureAxisRange(mMinTemperature->value(),
                                   mMaxTemperature->value());
}
