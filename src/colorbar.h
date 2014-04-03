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

#ifndef QFGUI_COLORBAR_H
#define QFGUI_COLORBAR_H

#include <QtWidgets/QWidget>

#include <qfrost.h>

namespace qfgui
{
QT_FORWARD_DECLARE_CLASS(ColorGenerator)
QT_FORWARD_DECLARE_CLASS(View)

class ColorBar : public QWidget
{
    Q_OBJECT
public:
    ColorBar(View *parent,
             const ColorGenerator *colorGenerator,
             bool showsTemperature,
             ColorBar *rightBar = NULL);
protected:
    void paintEvent(QPaintEvent *event);
    void hideEvent(QHideEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void updateVisibility(QFrost::BlockStyle style);
    void updateTemporaryHide(bool warnNeighbor = true);

private:
    const ColorGenerator *mColorGenerator;
    /// Нужно ли пропускать paintEvent. Устанавливать в true, например, при
    /// наведении курсора.
    bool mIsTemporaryHidden;
    bool mCursorIsHere;
    ColorBar *mNeighbour;
    bool mIsLeft;

    bool mShowsTemperature;

    void updatePos(const QSize &viewPortSize);
};

}

#endif // QFGUI_COLORBAR_H
