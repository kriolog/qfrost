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

#include "itemstabledialog.h"

#include <QtWidgets/QVBoxLayout>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QUndoStack>
#include <QtCore/QCoreApplication>

#include <itemviews/itemswidget.h>
#include <mainwindow.h>

using namespace qfgui;

ItemsTableDialog::ItemsTableDialog(ItemsWidget *parent)
    : QDialog(parent)
    , mItemsWidget(qobject_cast<ItemsWidget *>(parent->metaObject()->newInstance(Q_ARG(QWidget *, this),
                   Q_ARG(bool, false),
                   Q_ARG(ItemsModel *, parent->model()))))
{
    Q_ASSERT(mItemsWidget != NULL);

    setWindowFlags(windowFlags() ^ Qt::WindowContextHelpButtonHint);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(mItemsWidget);

    resize(780, 300);
}

void ItemsTableDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Undo)) {
        Q_ASSERT(parentWidget() != NULL);
        MainWindow *mainWindow = qobject_cast< MainWindow * >(parentWidget()->window());
        Q_ASSERT(mainWindow != NULL);
        mainWindow->undoStack()->undo();
    } else if (event->matches(QKeySequence::Redo)) {
        Q_ASSERT(parentWidget() != NULL);
        MainWindow *mainWindow = qobject_cast< MainWindow * >(parentWidget()->window());
        Q_ASSERT(mainWindow != NULL);
        mainWindow->undoStack()->redo();
    } else {
        /* Используем обработчик у QWidget (а не у QDialog), чтобы при нажатии
         * Enter оно не делало ничего (не нажимало на первую кнопку внизу) */
        QWidget::keyPressEvent(event);
    }
}
