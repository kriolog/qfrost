/*
 * Copyright (C) 2015-2016  Denis Pesotsky
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

#include "yearlyparamswidget.h"

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>

#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include "yearlyparamstabledialog.h"

using namespace qfgui;

YearlyParamsWidget::YearlyParamsWidget(QWidget* parent)
    : QWidget(parent)
    , mLabel(new QLabel(this))
    , mViewDataButton(new QPushButton(QIcon::fromTheme("document-preview"),
                                      tr("View Data")))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mLabel);
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    QPushButton *loadButton = new QPushButton(QIcon::fromTheme("document-open"),
                                              tr("Load..."));
    buttonsLayout->addWidget(loadButton);
    QPushButton *helpButton = new QPushButton(QIcon::fromTheme("help-hint"),
                                              tr("File Format Info"));
    buttonsLayout->addWidget(helpButton);
    buttonsLayout->addWidget(mViewDataButton);
    layout->addLayout(buttonsLayout);
    updateLabel();
    
    connect(loadButton, SIGNAL(clicked()), SLOT(loadFromFile()));
    connect(helpButton, SIGNAL(clicked()), SLOT(showHelp()));
    connect(mViewDataButton, SIGNAL(clicked()), SLOT(showDataDialog()));
}

void YearlyParamsWidget::setValues(const YearlyParams& v)
{
    mValues = v;
    updateLabel();
    mViewDataButton->setDisabled(v.isEmpty());
    emit valuesChanged();
}

void YearlyParamsWidget::updateLabel()
{
    if (mValues.isEmpty()) {
        mLabel->setText(tr("No data"));
    } else {
        QString text = tr("Loaded data for %1–%2").arg(mValues.firstKey()).arg(mValues.lastKey());
        QStringList missingYears;
        for (int i = mValues.firstKey(); i < mValues.lastKey(); ++i) {
            if (!mValues.contains(i)) {
               missingYears.append(QString::number(i)); 
            }
        }
        if (!missingYears.isEmpty()) {
            text.append(tr(" (except years %1)").arg(missingYears.join(", ")));
        }
        mLabel->setText(text);
    }
}

void YearlyParamsWidget::loadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Yearly Params File", "Dialog Title"),
                                                    QString(),
                                                    tr("CSV files (*.csv)"));
    if (!fileName.isEmpty()) {
        YearlyParams values;
        setValues(values);
    
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QMessageBox::warning(this, 
                                 tr("Error"),
                                 tr("Can not open file %1").arg(locale().quoteString(fileName)));
        }
        QTextStream in(&file);
        
        bool hasError = false;
        while (!in.atEnd()) {
            QString line = in.readLine().simplified();
            if (line.isEmpty()) {
                continue;
            }
            QStringList lineSplit = line.split(';');
            if (lineSplit.size() != 25) {
                lineSplit = line.split(',');
            }
            if (lineSplit.size() != 25) {
                QMessageBox::warning(this, 
                                     tr("Error"),
                                     tr("Bad input:\n%1").arg(line));
                hasError = true;
                break;
            }
            QList<QPair<double, double> > list;
            for (int i = 0; i < 12; ++i) {
                bool ok1, ok2;
                
                QString v1 = lineSplit.at(i+1);
                v1.replace(',', '.');
                QString v2 = lineSplit.at(i+1+12);
                v2.replace(',', '.');
                
                list.append(qMakePair(v1.toDouble(&ok1),
                                      v2.toDouble(&ok2)));
                if (!ok1 || !ok2) {
                    QMessageBox::warning(this, 
                                        tr("Error"),
                                        tr("Bad input:\n%1").arg(line));
                    hasError = true;
                    break;
                }
            }
            if (hasError) {
                break;
            }
            bool ok;
            values.insert(lineSplit.first().toInt(&ok), list);
            if (!ok) {
                QMessageBox::warning(this, 
                                    tr("Error"),
                                    tr("Bad input:\n%1").arg(line));
                hasError = true;
                break;
            }
        }
        
        if (!hasError) {
            setValues(values);
        }
    }
}

void YearlyParamsWidget::showHelp()
{
    QMessageBox::information(this, tr("Yearly Params File Format"),
                             tr("Supported file format is CSV with lines like following:<br>"
                                "<tt>year,t1,t2,…,t12,α1,α2,…,α12</tt><br><br>"
                                "You can either use comma or semicolon as fields separator "
                                "and either dot or comma as decimal separator.<br>"
                                "If some years are missing, data from previous year is used for calculations."));
}

void YearlyParamsWidget::showDataDialog()
{
    YearlyParamsTableDialog *dialog = new YearlyParamsTableDialog(mValues, this);
    dialog->exec();
}
