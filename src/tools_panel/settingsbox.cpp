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

#include <tools_panel/settingsbox.h>

#include <QtWidgets/QFormLayout>

using namespace qfgui;

SettingsBox::SettingsBox(const QString &title, QWidget *parent):
    QGroupBox(title, parent),
    mLayout(new QFormLayout(this))
{
    mLayout->setRowWrapPolicy(QFormLayout::WrapLongRows);
    mLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    mLayout->setContentsMargins(0, title.isEmpty() ? mLayout->contentsMargins().top() : 0,
                                0, 0);
    setFlat(true);
}

void SettingsBox::addRow(const QString &labelText, QWidget *field)
{
    mLayout->addRow(labelText, field);
}

void SettingsBox::addRow(const QString &labelText, QLayout *field)
{
    mLayout->addRow(labelText, field);
}

void SettingsBox::addRow(QWidget *field)
{
    mLayout->addWidget(field);
}

void SettingsBox::addSpacing()
{
    mLayout->addItem(new QSpacerItem(0, mLayout->verticalSpacing(),
                                     QSizePolicy::Minimum, QSizePolicy::Maximum));
}

QWidget *SettingsBox::labelForField(QWidget *field) const
{
    return mLayout->labelForField(field);
}

QWidget *SettingsBox::labelForField(QLayout *field) const
{
    return mLayout->labelForField(field);
}
