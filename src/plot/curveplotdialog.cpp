/*
 * Copyright (C) 2014-2015  Denis Pesotsky
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

static QGroupBox *createMinMaxGroupBox(const QString &title,
                                       QDoubleSpinBox *minSpinBox,
                                       QDoubleSpinBox *maxSpinBox,
                                       QPushButton *autoMinMax,
                                       QCheckBox *saveMinMax)
{
    QFormLayout *limits = new QFormLayout();
    limits->addRow(CurvePlotDialog::tr("Minimum:"), minSpinBox);
    limits->addRow(CurvePlotDialog::tr("Maximum:"), maxSpinBox);

    QHBoxLayout *limitsWithButton = new QHBoxLayout();
    limitsWithButton->addLayout(limits, 1);
    limitsWithButton->addWidget(autoMinMax);
    autoMinMax->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QGroupBox *groupBox = new QGroupBox(title);
    QVBoxLayout *groupBoxLayout = new QVBoxLayout(groupBox);
    groupBoxLayout->addLayout(limitsWithButton);
    groupBoxLayout->addWidget(saveMinMax);

    return groupBox;
}

CurvePlotDialog::CurvePlotDialog(Block *block,
                                 Qt::Orientation orientation,
                                 QWidget *parent)
    : QDialog(parent)
    , mPlot(new CurvePlot(orientation, this))
    , mOrientation(orientation)
    , mPlotTemperature(new QCheckBox(tr("&Temperature"), this))
    , mPlotThawedPard(new QCheckBox(tr("Thawed &volume fraction"), this))
    , mPlotTransitionTemperature(new QCheckBox(tr("Transition t&emperature"), this))
    , mShowModelDateText(new QCheckBox(tr("Show &model date in title")))
    , mMinTemperature(new PhysicalPropertySpinBox(Temperature, this))
    , mMaxTemperature(new PhysicalPropertySpinBox(Temperature, this))
    //: text from button that automatically set coordinate limits
    , mAutoMinMaxTemperature(new QPushButton("", this))
    , mSaveMinMaxTemperature(new QCheckBox(tr("Restore this ra&nge for next plot"), this))
    , mMinCoord(PhysicalPropertySpinBox::createSceneCoordinateSpinBox(this))
    , mMaxCoord(PhysicalPropertySpinBox::createSceneCoordinateSpinBox(this))
    //: text from button that automatically sets temperature limits
    , mAutoMinMaxCoord(new QPushButton("", this))
    , mSaveMinMaxCoord(new QCheckBox(tr("Rest&ore this range for next plot"), this))
    , mSlice(block->slice(orientation))
    , mTemperatures()
    , mThawedParts()
    , mTransitionTemperatures()
    , mCoordsMain()
    , mCoordsNormal()
    , mSavePNGButton(new QPushButton(tr("Save &Raster Image...")))
    , mSavePDFButton(new QPushButton(tr("Save Vector &Image...")))
    , mSavePrimaryData(new QPushButton(tr("&Save Primary Data...")))
    , mDialogButtons(new QDialogButtonBox(QDialogButtonBox::Close, this))
    , mIsUpdatingAdditionalLimits(false)
    , mSavedFileBaseName(Application::findMainWindow(this)->currentFileBasePath()
                         + '_' + modelDate().toString("yyyy-MM-dd") + '_' +
                         (orientation == Qt::Horizontal ? "Z_" : "X_") +
                         QString::number(orientation == Qt::Horizontal
                                         ? block->metersCenter().y()
                                         : block->metersCenter().x(),
                                         'f', mMaxCoord->decimals()))
    , mKnownCoordMin()
    , mKnownCoordMax()
    , mKnownTemperatureMin()
    , mKnownTemperatureMax()
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

        block->setMarkered(true);
    }

    updateKnownCoordLimits();
    updateKnownTemperatureLimits();

    connect(mMinCoord, SIGNAL(valueChanged(double)), SLOT(updateAdditionalLimits()));
    connect(mMaxCoord, SIGNAL(valueChanged(double)), SLOT(updateAdditionalLimits()));
    connect(mMinCoord, SIGNAL(valueChanged(double)), SLOT(setPlotRangeCoords()));
    connect(mMaxCoord, SIGNAL(valueChanged(double)), SLOT(setPlotRangeCoords()));
    connect(mMinCoord, SIGNAL(valueChanged(double)), SLOT(updateAutoMinMaxCoordButton()));
    connect(mMaxCoord, SIGNAL(valueChanged(double)), SLOT(updateAutoMinMaxCoordButton()));

    connect(mMinTemperature, SIGNAL(valueChanged(double)), SLOT(updateAdditionalLimits()));
    connect(mMaxTemperature, SIGNAL(valueChanged(double)), SLOT(updateAdditionalLimits()));
    connect(mMinTemperature, SIGNAL(valueChanged(double)), SLOT(setPlotRangeTemperature()));
    connect(mMaxTemperature, SIGNAL(valueChanged(double)), SLOT(setPlotRangeTemperature()));
    connect(mMinTemperature, SIGNAL(valueChanged(double)), SLOT(updateAutoMinMaxTemperatureButton()));
    connect(mMaxTemperature, SIGNAL(valueChanged(double)), SLOT(updateAutoMinMaxTemperatureButton()));

    connect(mAutoMinMaxCoord, SIGNAL(clicked()), SLOT(autoMinMaxCoord()));
    connect(mAutoMinMaxTemperature, SIGNAL(clicked()), SLOT(autoMinMaxTemperature()));

    autoMinMaxCoord();
    autoMinMaxTemperature();

    connect(mSavePNGButton, SIGNAL(clicked()), SLOT(savePNG()));
    connect(mSavePDFButton, SIGNAL(clicked()), SLOT(savePDF()));
    connect(mSavePrimaryData, SIGNAL(clicked()), SLOT(savePrimaryData()));

    mSavePNGButton->setToolTip(tr("Save plots to PNG file with specified size and scale."));
    mSavePDFButton->setToolTip(tr("Save plots to (vector) PDF file with specified size."));
    mSavePrimaryData->setToolTip(tr("Save plot primary data to simple text file."));

    static const QString minMaxDelimText = "\342\200\223";

    QGroupBox *plotElementsBox = new QGroupBox(tr("Plot Elements"), this);
    QVBoxLayout *plotElements = new QVBoxLayout(plotElementsBox);
    plotElements->addWidget(mPlotTemperature);
    plotElements->addWidget(mPlotThawedPard);
    plotElements->addWidget(mPlotTransitionTemperature);
    plotElements->addWidget(mShowModelDateText);

    QGroupBox *coordLimitBox = createMinMaxGroupBox(tr("Coordinate Range"),
                                                    mMinCoord,
                                                    mMaxCoord,
                                                    mAutoMinMaxCoord,
                                                    mSaveMinMaxCoord);
 
    QGroupBox *temperatureLimitBox = createMinMaxGroupBox(tr("Temperature Range"),
                                                          mMinTemperature,
                                                          mMaxTemperature,
                                                          mAutoMinMaxTemperature,
                                                          mSaveMinMaxTemperature);

    QGroupBox *saveBox = new QGroupBox(tr("Save Graph or Data"), this);
    QVBoxLayout *saveLayout = new QVBoxLayout(saveBox);
    saveLayout->addWidget(mSavePNGButton);
    saveLayout->addWidget(mSavePDFButton);
    saveLayout->addWidget(mSavePrimaryData);

    connect(mDialogButtons, SIGNAL(rejected()), SLOT(reject()));

    QVBoxLayout *settingsLayout = new QVBoxLayout();
    settingsLayout->addWidget(saveBox);
    settingsLayout->addWidget(plotElementsBox);
    settingsLayout->addWidget(coordLimitBox);
    settingsLayout->addWidget(temperatureLimitBox);
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

    // Цепляемся на rejected(), т.к. у нас есть только кнопка закрытия.
    connect(this, SIGNAL(rejected()), SLOT(emitSavingMinMax()));

    // Выбираем иконки для кнопок автолимита исходя из направления осей.
    if (orientation == Qt::Horizontal) {
        // Лучше использовать fromTheme вместе с полным названием иконки - иначе
        // не пройдёт копирование скриптом! Никаких масок и тернарных операторов.
        mAutoMinMaxCoord->setIcon(QIcon::fromTheme("zoom-fit-width"));
        mAutoMinMaxTemperature->setIcon(QIcon::fromTheme("zoom-fit-height"));
    } else {
        mAutoMinMaxCoord->setIcon(QIcon::fromTheme("zoom-fit-height"));
        mAutoMinMaxTemperature->setIcon(QIcon::fromTheme("zoom-fit-width"));
    }

    // Подбираем максимальный размер для этих иконок (по высоте спинбоксов).
    const int maxIconHeight = mMinCoord->height() + mMaxCoord->height();
    const QSize autoIconSize = QFrost::upperBoundIconSize(mAutoMinMaxCoord->icon(),
                                                          maxIconHeight);
    mAutoMinMaxCoord->setIconSize(autoIconSize);
    mAutoMinMaxTemperature->setIconSize(autoIconSize);

    setAttribute(Qt::WA_DeleteOnClose);
}

CurvePlotDialog::~CurvePlotDialog()
{
    foreach (Block *block, mSlice) {
        block->setMarkered(false);
    }
}

void CurvePlotDialog::loadMinMaxCoord(double min, double max)
{
    setMinMaxCoord(min, max);
    mSaveMinMaxCoord->setChecked(true);
}

void CurvePlotDialog::loadMinMaxTemperature(double min, double max)
{
    setMinMaxTemperature(min, max);
    mSaveMinMaxTemperature->setChecked(true);
}

void CurvePlotDialog::autoMinMaxCoord()
{
    setMinMaxCoord(mKnownCoordMin, mKnownCoordMax);
}

void CurvePlotDialog::autoMinMaxTemperature()
{
    setMinMaxTemperature(mKnownTemperatureMin, mKnownTemperatureMax);
}

void CurvePlotDialog::updateAdditionalLimits()
{
    if (mIsUpdatingAdditionalLimits) {
        return;
    }

    mIsUpdatingAdditionalLimits = true;

    const double minT = mMinTemperature->value();
    const double maxT = mMaxTemperature->value();
    static const double minTemperatureDiff = 1.0;
    mMinTemperature->setForcedMaximum(maxT - minTemperatureDiff);
    mMaxTemperature->setForcedMinimum(minT + minTemperatureDiff);

    const double minCoord = mMinCoord->value();
    const double maxCoord = mMaxCoord->value();
    static const double minCoordDiff = 0.2;
    mMinCoord->setMaximum(maxCoord - minCoordDiff);
    mMaxCoord->setMinimum(minCoord + minCoordDiff);

    mIsUpdatingAdditionalLimits = false;
}

void CurvePlotDialog::updateAutoMinMaxCoordButton()
{
    const bool isAuto = qFuzzyCompare(mMinCoord->value(), mKnownCoordMin) &&
                        qFuzzyCompare(mMaxCoord->value(), mKnownCoordMax);
    mAutoMinMaxCoord->setEnabled(!isAuto);
}

void CurvePlotDialog::updateAutoMinMaxTemperatureButton()
{
    const bool isAuto = qFuzzyCompare(mMinTemperature->value(), mKnownTemperatureMin) &&
                        qFuzzyCompare(mMaxTemperature->value(), mKnownTemperatureMax);
    mAutoMinMaxTemperature->setEnabled(!isAuto);
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
        const QPointF &center = block->metersCenter();
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

void CurvePlotDialog::emitSavingMinMax()
{
    if (mSaveMinMaxCoord->isChecked()) {
        emit savingMinMaxCoord(mMinCoord->value(),
                               mMaxCoord->value());
    }
    if (mSaveMinMaxTemperature->isChecked()) {
        emit savingMinMaxTemperature(mMinTemperature->value(),
                                     mMaxTemperature->value());
    }
}

QDate CurvePlotDialog::modelDate() const
{
    MainWindow *m = Application::findMainWindow(this);
    Q_ASSERT(m);
    return m->controlPanel()->computationControl()->currentDate();
}

void CurvePlotDialog::updateKnownCoordLimits()
{
    mKnownCoordMin = mCoordsMain.first() -
                     (mOrientation == Qt::Horizontal
                      ? mSlice.first()->metersRect().width()
                      : mSlice.first()->metersRect().height()) / 2.0;

    mKnownCoordMax = mCoordsMain.last() +
                     (mOrientation == Qt::Horizontal
                      ? mSlice.last()->metersRect().width()
                      : mSlice.last()->metersRect().height()) / 2.0;
}

void CurvePlotDialog::updateKnownTemperatureLimits()
{
    mKnownTemperatureMin = std::numeric_limits<double>::infinity();
    mKnownTemperatureMax = -std::numeric_limits<double>::infinity();

    foreach (double t, mTemperatures) {
        if (t < mKnownTemperatureMin) {
            mKnownTemperatureMin = t;
        }
        if (t > mKnownTemperatureMax) {
            mKnownTemperatureMax = t;
        }
    }

    mKnownTemperatureMin = qFloor(mKnownTemperatureMin);
    mKnownTemperatureMax = qCeil(mKnownTemperatureMax);

    const double temperatureRange = mKnownTemperatureMax - mKnownTemperatureMin;

    // Если полученный диапазон слишком мал, выставим его по average(minT, maxT)
    static const double minTemperatureRange = 5.0;
    if (temperatureRange < minTemperatureRange) {
        const double avgT = (mKnownTemperatureMin + mKnownTemperatureMax) / 2.0;
        static const double deltaT = minTemperatureRange / 2.0;

        mKnownTemperatureMin = qFloor(avgT - deltaT);
        mKnownTemperatureMax = qCeil(avgT + deltaT);
    }
}

void CurvePlotDialog::setMinMaxCoord(double min, double max)
{
    Q_ASSERT(min < max);

    mIsUpdatingAdditionalLimits = true;

    mMinCoord->setMaximum(max);
    mMaxCoord->setMinimum(min);

    mMinCoord->setValue(min);
    mMaxCoord->setValue(max);

    mIsUpdatingAdditionalLimits = false;
    updateAdditionalLimits();
}

void CurvePlotDialog::setMinMaxTemperature(double min, double max)
{
    Q_ASSERT(min < max);

    mIsUpdatingAdditionalLimits = true;

    mMinTemperature->resetForcedMaximum();
    mMaxTemperature->resetForcedMinimum();

    mMinTemperature->setValue(min);
    mMaxTemperature->setValue(max);

    mIsUpdatingAdditionalLimits = false;
    updateAdditionalLimits();
}

CurvePlotDialogSpawner::CurvePlotDialogSpawner(QWidget *parent)
    : QObject(parent)
    , mSavedMinMaxCoord(false)
    , mMinCoord()
    , mMaxCoord()
    , mSavedMinMaxTemperature(false)
    , mMinTemperature()
    , mMaxTemperature()
{

}

CurvePlotDialog *CurvePlotDialogSpawner::execDialog(Block *block,
                                                    Qt::Orientation orientation)
{
    QWidget *widget = qobject_cast<QWidget*>(parent());
    Q_ASSERT(widget);

    CurvePlotDialog *dialog = new CurvePlotDialog(block, orientation, widget);

    connect(dialog, SIGNAL(savingMinMaxCoord(double,double)),
            SLOT(saveMinMaxCoord(double,double)));
    connect(dialog, SIGNAL(savingMinMaxTemperature(double,double)),
            SLOT(saveMinMaxTemperature(double,double)));

    if (mSavedMinMaxCoord) {
        dialog->loadMinMaxCoord(mMinCoord, mMaxCoord);
        mSavedMinMaxCoord = false;
    }

    if (mSavedMinMaxTemperature) {
        dialog->loadMinMaxTemperature(mMinTemperature, mMaxTemperature);
        mSavedMinMaxTemperature = false;
    }

    dialog->exec();
}

void CurvePlotDialogSpawner::saveMinMaxCoord(double min, double max)
{
    mSavedMinMaxCoord = true;
    mMinCoord = min;
    mMaxCoord = max;
}

void CurvePlotDialogSpawner::saveMinMaxTemperature(double min, double max)
{
    mSavedMinMaxTemperature = true;
    mMinTemperature = min;
    mMaxTemperature = max;
}
