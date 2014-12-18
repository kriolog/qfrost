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

#include "itemswidget.h"

#include "itemstabledialog.h"
#include "itemsmodel.h"
#include "itemeditdialog.h"
#include "item.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableView>
#include <QtWidgets/QListView>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QVBoxLayout>

#include <naturalsortfilterproxymodel.h>
#include <units/physicalpropertydelegate.h>
#include <mainwindow.h>
#include <../3rdparty/HierarchicalHeaderView/HierarchicalHeaderView.h>

using namespace qfgui;

/// Колонка модели, которую следует показывать в компактном режиме,
/// а также та колонка, по которой мы по умолчанию сортируем вид.
static const int MainColumn = 1;

ItemsWidget::ItemsWidget(bool isCompact,
                         ItemsModel *model,
                         const QMetaObject &editDialogMetaObject,
                         QWidget *parent)
    : QWidget(parent)
    , mView(isCompact
            ? static_cast<QAbstractItemView *>(new QListView(this))
            : static_cast<QAbstractItemView *>(new QTableView(this)))
    , mModel(model)
    , mSortedModel(new NaturalSortFilterProxyModel(this))
    , mNewItem(new QPushButton(QIcon::fromTheme("list-add"), "", this))
    , mEditItem(new QPushButton(QIcon::fromTheme("document-edit"), "", this))
    , mDuplicateItem(new QPushButton(QIcon::fromTheme("edit-copy"), "", this))
    , mRemoveItem(new QPushButton(QIcon::fromTheme("list-remove"), "", this))
    , mEditDialogMetaObject(editDialogMetaObject)
    , mTableEditorWindowTitle()
{
    Q_ASSERT(mModel != NULL);
    mSortedModel->setSourceModel(mModel);
    mSortedModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mSortedModel->sort(MainColumn);
    mSortedModel->setDynamicSortFilter(true);
    if (isCompact) {
        QListView *tmp = static_cast<QListView *>(mView);
        // QAbstractItemView::SingleSelection не катит, ибо так нету unselect
        mView->setSelectionMode(QAbstractItemView::ExtendedSelection);
        mView->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tmp->setSpacing(1);
        tmp->setWordWrap(true);

        connect(tmp, SIGNAL(doubleClicked(QModelIndex)),
                SLOT(editItem(QModelIndex)));
    } else {
        QTableView *tmp = static_cast<QTableView *>(mView);

        HierarchicalHeaderView *hv = new HierarchicalHeaderView(Qt::Horizontal, tmp);
        tmp->setHorizontalHeader(hv);

        tmp->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tmp->verticalHeader()->hide();

        tmp->setItemDelegate(new PhysicalPropertyDelegate(tmp));

        tmp->sortByColumn(mSortedModel->sortColumn(), Qt::AscendingOrder);
        tmp->setSortingEnabled(true);
    }

    mView->setModel(mSortedModel);
    mView->setSelectionBehavior(QAbstractItemView::SelectRows);
    mView->setAlternatingRowColors(isCompact);

    if (isCompact) {
        QListView *tmp = static_cast<QListView *>(mView);
        tmp->setModelColumn(MainColumn);
    } else {
        QTableView *tmp = static_cast<QTableView *>(mView);
        // Прячем колонку с цветами
        tmp->hideColumn(0);
    }

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(QMargins());
    mainLayout->addWidget(mView);

    QHBoxLayout *soilsEditButtons = new QHBoxLayout();
    soilsEditButtons->setContentsMargins(QMargins());
    mainLayout->addLayout(soilsEditButtons);

    soilsEditButtons->addWidget(mNewItem);
    soilsEditButtons->addWidget(mDuplicateItem);
    soilsEditButtons->addWidget(mEditItem);
    soilsEditButtons->addWidget(mRemoveItem);

    if (isCompact) {
        // В компактном режиме нам эта кнопка не нужна, ибо есть двойной клик
        mEditItem->hide();
    } else {
        // А в некомпактном режиме на кнопки влезут надписи
        mNewItem->setText(tr("New"));
        mDuplicateItem->setText(tr("Duplicate"));
        mEditItem->setText(tr("Edit"));
        mRemoveItem->setText(tr("Remove"));
    }

    connect(mView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,
                                            QItemSelection)),
            this, SLOT(updateButtons()));
    connect(mView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,
                                            QItemSelection)),
            this, SIGNAL(selectionChanged()));

    connect(mNewItem, SIGNAL(clicked()), this, SLOT(newItem()));
    connect(mDuplicateItem, SIGNAL(clicked()), this, SLOT(duplicateSelectedItems()));
    connect(mRemoveItem, SIGNAL(clicked()), this, SLOT(removeSelectedItems()));
    connect(mEditItem, SIGNAL(clicked()), this, SLOT(editSelectedItems()));

    updateButtons();
}

void ItemsWidget::setToolTips(const QString &newItem,
                              const QString &duplicateItem,
                              const QString &editItems,
                              const QString &removeItems)
{
    mNewItem->setToolTip(newItem);
    mDuplicateItem->setToolTip(duplicateItem);
    mEditItem->setToolTip(editItems);
    mRemoveItem->setToolTip(removeItems);
}

Item *ItemsWidget::selectedItem() const
{
    const QList <QPersistentModelIndex > rows = selectedRows();
    return rows.size() == 1
           ? static_cast<Item *>(rows.first().internalPointer())
           : NULL;
}

