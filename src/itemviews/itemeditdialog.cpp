/*
 * Copyright (C) 2012  Denis Pesotsky
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

#include "itemeditdialog.h"

#include <itemviews/itemsmodel.h>

#include <QtWidgets/QDataWidgetMapper>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QAbstractItemView>

#include <units/physicalpropertydelegate.h>

using namespace qfgui;

ItemEditDialog::ItemEditDialog(ItemsModel *model,
                               const QStringList &forbiddenNames,
                               QWidget *parent)
    : QDialog(parent)
    , mModel(model)
    , mForbiddenNames(forbiddenNames)
    , mMapper(new QDataWidgetMapper(this))
    , mLayout(new QVBoxLayout(this))
    , mButtons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel))
    , mTopFormLayout(new QFormLayout)
{
    Q_ASSERT(model->rowCount() == 1);

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    mLayout->addLayout(mTopFormLayout);
    mLayout->addWidget(mButtons);

    QLineEdit *nameEdit = new QLineEdit(this);
    addRow(tr("&Name"), nameEdit);
    connect(nameEdit, SIGNAL(textChanged(QString)), SLOT(updateOkButton(QString)));

    mMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mMapper->setItemDelegate(new PhysicalPropertyDelegate(mMapper));
    mMapper->setModel(mModel);
    mMapper->setCurrentIndex(0);

    mMapper->addMapping(nameEdit, 1);

    /// Во время submit модель может извещать об измении всякой фигни, что
    /// будет изменять значения в замаппенных виджетах, а оно нам не надо
    disconnect(mModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
               mMapper, 0);

    connect(mButtons, SIGNAL(accepted()), SLOT(accept()));
    connect(mButtons, SIGNAL(rejected()), SLOT(reject()));
    connect(this, SIGNAL(accepted()), mMapper, SLOT(submit()));
}

void ItemEditDialog::addRow(const QString &labelText, QWidget *widget)
{
    mTopFormLayout->addRow(labelText, widget);
}

void ItemEditDialog::addWidget(QWidget *widget)
{
    mLayout->insertWidget(mLayout->count() - 1, widget);
}

void ItemEditDialog::addLayout(QLayout *layout)
{
    mLayout->insertLayout(mLayout->count() - 1, layout);
}

void ItemEditDialog::updateOkButton(const QString &name)
{
    const bool nameIsGood = !name.isEmpty()
                            && !mForbiddenNames.contains(name.simplified(),
                                    Qt::CaseInsensitive);
    mButtons->button(QDialogButtonBox::Ok)->setEnabled(nameIsGood);
}

void ItemEditDialog::disableChanges()
{
    QList<const char *> disabledWidgetClasses;
    disabledWidgetClasses << "QPushButton"
                          << "QLineEdit" // Это включает и спинбоксы
                          << "QComboBox"
                          << "QAbstractItemView";
    foreach(QWidget * widget, findChildren<QWidget *>()) {
        foreach(const char * className, disabledWidgetClasses) {
            if (widget->inherits(className)) {
                widget->setDisabled(true);
                break;
            }
        }
    }

    mButtons->button(QDialogButtonBox::Cancel)->setEnabled(true);
}
