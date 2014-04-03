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

#include <plot/plotdialog.h>

#include <mgl/mgl_qt.h>

#include <QtGui/QDialogButtonBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <mainwindow.h>

using namespace qfgui;

PlotDialog::PlotDialog(QWidget *parent)
    : QDialog(parent)
    , mMglWidget(new QMathGL())
    , mMainLayout(new QVBoxLayout(this))
{
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    mMglWidget->autoResize = true;

    QPushButton *saveButton = new QPushButton(tr("Save"), this);
    connect(saveButton, SIGNAL(clicked(bool)), SLOT(save()));

    mMainLayout->addWidget(mMglWidget, 1);
    mMainLayout->addWidget(saveButton);
    mMainLayout->addWidget(buttons);
}

void PlotDialog::save()
{
    MainWindow *m = qobject_cast<MainWindow *>(parentWidget());
    Q_ASSERT(m != NULL);
    QString filters;
    filters += tr("EPS files") + " (*.eps)" + ";;"
               + tr("PNG files") + " (*.png)" + ";;";

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Plot As"),
                       m->currentFileBasePath()
                       + ".eps",
                       filters);
    // TMP FIX: баг setExtension у MathGL (см. рассылку)
    QString fileBasePath = fileName;
    fileBasePath.chop(4);

    if (fileName.isNull()) {
        return;
    }

    if (QFileInfo(fileName).exists()) {
        if (!QFile(fileName).remove()) {
            QMessageBox::warning(this, tr("Save Error"),
                                 tr("Could not remove old file"));
            return;
        }
    }

    QString suffix = QFileInfo(fileName).suffix();
    if (suffix == "png") {
        mMglWidget->exportPNG(fileBasePath);
    } else if (suffix == "eps") {
        mMglWidget->exportEPS(fileBasePath);
    } else {
        QMessageBox::warning(this, tr("Save Error"), tr("Bad file format"));
        return;
    }

    if (!QFileInfo(fileName).exists()) {
        QMessageBox::warning(this, tr("Save Error"), tr("Could not save file"));
        return;
    }
}

void PlotDialog::redrawData()
{
    mMglWidget->update();
}

void PlotDialog::addLayout(QLayout *layout)
{
    mMainLayout->insertLayout(mMainLayout->count() - 1, layout);
}

void PlotDialog::addWidget(QWidget *widget)
{
    mMainLayout->insertWidget(mMainLayout->count() - 1, widget);
}

void PlotDialog::setDraw(mglDraw *draw)
{
    mMglWidget->setDraw(draw);
}

#include "plotdialog.moc"