void ItemsWidget::updateButtons()
{
    const bool anythingIsSelected = mView->selectionModel()->hasSelection();

    mRemoveItem->setEnabled(anythingIsSelected && !selectedRowsExceptFixed().isEmpty());
    mEditItem->setEnabled(anythingIsSelected);
    mDuplicateItem->setEnabled(selectedItem() != NULL);
}

void ItemsWidget::newItem()
{
    editItem(-1);
}

void ItemsWidget::duplicateSelectedItems()
{
    // FIXME На данный момент дублирование грунта доступно только при единичном
    // выделении (иначе кнопка недоступна). Если бы кнопка была доступна, при
    // множественном выделении, создавалось бы несколько undo-комманд.
    Q_ASSERT(selectedRows().size() == 1);
    foreach(const QPersistentModelIndex & index, selectedRows()) {
        mModel->duplicateItem(index.row());
    }
}

QList<QPersistentModelIndex> ItemsWidget::selectedRows() const
{
    Q_ASSERT(mView->selectionModel()->model() == mSortedModel);
    QList<QPersistentModelIndex> result;
    // HACK: вместо selectedRows приходится использовать такой велосипед, ибо
    //       Qt может развыделять ячейки без флага ItemIsEnabled при
    //       пересортировке QItemSelectionModel, после чего selectedRows
    //       возвращает пустой список.
    foreach(const QModelIndex & index, mView->selectionModel()->selectedIndexes()) {
        if (index.column() == 0) {
            result << QPersistentModelIndex(mSortedModel->mapToSource(index));
        }
    }
    return result;
}

QList< QPersistentModelIndex > ItemsWidget::selectedRowsExceptFixed() const
{
    if (!mModel->hasFixedItem()) {
        return selectedRows();
    }

    QList<QPersistentModelIndex> result;
    const int fixedRow = mModel->rowCount() - 1;
    if (fixedRow == 0) {
        // В модели единственная строка, которая не может не является fixed,
        // так что возвращаем пустой список
        return result;
    }
    Q_ASSERT(mView->selectionModel()->model() == mSortedModel);
    foreach(const QModelIndex & index, mView->selectionModel()->selectedIndexes()) {
        QPersistentModelIndex mappedIndex = mSortedModel->mapToSource(index);
        if (index.column() == 0 && mappedIndex.row() != fixedRow) {
            result << QPersistentModelIndex(mappedIndex);
        }
    }
    return result;
}

void ItemsWidget::removeSelectedItems()
{
    mModel->removeItems(selectedRowsExceptFixed());
}

void ItemsWidget::editSelectedItems()
{
    foreach(const QPersistentModelIndex & index, selectedRows()) {
        editItem(index.row());
    }
}

void ItemsWidget::editItem(int row)
{
    QObject *temporaryModelObject = mModel->metaObject()->newInstance(Q_ARG(ItemsModel *, mModel),
                                    Q_ARG(int, row));
    Q_ASSERT(temporaryModelObject != NULL);
    ItemsModel *temporaryModel = qobject_cast< ItemsModel * >(temporaryModelObject);
    Q_ASSERT(temporaryModel != NULL);
    QObject *dialogObject = mEditDialogMetaObject.newInstance(Q_ARG(ItemsModel *, temporaryModel),
                            Q_ARG(QStringList, mModel->forbiddenNamesList(row)),
                            Q_ARG(bool, row == -1),
                            Q_ARG(QWidget *, this));
    Q_ASSERT(dialogObject != NULL);
    ItemEditDialog *dialog = qobject_cast<ItemEditDialog *>(dialogObject);
    Q_ASSERT(dialog != NULL);
    Q_ASSERT(!dialog->testAttribute(Qt::WA_DeleteOnClose));

    const bool canEdit = !(mModel->hasFixedItem() && row == mModel->rowCount() - 1);
    if (!canEdit) {
        dialog->disableChanges();
    }

    if (dialog->exec() == QDialog::Accepted && canEdit) {
        mModel->merge(temporaryModel, row);
        if (row == -1) {
            selectNewestItem();
        }
    }
    temporaryModel->deleteLater();
    dialog->deleteLater();
}

void ItemsWidget::editItem(const QModelIndex &index)
{
    Q_ASSERT(index.model() == mSortedModel);
    editItem(mSortedModel->mapToSource(index).row());
}

void ItemsWidget::openTableEditor()
{
    ItemsTableDialog dialog(this);
    Q_ASSERT(!mTableEditorWindowTitle.isEmpty());
    dialog.setWindowTitle(mTableEditorWindowTitle);
    dialog.exec();
}

void ItemsWidget::selectNewestItem()
{
    QModelIndex begin;
    begin = mSortedModel->mapFromSource(mModel->index(0, 0));
    QModelIndex end;
    end = mSortedModel->mapFromSource(mModel->index(0, mModel->columnCount() - 1));
    QItemSelection newSelection(begin, end);
    mView->selectionModel()->select(newSelection,
                                    QItemSelectionModel::ClearAndSelect);
}

QList<Item *> ItemsWidget::sortedItems()
{
    QList<Item *> result;
    const int rowCount = mSortedModel->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        result << mModel->itemAt(mSortedModel->mapToSource(mSortedModel->index(i, 0)).row());
        Q_ASSERT(result.last() != NULL);
    }
    return result;
}
