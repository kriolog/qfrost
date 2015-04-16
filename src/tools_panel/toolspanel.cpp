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

#include <tools_panel/toolspanel.h>

#include <QtWidgets/QAction>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QToolTip>
#include <QtWidgets/QGroupBox>

#include <mainwindow.h>
#include <tools_panel/blockcreatorpanel.h>
#include <tools_panel/selectionpanel.h>
#include <tools_panel/toolsettings.h>
#include <tools_panel/rectangulartoolsettings.h>
#include <tools_panel/rectangulartoolpanel.h>
#include <tools_panel/curveplottoolpanel.h>
#include <tools_panel/curveplottoolsettings.h>

using namespace qfgui;

InfoWidget::InfoWidget(const QString &text, QWidget *buddy,
                       bool needHelpButton, QWidget *parent)
    : QWidget(parent)
    , mBuddy(buddy)
    , mText(text)
    , mNeedHelpButton(needHelpButton)
{
    // Разделяем параграфы
    mText.replace("<br>", "<br><br>");
    mText.replace("<br/>", "<br><br>");
    mText.replace("<br />", "<br><br>");
    QVBoxLayout *layout = new QVBoxLayout(this);
    if (!mNeedHelpButton && !mText.isEmpty()) {
        QLabel *label = new QLabel(mText, this);
        label->setWordWrap(true);
        layout->addWidget(label);
    }

    if (mBuddy != NULL) {
        layout->addWidget(mBuddy);
    }
}

