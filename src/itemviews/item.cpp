/*
 * Copyright (C) 2012-2013  Denis Pesotsky
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

#include "item.h"

#include <QMetaProperty>

#include <soils/soil.h>

using namespace qfgui;

static void registerMetatypes()
{
    bool wasAlreadyRun = false;
    if (!wasAlreadyRun) {
        qRegisterMetaTypeStreamOperators<qint32>("qint32");
        qRegisterMetaTypeStreamOperators<QList<double> >("QList<double>");
        qRegisterMetaTypeStreamOperators<DoubleMap>("DoubleMap");
        wasAlreadyRun = true;
    }
}

Item::Item(const QString &name, const QColor &color)
    : QObject()
    , mName(name)
    , mColor(color)
    , mBrush(color)
    , mID()
    , mUndoReferenceCount(0)
{
    registerMetatypes();
}

Item::Item()
    : QObject()
    , mName()
    , mColor()
    , mBrush(Qt::white) // это чтобы оно не было Qt::NoBrush после смены цвета
    , mID()
    , mUndoReferenceCount(0)
{
    registerMetatypes();
}

Item::~Item()
{
    qDebug("Item '%s' (%s) destroyed",
           qPrintable(name()),
           metaObject()->className());
}

void Item::setName(const QString &name)
{
    if (mName != name) {
        mName = name;
        emit nameChanged(mName);
    }
}

void Item::setColor(const QColor &color)
{
    if (mColor != color) {
        mColor = color;
        mBrush.setColor(mColor);
        emit colorChanged(mColor);
    }
}

void Item::applyChanges(const ItemChanges &changes, bool useSecondValues)
{
    typedef QList<QPair<QString, QVariant > > SortedChanges;

    SortedChanges sortedChanges;
    foreach(const QString & prop, priorityProperties()) {
        ItemChanges::ConstIterator it = changes.constFind(prop);
        if (it != changes.constEnd()) {
            sortedChanges << qMakePair(it.key(),
                                       !useSecondValues ? it->first : it->second);
        }
    }
    for (ItemChanges::ConstIterator it = changes.constBegin(); it != changes.constEnd(); ++it) {
        if (!priorityProperties().contains(it.key())) {
            sortedChanges << qMakePair(it.key(),
                                       !useSecondValues ? it->first : it->second);
        }
    }

    for (SortedChanges::ConstIterator it = sortedChanges.constBegin();
            it != sortedChanges.constEnd(); ++it) {
        if (!it->second.isValid()) {
            continue;
        }
        bool ok = setProperty(qPrintable(it->first), it->second);
        Q_ASSERT(ok);
    }
}

void Item::copyPropertiesFrom(const Item *other)
{
    Q_ASSERT(metaObject()->className() == other->metaObject()->className());

    const Soil *soil = qobject_cast<const Soil *>(other);
    const bool isSoilWithCurve = ((soil != NULL)
                                  && soil->usesUnfrozenWaterCurve());
    ItemChanges changes;
    for (int i = Item::staticMetaObject.propertyOffset(); i < metaObject()->propertyCount(); ++i) {
        QMetaProperty property = metaObject()->property(i);
        QString propertyName = property.name();
        if (isSoilWithCurve && (propertyName == "transitionTemperature"
                                || propertyName == "transitionHeat")) {
            // Температура/теплота ф.п. пересчитываются сами, если грунт
            // использует кривую незамёрзшей воды
            continue;
        }
        changes[propertyName] = qMakePair(QVariant(), property.read(other));
    }

    applyChanges(changes, true);
}

QString Item::shortPropertyNameGenetive(const QString &propertyName)
{
    if (propertyName == "color") {
        //: In genetive case
        return tr("color", "genetive");
    } else if (propertyName == "name") {
        //: In genetive case
        return tr("name", "genetive");
    } else {
        qWarning("%s::shortPropertyNameGenetive: no result for %s",
                 metaObject()->className(),
                 qPrintable(propertyName));
        return "???";
    }
}


void Item::save(QDataStream &out) const
{
    qDebug("saving item '%s' of type %s", qPrintable(mName), metaObject()->className());
    for (int i = Item::staticMetaObject.propertyOffset();
            i < metaObject()->propertyCount(); ++i) {
        QVariant v = metaObject()->property(i).read(this);
        qDebug("... property '%s' (type id: %i [%s])",
               metaObject()->property(i).name(),
               v.userType(),
               v.typeName());
        Q_ASSERT(v.isValid());
        Q_ASSERT(v.type() != QVariant::UInt);
        // int может разниться в размере в разных окружениях, так что сохраняем qint32
        out << (v.type() == QVariant::Int
                ? QVariant(qint32(v.toInt()))
                : v);
        if (out.status() != QDataStream::Ok) {
            throw false;
        }
    }
    out << mID;
    qDebug("done saving item!");
}

void Item::load(QDataStream &in)
{
    for (int i = Item::staticMetaObject.propertyOffset();
            i < metaObject()->propertyCount(); ++i) {
        QVariant v;
        in >> v;
        if (v.typeName() == "qint32") {
            v = QVariant(int(v.value<qint32>()));
        }
        if (v.userType() != metaObject()->property(i).userType()) {
            qDebug("Failed loading property %s: bad type. Check metatypes registering order!",
                   metaObject()->property(i).name());
            throw false;
        }
        if (!v.isValid()) {
            throw false;
        }
        bool ok = setProperty(metaObject()->property(i).name(), v);
        if (in.status() != QDataStream::Ok || !ok) {
            throw false;
        }
    }
    in >> mID;
    if (in.status() != QDataStream::Ok) {
        throw false;
    }
}
