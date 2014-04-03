/*
 * Copyright (C) 2010-2012  Denis Pesotsky
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

#ifndef QFGUI_ITEMSWIDGET_H
#define QFGUI_ITEMSWIDGET_H

#include <QtWidgets/QWidget>

QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QAbstractItemView)
QT_FORWARD_DECLARE_CLASS(QSortFilterProxyModel)
QT_FORWARD_DECLARE_CLASS(QModelIndex)
QT_FORWARD_DECLARE_CLASS(QAbstractItemDelegate)
QT_FORWARD_DECLARE_CLASS(QPersistentModelIndex)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Item)

QT_FORWARD_DECLARE_CLASS(ItemsModel)

class ItemsWidget : public QWidget
{
    Q_OBJECT
public:
    Q_INVOKABLE ItemsWidget(bool isCompact,
                            ItemsModel *model,
                            const QMetaObject &editDialogMetaObject,
                            QWidget *parent);

    /// Указатель на выбранный элемент.
    /// NULL, если выбрано несколько или ни одного.
    Item *selectedItem() const;

    ItemsModel *model() const {
        return mModel;
    }

    /// Список записей, отсортированных так же, как во view
    QList<Item *> sortedItems();

public slots:
    void updateButtons();

    void newItem();
    void duplicateSelectedItems();
    void editSelectedItems();
    void removeSelectedItems();
    void editItem(int row);
    void editItem(const QModelIndex &index);

    void openTableEditor();

signals:
    void selectionChanged();

protected:
    QAbstractItemView *view() const {
        return mView;
    }

    void setToolTips(const QString &newItem,
                     const QString &duplicateItem,
                     const QString &editItems,
                     const QString &removeItems);

    /// Устанавливает заголовок для открываемого окна.
    /// @warning заголовок следует давать БЕЗ постфикса!
    void setTableEditorWindowTitle(const QString &s) {
        mTableEditorWindowTitle = s;
    }

private:
    /// Выделяет последнюю созданную запись (запись в нулевой строке)
    void selectNewestItem();

    /// Список устойчивых индексов по 0 столбцу всех целиком выбранных строк
    QList<QPersistentModelIndex> selectedRows() const;

    /// Список устойчивых индексов по 0 столбцу всех целиком выбранных строк,
    /// кроме неудаляемых и неизменяемых.
    QList<QPersistentModelIndex> selectedRowsExceptFixed() const;

    QAbstractItemView *const mView;
    ItemsModel *const mModel;
    QSortFilterProxyModel *const mSortedModel;

    QPushButton *const mNewItem;
    QPushButton *const mEditItem;
    QPushButton *const mDuplicateItem;
    QPushButton *const mRemoveItem;

    const QMetaObject mEditDialogMetaObject;

    QString mTableEditorWindowTitle;
};

}

#endif // QFGUI_ITEMSWIDGET_H
