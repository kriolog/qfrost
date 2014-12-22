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
#include <QMessageBox>
#include <QtMath>
#include <QFile>
#include <QTextStream>

#include <application.h>
#include <dialog.h>
#include <units/physicalpropertyspinbox.h>
#include <units/units.h>
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
    , mShowModelDateText(new QCheckBox(tr("Show &model date in title")))
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
    , mSavePNGButton(new QPushButton("Save PNG &Image"))
    , mSavePDFButton(new QPushButton("Save PDF &Document"))
    , mSavePrimaryData(new QPushButton("&Save Primary Data"))
    , mDialogButtons(new QDialogButtonBox(QDialogButtonBox::Close, this))
    , mIsUpdatingAdditionalLimits(false)
    , mSavedFileBaseName(Application::findMainWindow(this)->currentFileBasePath()
                         + '_' + modelDate().toString("yyyy-MM-dd") + '_' +
                         (orientation == Qt::Horizontal ? "Z_" : "X_") +
                         QString::number(orientation == Qt::Horizontal
                                         ? block->metersCenter().y()
                                         : block->metersCenter().x(),
                                         'f', mMaxCoord->decimals()))
{
    Q_ASSERT(!mSlice.isEmpty());

    setWindowTitle(tr("Curve Plot"));

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
    connect(autoLimitCoord, SIGNAL(clicked()),
            SLOT(autoMinMaxCoord()));
    autoMinMaxTemperature();

    //: automatically set t limits
    QPushButton *autoLimitTemperature = new QPushButton(tr("A&uto"));
    connect(autoLimitTemperature, SIGNAL(clicked()),
            SLOT(autoMinMaxTemperature()));
    autoMinMaxCoord();

    connect(mSavePNGButton, SIGNAL(clicked()), SLOT(savePNG()));
    connect(mSavePDFButton, SIGNAL(clicked()), SLOT(savePDF()));
    connect(mSavePrimaryData, SIGNAL(clicked()), SLOT(savePrimaryData()));

    static const QString minMaxDelimText = "\342\200\223";

    QGroupBox *plotElementsBox = new QGroupBox(tr("Plot Elements"), this);
    QVBoxLayout *plotElements = new QVBoxLayout(plotElementsBox);
    plotElements->addWidget(mPlotTemperature);
    plotElements->addWidget(mPlotThawedPard);
    plotElements->addWidget(mPlotTransitionTemperature);
    plotElements->addWidget(mShowModelDateText);

    QGroupBox *coordLimitBox = new QGroupBox(tr("Coordinate Range"), this);
    QFormLayout *coordLimits = new QFormLayout(coordLimitBox);
    coordLimits->addRow(tr("Minimum:"), mMinCoord);
    coordLimits->addRow(tr("Maximum:"), mMaxCoord);
    coordLimits->addRow(autoLimitCoord);
 
    QGroupBox *temperatureLimitBox = new QGroupBox(tr("Temperature Range"), this);
    QFormLayout *temperatureLimits = new QFormLayout(temperatureLimitBox);
    temperatureLimits->addRow(tr("Minimum:"), mMinTemperature);
    temperatureLimits->addRow(tr("Maximum:"), mMaxTemperature);
    temperatureLimits->addRow(autoLimitTemperature);

    QGroupBox *saveBox = new QGroupBox(tr("Save Graph or Data"), this);
    QVBoxLayout *saveLayout = new QVBoxLayout(saveBox);
    saveLayout->addWidget(mSavePNGButton);
    saveLayout->addWidget(mSavePDFButton);
    saveLayout->addWidget(mSavePrimaryData);

    connect(mDialogButtons, SIGNAL(rejected()), SLOT(reject()));

    QVBoxLayout *settingsLayout = new QVBoxLayout();
    settingsLayout->addWidget(plotElementsBox);
    settingsLayout->addWidget(coordLimitBox);
    settingsLayout->addWidget(temperatureLimitBox);
    settingsLayout->addWidget(saveBox);
    settingsLayout->addStretch(1);

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

    mPlot->setModelDate(modelDate());
 
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

bool CurvePlotDialog::savePDF()
{
    static const QString extension = ".pdf";

    QString fileName = Dialog::getSaveFileName(this,
                                               tr("Save PDF", "Dialog Title"),
                                               mSavedFileBaseName + extension,
                                               tr("PDF files")
                                               + QString(" (*%1)").arg(extension));

    if (fileName.isEmpty()) {
        return false;
    }

    if (!mPlot->savePDF(fileName)) {
        QMessageBox::warning(this, tr("Save Failed"),
                             tr("Can not save file %1.")
                             .arg(locale().quoteString(fileName)));
        return false;
    }

    return true;
}

bool CurvePlotDialog::savePNG()
{
    static const QString extension = ".png";

    QString fileName = Dialog::getSaveFileName(this,
                                               tr("Save PNG", "Dialog Title"),
                                               mSavedFileBaseName + extension,
                                               tr("PDF files")
                                               + QString(" (*%1)").arg(extension));

    if (fileName.isEmpty()) {
        return false;
    }

    if (!mPlot->savePNG(fileName, mPlot->width(), mPlot->height(), 2.0)) {
        QMessageBox::warning(this, tr("Save Failed"),
                             tr("Can not save file %1.")
                             .arg(locale().quoteString(fileName)));
        return false;
    }

    return true;
}

bool CurvePlotDialog::savePrimaryData()
{
    static const QString extension = ".txt";

    QString fileName = Dialog::getSaveFileName(this,
                                               tr("Save Primary Data", "Dialog Title"),
                                               mSavedFileBaseName + extension,
                                               tr("Text files")
                                               + QString(" (*%1)").arg(extension));

    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Save Failed"),
                             tr("Can not write to file %1.")
                             .arg(locale().quoteString(fileName)));
        return false;
    }

    QTextStream out(&file);

    out << "#X\tZ\tT\tVth\tTbf\n";

    const int temperatureDecimals = Units::decimals(Temperature);
    foreach (const Block *block, mSlice) {
        const QPointF center = block->metersCenter();
        out << center.x() << "\t"
            << center.y() << "\t"
            << QString::number(block->soilBlock()->temperature(),
                               'f', temperatureDecimals) << "\t"
            << qRound(block->soilBlock()->thawedPart() * 100.0) << "\t"
            << QString::number(block->soilBlock()->transitionTemperature(),
                               'f', temperatureDecimals) << "\n";
    }

    return true;
}

QDate CurvePlotDialog::modelDate() const
{
    MainWindow *m = Application::findMainWindow(this);
    Q_ASSERT(m);
    return m->controlPanel()->computationControl()->currentDate();
}