ToolsPanel::ToolsPanel(MainWindow *parent): QDockWidget(tr("Tools Panel"), parent),
    mTools(new QActionGroup(this)),
    mPickNoTool(new QAction(QIcon(":/tools/no_tool.png"),
                            tr("&No Tool"), this)),
    mPickBlockCreator(new QAction(QIcon(":/tools/blocks_creator.png"),
                                  tr("&Blocks Creator"), this)),
    mPickBoundaryPolygonCreator(new QAction(QIcon(":/tools/boundary_polygon.png"),
                                            tr("&Boundary Polygon Creator"), this)),
    mPickRectangleSelection(new QAction(QIcon(":/tools/rectangle_selection.png"),
                                        tr("&Rectangle Selection"), this)),
    mPickBoundaryEllipseCreator(new QAction(QIcon(":/tools/boundary_ellipse.png"),
                                            tr("&Boundary Ellipse Creator"), this)),
    mPickBoundaryConditionsCreator(new QAction(QIcon(":/tools/boundary_condition.png"),
                                   tr("&Boundary Conditions Applicator"), this)),
    mPickPolygonalSelection(new QAction(QIcon(":/tools/polygonal_selection.png"),
                                        tr("&Polygonal Selection"), this)),
    mPickEllipseSelection(new QAction(QIcon(":/tools/ellipse_selection.png"),
                                        tr("&Ellipse Selection"), this)),
    mPickCurvePlot(new QAction(QIcon(":/tools/curve_plot.png"),
                                     tr("&Curve Plot"), this)),
    mToolBeforeBlocking(mPickNoTool),
    mToolsPanels(new QStackedWidget(this)),
    mToolTitle(new QLabel(this)),
    mHelpAction(new QAction(QIcon::fromTheme("help-hint"), "", this)),
    mHelpButton(new QToolButton(this)),
    mToolSettingsLayout(new QVBoxLayout())
{
    setAllowedAreas(Qt::DockWidgetAreas(Qt::LeftDockWidgetArea +
                                        Qt::RightDockWidgetArea));
    setFeatures(DockWidgetFeatures(QDockWidget::DockWidgetMovable +
                                   QDockWidget::DockWidgetFloatable));
    setMaximumWidth(300);

    QWidget *widget = new QWidget(this);
    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setWidget(widget);
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);

    QString escHint(tr("To cancel drawing, press <b>Esc</b> or any tool icon."));
    QString polygonDelHint(tr("Pressing <b>Backspace</b> will delete last added point."));

    // Должно совпадать с порядком в QFrost::ToolType!
    addTool(mPickNoTool, "");
    addTool(mPickPolygonalSelection,
            tr("Create polygon with sequential clicks to select blocks and "
               "you will be able to set their soil and staring conditions.")
            + "<br>" + polygonDelHint + "<br>" + escHint);
    addTool(mPickRectangleSelection,
            tr("Select wanted blocks and you will be able to set their soil "
               "and staring conditions.") + " " + escHint,
            new RectangularToolPanel(this));
    addTool(mPickBoundaryPolygonCreator,
            tr("Draw wanted polygon with sequential clicks and add it with "
               "<b>Enter</b> or subtract it with <b>Shift+Enter</b>.")
            + "<br>" + polygonDelHint + "<br>" + escHint);
    addTool(mPickBoundaryEllipseCreator,
            tr("Draw wanted ellipse and add it with <b>Enter</b> or subtract "
               " it with <b>Shift+Enter</b>.") + " " + escHint,
            new RectangularToolPanel(this));
    addTool(mPickBoundaryConditionsCreator,
            tr("Tool for choosing boundary conditions on polygons' borders.<br>"
               "First click selects first point. All following "
               "clicks allow to choose condition for selected "
               "part and start next part selection.<br>"
               "Press <b>Esc</b> or any tool icon to stop sequence."));
    addTool(mPickBlockCreator,
            tr("Select rectangle, choose additional properties and create "
               "blocks in one of two following ways.<br>"
               "Press <b>Enter</b> to create all block according to geometrical progression.<br>"
               "Press <b>Shift+Enter</b> to create all blocks according to "
               "geometrical progression and additional smaller block at end "
               "of each row and column, if it's needed to fully fill the rectangle.")
            + "<br>" + escHint,
            new BlockCreatorPanel(this), true);
    addTool(mPickEllipseSelection,
            tr("Select wanted blocks and you will be able to set their soil "
            "and staring conditions.") + " " + escHint,
            new RectangularToolPanel(this));
    addTool(mPickCurvePlot,
            tr("Select single block to plot temperature (and thawed part) graphs "
               "for horizontal or vertical slice through it.<br>"
               "Resulting graphs and initial (numerical) data can be saved to file system."),
            new CurvePlotToolPanel(this));

    QVBoxLayout *toolsLayout = new QVBoxLayout();
    toolsLayout->setSpacing(0);
    toolsLayout->setMargin(0);
    toolsLayout->setContentsMargins(QMargins());
    mainLayout->addLayout(toolsLayout);

    QMap<QAction *, QToolButton *> buttons;
    foreach(QAction * action, mTools->actions()) {
        action->setCheckable(true);
        QToolButton *button = new QToolButton(this);
        button->setDefaultAction(action);
        button->setIconSize(QSize(36, 36));
        button->setAutoRaise(true);
        buttons[action] = button;
    }

    QHBoxLayout *l1 = new QHBoxLayout();
    l1->setSpacing(0);
    l1->setMargin(0);
    l1->setContentsMargins(QMargins());
    l1->addWidget(buttons[mPickNoTool]);
    l1->addWidget(buttons[mPickBlockCreator]);
    l1->addWidget(buttons[mPickCurvePlot]);
    toolsLayout->addLayout(l1);

    QGroupBox *selectionTools = new QGroupBox(tr("Blocks selection"));
    selectionTools->setAlignment(Qt::AlignHCenter);
    selectionTools->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    QHBoxLayout *l2 = new QHBoxLayout(selectionTools);
    l2->setSpacing(0);
    l2->setMargin(0);
    l2->setContentsMargins(QMargins());
    l2->addWidget(buttons[mPickRectangleSelection]);
    l2->addWidget(buttons[mPickPolygonalSelection]);
    l2->addWidget(buttons[mPickEllipseSelection]);
    toolsLayout->addWidget(selectionTools);

    QGroupBox *boundaryTools = new QGroupBox(tr("Boundary polygons"));
    boundaryTools->setAlignment(Qt::AlignHCenter);
    boundaryTools->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    QHBoxLayout *l3 = new QHBoxLayout(boundaryTools);
    l3->setSpacing(0);
    l3->setMargin(0);
    l3->setContentsMargins(QMargins());
    l3->addWidget(buttons[mPickBoundaryPolygonCreator]);
    l3->addWidget(buttons[mPickBoundaryEllipseCreator]);
    l3->addWidget(buttons[mPickBoundaryConditionsCreator]);
    toolsLayout->addWidget(boundaryTools);

    mToolTitle->setWordWrap(true);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    mHelpButton->setDefaultAction(mHelpAction);
    connect(mHelpAction, SIGNAL(triggered()), this, SLOT(showToolTip()));
    titleLayout->addWidget(mToolTitle);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(QMargins());
    titleLayout->addWidget(mHelpButton);

    mToolSettingsLayout->setMargin(0);
    mToolSettingsLayout->addLayout(titleLayout);
    mToolSettingsLayout->addWidget(mToolsPanels);
    mainLayout->addLayout(mToolSettingsLayout);

    connect(mTools, SIGNAL(triggered(QAction *)),
            SLOT(slotToolChosen(QAction *)));
}

