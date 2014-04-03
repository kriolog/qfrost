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

#include "boundaryconditionsremovecommand.h"

#include <boundary_conditions/boundarycondition.h>
#include <boundarypolygoncalc.h>
#include <boundarypolygon.h>
#include <mainwindow.h>
#include <application.h>

using namespace qfgui;



BoundaryConditionsRemoveCommand::BoundaryConditionsRemoveCommand(const QList<Item *> &items)
    : ItemsRemoveCommand(items)
    , mAffectedPolygons()
{
    Q_ASSERT(!items.isEmpty());
    QList<const BoundaryCondition *> conditionsList;
    foreach(Item * item, items) {
        Q_ASSERT(qobject_cast<BoundaryCondition *>(item) != NULL);
        conditionsList << qobject_cast<const BoundaryCondition *>(item);
    }

    BoundaryPolygonCalc calc(Application::findMainWindow(items.first())->qfScene());
    QList<QPair<BoundaryPolygon *, QList<Vertex> > > t;
    t = calc.removeConditionsOperation(conditionsList);
    QList<QPair<BoundaryPolygon *, QList<Vertex> > >::ConstIterator it;
    for (it = t.constBegin(); it != t.constEnd(); ++it) {
        Q_ASSERT(!mAffectedPolygons.contains(it->first));
        mAffectedPolygons[it->first] = qMakePair(it->first->corners(),
                                       it->second);
    }

    setText(names().size() == 1
            ? (QUndoStack::tr("remove boundary cond. %1", "single")
               .arg(names().first()) + "\n"
               //: Short action name. Accusative case.
               + QUndoStack::tr("remove b. cond."))
            : (QUndoStack::tr("remove boundary cond. %1", "plural").arg(names().join(", ")) + "\n"
               //: Short action name. Accusative case.
               + QUndoStack::tr("remove %n b. cond.", "", names().size())));
}


void BoundaryConditionsRemoveCommand::redo()
{
    ItemsRemoveCommand::redo();
    for (ChangedPolygons::Iterator it = mAffectedPolygons.begin();
            it != mAffectedPolygons.end(); ++it) {
        it.key()->setCorners(it->second);
    }
}

void BoundaryConditionsRemoveCommand::undo()
{
    ItemsRemoveCommand::undo();
    for (ChangedPolygons::Iterator it = mAffectedPolygons.begin();
            it != mAffectedPolygons.end(); ++it) {
        it.key()->setCorners(it->first);
    }
}
