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

#include "colorbar.h"

#include <graphicsviews/view.h>
#include <colorgenerator.h>
#include <mainwindow.h>
#include <control_panel/controlpanel.h>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QScrollBar>

using namespace qfgui;

ColorBar::ColorBar(View *parent,
                   const ColorGenerator *colorGenerator,
                   bool showsTemperature,
                   ColorBar *rightBar)
    : QWidget(parent)
    , mColorGenerator(colorGenerator)
    , mIsTemporaryHidden(false)
    , mNeighbour(rightBar)
    , mIsLeft(rightBar != NULL)
    , mShowsTemperature(showsTemperature)
    , mTemperatureZeroTick()
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    resize(40, mShowsTemperature ? 300 : 200);

    parent->viewport()->installEventFilter(this);
    if (mIsLeft) {
        mNeighbour->installEventFilter(this);
        mNeighbour->mNeighbour = this;
    }
    connect(colorGenerator, SIGNAL(changed(bool)), SLOT(update()));

    MainWindow *m = qobject_cast< MainWindow * >(window());
    Q_ASSERT(m != NULL);
    Q_ASSERT(m->controlPanel() != NULL);
    connect(m->controlPanel(), SIGNAL(signalChangeBlockStyle(QFrost::BlockStyle)),
            SLOT(updateVisibility(QFrost::BlockStyle)));

    connect(parent, SIGNAL(colorSchemeChanged()), SLOT(update()));

    hide();
}

bool ColorBar::eventFilter(QObject *object, QEvent *event)
{
    View *const view = qobject_cast<View * >(parent());
    if (view == NULL) {
        // Евент, вероятно, вызван при закрытии окна
        return false;
    }

    QWidget *const viewport = view->viewport();

    if (object == viewport) {
        switch (event->type()) {
        case QEvent::Resize: {
            QResizeEvent *resizeEvent = static_cast<QResizeEvent *>(event);
            updatePos(resizeEvent->size());
            break;
        }
        case QEvent::MouseMove:
        case QEvent::HoverMove:
            if (isVisible()) {
                /// Отступ в пикселах, при достижении которого курсором мы прячемся
                static const int margin = 15;

                const QPoint mousePos = static_cast<QMouseEvent *>(event)->pos();
                bool cursorIsInside = geometry().adjusted(-margin, -margin,
                                      margin, margin)
                                      .contains(mousePos);
                mCursorIsHere = cursorIsInside;
                updateTemporaryHide();
            }
            break;
        case QEvent::Leave:
            mCursorIsHere = false;
            updateTemporaryHide();
            break;
        default:
            break;
        }
    } else {
        Q_ASSERT(object == mNeighbour && mIsLeft);
        switch (event->type()) {
        case QEvent::Move:
        case QEvent::Hide:
        case QEvent::Show:
        case QEvent::Resize:
            updatePos(viewport->size());
            break;
        default:
            break;
        }
    }

    return false;
}

void ColorBar::hideEvent(QHideEvent *event)
{
    if (mCursorIsHere) {
        mCursorIsHere = false;
        updateTemporaryHide();
    }
    QWidget::hideEvent(event);
}

void ColorBar::updateTemporaryHide(bool warnNeighbor)
{
    Q_ASSERT(mNeighbour != NULL);
    bool mustHide = isVisible() && (mCursorIsHere || mNeighbour->mCursorIsHere);
    if (mustHide != mIsTemporaryHidden) {
        mIsTemporaryHidden = mustHide;
        if (warnNeighbor) {
            mNeighbour->updateTemporaryHide(false);
        }
        update();
    }
}

void ColorBar::updatePos(const QSize &viewPortSize)
{
    int posX;
    int posY = viewPortSize.height() - height() - 10;
    if (mIsLeft && mNeighbour->isVisible()) {
        posX = mNeighbour->pos().x() - 10;
    } else {
        posX = viewPortSize.width() - 12;
    }
    posX -= width();
    Q_ASSERT(qobject_cast< View * >(parent()) != NULL);
    move(static_cast<View *>(parent())->viewport()->mapToParent(QPoint(posX, posY)));
}

void ColorBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    if (mIsTemporaryHidden) {
        return;
    }
    View *view = qobject_cast<View *>(parent());
    Q_ASSERT(view != NULL);
    QPainter p(this);
    //p.fillRect(rect(), Qt::yellow);
    if (mShowsTemperature) {
        mColorGenerator->drawTemperatureLegend(&p, rect(), view->isLight(),
                                               mTemperatureZeroTick);
    } else {
        mColorGenerator->drawThawedPartLegend(&p, rect(), view->isLight());
    }
}

void ColorBar::updateVisibility(QFrost::BlockStyle style)
{
    bool mustShow;
    switch (style) {
    case QFrost::blockShowsTemperature:
    case QFrost::blockShowsTemperatureField:
    case QFrost::blockShowsTemperatureDiffField:
        mustShow = mShowsTemperature;
        break;
    case QFrost::blockShowsThawedPartField:
        mustShow = !mShowsTemperature;
        break;
    case QFrost::blockShowsConditionField:
        mustShow = true;
        break;
    case QFrost::blockShowsSoil:
    case QFrost::blockShowsBoundaryConditions:
    default:
        mustShow = false;
    }
    mTemperatureZeroTick = (style == QFrost::blockShowsTemperatureDiffField)
                           ? "T<sub>bf</sub>"
                           : "0";
    setVisible(mustShow);
}
