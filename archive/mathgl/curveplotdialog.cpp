/*
 * Copyright (C) 2011-2012  Denis Pesotsky
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

#include <plot/curveplotdialog.h>

#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>
#include <QtGui/QSpinBox>
#include <QtGui/QFormLayout>
#include <QtCore/qmath.h>

#include <plot/curvedraw.h>
#include <scene.h>
#include <block.h>

using namespace qfgui;

CurvePlotDialog::CurvePlotDialog(const Scene *scene, QWidget *parent)
    : PlotDialog(parent)
    , mMinT(new QSpinBox(this))
    , mMaxT(new QSpinBox(this))
    , mMinCoord(new QSpinBox(this))
    , mMaxCoord(new QSpinBox(this))
{
    Q_ASSERT(scene->is1D());
    resize(300, 700);

    setWindowTitle("Curve Plot");

    createDraw(scene);

    connect(mDraw, SIGNAL(updated()), this, SLOT(redrawData()));

    mMinCoord->setMinimum(qFloor(-QFrost::sceneHalfSizeInMeters) + 1);
    mMaxCoord->setMaximum(qCeil(QFrost::sceneHalfSizeInMeters));

    mMinT->setMinimum(-99);
    mMaxT->setMaximum(100);

    autoLimitsT();
    autoLimitsZ();

    updateLimits();

    connect(mMinCoord, SIGNAL(valueChanged(int)),
            this, SLOT(updateLimits()));
    connect(mMaxCoord, SIGNAL(valueChanged(int)),
            this, SLOT(updateLimits()));
    connect(mMinT, SIGNAL(valueChanged(int)),
            this, SLOT(updateLimits()));
    connect(mMaxT, SIGNAL(valueChanged(int)),
            this, SLOT(updateLimits()));

    connect(mMinCoord, SIGNAL(valueChanged(int)),
            mDraw, SLOT(setMinCoord(int)));
    connect(mMaxCoord, SIGNAL(valueChanged(int)),
            mDraw, SLOT(setMaxCoord(int)));
    connect(mMinT, SIGNAL(valueChanged(int)),
            mDraw, SLOT(setMinT(int)));
    connect(mMaxT, SIGNAL(valueChanged(int)),
            mDraw, SLOT(setMaxT(int)));

    //: automatically set t limits
    QPushButton *tAutoLimit = new QPushButton(tr("Auto"));
    connect(tAutoLimit, SIGNAL(clicked()), this, SLOT(autoLimitsT()));
    //: automatically set z limits
    QPushButton *zAutoLimit = new QPushButton(tr("Auto"));
    connect(zAutoLimit, SIGNAL(clicked()), this, SLOT(autoLimitsZ()));

    QHBoxLayout *tLimits = new QHBoxLayout;
    tLimits->addWidget(mMinT);
    tLimits->addWidget(mMaxT);
    tLimits->addWidget(tAutoLimit);
    QHBoxLayout *zLimits = new QHBoxLayout;
    zLimits->addWidget(mMinCoord);
    zLimits->addWidget(mMaxCoord);
    zLimits->addWidget(zAutoLimit);

    QFormLayout *limits = new QFormLayout;
    limits->addRow(tr("t:"), tLimits);
    limits->addRow(tr("z:"), zLimits);

    QCheckBox *plotTemperatesBox = new QCheckBox(tr("Draw t/v"));
    connect(plotTemperatesBox, SIGNAL(toggled(bool)),
            mDraw, SLOT(setPlotTemperatures(bool)));
    connect(plotTemperatesBox, SIGNAL(toggled(bool)),
            mMinT, SLOT(setEnabled(bool)));
    connect(plotTemperatesBox, SIGNAL(toggled(bool)),
            mMaxT, SLOT(setEnabled(bool)));
    connect(plotTemperatesBox, SIGNAL(toggled(bool)),
            mMaxT, SLOT(setEnabled(bool)));
    connect(plotTemperatesBox, SIGNAL(toggled(bool)),
            tAutoLimit, SLOT(setEnabled(bool)));
    plotTemperatesBox->setChecked(true);

    addWidget(plotTemperatesBox);
    addLayout(limits);

    setDraw(mDraw);

    resize(250, 600);
}

/// TODO: вынести это в qfrost.h
bool zLessThan(const Block *b1, const Block *b2)
{
    QPointF center1 = b1->rect().center();
    QPointF center2 = b2->rect().center();
    return center1.y() < center2.y();
}

void CurvePlotDialog::createDraw(const Scene *scene)
{
    QList<Block *> blocks = scene->blocks();
    QVector<float> coords;
    QVector<float> temperatures;
    QVector<float> transitionTemperatures;
    QVector<float> thawedParts;
    qSort(blocks.begin(), blocks.end(), zLessThan);
    foreach(Block * block, blocks) {
        const qfcore::SoilBlock *soilBlock = block->soilBlock();
        Q_ASSERT(block->soilBlock()->thawedPartIsOk());
        coords << QFrost::meters(block->rect().center().y());
        temperatures << soilBlock->temperature();
        transitionTemperatures << soilBlock->transitionTemperature();
        thawedParts << soilBlock->thawedPart();
    }
    mDraw = new CurveDraw(coords,
                          temperatures,
                          transitionTemperatures,
                          thawedParts,
                          this);
}

void CurvePlotDialog::updateLimits()
{
    mMinCoord->setMaximum(mMaxCoord->value() - 1);
    mMaxCoord->setMinimum(mMinCoord->value() + 1);
    mMinT->setMaximum(mMaxT->value() - 1);
    mMaxT->setMinimum(mMinT->value() + 1);
}

void CurvePlotDialog::autoLimitsT()
{
    mDraw->autoLimitsT();
    mMinT->setMaximum(999999);
    mMaxT->setMinimum(-999999);

    mMinT->blockSignals(true);
    mMaxT->blockSignals(true);

    mMinT->setValue(mDraw->minT());
    mMaxT->setValue(mDraw->maxT());

    mMinT->blockSignals(false);
    mMaxT->blockSignals(false);

    updateLimits();
}

void CurvePlotDialog::autoLimitsZ()
{
    mDraw->autoLimitsZ();
    mMinCoord->setMaximum(999999);
    mMaxCoord->setMinimum(-999999);

    mMinCoord->blockSignals(true);
    mMaxCoord->blockSignals(true);

    mMinCoord->setValue(mDraw->minCoord());
    mMaxCoord->setValue(mDraw->maxCoord());

    mMinCoord->blockSignals(false);
    mMaxCoord->blockSignals(false);

    updateLimits();
}

#include "curveplotdialog.moc"