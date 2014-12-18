/*
 * Copyright (C) 2010-2012  Denis Pesotsky, Maxim Torgonsky
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

#ifndef QFGUI_TOOLSPANEL_H
#define QFGUI_TOOLSPANEL_H

#include <QtWidgets/QDockWidget>

#include <QtCore/QMap>

#include <qfrost.h>

QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QStackedWidget)
QT_FORWARD_DECLARE_CLASS(QActionGroup)
QT_FORWARD_DECLARE_CLASS(QToolButton)

namespace qfgui
{

class ToolSettings;

QT_FORWARD_DECLARE_CLASS(MainWindow)

class InfoWidget : public QWidget
{
    Q_OBJECT
public:
    InfoWidget(const QString &text, QWidget *buddy = 0,
               bool needHelpButton = false, QWidget *parent = 0);
    QWidget *buddy() {
        return mBuddy;
    }
    const QString &text() const {
        return mText;
    }
    bool needHelpButton() const {
        return mNeedHelpButton;
    }
private:
    QWidget *mBuddy;
    QString mText;
    bool mNeedHelpButton;
};

class ToolsPanel : public QDockWidget
{
    Q_OBJECT
public:
    ToolsPanel(MainWindow *parent = 0);
    QAction *checkedAction() const;
    void triggerDefaultTool();

    QMap<QFrost::ToolType, ToolSettings *> toolsSettings();

private:
    QActionGroup *mTools;
    QAction *mPickNoTool;
    QAction *mPickBlockCreator;
    QAction *mPickBoundaryPolygonCreator;
    QAction *mPickRectangleSelection;
    QAction *mPickBoundaryEllipseCreator;
    QAction *mPickBoundaryConditionsCreator;
    QAction *mPickPolygonalSelection;
    QAction *mPickEllipseSelection;

    /// Инструмент, выбранный при последнем вызове slotBlockTools(true)
    QAction *mToolBeforeBlocking;

    QStackedWidget *mToolsPanels;

    QLabel *mToolTitle;

    QAction *mHelpAction;
    QToolButton *mHelpButton;

    void addTool(QAction *action,
                 const QString &text = QString(),
                 QWidget *widget = 0, bool needHelpButton = false);

    /// Указатель на виджет с номером @p index в mToolsPanels.
    /// Имеется в виду QScrollBarArea::widget(QStackedWidget::widget)
    QWidget *innerWidget(int index);

    InfoWidget *currentInfoWidget();

public slots:
    /**
     * Слот, принимающий сигнал от @a mToolsSelectGroup, отправляющий
     * соответствующий сигнал signalToolChosen и меняющий
     * виджет в @a mToolsPanels на нужный
     */
    void slotToolChosen(QAction *action);
    
    /**
     * Слот, блокирующий и разблокирующий возможность выбрать инструмент.
     * При блокировке выбирается инструмент "курсор".
     * При разблокировке возврается инструмент, выбранный до неё.
     */
    void slotBlockTools(bool doBlock);
    
    void showToolTip();

signals:
    void toolPicked(QFrost::ToolType);
};

}

#endif // QFGUI_TOOLSPANEL_H
