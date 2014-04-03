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

#include "checkboxdelegate.h"

#include <QtWidgets/QApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QAbstractItemView>
#include <QtCore/QAbstractItemModel>

using namespace qfgui;

static QRect checkBoxRect(const QStyleOptionViewItem &viewItemStyleOptions)
{
    QStyleOptionButton checkBoxStyleOption;
    QRect checkBoxRect = QApplication::style()->subElementRect(
                             QStyle::SE_CheckBoxIndicator,
                             &checkBoxStyleOption);
    QPoint checkBoxPoint(viewItemStyleOptions.rect.x() +
                         viewItemStyleOptions.rect.width() / 2 -
                         checkBoxRect.width() / 2,
                         viewItemStyleOptions.rect.y() +
                         viewItemStyleOptions.rect.height() / 2 -
                         checkBoxRect.height() / 2);
    return QRect(checkBoxPoint, checkBoxRect.size());
}

CheckBoxDelegate::CheckBoxDelegate(QAbstractItemView *parent)
    : QStyledItemDelegate(parent)
    , mCheckboxMouseOver()
    , mCheckboxIsPressedDown()
    , mCheckboxHasFocus()
{
    if (!parent->hasMouseTracking()) {
        // Чтобы работало изменение вида при наведении мышки
        parent->setMouseTracking(true);
    }
}

QWidget *CheckBoxDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    Q_UNUSED(parent)
    Q_UNUSED(option)
    Q_UNUSED(index)
    // Создавать редактирующий виджет не надо, он рисуется в paint
    return NULL;
}

void CheckBoxDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    QStyleOptionViewItemV4 opt;
    opt = option;
    initStyleOption(&opt, index);
    opt.text = QString();

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

    bool checked = index.model()->data(index, Qt::DisplayRole).toBool();

    // TODO: во время нажатия можно рисовать серую галку (как делает QCheckBox)
    QStyleOptionButton checkBoxStyleOption;
    checkBoxStyleOption.state |= QStyle::State_Enabled;
    if (checked) {
        checkBoxStyleOption.state |= QStyle::State_On;
    } else {
        checkBoxStyleOption.state |= QStyle::State_Off;
    }
    if (mCheckboxMouseOver) {
        checkBoxStyleOption.state |= QStyle::State_MouseOver;
        if (mCheckboxIsPressedDown) {
            checkBoxStyleOption.state |= QStyle::State_Sunken;
        }
    }
    if (mCheckboxHasFocus) {
        checkBoxStyleOption.state |= QStyle::State_HasFocus;
    }
    checkBoxStyleOption.rect = checkBoxRect(opt);

    QApplication::style()->drawControl(QStyle::CE_CheckBox,
                                       &checkBoxStyleOption,
                                       painter);

}

bool CheckBoxDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index)
{
    if ((event->type() == QEvent::MouseButtonRelease) ||
            (event->type() == QEvent::MouseButtonDblClick)) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton ||
                !checkBoxRect(option).contains(mouseEvent->pos())) {
            return false;
        }
        if (event->type() == QEvent::MouseButtonDblClick) {
            // Пожираем событие, ведь мы не создаём редактор
            return true;
        } else {
            // Отпускание левой кнопки мыши...
            // FIXME: не всегда работает, т.к. отпускание мышки нам не
            // посылается, если курсор находится за нашими пределами.
            mCheckboxIsPressedDown = false;
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() != Qt::Key_Space && keyEvent->key() != Qt::Key_Select) {
            return false;
        }
    } else if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        mCheckboxMouseOver = checkBoxRect(option).contains(mouseEvent->pos());
        return true;
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() != Qt::LeftButton) {
            return false;
        }
        mCheckboxIsPressedDown = checkBoxRect(option).contains(mouseEvent->pos());
        return true;
    } else if (event->type() == QEvent::FocusIn) {
        // FIXME: не попадает сюда
        mCheckboxHasFocus = true;
        return true;
    } else if (event->type() == QEvent::FocusOut) {
        // FIXME: не попадает сюда
        mCheckboxHasFocus = false;
        return true;
    } else {
        return false;
    }

    bool checked = index.data(Qt::DisplayRole).toBool();
    return model->setData(index, !checked, Qt::EditRole);
}
