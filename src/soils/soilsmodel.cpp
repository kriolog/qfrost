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

#include "soilsmodel.h"

#include <mainwindow.h>
#include <qfrost.h>
#include <units.h>
#include <soils/soil.h>

#include <QtGui/QStandardItemModel>

#include <HierarchicalHeaderView.h>

using namespace qfgui;

static QList<QColor> defaultColors()
{
    QList<QColor> colors;
    colors << QColor(250, 250, 90)
           << QColor(255, 128, 255)
           << QColor(255, 180, 90)
           << QColor(220, 100, 100)
           << QColor(0,   180, 180)
           << QColor(80,  200, 80)
           << QColor(20,  140, 240)
           << QColor(180, 180, 180)
           << QColor(60,  220, 220)
           << QColor(170, 90,  180);
    return colors;
}

static QHash<int, PhysicalProperty> physicalProperties()
{
    QHash<int, PhysicalProperty> result;
    result.insert(SM_ConductivityThawed, Conductivity);
    result.insert(SM_ConductivityFrozen, Conductivity);
    result.insert(SM_CapacityFrozen, Capacity);
    result.insert(SM_CapacityThawed, Capacity);
    result.insert(SM_TransitionTemperature, Temperature);
    result.insert(SM_TransitionHeat, TransitionHeat);
    result.insert(SM_MoistureTotal, Moisture);
    result.insert(SM_DryDensity, Density);
    result.insert(SM_InternalHeatSourcePowerDensity, PowerDensity);
    return result;
}

SoilsModel::SoilsModel(QWidget *parent)
    : ItemsModel(Soil::staticMetaObject, parent,
                 defaultColors(), physicalProperties())
    , mHorizontalHeaderModel(new QStandardItemModel(this))
{
    Q_ASSERT(parent != NULL);
    updateHorizontalHeaderModel();
    connect(Units::units(this), SIGNAL(changed()), SLOT(updateHorizontalHeaderModel()));
}

SoilsModel::SoilsModel(ItemsModel *other, int row)
    : ItemsModel(other, row)
    , mHorizontalHeaderModel(NULL)
{
    Q_ASSERT(qobject_cast< SoilsModel * >(other) != NULL);
}

QVariant SoilsModel::data(const QModelIndex &index, int role) const
{
    if (role == HierarchicalHeaderView::HorizontalHeaderDataRole) {
        Q_ASSERT(mHorizontalHeaderModel != NULL);
        QVariant v;
        v.setValue(static_cast<QObject *>(mHorizontalHeaderModel));
        return v;
    }

    return ItemsModel::data(index, role);
}

Qt::ItemFlags SoilsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = ItemsModel::flags(index);
    if (index.isValid()) {
        bool isEditable = true;
        switch (index.column()) {
        case SM_Name:
        case SM_ConductivityThawed:
        case SM_ConductivityFrozen:
        case SM_CapacityThawed:
        case SM_CapacityFrozen:
        case SM_Color:
        case SM_UsesUnfrozenWaterCurve:
        case SM_InternalHeatSourcePowerDensity:
            break;
        case SM_TransitionTemperature:
        case SM_TransitionHeat:
            isEditable = !data(this->index(index.row(), SM_UsesUnfrozenWaterCurve)).toBool();
            break;
        case SM_UnfrozenWaterCurve:
        case SM_MoistureTotal:
        case SM_DryDensity:
            isEditable = data(this->index(index.row(), SM_UsesUnfrozenWaterCurve)).toBool();
            break;
        }
        if (!isEditable) {
            result ^= Qt::ItemIsEditable | Qt::ItemIsEnabled;
        }
    }
    return result;
}

void SoilsModel::updateHorizontalHeaderModel()
{
    Q_ASSERT(mHorizontalHeaderModel != NULL);
    mHorizontalHeaderModel->clear();
    QList<QStandardItem *> l;
    for (int i = 0; i < columnCount(); ++i) {
        Q_ASSERT(l.isEmpty());
        QStandardItem *item = NULL;
        switch (i) {
        case SM_Name:
            item = new QStandardItem(tr("Soil"));
            break;
        case SM_ConductivityThawed: {
            item = new QStandardItem(tr("\316\273")
                                     + Units::unit(this, Conductivity).headerSuffix());
            l << new QStandardItem(tr("thawed"));
            item->appendColumn(l);
            l.clear();
            l << new QStandardItem(tr("frozen"));
            item->appendColumn(l);
            l.clear();
        }
        ++i;
        break;
        case SM_ConductivityFrozen:
            Q_ASSERT(false);
            break;
        case SM_CapacityThawed: {
            item = new QStandardItem(tr("C")
                                     + Units::unit(this, Capacity).headerSuffix());
            l << new QStandardItem(tr("thawed"));
            item->appendColumn(l);
            l.clear();
            l << new QStandardItem(tr("frozen"));
            item->appendColumn(l);
            l.clear();
        }
        ++i;
        break;
        case SM_CapacityFrozen:
            break;
        case SM_TransitionTemperature:
            item = new QStandardItem(tr("T<sub>bf</sub>")
                                     + Units::unit(this, Temperature).headerSuffix());
            break;
        case SM_TransitionHeat:
            item = new QStandardItem(tr("Q<sub>ph</sub>")
                                     + Units::unit(this, TransitionHeat).headerSuffix());
            break;
        case SM_Color:
            item = new QStandardItem(tr("Color"));
            break;
        case SM_UsesUnfrozenWaterCurve:
            item = new QStandardItem(tr("Uses\nCurve"));
            break;
        case SM_UnfrozenWaterCurve:
            item = new QStandardItem(tr("Unfrozen\nWater Curve"));
            break;
        case SM_MoistureTotal:
            item = new QStandardItem(tr("W<sub>tot</sub>")
                                     + Units::unit(this, Moisture).headerSuffix());
            break;
        case SM_DryDensity:
            item = new QStandardItem(tr("\317\201<sub>d</sub>")
                                     + Units::unit(this, Density).headerSuffix());
            break;
        case SM_InternalHeatSourcePowerDensity:
            item = new QStandardItem(tr("F")
                                     + Units::unit(this, PowerDensity).headerSuffix());
            break;
        default:
            Q_ASSERT(false);
        }
        Q_ASSERT(item != NULL);
        l.append(item);
        mHorizontalHeaderModel->appendColumn(l);
        l.clear();
    }
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}

QVariant SoilsModel::minimum(const QModelIndex &index) const
{
    if (index.column() == SM_MoistureTotal) {
        const Soil *soil = static_cast<const Soil *>(index.internalPointer());
        return soil->moistureTotalMinimum();
    }
    return QVariant();
}

QVariant SoilsModel::maximum(const QModelIndex &index) const
{
    if (index.column() == SM_TransitionTemperature) {
        return 0.0;
    }
    if (index.column() == SM_MoistureTotal) {
        const Soil *soil = static_cast<const Soil *>(index.internalPointer());
        return soil->moistureTotalMaximum();
    }
    return QVariant();
}

void SoilsModel::emitDataChanged(const QModelIndex &changedIndex)
{
    if (changedIndex.column() == SM_UsesUnfrozenWaterCurve) {
        const int row = changedIndex.row();
        emit dataChanged(index(row, SM_TransitionTemperature),
                         index(row, SM_DryDensity));
    } else {
        ItemsModel::emitDataChanged(changedIndex);
    }
}
