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

#include "unitsmenu.h"

#include <units.h>
#include <QTimer>

using namespace qfgui;

UnitsMenu::UnitsMenu(QWidget *parent)
    : QMenu(tr("System of &Units"), parent)
{
    setIcon(QIcon::fromTheme("accessories-calculator"));

    UnitsSystemActionGroup *actionsGroup = new UnitsSystemActionGroup(this);
    foreach(QAction * action, actionsGroup->actions()) {
        addAction(action);
    }

    QTimer *t = new QTimer(this);
    t->setSingleShot(false);
    //t->start(3000);
    connect(t, SIGNAL(timeout()), this, SLOT(lol()));
}

void UnitsMenu::lol()
{
    int i;
    for (i = 0; i < Units::SystemsNum; ++i) {
        if (actions().at(i)->isChecked()) {
            break;
        }
    }
    ++i;
    if (i == Units::SystemsNum) {
        i = 0;
    }
    actions().at(i)->trigger();
}
