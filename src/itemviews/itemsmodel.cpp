/*
 * Copyright (C) 2010-2015  Denis Pesotsky
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

#include "itemsmodel.h"

#include <QtCore/QChildEvent>
#include <QtCore/QMetaProperty>
#include <QtCore/QLocale>
#include <QtWidgets/QUndoStack>

#include <itemviews/item.h>
#include <mainwindow.h>
#include <undo/itemviews/itemeditcommand.h>
#include <undo/itemviews/itemaddcommand.h>
#include <undo/itemviews/itemsremovecommand.h>

using namespace qfgui;

static QList<int> notifySignals(const QMetaObject &metaObject)
{
    Q_ASSERT(metaObject.superClass()->className() == Item::staticMetaObject.className());
    QList<int> result;

    for (int i = Item::staticMetaObject.propertyOffset(); i < metaObject.propertyCount(); ++i) {
        result << metaObject.property(i).notifySignalIndex();
    }

    return result;
}

static QList<const char *> properties(const QMetaObject &metaObject)
{
    Q_ASSERT(metaObject.superClass()->className() == Item::staticMetaObject.className());
    QList<const char *> result;

    for (int i = Item::staticMetaObject.propertyOffset(); i < metaObject.propertyCount(); ++i) {
        result << metaObject.property(i).name();
    }

    return result;
}

static QUndoStack *getUndoStack(QWidget *widget)
{
    Q_ASSERT(widget != NULL && widget->window() != NULL);
    MainWindow *mainWindow = qobject_cast< MainWindow * >(widget->window());
    if (mainWindow == NULL) {
        mainWindow = qobject_cast< MainWindow * >(widget->window()->parentWidget()->window());
    }
    return mainWindow != NULL ? mainWindow->undoStack() : NULL;
}

ItemsModel::ItemsModel(const QMetaObject &itemsMetaObject,
                       QWidget *parent,
                       const QList<QColor> &defaultColors,
                       const QHash<int, PhysicalProperty> &physicalProperties,
                       const Item *fixedItem)
    : QAbstractTableModel(parent)
    , mItemsMetaObject(itemsMetaObject)
    , mItems()
    , mProperties(properties(itemsMetaObject))
    , mNotifySignals(notifySignals(itemsMetaObject))
    , mDefaultColors(defaultColors)
    , mPhysicalProperties(physicalProperties)
    , mUndoStack(getUndoStack(parent))
    , mRecordingRow(-1)
    , mRecordedChanges()
    , mFixedItem(fixedItem)
    , mHasFixedItem(mFixedItem != NULL)
{
    Q_ASSERT(mProperties.size() >= 2);
    Q_ASSERT(strcmp(mProperties.at(0), "color") == 0);
    Q_ASSERT(strcmp(mProperties.at(1), "name") == 0);
}

ItemsModel::ItemsModel(ItemsModel *other, int row)
    : QAbstractTableModel(other)
    , mItemsMetaObject(other->mItemsMetaObject)
    , mItems()
    , mProperties(other->mProperties)
    , mDefaultColors(other->mDefaultColors)
    , mPhysicalProperties(other->mPhysicalProperties)
    , mUndoStack(NULL)
    , mRecordingRow(-1)
    , mRecordedChanges()
    , mFixedItem(NULL)
    , mHasFixedItem(false)
{
    QObject *newItem;
    if (row != -1) {
        Q_ASSERT(row < other->rowCount());
        newItem = mItemsMetaObject.newInstance(Q_ARG(const Item *, other->itemAt(row)));
    } else {
        newItem = mItemsMetaObject.newInstance(Q_ARG(QString, other->nextName()),
                                               Q_ARG(QColor, other->nextColor()));
    }
    Q_ASSERT(newItem != NULL);
    Q_ASSERT(qobject_cast<Item *>(newItem) != NULL);
    newItem->setParent(this);
    Q_ASSERT(mItems.size() == 1);
}

QVariant ItemsModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(mProperties.size() >= 2);

    if (!index.isValid()) {
        Q_ASSERT(false);
        return QVariant();
    }

    if (index.row() >= rowCount()) {
        Q_ASSERT(false);
        return QVariant();
    }

    if (index.column() >= columnCount()) {
        Q_ASSERT(false);
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == QFrost::DirectEditRole) {
        return itemAt(index.row())->property(mProperties.at(index.column()));
    }

    if (role == Qt::TextAlignmentRole) {
        return index.column() == 1
               ? Qt::AlignLeft + Qt::AlignVCenter
               : Qt::AlignCenter;
    }

    if (role == QFrost::PhysicalPropertyRole) {
        QHash<int, PhysicalProperty>::ConstIterator it;
        it = mPhysicalProperties.constFind(index.column());
        if (it == mPhysicalProperties.constEnd()) {
            return QVariant();
        } else {
            return *it;
        }
    }

    if (role == QFrost::MinimumRole) {
        return minimum(index);
    }

    if (role == QFrost::MaximumRole) {
        return maximum(index);
    }

    if (role == Qt::DecorationRole && index.column() == 1) {
        return data(this->index(index.row(), 0));
    }

    return QVariant();
}

Qt::ItemFlags ItemsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (!mHasFixedItem || index.internalPointer() != mFixedItem) {
        result |= Qt::ItemIsEditable;
    }
    return result;
}

bool ItemsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        Q_ASSERT(false);
        return false;
    }

    if (index.row() >= rowCount()) {
        Q_ASSERT(false);
        return false;
    }

    if (index.column() >= columnCount()) {
        Q_ASSERT(false);
        return false;
    }

    if (role != Qt::EditRole && role != QFrost::DirectEditRole) {
        Q_ASSERT(false);
        return false;
    }

    if (mHasFixedItem && index.row() == rowCount() - 1) {
        return false;
    }

    const QVariant oldValue = index.data(role);

    if (oldValue.type() != value.type()) {
        Q_ASSERT(false);
        return false;
    }

    if (oldValue == value) {
        return false;
    }

    // FIXME: у полей с наличием PhysicalPropertyRole надо проверять равенство
    //        до видимого знака (чтобы при смене ед. изм. оно не меняло числа)

    /* QVariant::operator== при сравнении нестандартных типов сравнивает
     * адреса переменных, так что надо допроверить равенство значений */
    if (value.canConvert<DoubleMap>()) {
        if (oldValue.value<DoubleMap>() == value.value<DoubleMap>()) {
            return false;
        }
    }
    if (value.canConvert<QList<double> >()) {
        if (oldValue.value<QList<double> >() == value.value<QList<double> >()) {
            return false;
        }
    }

    if (index.column() == 1) {
        if (!itemCanBeRenamed(index.row(), value.toString())) {
            return false;
        }
    }

    if (oldValue.type() == QVariant::String) {
        if (oldValue.toString() == value.toString().simplified()) {
            return false;
        }
    }

    if (role != QFrost::DirectEditRole) {
        startRecordingChanges(index.row());
    }

    bool ok = mItems.at(index.row())->setProperty(mProperties.at(index.column()), value);
    Q_ASSERT(ok);

    if (role != QFrost::DirectEditRole) {
        stopRecordingChanges();
    }
    return true;
}

