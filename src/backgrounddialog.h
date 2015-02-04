/*
 * Copyright (C) 2014-2015  Denis Pesotsky
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
QT_FORWARD_DECLARE_CLASS(QCheckBox)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Cross)
QT_FORWARD_DECLARE_CLASS(ViewBase)

class BackgroundDialog : public QDialog
{
    Q_OBJECT
public:
    BackgroundDialog(const QString &imageFileName,
                     const QPixmap &pixmap, 
                     QWidget *parent = NULL);

signals:
    void accepted(const QPixmap &pixmap, const QTransform &transform);

protected:
    bool eventFilter(QObject *object, QEvent *event);

    void showEvent(QShowEvent *event);

private slots:
    /// Отправляет сигналом результат, сохраняет файл привязки (если отмечена
    /// соответствующая галочка) и вызывает accept().
    /// Если получается неравномерное масштабирование, предупреждает об этом.
    void acceptAndSendResult();

    void updateCross1Pos();
    void updateCross2Pos();

    /// Обновляет доступность кнопки OK исходя из правильности введённых данных.
    void checkCrossesPos();

    void startPlacingCross1();
    void startPlacingCross2();
    void finishPlacingCross();

    /// Сохраняет файл привязки. Если файл уже существует, сперва спрашивает,
    /// нужно ли его перезаписывать.
    /// @returns был ли сохранён файл.
    bool saveReferenceFile();

    /// Если файл привязки существует, пытается загрузмть его, снимает галочку 
    /// сохранения (mSaveReferenceFile) и уведомляет об успешной загрузке.
    bool tryLoadReferenceFile();

    /// Открывает диалог автоматической установки значения mCross1SceneX исходя
    /// из прочих введённых данных с указанием соотношения масштабирования x:y.
    void autoSetCross1SceneX();
    /// Открывает диалог автоматической установки значения mCross1SceneY исходя
    /// из прочих введённых данных с указанием соотношения масштабирования x:y.
    void autoSetCross1SceneY();
    /// Открывает диалог автоматической установки значения mCross2SceneX исходя
    /// из прочих введённых данных с указанием соотношения масштабирования x:y.
    void autoSetCross2SceneX();
    /// Открывает диалог автоматической установки значения mCross2SceneY исходя
    /// из прочих введённых данных с указанием соотношения масштабирования x:y.
    void autoSetCross2SceneY();

    /// Показывает окошко с оповещением об успешной загрузке файла привязки.
    void showReferenceFileNotification();

private:
    /// Устанавливает значение mCross1SceneX исходя из прочих введённых данных
    /// так, чтобы картинка масштабировалась с соотношением (x:y), равным @p d.
    void autoSetCross1SceneX(double d);
    /// Устанавливает значение mCross1SceneY исходя из прочих введённых данных
    /// так, чтобы картинка масштабировалась с соотношением (x:y), равным @p d.
    void autoSetCross1SceneY(double d);
    /// Устанавливает значение mCross2SceneX исходя из прочих введённых данных
    /// так, чтобы картинка масштабировалась с соотношением (x:y), равным @p d.
    void autoSetCross2SceneX(double d);
    /// Устанавливает значение mCross2SceneY исходя из прочих введённых данных
    /// так, чтобы картинка масштабировалась с соотношением (x:y), равным @p d.
    void autoSetCross2SceneY(double d);

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

    /// Путь к файлу привязки.
    const QString mReferenceFileName;

    QCheckBox *mSaveReferenceFile;

    /// Расширение файла привязки картинки (начинается с точки).
    static const QString kReferenceFileExtension;

    /// Должны ли мы показать сообщение о загрузке файла привязки в showEvent().
    bool mNeedReferenceFileNotification;
};
}

#endif // QFGUI_BACKGROUNDDIALOG_H
