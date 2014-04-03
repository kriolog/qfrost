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

#ifndef QFGUI_PHYSICALPROPERTYDELEGATE_H
#define QFGUI_PHYSICALPROPERTYDELEGATE_H

#include <QtWidgets/QStyledItemDelegate>

QT_FORWARD_DECLARE_CLASS(QDataWidgetMapper)

namespace qfgui
{

class PhysicalPropertyDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    PhysicalPropertyDelegate(QAbstractItemView *parent,
                             bool enterStartsNextItemEditing = false);
    PhysicalPropertyDelegate(QDataWidgetMapper *parent);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;

protected:
    bool eventFilter(QObject *object, QEvent *event);

private:
    /// Возвращает изменённый @p option, в котором учтено преобразование в СИ
    QStyleOptionViewItemV4 modifiedOptionViewItem(const QStyleOptionViewItem &option,
            const QModelIndex &index) const;

    bool mEnterStartsNextItemEditing;

private slots:
    /// Испускает sizeHintChanged и делает update для каждого занятого индекса
    void updateView();
};

}

#endif // QFGUI_PHYSICALPROPERTYDELEGATE_H