QModelIndex ItemsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent != QModelIndex()) {
        Q_ASSERT(false);
        return QModelIndex();
    }

    if (row > rowCount()) {
        Q_ASSERT(false);
        return QModelIndex();
    }

    if (column > columnCount()) {
        Q_ASSERT(false);
        return QModelIndex();
    }

    return createIndex(row, column, itemAt(row));
}

Item *ItemsModel::itemAt(int row) const
{
    Q_ASSERT(row < rowCount());
    return (!mHasFixedItem || row < rowCount() - 1)
           ? mItems.at(row)
           : const_cast<Item *>(mFixedItem);
}

QList<const Item *> ItemsModel::items() const
{
    QList<const Item *> result;
    foreach(Item * item, mItems) {
        result << item;
    }
    if (mHasFixedItem) {
        result << mFixedItem;
    }
    return result;
}

bool ItemsModel::itemCanBeRenamed(int position, const QString &name) const
{
    QString newName = name.simplified();
    if (newName.isEmpty()) {
        return false;
    }

    return !forbiddenNamesList(position).contains(newName, Qt::CaseInsensitive);
}

void ItemsModel::merge(const ItemsModel *other, int row)
{
    Q_ASSERT(other->mItems.size() == 1);
    if (mHasFixedItem && row == rowCount() - 1) {
        return;
    }
    if (row != -1) {
        Q_ASSERT(columnCount() == other->columnCount());
        QHash<int, QVariant> oldValues;
        if (mUndoStack != NULL) {
            for (int i = 0; i < columnCount(); ++i) {
                oldValues.insert(i, index(row, i).data(Qt::EditRole));
            }
        }
        startRecordingChanges(row);
        mItems.at(row)->copyPropertiesFrom(other->mItems.at(0));
        stopRecordingChanges();
    } else {
        addItem(other->mItems.first());
    }
}

