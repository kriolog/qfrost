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

#ifndef QFGUI_ITEMSMODEL_H
#define QFGUI_ITEMSMODEL_H

#include <QtCore/QAbstractTableModel>

#include <qfrost.h>

QT_FORWARD_DECLARE_CLASS(QUndoStack)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Item)
QT_FORWARD_DECLARE_CLASS(ItemEditCommand)

class ItemsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ItemsModel(const QMetaObject &itemsMetaObject,
               QWidget *parent,
               const QList<QColor> &defaultColors,
               const QHash<int, PhysicalProperty> &physicalProperties,
               const Item *fixedItem = NULL);

    /// Создаёт модель с единственной записью, которая является или копией
    /// записи из @p row, или новой записью (если @p row = -1)
    ItemsModel(ItemsModel *other, int row);

    int rowCount(const QModelIndex &parent = QModelIndex()) const {
        Q_UNUSED(parent)
        return mFixedItem == NULL
               ? mItems.size()
               : mItems.size() + 1;
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const {
        Q_UNUSED(parent)
        return mProperties.size();
    }

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /// Устанавливает значение в @p index равным @p value, если значения различны.
    /// Если @p role равна QFrost::DirectEditRole, делает это напрямую; если
    /// Qt::EditRole - или через undo (при наличии @m mUndoStack), или напрямую.
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    /// Индекс записи в строке @p row и колонке @p column с установленным internalPointer.
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    /// Запись в строке @p position можно переименовать в @p name (новое имя уникально).
    bool itemCanBeRenamed(int position, const QString &name) const;

    /// Номер строки, в которой находится @p item или -1, если нет такой записи.
    int rowFromItem(const Item *item) const;

    Item *itemAt(int row) const;

    /// Список запрещённых (использованных) названий.
    /// @param row строка, имя записи в которой следует исключить из результата.
    QStringList forbiddenNamesList(int row = -1) const;

    /// Копирует все параметры записи из нулевой строки @p other в @p row.
    /// Если @p row = -1, переносит оттуда запись в нулевую строку.
    void merge(const ItemsModel *other, int row);

    /// Создаёт дупликат записи в строке @p row;
    void duplicateItem(int row);

    /// Удаляет элементы в строках с индексами @p rows.
    /// Столбцы индексов должны быть равны 0.
    void removeItems(const QList<QPersistentModelIndex> &rows);

    /// Имеется ли здесь неудаляемая неизменяемая запись в последней строке
    bool hasFixedItem() const {
        return mHasFixedItem;
    }

    /// Несортированный список всех записей (включая фиксированную, если такая есть)
    QList<const Item *> items() const;

    /// Нумерует наши итемы и сохраняет их в @p out
    void save(QDataStream &out) const;

    QList<const Item *> load(QDataStream &in, int version);

protected:
    void childEvent(QChildEvent *event);

    /// Минимальное значение для величины в @p index.
    /// Стандартная реализация возвращает QVariant().
    virtual QVariant minimum(const QModelIndex &index) const;

    /// Максимальное значение для величины в @p index.
    /// Стандартная реализация возвращает QVariant().
    virtual QVariant maximum(const QModelIndex &index) const;

    /// Имя по умолчанию для новой записи. Пример: "New Soil".
    virtual QString newItemName() const = 0;
    /// Шаблон имён для новых записей. Пример: "New Soil %1".
    virtual QString newItemNameTemplate() const = 0;

    virtual void emitDataChanged(const QModelIndex &changedIndex);

private slots:
    /// Испукается dataChanged исходя из отправителя и изменённого свойства.
    void emitDataChanged();

private:
    /// Добавляет @p item к нам.
    /// Делается через undo-команду (если mUndoStack != NULL)
    void addItem(Item *item, const QString &originalItemName = QString());

    /// Список использованных цветов.
    QList<QColor> usedColorsList() const;

    /// Следующий цвет для новой записи. Если не использованы все цвета из
    /// @m mColorsToAssign, берётся оттуда; иначе генерируется случайный.
    QColor nextColor() const;

    /// Следующее имя для новой записи.
    QString nextName() const;

    /// Следующее название дубликата записи в строке @p row
    QString nextCopyName(int row) const;

    /// QMetaObject хранящихся в нас записей
    const QMetaObject mItemsMetaObject;

    /// Список имеющихся в модели записей.
    QList<Item *> mItems;

    /// Список свойств записи, которые используются для значений в столбцах
    /// (в порядке следования столбцов).
    const QList<const char *> mProperties;

    /// Список индексов сигналов, соответствующих свойствам из @m mProperties
    /// (в порядке следования столбцов).
    const QList<int> mNotifySignals;

    /// Список стандартных цветов для записей.
    const QList<QColor> mDefaultColors;

    /// Список физических свойств значений в столбцах.
    const QHash<int, PhysicalProperty> mPhysicalProperties;

    /// Стек undo, который мы должны использовать, или NULL, если изменения
    /// следует делать напрямую.
    QUndoStack *const mUndoStack;

    /// Строка, изменения которой записываются, или -1, если изменения
    /// записывать не надо
    int mRecordingRow;
    /// Список изменённых значений (и старые значения), составленный при
    /// mRecordingRow, отличном от -1
    ItemChanges mRecordedChanges;

    /// Элемент, всегда присутствующий здесь. Его изменение/удаление невозможно.
    const Item *const mFixedItem;
    bool mHasFixedItem;

    /// Начинает записывать изменения столбцов в строке @p row
    void startRecordingChanges(int row);
    /// Заканчивает записывать изменения столбцов, чистит mRecordedChanges и
    /// создаёт undo-команду с проделанными изменениями
    void stopRecordingChanges();

    void connectTo(Item *item);
};

}

#endif // QFGUI_ITEMSMODEL_H
