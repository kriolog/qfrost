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

#include "correctedstyle.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyleFactory>

using namespace qfgui;

CorrectedStyle::CorrectedStyle()
    : QStyle()
    , mStyleForSplitters(QStyleFactory::create("fusion"))
{
    if (mStyleForSplitters == NULL) {
        qFatal("CorrectedStyle: Can't create style");
    }
    mStyleForSplitters->setParent(this);
}

QPixmap CorrectedStyle::generatedIconPixmap(QIcon::Mode iconMode,
        const QPixmap &pixmap,
        const QStyleOption *opt) const
{
    return QApplication::style()->generatedIconPixmap(iconMode, pixmap, opt);
}

QPixmap CorrectedStyle::standardPixmap(QStyle::StandardPixmap standardPixmap,
                                       const QStyleOption *opt,
                                       const QWidget *widget) const
{
    return QApplication::style()->standardPixmap(standardPixmap, opt, widget);
}

int CorrectedStyle::styleHint(QStyle::StyleHint stylehint,
                              const QStyleOption *opt, const QWidget *widget,
                              QStyleHintReturn *returnData) const
{
    return QApplication::style()->styleHint(stylehint, opt, widget, returnData);
}

QSize CorrectedStyle::sizeFromContents(QStyle::ContentsType ct,
                                       const QStyleOption *opt,
                                       const QSize &contentsSize,
                                       const QWidget *w) const
{
    return QApplication::style()->sizeFromContents(ct, opt, contentsSize, w);
}

int CorrectedStyle::pixelMetric(QStyle::PixelMetric metric,
                                const QStyleOption *option,
                                const QWidget *widget) const
{
    if (metric == QStyle::PM_DockWidgetSeparatorExtent) {
        return mStyleForSplitters->pixelMetric(metric, option, widget);
    } else {
        return QApplication::style()->pixelMetric(metric, option, widget);
    }
}

QRect CorrectedStyle::subControlRect(QStyle::ComplexControl cc,
                                     const QStyleOptionComplex *opt,
                                     QStyle::SubControl sc,
                                     const QWidget *widget) const
{
    return QApplication::style()->subControlRect(cc, opt, sc, widget);
}

QStyle::SubControl CorrectedStyle::hitTestComplexControl(QStyle::ComplexControl cc,
        const QStyleOptionComplex *opt,
        const QPoint &pt,
        const QWidget *widget) const
{
    return QApplication::style()->hitTestComplexControl(cc, opt, pt, widget);
}

void CorrectedStyle::drawComplexControl(QStyle::ComplexControl cc,
                                        const QStyleOptionComplex *opt,
                                        QPainter *p, const QWidget *widget) const
{
    QApplication::style()->drawComplexControl(cc, opt, p, widget);
}

QRect CorrectedStyle::subElementRect(QStyle::SubElement subElement,
                                     const QStyleOption *option,
                                     const QWidget *widget) const
{
    return QApplication::style()->subElementRect(subElement, option, widget);
}

void CorrectedStyle::drawControl(QStyle::ControlElement element,
                                 const QStyleOption *opt, QPainter *p,
                                 const QWidget *w) const
{
    QApplication::style()->drawControl(element, opt, p, w);
}

void CorrectedStyle::drawPrimitive(QStyle::PrimitiveElement pe,
                                   const QStyleOption *opt, QPainter *p,
                                   const QWidget *w) const
{
    if (pe == QStyle::PE_IndicatorDockWidgetResizeHandle) {
        mStyleForSplitters->drawPrimitive(pe, opt, p, w);
    } else {
        QApplication::style()->drawPrimitive(pe, opt, p, w);
    }
}

void CorrectedStyle::polish(QWidget *w)
{
    QApplication::style()->polish(w);
}

void CorrectedStyle::unpolish(QWidget *w)
{
    QApplication::style()->unpolish(w);
}

void CorrectedStyle::polish(QApplication *a)
{
    QApplication::style()->polish(a);
}

void CorrectedStyle::unpolish(QApplication *a)
{
    QApplication::style()->unpolish(a);
}

void CorrectedStyle::polish(QPalette &p)
{
    QApplication::style()->polish(p);
}

QRect CorrectedStyle::itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const
{
    return QApplication::style()->itemTextRect(fm, r, flags, enabled, text);
}

QRect CorrectedStyle::itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const
{
    return QApplication::style()->itemPixmapRect(r, flags, pixmap);
}

void CorrectedStyle::drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    QApplication::style()->drawItemText(painter, rect, flags, pal, enabled, text, textRole);
}

void CorrectedStyle::drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const
{
    QApplication::style()->drawItemPixmap(painter, rect, alignment, pixmap);
}

QPalette CorrectedStyle::standardPalette() const
{
    return QApplication::style()->standardPalette();
}
