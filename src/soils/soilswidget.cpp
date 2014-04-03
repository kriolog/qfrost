/*
 * Copyright (C) 2010-2013  Denis Pesotsky
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

#include <soils/soilswidget.h>


#include <QtWidgets/QTableView>

#include <checkboxdelegate.h>
#include <soils/soileditdialog.h>

using namespace qfgui;

SoilsWidget::SoilsWidget(QWidget *parent, bool isCompact, ItemsModel *model)
    : ItemsWidget(isCompact,
                  // FIXME: почему если тут сделать SoilsModel(this), оно падает?
                  model == NULL ? new SoilsModel(parent) : model,
                  SoilEditDialog::staticMetaObject,
                  parent)
{
    // HACK: если бы мы сделали в списке инициализации new SoilsModel(this),
    //       оно бы упало, так что нормального пэрента даём здесь
    if (model == NULL) {
        this->model()->setParent(this);
    }
    Q_ASSERT(qobject_cast<SoilsModel *>(model) != NULL || model == NULL);
    if (!isCompact) {
        QTableView *tmp = qobject_cast<QTableView *>(view());
        Q_ASSERT(tmp != NULL);
        tmp->setItemDelegateForColumn(SM_UsesUnfrozenWaterCurve,
                                      new CheckBoxDelegate(tmp));
        tmp->hideColumn(SM_UnfrozenWaterCurve);
    }

    setToolTips(tr("Create new soil"),
                tr("Duplicate selected soil"),
                tr("Edit selected soils"),
                tr("Remove selected soils"));

    setTableEditorWindowTitle(tr("Soils Table Editor"));
}
