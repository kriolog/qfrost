/*
 * Copyright (C) 2012-2014  Denis Pesotsky
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

#include "welcomedialog.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QAction>
#include <QtWidgets/QRadioButton>

#include <mainwindow.h>
#include "control_panel/controlpanel.h"
#include "control_panel/computationcontrol.h"
#include "units.h"

using namespace qfgui;

WelcomeDialog::WelcomeDialog(MainWindow *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("New Document Settings"));

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);
    setAttribute(Qt::WA_DeleteOnClose, true);

    setMinimumWidth(350);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QGridLayout *titleLayout = new QGridLayout();
    mainLayout->addLayout(titleLayout);
    mainLayout->addStretch();
    titleLayout->setColumnStretch(0, 1);

    QLabel *pictureLabel = new QLabel(this);
    pictureLabel->setPixmap(QApplication::windowIcon().pixmap(128));
    titleLayout->addWidget(pictureLabel, 0 , 1,
                           Qt::AlignHCenter | Qt::AlignVCenter);

    QVBoxLayout *textLayout = new QVBoxLayout();
    QLabel *label1 = new QLabel(tr("Welcome to <b>%1</b>!").arg(QCoreApplication::applicationName()));
    // FIXME брать названия панели и меню прямо из них
    QLabel *label2 = new QLabel(tr("Please choose settings for new document."
                                   "You can change this settings at any time"
                                   " in <i>%1</i> panel and in <i>%2</i> menu.")
                                .arg(parent->controlPanel()->computationTabText())
                                .arg(parent->settingsMenuText()));
    label2->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    label2->setWordWrap(true);
    textLayout->addWidget(label1);
    textLayout->addWidget(label2);
    titleLayout->addLayout(textLayout, 0, 0,
                           Qt::AlignLeft | Qt::AlignVCenter);

    QGroupBox *unitsGroupBox = new QGroupBox(tr("&Units System"), this);
    QVBoxLayout *unitsLayout = new QVBoxLayout(unitsGroupBox);
    QActionGroup *unitsActionGroup = new UnitsSystemActionGroup(this);
    foreach(QAction * unitAction, unitsActionGroup->actions()) {
        QRadioButton *unitRadioButton = new QRadioButton(unitAction->text(), unitsGroupBox);
        if (unitAction->isChecked()) {
            unitRadioButton->setChecked(true);
        }
        connect(unitRadioButton, SIGNAL(toggled(bool)),
                unitAction, SLOT(setChecked(bool)));
        unitsLayout->addWidget(unitRadioButton);
    }
    mainLayout->addWidget(unitsGroupBox);

    QFormLayout *formLayout = new QFormLayout;
    mainLayout->addLayout(formLayout);

    QComboBox *problemTypeOrig = parent->controlPanel()->computationControl()->problemTypeComboBox();
    QComboBox *problemType = new QComboBox(this);
    problemType->addItem(tr("Flat"));
    problemType->addItem(tr("Axially Symmetric"));
    connect(problemType, SIGNAL(currentIndexChanged(int)),
            problemTypeOrig, SLOT(setCurrentIndex(int)));
    formLayout->addRow(tr("&Problem Type:"), problemType);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok);
    mainLayout->addWidget(buttons);

    connect(buttons, SIGNAL(accepted()), SLOT(accept()));
    disableUndoBinders();
}

void WelcomeDialog::disableUndoBinders()
{
    qDebug("Disabling undo binders");
    Q_ASSERT(testAttribute(Qt::WA_DeleteOnClose));
    MainWindow *m = qobject_cast<MainWindow *>(parent());
    Q_ASSERT(m != NULL);
    m->controlPanel()->computationControl()->problemTypeComboBox()->setProperty(QFrost::UndoBinderIsEnabled, false);
}

void WelcomeDialog::enableUndoBinders()
{
    qDebug("Enabling undo binders");
    MainWindow *m = qobject_cast<MainWindow *>(parent());
    Q_ASSERT(m != NULL);
    m->controlPanel()->computationControl()->problemTypeComboBox()->setProperty(QFrost::UndoBinderIsEnabled, true);
}
