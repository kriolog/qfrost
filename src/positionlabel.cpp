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

#include "positionlabel.h"

#include <qfrost.h>
#include <units/units.h>

#include <QtCore/QLocale>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStackedWidget>

using namespace qfgui;

PositionLabel::PositionLabel(const QString &title,
                             QWidget *parent)
    : QFrame(parent)
    , mXLabel(new QLabel(this))
    , mYLabel(new QLabel(this))
    , mPositionText(new QStackedWidget(this))
{
    mXLabel->setAlignment(Qt::AlignRight);
    mYLabel->setAlignment(Qt::AlignLeft);

    QLabel *noPointLabel = new QLabel("\342\200\224", this);
    noPointLabel->setAlignment(Qt::AlignCenter);

    QLabel *titleLabel = new QLabel(title + ":", this);
    titleLabel->setAlignment(Qt::AlignRight);

    QWidget *coordsWidget = new QWidget(this);
    QHBoxLayout *coordsLayout = new QHBoxLayout(coordsWidget);
    coordsLayout->setSpacing(0);
    coordsLayout->setMargin(0);
    coordsLayout->addWidget(mXLabel);
    coordsLayout->addWidget(new QLabel("; ", this));
    coordsLayout->addWidget(mYLabel);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->addWidget(titleLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(mPositionText);

    mPositionText->addWidget(noPointLabel);
    mPositionText->addWidget(coordsWidget);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QString PositionLabel::metersString(double v) const
{
    QString number = locale().toString(QFrost::meters(v), 'f', 2);
    number.replace("-", "\342\210\222");
    return number + Units::meterSuffix();
}

void PositionLabel::updateText(const QPointF &point)
{
    bool isRealPoint = (point != QFrost::noPoint);
    mPositionText->setCurrentIndex(isRealPoint);
    if (isRealPoint) {
        mXLabel->setText(metersString(point.x()));
        mYLabel->setText(metersString(point.y()));
    }
}

void PositionLabel::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    updateText(QPointF(-QFrost::sceneHalfSize * 2, -QFrost::sceneHalfSize * 2));
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents
                                | QEventLoop::ExcludeSocketNotifiers);
    mXLabel->setMinimumWidth(mXLabel->sizeHint().width());
    mYLabel->setMinimumWidth(mYLabel->sizeHint().width());
    updateText(QFrost::noPoint);
}
