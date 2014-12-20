/*
 * Copyright (C) 2014  Denis Pesotsky
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
 * 
 */

#ifndef QFGUI_BACKGROUNDDIALOG_H
#define QFGUI_BACKGROUNDDIALOG_H

#include <QDialog>

#include <QElapsedTimer>

QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QGraphicsPixmapItem)
QT_FORWARD_DECLARE_CLASS(QSpinBox)
QT_FORWARD_DECLARE_CLASS(QDoubleSpinBox)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Cross)
QT_FORWARD_DECLARE_CLASS(ViewBase)

class BackgroundDialog : public QDialog
{
    Q_OBJECT
public:
    BackgroundDialog(const QPixmap &pixmap, QWidget *parent = NULL);

signals:
    void accepted(const QPixmap &pixmap, const QTransform &transform);

protected:
    bool eventFilter(QObject *object, QEvent *event);

private slots:
    void acceptAndSendResult();

    void updateCross1Pos();
    void updateCross2Pos();

    /// Обновляет доступность кнопки OK исходя из правильности введённых данных.
    void checkCrossesPos();

    void startPlacingCross1();
    void startPlacingCross2();
    void finishPlacingCross();

private:
    ViewBase *const mView;

    QDialogButtonBox *const mButtons;

    QGraphicsPixmapItem *const mPixmapItem;

    Cross *const mCross1;
    Cross *const mCross2;

    QSpinBox *const mCross1PixmapX;
    QSpinBox *const mCross1PixmapY;
    QSpinBox *const mCross2PixmapX;
    QSpinBox *const mCross2PixmapY;

    QDoubleSpinBox *const mCross1SceneX;
    QDoubleSpinBox *const mCross1SceneY;
    QDoubleSpinBox *const mCross2SceneX;
    QDoubleSpinBox *const mCross2SceneY;

    QPushButton *mPlaceCross1Button;
    QPushButton *mPlaceCross2Button;

    bool mIsPlacingCross1;
    bool mIsPlacingCross2;

    QElapsedTimer mViewPressTimer;
};
}

#endif // QFGUI_BACKGROUNDDIALOG_H
