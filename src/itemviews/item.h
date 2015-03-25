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

#ifndef QFGUI_ITEM_H
#define QFGUI_ITEM_H

#include <QtCore/QObject>

#include <QtCore/QHash>
#include <QtCore/QMap>
#include <QtCore/QMetaType>

#include <QtGui/QColor>
#include <QtGui/QBrush>

#include <qfrost.h>

typedef QMap<double, double> DoubleMap;

Q_DECLARE_METATYPE(qint32)
Q_DECLARE_METATYPE(DoubleMap)
Q_DECLARE_METATYPE(QList<double>)

namespace qfgui
{

class Item : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    Item(const QString &name, const QColor &color);

    /// Конструктор для случая, если в дальнейшем планируется делать load()
    Item();

    virtual ~Item();

    const QString &name() const {
        return mName;
    }

    const QColor &color() const {
        return mColor;
    }

    inline const QBrush *brush() const {
        return &mBrush;
    }

    /** Применяет первые или вторые значения (в зависимости
     * от @p useSecondValues) из @p changes. */
    void applyChanges(const ItemChanges &changes, bool useSecondValues);

    /// Копирует значения всех свойств из @p other
    void copyPropertiesFrom(const Item *other);

    /// Значение свойства @p propertyName в родительном падеже
    virtual QString shortPropertyNameGenetive(const QString &propertyName);

    /// Наш порядковый номер, используемый при сохранении, чтобы сущности в
    /// сцене могли сохранять связь с нами
    qint32 id() const {
        return mID;
    }

    void setID(qint32 id) {
        mID = id;
    }

    /// Сохраняет в @p out все наши properties
    void save(QDataStream &out) const;
    /// Загружает из @p in все наши properties
    void load(QDataStream &in, int version);

    /**
     * Увеличивает счётик кол-во ссылающихся на этот элемент undo-команд.
     * Этот счётчик нужен для того, чтобы элемент сам удалялся, если на него
     * больше не ссылается ни одной undo-команды, и при этом он не находится в
     * модели. @sa decreaseUndoReferenceCount */
    inline void increaseUndoReferenceCount() {
        ++mUndoReferenceCount;
    }

    /**
     * Уменьшает счётик кол-ва ссылающихся на этот элемент undo-команд и удаляет
     * этот элемент, если счётчик стал равен нулю и при этом мы не находимся в
     * модели. @sa increaseUndoReferenceCount */
    inline void decreaseUndoReferenceCount() {
        Q_ASSERT(mUndoReferenceCount > 0);
        --mUndoReferenceCount;
        if (mUndoReferenceCount == 0 && parent() == NULL) {
            delete this;
        }
    }

protected:
    /// Изменения этих свойств следует применять в первую очередь
    virtual QList<QString> priorityProperties() {
        return QList<QString>();
    }

    /// Количество свойств, не хватающих в версии файла .qfrost @p version, по
    /// сравнению с текущей версией.
    /// Нужно для обратной совместимости (которая будет работать, если свойства
    /// только ДОБАВЛЯЛИСЬ - иначе обратную совместимость не сохранить).
    virtual int propertiesLackCount(int version) { return 0; }

public slots:
    void setName(const QString &name);
    void setColor(const QColor &color);

signals:
    void nameChanged(const QString &name);
    void colorChanged(const QColor &color);

private:
    QString mName;
    QColor mColor;
    QBrush mBrush;
    qint32 mID;

    /// Кол-во ссылающихся на этот элемент undo-команд
    ushort mUndoReferenceCount;
};

}

#endif // QFGUI_ITEM_H