void ToolsPanel::showToolTip()
{
    QToolTip::showText(QCursor::pos(), mHelpAction->toolTip(), mHelpButton);
}

void ToolsPanel::addTool(QAction *action, const QString &text,
                         QWidget *widget, bool needHelpButton)
{
    mTools->addAction(action);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setAlignment(Qt::AlignHCenter);
    scrollArea->setWidget(new InfoWidget(text, widget, needHelpButton));
    scrollArea->setMinimumSize(QSize(150, 200));
    scrollArea->setWidgetResizable(true);
    mToolsPanels->addWidget(scrollArea);
}

void ToolsPanel::triggerDefaultTool()
{
    mPickBlockCreator->trigger();
}

void ToolsPanel::slotToolChosen(QAction *action)
{
    mToolTitle->setText(QString("<center><b>%1</b></center>").
                        arg(action->text().remove("&")));
    QFrost::ToolType newToolType;
    newToolType = static_cast<QFrost::ToolType>(mTools->actions().indexOf(action));
    mToolsPanels->setCurrentIndex(newToolType);
    mHelpButton->setVisible(currentInfoWidget()->needHelpButton()); /////FUCK было setShown
    mHelpAction->setToolTip(currentInfoWidget()->text());
    emit toolPicked(newToolType);
}

QAction *ToolsPanel::checkedAction() const
{
    return mTools->checkedAction();
}

void ToolsPanel::slotBlockTools(bool doBlock)
{
    if (doBlock) {
        mToolBeforeBlocking = mTools->checkedAction();
        mPickNoTool->trigger();
    } else {
        mToolBeforeBlocking->trigger();
    }
    setDisabled(doBlock);
}

QMap< QFrost::ToolType, ToolSettings * > ToolsPanel::toolsSettings()
{
    QMap< QFrost::ToolType, ToolSettings * > result;
    QFrost::ToolType i;

    i = QFrost::blockCreator;
    BlockCreatorPanel *p1 = qobject_cast<BlockCreatorPanel *>(innerWidget(i));
    Q_ASSERT(p1 != NULL);
    result[i] = p1->toolSettings();

    i = QFrost::rectangularSelection;
    RectangularToolPanel *p2 = qobject_cast<RectangularToolPanel *>(innerWidget(i));
    Q_ASSERT(p2 != NULL);
    result[i] = p2->toolSettings();

    i = QFrost::boundaryEllipseCreator;
    RectangularToolPanel *p3 = qobject_cast<RectangularToolPanel *>(innerWidget(i));
    Q_ASSERT(p3 != NULL);
    result[i] = p3->toolSettings();

    i = QFrost::ellipseSelection;
    RectangularToolPanel *p4 = qobject_cast<RectangularToolPanel *>(innerWidget(i));
    Q_ASSERT(p4 != NULL);
    result[i] = p4->toolSettings();

    i = QFrost::curvePlot;
    CurvePlotToolPanel *p5 = qobject_cast<CurvePlotToolPanel *>(innerWidget(i));
    Q_ASSERT(p5 != NULL);
    result[i] = p5->toolSettings();

    return result;
}

QWidget *ToolsPanel::innerWidget(int index)
{
    QScrollArea *s = qobject_cast<QScrollArea *>(mToolsPanels->widget(index));
    Q_ASSERT(s != NULL);
    QWidget *w = s->widget();
    InfoWidget *iw = qobject_cast<InfoWidget *>(w);
    if (iw != NULL) {
        return iw->buddy();
    } else {
        return w;
    }
}

InfoWidget *ToolsPanel::currentInfoWidget()
{
    QScrollArea *s = qobject_cast<QScrollArea *>(mToolsPanels->currentWidget());
    Q_ASSERT(s != NULL);
    QWidget *w = s->widget();
    InfoWidget *iw = qobject_cast<InfoWidget *>(w);
    Q_ASSERT(iw != NULL);
    return iw;
}
