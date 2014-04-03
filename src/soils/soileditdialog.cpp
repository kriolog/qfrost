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

#include <soils/soileditdialog.h>

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDataWidgetMapper>

#include <soils/soilsmodel.h>
#include <soils/soil.h>
#include <soils/unfrozenwaterwidget.h>
#include <core/soilblock.h>
#include <physicalpropertyspinbox.h>
#include <qfrost.h>
#include <mainwindow.h>

using namespace qfgui;

SoilEditDialog::SoilEditDialog(ItemsModel *model,
                               const QStringList &forbiddenNames,
                               bool isNewItem,
                               QWidget *parent)
    : ItemEditDialog(model, forbiddenNames, parent)
    , mConductivityTh(new PhysicalPropertySpinBox(this))
    , mConductivityFr(new PhysicalPropertySpinBox(this))
    , mCapacityTh(new PhysicalPropertySpinBox(this))
    , mCapacityFr(new PhysicalPropertySpinBox(this))
    , mTransitionTemperature(new PhysicalPropertySpinBox(this))
    , mTransitionHeat(new PhysicalPropertySpinBox(this))
    , mInternalHeatSourcePowerDensity(new PhysicalPropertySpinBox(this))
    , mUnfrozenWater(new UnfrozenWaterWidget(this))
{
    Q_ASSERT(qobject_cast< SoilsModel * >(model) != NULL);

    setWindowTitle((isNewItem
                    ? tr("New Soil")
                    : tr("Edit Soil %1")
                    .arg(locale().quoteString(model->itemAt(0)->name())))
                   + MainWindow::windowsPostfix());

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->setContentsMargins(QMargins());
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QVBoxLayout *rightLayout = new QVBoxLayout();

    mainLayout->addLayout(leftLayout, 1);
    mainLayout->addLayout(rightLayout, 1);

    addLayout(mainLayout);

    /**************************** Теплопроводность ****************************/
    QGroupBox *conductivityBox = new QGroupBox(tr("Heat Conductivity \316\273"),
            this);
    QFormLayout *conductivityLayout = new QFormLayout(conductivityBox);
    conductivityLayout->addRow(tr("&Thawed"), mConductivityTh);
    conductivityLayout->addRow(tr("&Frozen"), mConductivityFr);
    leftLayout->addStretch();
    leftLayout->addWidget(conductivityBox);
    /**************************************************************************/

    /****************************** Теплоёмкость ******************************/
    QGroupBox *capacityBox = new QGroupBox(tr("Heat Capacity C"), this);
    QFormLayout *capacityLayout = new QFormLayout(capacityBox);
    capacityLayout->addRow(tr("Thawe&d"), mCapacityTh);
    capacityLayout->addRow(tr("Froze&n"), mCapacityFr);
    leftLayout->addStretch();
    leftLayout->addWidget(capacityBox);
    /**************************************************************************/

    /**************************** Фазовые переходы ****************************/
    QGroupBox *transitionBox = new QGroupBox(tr("Phase Transition Parameters"),
            this);
    QFormLayout *transitionLayout = new QFormLayout(transitionBox);

    transitionLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    transitionLayout->addRow(tr("T&emperature T<sub>bf</sub>"),
                             mTransitionTemperature);
    transitionLayout->addRow(tr("&Heat (per unit volume) Q<sub>ph</sub>"),
                             mTransitionHeat);
    leftLayout->addStretch();
    leftLayout->addWidget(transitionBox);

    connect(mUnfrozenWater, SIGNAL(unfrozenWaterUsageToggled(bool)),
            mTransitionHeat, SLOT(setDisabled(bool)));
    connect(mUnfrozenWater, SIGNAL(unfrozenWaterUsageToggled(bool)),
            mTransitionTemperature, SLOT(setDisabled(bool)));

    connect(mUnfrozenWater, SIGNAL(unfrozenWaterUsageToggled(bool)),
            transitionLayout->labelForField(mTransitionHeat),
            SLOT(setDisabled(bool)));
    connect(mUnfrozenWater, SIGNAL(unfrozenWaterUsageToggled(bool)),
            transitionLayout->labelForField(mTransitionTemperature),
            SLOT(setDisabled(bool)));
    /**************************************************************************/

    /****************************** Прочее ************************************/
    QGroupBox *miscBox = new QGroupBox(tr("Miscellaneous"), this);
    QFormLayout *miscLayout = new QFormLayout(miscBox);
    miscLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    miscLayout->addRow(tr("&Int. heat sources power density F"),
                       mInternalHeatSourcePowerDensity);

    leftLayout->addStretch();
    leftLayout->addWidget(miscBox);
    leftLayout->addStretch();

    /**************************** Кривая нез. воды ****************************/
    rightLayout->addStretch();
    rightLayout->addWidget(mUnfrozenWater);
    rightLayout->addStretch();

    /**************************************************************************/

    Q_ASSERT(mapper() != NULL);
    mapper()->addMapping(mConductivityFr, SM_ConductivityFrozen);
    mapper()->addMapping(mConductivityTh, SM_ConductivityThawed);
    mapper()->addMapping(mCapacityFr, SM_CapacityFrozen);
    mapper()->addMapping(mCapacityTh, SM_CapacityThawed);
    mapper()->addMapping(mTransitionHeat, SM_TransitionHeat);
    mapper()->addMapping(mTransitionTemperature, SM_TransitionTemperature);
    mapper()->addMapping(mInternalHeatSourcePowerDensity,
                         SM_InternalHeatSourcePowerDensity);
    mUnfrozenWater->setMapper(mapper());

    mapper()->revert();

    connect(mUnfrozenWater, SIGNAL(unfrozenWaterUsageToggled(bool)),
            SLOT(updateTransitionHeatAndTemperatureConnections(bool)));

    Soil *soil = qobject_cast<Soil *>(model->itemAt(0));
    Q_ASSERT(soil != NULL);
    updateTransitionHeatAndTemperatureConnections(soil->usesUnfrozenWaterCurve());
    mUnfrozenWater->connectTo(soil);

    // Принимаем минимальный размер
    resize(0, 0);
}

void SoilEditDialog::updateTransitionHeatAndTemperatureConnections(bool usesUnfrozenWaterCurve)
{
    Soil *soil = qobject_cast<Soil *>(model()->itemAt(0));
    Q_ASSERT(soil != NULL);
    if (usesUnfrozenWaterCurve) {
        // Рассчитанные в грунте значения присваиваются спинбоксам
        connect(soil, SIGNAL(transitionTemperatureChanged(double)),
                mTransitionTemperature, SLOT(setValue(double)));
        connect(soil, SIGNAL(transitionHeatChanged(double)),
                mTransitionHeat, SLOT(setValue(double)));

        // (Округленные) рассчитанные значения не присваиваются грунту
        mapper()->removeMapping(mTransitionTemperature);
        mapper()->removeMapping(mTransitionHeat);

        mTransitionTemperature->setValue(soil->transitionTemperature());
        mTransitionHeat->setValue(soil->transitionHeat());
    } else {
        mTransitionTemperature->disconnect(soil);
        mTransitionHeat->disconnect(soil);

        mapper()->addMapping(mTransitionHeat, SM_TransitionHeat);
        mapper()->addMapping(mTransitionTemperature, SM_TransitionTemperature);
    }
}
