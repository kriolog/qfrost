/*
 * Copyright (C) 2012  Denis Pesotsky
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

#include "physicalpropertydelegate.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QAbstractItemView>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QDataWidgetMapper>

#include <physicalpropertyspinbox.h>
#include <qfrost.h>
#include <soils/soilsmodel.h>
#include <units.h>

using namespace qfgui;

PhysicalPropertyDelegate::PhysicalPropertyDelegate(QAbstractItemView *parent,
        bool enterStartsNextItemEditing)

    : QStyledItemDelegate(parent)
    , mEnterStartsNextItemEditing(enterStartsNextItemEditing)
{
    connect(Units::units(this), SIGNAL(changed()),
            this, SLOT(updateView()));
}

PhysicalPropertyDelegate::PhysicalPropertyDelegate(QDataWidgetMapper *parent)
    : QStyledItemDelegate(parent)
    , mEnterStartsNextItemEditing(false)
{

}

QWidget *PhysicalPropertyDelegate::createEditor(QWidget *parent,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QVariant physicalPropertyVariant = index.data(QFrost::PhysicalPropertyRole);

    if (!physicalPropertyVariant.isValid()) {
        // Это не физическая величина, создадим стандартный редактор
        return QStyledItemDelegate::createEditor(parent, option, index);
    } else {
        Q_ASSERT(physicalPropertyVariant.type() == QVariant::Int);

        PhysicalPropertySpinBox *editor;
        editor = new PhysicalPropertySpinBox(static_cast<PhysicalProperty>(physicalPropertyVariant.toInt()),
                                             parent);
        editor->hideSuffix();
        editor->setAlignment(Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()));
        editor->setFrame(false);
        editor->setMinimumWidth(editor->sizeHint().width());

        return editor;
    }
}

void PhysicalPropertyDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QVariant physicalPropertyVariant = index.data(QFrost::PhysicalPropertyRole);
    if (physicalPropertyVariant.isValid()) {
        bool ok = editor->setProperty("physicalProperty", physicalPropertyVariant);
        if (!ok) {
            qWarning("PhysicalPropertyDelegate::setEditorData: failed applying "
                     "\"physicalProperty\" property to editor at index that "
                     "has valid PhysicalPropertyRole data!");
        }
    }

    QVariant minValueVariant = index.data(QFrost::MinimumRole);
    if (minValueVariant.isValid()) {
        bool ok = editor->setProperty("forcedMinimum", minValueVariant);
        if (!ok) {
            qWarning("PhysicalPropertyDelegate::setEditorData: failed applying "
                     "\"forcedMinimum\" property to editor at index that "
                     "has valid MinimumRole data!");
        }
    }

    QVariant maxValueVariant = index.data(QFrost::MaximumRole);
    if (maxValueVariant.isValid()) {
        bool ok = editor->setProperty("forcedMaximum", maxValueVariant);
        if (!ok) {
            qWarning("PhysicalPropertyDelegate::setEditorData: failed applying "
                     "\"forcedMaximum\" property to editor at index that "
                     "has valid MaximumRole data!");
        }
    }

    QStyledItemDelegate::setEditorData(editor, index);
}

QStyleOptionViewItemV4 PhysicalPropertyDelegate::modifiedOptionViewItem(const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QVariant physicalPropertyVariant = index.data(QFrost::PhysicalPropertyRole);
    if (!physicalPropertyVariant.isValid()) {
        return option;
    } else {
        QStyleOptionViewItemV4 result = option;
        initStyleOption(&result, index);

        const Unit &u = Units::unit(this, physicalPropertyVariant.toInt());
        double value = u.fromSI(index.data().toDouble());
        result.text = result.locale.toString(value, 'f', u.decimalsVisible());

        // Отрезаем нули в конце
        if (result.text.contains(result.locale.decimalPoint())) {
            while (result.text.endsWith('0')) {
                result.text.chop(1);
            }
            if (result.text.endsWith(result.locale.decimalPoint())) {
                result.text.chop(1);
            }
        }

        return result;
    }
}

void PhysicalPropertyDelegate::paint(QPainter *painter,
                                     const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    QVariant physicalPropertyVariant = index.data(QFrost::PhysicalPropertyRole);
    if (!physicalPropertyVariant.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
    } else {
        QStyle *style = QApplication::style();
        QStyleOptionViewItemV4 opt = modifiedOptionViewItem(option, index);
        style->drawControl(QStyle::CE_ItemViewItem,
                           &opt,
                           painter);
    }
}

QSize PhysicalPropertyDelegate::sizeHint(const QStyleOptionViewItem &option,
        const QModelIndex &index) const
{
    QVariant physicalPropertyVariant = index.data(QFrost::PhysicalPropertyRole);
    QSize result;
    if (!physicalPropertyVariant.isValid()) {
        result = QStyledItemDelegate::sizeHint(option, index);
    } else {
        QStyle *style = QApplication::style();
        QStyleOptionViewItemV4 opt = modifiedOptionViewItem(option, index);
        // FIXME: что за contentsSize (3й параметр)? Посмотреть в исходниках Qt!
        result =  style->sizeFromContents(QStyle::CT_ItemViewItem,
                                          &opt,
                                          QSize());
    }
    return result;
}

void PhysicalPropertyDelegate::updateView()
{
    QAbstractItemView *view = qobject_cast<QAbstractItemView *>(parent());
    Q_ASSERT(view != NULL);
    QAbstractItemModel *model = qobject_cast<QAbstractItemModel *>(view->model());
    Q_ASSERT(model != NULL);

    QModelIndexList occupiedIndexes;

    int rowCount = model->rowCount();
    int columnCount = model->columnCount();

    for (int row = 0; row < rowCount; ++row) {
        for (int column = 0; column < columnCount; ++column) {
            QModelIndex index = model->index(row, column);
            if (view->itemDelegate(index) == this) {
                occupiedIndexes << index;
            }
        }
    }

    foreach(const QModelIndex & index, occupiedIndexes) {
        emit sizeHintChanged(index);
        // Это тоже надо делать, ибо view не всегда реагирует на sizeHintChanged
        view->update(index);
    }
}

bool PhysicalPropertyDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (mEnterStartsNextItemEditing) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                QKeyEvent newEvent(QEvent::KeyPress, Qt::Key_Tab, keyEvent->modifiers());
                return QStyledItemDelegate::eventFilter(object, &newEvent);
            }
        }
    }
    return QStyledItemDelegate::eventFilter(object, event);
}
