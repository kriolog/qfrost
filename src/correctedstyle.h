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

#ifndef QFGUI_CORRECTEDSTYLE_H
#define QFGUI_CORRECTEDSTYLE_H

#include <qstyle.h>


namespace qfgui
{

/**
 * This style allows to draw dock widget separators in QMainWindow in Windows.
 * It uses QApplication::style() for everything except
 * drawPrimitive(PE_IndicatorDockWidgetResizeHandle, ...) and
 * pixelMetric(QStyle::PM_DockWidgetSeparatorExtent, ...).
 * Just do setStyle(new CorrectedStyle) for your mainwindow (with approptiate
 * ifdefs to detect Windows platform).
 */
class CorrectedStyle : public QStyle
{
    Q_OBJECT

public:
    CorrectedStyle();
    virtual QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap, const QStyleOption *opt) const;
    virtual QPixmap standardPixmap(QStyle::StandardPixmap standardPixmap, const QStyleOption *opt = 0, const QWidget *widget = 0) const;
    virtual int styleHint(QStyle::StyleHint stylehint, const QStyleOption *opt = 0, const QWidget *widget = 0, QStyleHintReturn *returnData = 0) const;
    virtual QSize sizeFromContents(QStyle::ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *w = 0) const;
    virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option = 0, const QWidget *widget = 0) const;
    virtual QRect subControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *opt, QStyle::SubControl sc, const QWidget *widget = 0) const;
    virtual QStyle::SubControl hitTestComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt, const QPoint &pt, const QWidget *widget = 0) const;
    virtual void drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *widget = 0) const;
    virtual QRect subElementRect(QStyle::SubElement subElement, const QStyleOption *option, const QWidget *widget = 0) const;
    virtual void drawControl(QStyle::ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
    virtual void drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
    virtual void polish(QWidget *);
    virtual void unpolish(QWidget *);
    virtual void polish(QApplication *);
    virtual void unpolish(QApplication *a);
    virtual void polish(QPalette &p);
    virtual QRect itemTextRect(const QFontMetrics &fm, const QRect &r, int flags, bool enabled, const QString &text) const;
    virtual QRect itemPixmapRect(const QRect &r, int flags, const QPixmap &pixmap) const;
    virtual void drawItemText(QPainter *painter, const QRect &rect, int flags, const QPalette &pal, bool enabled, const QString &text, QPalette::ColorRole textRole = QPalette::NoRole) const;
    virtual void drawItemPixmap(QPainter *painter, const QRect &rect, int alignment, const QPixmap &pixmap) const;
    virtual QPalette standardPalette() const;

private:
    QStyle *mStyleForSplitters;
};

}

#endif // QFGUI_CORRECTEDSTYLE_H
