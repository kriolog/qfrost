/*
 * Copyright (C) 2012-2015  Denis Pesotsky
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

#include "sortedpointswidget.h"

#include <soils/sortedpointsmodel.h>
#include <plot/sortedpointsplot.h>
#include <units/units.h>
#include <units/physicalpropertydelegate.h>
#include <units/physicalpropertyspinbox.h>

#include <QtWidgets/QTableView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialogButtonBox>
#include <QtCore/QPersistentModelIndex>

using namespace qfgui;

SortedPointsWidget::SortedPointsWidget(PhysicalProperty xProp,
                                       const QString &xName,
                                       const QString &xNameFull,
                                       PhysicalProperty yProp,
                                       const QString &yName,
                                       const QString &yNameFull,
                                       QWidget *parent)
    : QWidget(parent)
    , mModel(new SortedPointsModel(xName, yName, xProp, yProp, this))
    , mView(new QTableView(this))
    , mNewPoint(new QPushButton(QIcon::fromTheme("list-add"), "", this))
    , mRemovePoint(new QPushButton(QIcon::fromTheme("list-remove"), "", this))
    , mXNameFull(xNameFull)
    , mYNameFull(yNameFull)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());

    mainLayout->addWidget(mView, 1);

    mView->setMinimumHeight(180);

    mView->setModel(mModel);

    mView->setItemDelegate(new PhysicalPropertyDelegate(mView));

    mView->setSelectionBehavior(QAbstractItemView::SelectRows);

    mView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    mView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(mModel, SIGNAL(valuesChanged()), SLOT(emitValuesChanged()));

    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    buttonsLayout->setContentsMargins(QMargins());
    mainLayout->addLayout(buttonsLayout);
    buttonsLayout->addWidget(mNewPoint);
    buttonsLayout->addWidget(mRemovePoint);

    mainLayout->addWidget(new SortedPointsPlot(mModel, this));

    mNewPoint->setToolTip(tr("Add new point"));
    mRemovePoint->setToolTip(tr("Remove selected points"));

    connect(mView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(updateButtons()));
    updateButtons();

    connect(mNewPoint, SIGNAL(clicked()), SLOT(openNewPointDialog()));
    connect(mRemovePoint, SIGNAL(clicked()), SLOT(removeSelectedPoints()));
}

void SortedPointsWidget::setValues(const DoubleMap &data)
{
    static_cast<SortedPointsModel *>(mView->model())->setValues(data);
}

const DoubleMap &SortedPointsWidget::values() const
{
    Q_ASSERT(mView->model() != NULL);
    return static_cast<SortedPointsModel *>(mView->model())->values();
}

void SortedPointsWidget::emitValuesChanged()
{
    emit valuesChanged(values());
}

void SortedPointsWidget::updateButtons()
{
    const bool anythingIsSelected = mView->selectionModel()->hasSelection();
    mRemovePoint->setEnabled(anythingIsSelected);
}

void SortedPointsWidget::openNewPointDialog()
{
    SortedPointsNewPointDialog *dialog = new SortedPointsNewPointDialog(mModel,
                                                                        mXNameFull,
                                                                        mYNameFull,
                                                                        this);
    if (dialog->exec() == QDialog::Accepted) {
        mModel->addPoint(dialog->x(), dialog->y());
    }
}

void SortedPointsWidget::removeSelectedPoints()
{
    foreach (const QPersistentModelIndex &index, selectedRows()) {
        mModel->removeRow(index.row());
    }
}

QList<QPersistentModelIndex> SortedPointsWidget::selectedRows() const
{
    Q_ASSERT(mView->selectionModel()->model() == mModel);
    QList<QPersistentModelIndex> result;
    // HACK: вместо selectedRows приходится использовать такой велосипед, ибо
    //       Qt может развыделять ячейки без флага ItemIsEnabled при
    //       пересортировке QItemSelectionModel, после чего selectedRows
    //       возвращает пустой список.
    foreach(const QModelIndex & index, mView->selectionModel()->selectedIndexes()) {
        if (index.column() == 0) {
            result << QPersistentModelIndex(index);
        }
    }
    return result;
}

static QString nameForLabel(const QString &name)
{
    QStringList tokens = name.split(' ');
    if (tokens.size() <= 1) {
        return name;
    } else {
        return tokens.last();
    }
}

SortedPointsNewPointDialog::SortedPointsNewPointDialog(const SortedPointsModel *model,
                                                       const QString &xName,
                                                       const QString &yName,
                                                       QWidget *parent)
    : QDialog(parent)
    , mModel(model)
    , mSpinBoxX(new PhysicalPropertySpinBox(model->propertyX(), this))
    , mSpinBoxY(new PhysicalPropertySpinBox(model->propertyY(), this))
    , mInvalidInputNotice(new QLabel(this))
    , mButtons(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this))
    , mXNameForLabel(nameForLabel(xName))
    , mYNameForLabel(nameForLabel(yName))
    , kLabelMaskInvalidYBothBounds(tr("The %1 must be more than %2 and less than %3!")
                                   .arg(mYNameForLabel))
{
    setWindowTitle(tr("New Point"));
    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    mInvalidInputNotice->setText("<i>" + kLabelMaskInvalidYBothBounds.arg("999.9%").arg("999.9%") + "</i>");
    mInvalidInputNotice->updateGeometry();
    mInvalidInputNotice->setMinimumWidth(mInvalidInputNotice->sizeHint().width());

    mInvalidInputNotice->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setRowWrapPolicy(QFormLayout::DontWrapRows);
    formLayout->addRow(xName + ':', mSpinBoxX);
    formLayout->addRow(yName + ':', mSpinBoxY);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();
    mainLayout->addWidget(mInvalidInputNotice);
    mainLayout->addWidget(mButtons);

    mButtons->button(QDialogButtonBox::Ok)->setText(tr("Add point"));
    connect(mButtons, SIGNAL(accepted()), SLOT(accept()));
    connect(mButtons, SIGNAL(rejected()), SLOT(reject()));

    mSpinBoxX->setForcedMaximum(0.0);

    if (!mModel->values().isEmpty()) {
        const DoubleMap::ConstIterator it = mModel->values().constBegin();
        mSpinBoxX->setValue(it.key());
        mSpinBoxY->setValue(it.value());
    }

    connect(mSpinBoxX, SIGNAL(valueChanged(double)), SLOT(checkInput()));
    connect(mSpinBoxY, SIGNAL(valueChanged(double)), SLOT(checkInput()));
    checkInput();
}

double SortedPointsNewPointDialog::x() const
{
    return mSpinBoxX->value();
}

double SortedPointsNewPointDialog::y() const
{
    return mSpinBoxY->value();
}


void SortedPointsNewPointDialog::checkInput()
{
    bool xIsInvalid = false;
    bool yIsInvalid = false;

    QString textForLabel;
    // Проверяем, не совпадает ли текущий X с каким-либо из имеющихся в модели
    foreach (const double existingX, mModel->values().keys()) {
        if (qFuzzyCompare(x(), existingX)) {
            xIsInvalid = true;
            textForLabel = tr("Already has point with this %1!").arg(mXNameForLabel);
        }
        if (existingX > x()) {
            break;
        }
    }

    if (!xIsInvalid && !mModel->values().isEmpty()) {
        double lowerY = -std::numeric_limits<double>::infinity();
        double upperY = std::numeric_limits<double>::infinity();

        const Unit unitY = Units::unit(this, mModel->propertyY());

        const DoubleMap::ConstIterator itUpper = mModel->values().upperBound(x());
        if (itUpper != mModel->values().constEnd()) {
            upperY = itUpper.value();
            if (itUpper != mModel->values().constBegin()) {
                const DoubleMap::ConstIterator itLower = itUpper - 1;
                lowerY = *itLower;
                textForLabel = kLabelMaskInvalidYBothBounds
                               .arg(unitY.textFromSI(lowerY))
                               .arg(unitY.textFromSI(upperY));
            } else {
                textForLabel = tr("The %1 must be less than %2!")
                               .arg(mYNameForLabel)
                               .arg(unitY.textFromSI(upperY));
            }
        } else {
            Q_ASSERT(!mModel->values().isEmpty());
            const DoubleMap::ConstIterator itLower = mModel->values().constEnd() - 1;
            lowerY = *itLower;
            textForLabel = tr("The %1 must be more than %2!")
                           .arg(mYNameForLabel)
                           .arg(unitY.textFromSI(lowerY));
        }

        const double d = unitY.minStepSI() / 2.0;

        if (y() < lowerY + d || y() > upperY - d) {
            yIsInvalid = true;
        }
    }

    QFrost::setInputValidity(mSpinBoxX, xIsInvalid);
    QFrost::setInputValidity(mSpinBoxY, yIsInvalid);

    const bool isInvalid = xIsInvalid || yIsInvalid;

    mInvalidInputNotice->setVisible(isInvalid);
    if (isInvalid) {
        mInvalidInputNotice->setText(QString("<i>%1</i>").arg(textForLabel));
    }

    mButtons->button(QDialogButtonBox::Ok)->setEnabled(!isInvalid);
}
