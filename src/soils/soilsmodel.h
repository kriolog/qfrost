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

#ifndef QFGUI_SOILSMODEL_H
#define QFGUI_SOILSMODEL_H

#include <itemviews/itemsmodel.h>

QT_FORWARD_DECLARE_CLASS(QStandardItemModel)

namespace qfgui
{

enum SoilsModelColumn {
    SM_Color = 0,
    SM_Name = 1,
    SM_ConductivityThawed = 2,
    SM_ConductivityFrozen = 3,
    SM_CapacityThawed = 4,
    SM_CapacityFrozen = 5,
    SM_TransitionTemperature = 6,
    SM_TransitionHeat = 7,
    SM_UsesUnfrozenWaterCurve = 8,
    SM_UnfrozenWaterCurve = 9,
    SM_MoistureTotal = 10,
    SM_DryDensity = 11,
    SM_InternalHeatSourcePowerDensity = 12
};

class SoilsModel : public ItemsModel
{
    Q_OBJECT
public:
    SoilsModel(QWidget *parent);
    Q_INVOKABLE SoilsModel(ItemsModel *other, int row);

    /// ItemsModel::data + поддержка HierarchicalHeaderView
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QString newItemName() const {
        //: Default name for new soil (like "New Folder" for folders in Windows)
        return tr("New Soil");
    }
    QString newItemNameTemplate() const {
        //: Default name for new soil (%1 is number)
        return tr("New Soil %1");
    }

protected:
    QVariant minimum(const QModelIndex &index) const;
    QVariant maximum(const QModelIndex &index) const;

    void emitDataChanged(const QModelIndex &changedIndex);

private:
    /// Иерархическая модель для горизонтального хедера
    QStandardItemModel *mHorizontalHeaderModel;

private slots:
    /// Модифицирует @m mHeaderModel исходя из выбранной системы единиц
    void updateHorizontalHeaderModel();
};

}

#endif // QFGUI_SOILSMODEL_H
