/*
 * Copyright (C) 2010-2014  Denis Pesotsky
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

#include <rectangulartoolpanel.h>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QGridLayout>

#include <smartdoublespinbox.h>
#include <tools_panel/rectangulartoolsettings.h>
#include <qfrost.h>
#include <units/physicalpropertyspinbox.h>

using namespace qfgui;

RectangularToolPanel::RectangularToolPanel(QWidget *parent, bool showHeader,
        RectangularToolSettings *settings)
    : SettingsBox(showHeader ? tr("Geometry") : "" , parent)
    , mRectX(PhysicalPropertySpinBox::createSceneCoordinateSpinBox())
    , mRectY(PhysicalPropertySpinBox::createSceneCoordinateSpinBox())
    , mRectWidth(PhysicalPropertySpinBox::createSceneCoordinateSpinBox())
    , mRectHeight(PhysicalPropertySpinBox::createSceneCoordinateSpinBox())
    , mBasepoints(new QButtonGroup(this))
    , mMustEmitRectChanges(true)
    , mSettings(settings == NULL ? new RectangularToolSettings(this) : settings)
{
    mRectWidth->setMinimum(0);
    mRectWidth->setMaximum(2 * QFrost::sceneHalfSizeInMeters);
    Q_ASSERT(qobject_cast<SmartDoubleSpinBox*>(mRectHeight));
    static_cast<SmartDoubleSpinBox*>(mRectHeight)->readProperties(static_cast<SmartDoubleSpinBox*>(mRectWidth));

    addRow(tr("&X-Pos:"), mRectX);
    addRow(tr("&Y-Pos:"), mRectY);
    addSpacing();
    mRectX->setToolTip(tr("Horizontal position of current basepoint"));
    mRectY->setToolTip(tr("Vertical position of current basepoint"));

    addRow(tr("&Width:"), mRectWidth);
    addRow(tr("&Height:"), mRectHeight);
    addSpacing();

    mRectWidth->setToolTip(tr("Width of tool"));
    mRectHeight->setToolTip(tr("Height of tool"));

    mBasepoints->addButton(new QRadioButton(this), 1);
    mBasepoints->addButton(new QRadioButton(this), 2);
    mBasepoints->addButton(new QRadioButton(this), 3);
    mBasepoints->addButton(new QRadioButton(this), 4);

    mBasepoints->button(1)->setToolTip(tr("Top left"));
    mBasepoints->button(2)->setToolTip(tr("Top right"));
    mBasepoints->button(3)->setToolTip(tr("Bottom left"));
    mBasepoints->button(4)->setToolTip(tr("Bottom right"));

    //TODO: Сделать виджет как в свойствах прямоугольников Scribus (с рамкой).
    //      Ещё можно для некоторых инструментов сделать центральный basepoint.
    QGridLayout *basepointsLayout = new QGridLayout;
    int i = 0;
    foreach(QAbstractButton * button, mBasepoints->buttons()) {
        Qt::Alignment alignment;
        if (i % 2 == 0) {
            alignment = Qt::AlignRight;
        } else {
            alignment = Qt::AlignLeft;
        }
        basepointsLayout->addWidget(button, i - i % 2, i % 2, 1, 1, alignment);
        ++i;
    }
    basepointsLayout->setSpacing(0);

    addRow(tr("Basepoint:"), basepointsLayout);

    QWidget *basepointsLbl = labelForField(basepointsLayout);
    basepointsLbl->setSizePolicy(basepointsLbl->sizePolicy().horizontalPolicy(),
                                 QSizePolicy::MinimumExpanding);
    basepointsLbl->setToolTip(tr("Corner which from tool starts.<br>"
                                 "Tool measurments are referenced from it."));


    connect(mRectHeight, SIGNAL(valueChanged(double)), SLOT(slotSetSize()));
    connect(mRectWidth, SIGNAL(valueChanged(double)), SLOT(slotSetSize()));
    connect(mRectX, SIGNAL(valueChanged(double)), SLOT(slotSetBasepointPos()));
    connect(mRectY, SIGNAL(valueChanged(double)), SLOT(slotSetBasepointPos()));
    connect(mBasepoints, SIGNAL(buttonClicked(int)), SLOT(slotSetBasepoint()));

    connect(mSettings, SIGNAL(sizeChanged(bool)), SLOT(slotChangeSize(bool)));
    connect(mSettings, SIGNAL(basepointPosChanged(bool)), SLOT(slotChangeBasepointPos(bool)));

    connect(mSettings, SIGNAL(isNowEnabled(bool)), SLOT(setGeometrySettingsEnabled(bool)));

    slotChangeSize(true);
    slotChangeBasepointPos(true);

    mBasepoints->button(1)->click();

    updateSizesMaximums();

    setGeometrySettingsEnabled(false);
}

void RectangularToolPanel::updateSizesMaximums()
{
    if (selectedBasepoint() == Qt::TopLeftCorner ||
            selectedBasepoint() == Qt::BottomLeftCorner) {
        mRectWidth->setMaximum(QFrost::sceneHalfSizeInMeters - mRectX->value());
    } else {
        mRectWidth->setMaximum(QFrost::sceneHalfSizeInMeters + mRectX->value());
    }

    if (selectedBasepoint() == Qt::TopLeftCorner ||
            selectedBasepoint() == Qt::TopRightCorner) {
        mRectHeight->setMaximum(QFrost::sceneHalfSizeInMeters - mRectY->value());
    } else {
        mRectHeight->setMaximum(QFrost::sceneHalfSizeInMeters + mRectY->value());
    }
}

void RectangularToolPanel::slotSetBasepointPos()
{
    if (!mMustEmitRectChanges) {
        return;
    }
    updateSizesMaximums();
    mSettings->setBasepointPos(QPointF(mRectX->value(), mRectY->value()), false);
}

void RectangularToolPanel::slotSetSize()
{
    if (!mMustEmitRectChanges) {
        return;
    }
    mSettings->setSize(QSizeF(mRectWidth->value(), mRectHeight->value()), false);
}

void RectangularToolPanel::slotSetBasepoint()
{
    /* Вырубаем максимумы размера, чтобы не скукожиться, если находимся у края
     * сцены, а опорная точка изменилась из противолежащей краю на прилежащую */
    mRectHeight->setMaximum(10 * QFrost::sceneHalfSize);
    mRectWidth->setMaximum(10 * QFrost::sceneHalfSize);
    // Изменяем настройки. Оттуда поступит сигнал об изменении позиции
    mSettings->setBasepoint(selectedBasepoint());
    // После обработки этого сигнала, делаем нужные лимиты
    updateSizesMaximums();
}