void ItemsModel::duplicateItem(int row)
{
    Q_ASSERT(row >= 0);
    Q_ASSERT(row < rowCount());
    Item *origItem = itemAt(row);
    QObject *newItem = mItemsMetaObject
                       .newInstance(Q_ARG(const Item *, origItem),
                                    Q_ARG(QString, nextCopyName(row)),
                                    Q_ARG(QColor, nextColor()));
    Q_ASSERT(newItem != NULL);
    Q_ASSERT(qobject_cast< Item * >(newItem) != NULL);
    addItem(qobject_cast< Item * >(newItem), origItem->name());
}

QVariant ItemsModel::minimum(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QVariant();
}

QVariant ItemsModel::maximum(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QVariant();
}

void ItemsModel::emitDataChanged()
{
    Q_ASSERT(sender() != NULL);
    Q_ASSERT(senderSignalIndex() != -1);

    Q_ASSERT(qobject_cast<Item *>(sender()) != NULL);
    Q_ASSERT(sender()->parent() == this);

    const int row = rowFromItem(qobject_cast<Item *>(sender()));
    Q_ASSERT(row != -1);
    const int column = mNotifySignals.indexOf(senderSignalIndex());
    Q_ASSERT(column != -1);

    const QModelIndex changedIndex = index(row, column);
    if (mRecordingRow != -1) {
        Q_ASSERT(mRecordingRow == row);
        Q_ASSERT(mRecordedChanges.contains(mProperties.at(row)));
        mRecordedChanges[mProperties.at(column)].second = data(changedIndex);
    }
    emitDataChanged(changedIndex);
}

void ItemsModel::emitDataChanged(const QModelIndex &changedIndex)
{
    emit dataChanged(changedIndex, changedIndex);
}

QStringList ItemsModel::forbiddenNamesList(int row) const
{
    QStringList result;
    foreach(const Item * item, mItems) {
        result << item->name();
    }
    if (row != -1) {
        result.removeAt(row);
    }
    return result;
}

QList< QColor > ItemsModel::usedColorsList() const
{
    QList<QColor> result;
    foreach(const Item * item, mItems) {
        result << item->color();
    }
    return result;
}

QColor ItemsModel::nextColor() const
{
    QList<QColor> unusedColors = mDefaultColors;

    foreach(const QColor &color, usedColorsList()) {
        unusedColors.removeOne(color);
    }

    if (!unusedColors.isEmpty()) {
        return unusedColors.first();
    } else {
        return QColor(qrand() % 256, qrand() % 256, qrand() % 256);
    }
}

QString ItemsModel::nextName() const
{
    QString result;
    result = newItemName();
    QStringList forbiddenNames = forbiddenNamesList();
    int i = 0;
    QString nameTemplate = newItemNameTemplate();
    if (!nameTemplate.contains("%1")) {
        qWarning("ItemsModel: New item name template must contain placeholder! "
                 "Check translation file for %s.", metaObject()->className());
        nameTemplate = "%1";
    }
    while (forbiddenNames.contains(result)) {
        result = nameTemplate.arg(++i);
    }
    return result;
}

QString ItemsModel::nextCopyName(int row) const
{
    Q_ASSERT(row < rowCount());
    const QString otherName = itemAt(row)->name();
    //: Name for the first item duplicate. Example: "Sand — copy".
    QString result = tr("%1 — copy").arg(otherName);
    QStringList usedNames = forbiddenNamesList();
    int i = 1;
    while (usedNames.contains(result)) {
        //: Name for item duplicate. Example: "Sand — copy 1".
        result = tr("%1 — copy %2").arg(otherName).arg(++i);
    }
    return result;
}

void ItemsModel::connectTo(Item *item)
{
    Q_ASSERT(item->parent() == this);
    const QMetaObject *m = item->metaObject();
    foreach(int signalIndex, mNotifySignals) {
        Q_ASSERT(m->method(signalIndex).methodType() == QMetaMethod::Signal);
        QString signal = m->method(signalIndex).methodSignature();
        // См. qobjectdefs.h, часть с define SIGNAL(a)
        signal.prepend('2');
        bool ok = connect(item, qPrintable(signal), SLOT(emitDataChanged()));
        Q_ASSERT(ok);
    }
}

void ItemsModel::childEvent(QChildEvent *event)
{
    QObject::childEvent(event);
    Item *item = qobject_cast< Item * >(event->child());
    if (item != NULL) {
        if (event->type() == QEvent::ChildAdded) {
            Q_ASSERT(!mItems.contains(item));
            beginInsertRows(QModelIndex(), 0, 0);
            mItems.prepend(item);
            connectTo(item);
            endInsertRows();
        } else if (event->type() == QEvent::ChildRemoved) {
            const int row = rowFromItem(item);
            Q_ASSERT(row != -1);
            beginRemoveRows(QModelIndex(), row, row);
            disconnect(item, 0, 0, 0);
            mItems.removeAt(row);
            endRemoveRows();
        }
    }
}

