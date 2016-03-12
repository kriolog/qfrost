/*
 * Copyright (C) 2015  Denis Pesotsky
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

using namespace qfgui;

YearlyParamsWidget::YearlyParamsWidget(QWidget* parent)
    : QWidget(parent)
    , mLabel(new QLabel(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(mLabel);
    QPushButton *loadButton = new QPushButton(tr("Load"));
    layout->addWidget(loadButton);
    updateLabel();
    
    connect(loadButton, SIGNAL(clicked()), SLOT(loadFromFile()));
}

void YearlyParamsWidget::setValues(const YearlyParams& v)
{
    mValues = v;
    updateLabel();
    emit valuesChanged();
}

void YearlyParamsWidget::updateLabel()
{
    if (mValues.isEmpty()) {
        mLabel->setText(tr("No data"));
    } else {
        QString text = tr("Data for years %1-%2").arg(mValues.firstKey()).arg(mValues.lastKey());
        QStringList missingYears;
        for (int i = mValues.firstKey(); i < mValues.lastKey(); ++i) {
            if (!mValues.contains(i)) {
               missingYears.append(QString::number(i)); 
            }
        }
        if (!missingYears.isEmpty()) {
            text.append("(except years " + missingYears.join(", ") + ")");
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