void RectangularToolPanel::slotChangeBasepointPos(bool needThisSignal)
{
    if (!needThisSignal) {
        return;
    }
    mMustEmitRectChanges = false;
    mRectX->setValue(mSettings->x());
    mRectY->setValue(mSettings->y());
    mMustEmitRectChanges = true;
    updateSizesMaximums();
}

void RectangularToolPanel::slotChangeSize(bool needThisSignal)
{
    if (!needThisSignal) {
        return;
    }
    mMustEmitRectChanges = false;
    mRectHeight->setValue(mSettings->height());
    mRectWidth->setValue(mSettings->width());
    mMustEmitRectChanges = true;
}

Qt::Corner RectangularToolPanel::selectedBasepoint() const
{
    int id = mBasepoints->checkedId();
    Qt::Corner result;
    switch (id) {
    case 1:
    default:
        result = Qt::TopLeftCorner;
        break;
    case 2:
        result = Qt::TopRightCorner;
        break;
    case 3:
        result = Qt::BottomLeftCorner;
        break;
    case 4:
        result = Qt::BottomRightCorner;
        break;
    }
    return result;
}

void RectangularToolPanel::setGeometrySettingsEnabled(bool enabled)
{
    mRectX->setEnabled(enabled);
    mRectY->setEnabled(enabled);
    mRectWidth->setEnabled(enabled);
    mRectHeight->setEnabled(enabled);

    // устанавливаем значения полей по нулям, если нас вырубили
    if (!enabled) {
        mMustEmitRectChanges = false;
        mRectX->setValue(0);
        mRectY->setValue(0);
        mRectWidth->setValue(0);
        mRectHeight->setValue(0);
        mMustEmitRectChanges = true;
    }
}