void ItemsModel::addItem(Item *item, const QString &originalItemName)
{
    if (mUndoStack != NULL) {
        mUndoStack->push(new ItemAddCommand(item, this, originalItemName));
    } else {
        item->setParent(this);
    }
}

void ItemsModel::removeItems(const QList<QPersistentModelIndex> &rows)
{
    if (rows.isEmpty()) {
        qWarning("ItemsModel::removeItems: Empty rows list!");
        return;
    }
    Q_ASSERT(!mHasFixedItem || !rows.contains(index(rowCount() - 1, 0)));
    if (mUndoStack != NULL) {
        QList<Item *> items;
        foreach(const QPersistentModelIndex & index, rows) {
            Q_ASSERT(index.column() == 0);
            items << mItems.at(index.row());
        }
        mUndoStack->push(ItemsRemoveCommand::createCommand(items));
    } else {
        foreach(const QPersistentModelIndex & index, rows) {
            Q_ASSERT(index.column() == 0);
            mItems.at(index.row())->setParent(NULL);
        }
    }
}

int ItemsModel::rowFromItem(const Item *item) const
{
    return mItems.indexOf(const_cast<Item *>(item));
}

void ItemsModel::startRecordingChanges(int row)
{
    Q_ASSERT(row >= 0);
    Q_ASSERT(row < rowCount());
    Q_ASSERT(mRecordingRow == -1);
    Q_ASSERT(mRecordedChanges.isEmpty());

    if (mUndoStack == NULL) {
        return;
    }

    // Записываем в списке (будущих) изменений текущие значения
    for (QList <const char * >::ConstIterator i = mProperties.constBegin();
            i != mProperties.constEnd(); ++i) {
        mRecordedChanges.insert(*i,
                                qMakePair(index(row, i - mProperties.constBegin()).data(),
                                          QVariant()));
    }

    // Начинаем записывать изменения
    mRecordingRow = row;
}

void ItemsModel::stopRecordingChanges()
{
    if (mUndoStack == NULL) {
        return;
    }

    Q_ASSERT(mRecordingRow != -1);
    Q_ASSERT(!mRecordedChanges.isEmpty());
    Q_ASSERT(mRecordedChanges.size() == mProperties.size());

    QList<ItemChanges::Iterator> nullChanges;
    // Удаляем из списка изменений не изменённые записи
    for (ItemChanges::Iterator i = mRecordedChanges.begin();
            i != mRecordedChanges.end(); ++i) {
        if (!i->second.isValid()) {
            nullChanges << i;
        }
    }
    foreach(const ItemChanges::Iterator & it, nullChanges) {
        mRecordedChanges.erase(it);
    }
    // Создаём undo-команду
    if (!mRecordedChanges.isEmpty()) {
        mUndoStack->push(ItemEditCommand::createCommand(itemAt(mRecordingRow),
                         mRecordedChanges));
    }
    // Прекращаем записывать изменения
    mRecordingRow = -1;
    // и чистим список внесённых измений
    mRecordedChanges.clear();
}

void ItemsModel::save(QDataStream &out) const
{
    Q_ASSERT(mFixedItem == NULL || mFixedItem->id() < 0);
    out << qint32(mItems.size());
    // Нумеруем итемы (кроме фиксированного) и сразу сохраняем их
    for (int i = 0; i < mItems.size(); ++i) {
        Item *item = mItems.at(i);
        item->setID(i);
        item->save(out);
    }
}

QList<const Item *> ItemsModel::load(QDataStream &in)
{
    Q_ASSERT(mFixedItem == NULL || mFixedItem->id() < 0);
    qint32 itemsNum;
    in >> itemsNum;

    qDebug("begin");
    QList<const Item *> result;
    for (int i = 0; i < itemsNum; ++i) {
        qDebug("%i", i);
        QObject *object = mItemsMetaObject.newInstance();
        Q_ASSERT(object != NULL);
        Item *item = qobject_cast<Item *>(object);
        Q_ASSERT(item != NULL);
        qDebug("%i 2", i);
        item->load(in);
        qDebug("%i 2.5", i);
        item->setParent(this);

        result << item;
        Q_ASSERT(result.last() != NULL);
        qDebug("%i 3", i);
    }
    qDebug("end");
    return result;
}
