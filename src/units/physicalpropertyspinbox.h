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

#ifndef QFGUI_PHYSICALPROPERTYSPINBOX_H
#define QFGUI_PHYSICALPROPERTYSPINBOX_H

#include <smartdoublespinbox.h>
#include <qfrost.h>

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Unit)

/**
 * Редактор для скалярных свойств.
 * Оформлен в виде spin box.
 * Величина, хранящаяся в value(), соотвествует СИ, а отображаемая
 * пользоваталю -- тому, в чём он хочет её видеть.
 */
class PhysicalPropertySpinBox : public SmartDoubleSpinBox
{
    Q_OBJECT
    Q_PROPERTY(int physicalProperty
               READ physicalProperty
               WRITE setPhysicalProperty)
    Q_PROPERTY(double forcedMinimum
               READ forcedMinimum
               WRITE setForcedMinimum)
    Q_PROPERTY(double forcedMaximum
               READ forcedMaximum
               WRITE setForcedMaximum)
public:
    PhysicalPropertySpinBox(PhysicalProperty property,
                            QWidget *parent);

    PhysicalPropertySpinBox(QWidget *parent);

    QString textFromValue(double val) const;
    double valueFromText(const QString &text) const;
    void hideSuffix();
    QSize sizeHint() const;
    void stepBy(int steps);

    int physicalProperty() const {
        return mProperty;
    }
    double forcedMinimum() const;
    double forcedMaximum() const;

    static QDoubleSpinBox *createSceneCoordinateSpinBox();

public slots:
    void setPhysicalProperty(int p);
    /// Выставляет минимум в @p min; не меняет его при смене единиц измерения
    void setForcedMinimum(double min);
    /// Выставляет максимум в @p max; не меняет его при смене единиц измерения
    void setForcedMaximum(double max);

protected:
    QValidator::State validate(QString &input, int &pos) const;
    StepEnabled stepEnabled() const;

private:
    PhysicalProperty mProperty;
    bool mNeedSuffix;
    bool mMinimumIsForced;
    bool mMaximumIsForced;
    SmartDoubleSpinBox *mHelperSpinBox;

    void setSuffix(const QString &suffix) {
        SmartDoubleSpinBox::setSuffix(suffix);
    }

    const Unit *mUnit;

    void getPropertiesFromUnit();

private slots:
    void updateUnit();
    void updateHelperValue(double d);
};

}

#endif // QFGUI_PHYSICALPROPERTYSPINBOX_H
