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
#include <QtGui/QIcon>

using namespace qfgui;

PositionLabel::PositionLabel(const QString &title, QWidget *parent)
    : QFrame(parent)
    , mTitleLabel(new QLabel(title + ":", this))
    , mXLabel(new QLabel(this))
    , mYLabel(new QLabel(this))
    , mPositionText(new QStackedWidget(this))
{
    init();
}

PositionLabel::PositionLabel(const QIcon &icon, QWidget *parent)
    : QFrame(parent)
    , mTitleLabel(new QLabel(this))
    , mXLabel(new QLabel(this))
    , mYLabel(new QLabel(this))
    , mPositionText(new QStackedWidget(this))
{
    const int iconDimension = 16;
    mTitleLabel->setPixmap(icon.pixmap(iconDimension, iconDimension));

    init();
}

void PositionLabel::init()
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

    mXLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    mYLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QLabel *noPointLabel = new QLabel("\342\200\224", this);
    noPointLabel->setAlignment(Qt::AlignCenter);

    mTitleLabel->setAlignment(mTitleLabel->pixmap()->isNull()
                              ? Qt::AlignCenter
                              : Qt::AlignBottom); // лучше для выбранных иконок

    QWidget *coordsWidget = new QWidget(this);
    QHBoxLayout *coordsLayout = new QHBoxLayout(coordsWidget);
    coordsLayout->setSpacing(0);
    coordsLayout->setMargin(0);
    coordsLayout->setContentsMargins(QMargins());
    coordsLayout->addWidget(mXLabel);
    coordsLayout->addWidget(new QLabel("; ", this));
    coordsLayout->addWidget(mYLabel);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addWidget(mTitleLabel);
    mainLayout->addSpacing(2);
    mainLayout->addStretch();
    mainLayout->addWidget(mPositionText);
    mainLayout->addStretch();

    mPositionText->addWidget(noPointLabel);
    mPositionText->addWidget(coordsWidget);
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
    const double sceneUnits = QFrost::sceneUnits(420);
    updateText(QPointF(-sceneUnits, -sceneUnits));
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents
                                | QEventLoop::ExcludeSocketNotifiers);
    mXLabel->setMinimumWidth(mXLabel->sizeHint().width());
    mYLabel->setMinimumWidth(mYLabel->sizeHint().width());
    updateText(QFrost::noPoint);
}
