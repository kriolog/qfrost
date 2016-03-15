/*
 * Copyright (C) 2016  Denis Pesotsky
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

#include "yearlyparamstabledialog.h"

#include <QtWidgets/QTableView>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGroupBox>

#include "yearlyparamstablemodel.h"

using namespace qfgui;

YearlyParamsTableDialog::YearlyParamsTableDialog(const YearlyParams& params,
                                                 QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Yearly Boundary Condition Data"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    
    YearlyParamsTableModel *tModel = new YearlyParamsTableModel(params,
                                                                YPTT_Temperature,
                                                                this);
    YearlyParamsTableModel *aModel = new YearlyParamsTableModel(params,
                                                                YPTT_HeatTransferFactor,
                                                                this);
    
    QTableView *tView = new QTableView(this);
    tView->setModel(tModel);
    tView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tView->setAlternatingRowColors(true);
    QGroupBox *tBox = new QGroupBox(tr("Temperature"), this);
    (new QVBoxLayout(tBox))->addWidget(tView);
    
    QTableView *aView = new QTableView(this);
    aView->setModel(aModel);
    aView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    aView->setAlternatingRowColors(true);
    QGroupBox *aBox = new QGroupBox(tr("Heat Transfer Factor"), this);
    (new QVBoxLayout(aBox))->addWidget(aView);
    
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttons, SIGNAL(accepted()), SLOT(accept()));
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(tBox);
    layout->addWidget(aBox);
    layout->addWidget(buttons);
    
    // HACK: принимаем мин. ширину родительского диалога, чтобы всё влезло
    setMinimumWidth(parentWidget()->window()->sizeHint().width());
}