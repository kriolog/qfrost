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

#ifndef QFGUI_CURVEPLOTDIALOG_H
#define QFGUI_CURVEPLOTDIALOG_H

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(QCheckBox)
QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QDoubleSpinBox)

namespace qfgui {

QT_FORWARD_DECLARE_CLASS(Block)
QT_FORWARD_DECLARE_CLASS(CurvePlot)
QT_FORWARD_DECLARE_CLASS(PhysicalPropertySpinBox)

class CurvePlotDialog : public QDialog
{
    Q_OBJECT
public:
    CurvePlotDialog(Block *block,
                    Qt::Orientation orientation,
                    QWidget *parent = NULL);

    virtual ~CurvePlotDialog();

    void loadMinMaxCoord(double min, double max);
    void loadMinMaxTemperature(double min, double max);

signals:
    /// Испускается после подтверждения, если надо запомнить лимиты координат.
    void savingMinMaxCoord(double min, double max);

    /// Испускается после подтверждения, если надо запомнить лимиты температур.
    void savingMinMaxTemperature(double min, double max);

private slots:
    /// Устанавливает значения mMinCoord и mMaxCoord исходя из mCoordsMain.
    void autoMinMaxCoord();

    /// Устанавливает значения mMinT и mMaxT исходя из mTemperatures.
    void autoMinMaxTemperature();

    /// Устанавливает максимумы для mMinTemperature/mMinCoord и минимумы для
    /// mMaxTemperature/mMaxCoord исходя из введённых в них значениях.
    void updateAdditionalLimits();

    /// Устанавливает доступность кнопки автоподбора пределов координаты исходя
    /// из введённых значений (кнопка доступна, если они не автоподобраны).
    void updateAutoMinMaxCoordButton();

    /// Устанавливает доступность кнопки автоподбора пределов температуры исходя
    /// из введённых значений (кнопка доступна, если они не автоподобраны).
    void updateAutoMinMaxTemperatureButton();

    /// Устанавливает диапазон оси координат у mPlot в соответствие 
    /// со значениями mMinCoord и mMaxCoord.
    void setPlotRangeCoords();

    /// Устанавливает диапазон оси температур у mPlot в соответствие 
    /// со значениями mMinTemperature и mMaxTemperature.
    void setPlotRangeTemperature();

    /// Открывает диалог сохранения графика в PDF.
    bool savePDF();

    /// Открывает диалог сохранения графика в PNG.
    bool savePNG();

    /// Открывает диалог сохранения первичных данных.
    bool savePrimaryData();

    /// Испускает savingMinMaxCoord / savingMinMaxTemperature, если есть галки.
    void emitSavingMinMax();

private:
    /// Текущее время модели.
    QDate modelDate() const;

    /// Обновляет mMinCoordLimit и mMaxCoordLimit для активного среза.
    void updateKnownCoordLimits();
    /// Обновляет mMinTemperatureLimit/mMaxTemperatureLimit для активного среза.
    void updateKnownTemperatureLimits();

    void setMinMaxCoord(double min, double max);
    void setMinMaxTemperature(double min, double max);

    CurvePlot *mPlot;

    const Qt::Orientation mOrientation;

    QCheckBox *mPlotTemperature;
    QCheckBox *mPlotThawedPard;
    QCheckBox *mPlotTransitionTemperature;

    QCheckBox *mShowModelDateText;

    PhysicalPropertySpinBox *mMinTemperature;
    PhysicalPropertySpinBox *mMaxTemperature;
    QPushButton *mAutoMinMaxTemperature;
    QCheckBox *mSaveMinMaxTemperature;

    QDoubleSpinBox *mMinCoord;
    QDoubleSpinBox *mMaxCoord;
    QPushButton *mAutoMinMaxCoord;
    QCheckBox *mSaveMinMaxCoord;

    QList<Block*> mSlice;
    QVector<double> mTemperatures;
    QVector<double> mThawedParts;
    QVector<double> mTransitionTemperatures;
    QVector<double> mCoordsMain;
    QVector<double> mCoordsNormal;

    QPushButton *mSavePNGButton;
    QPushButton *mSavePDFButton;
    QPushButton *mSavePrimaryData;

    QDialogButtonBox *mDialogButtons;

    bool mIsUpdatingAdditionalLimits;

    /// Название (полный путь минус расширение) сохраняемых файлов.
    const QString mSavedFileBaseName;

    double mKnownCoordMin;
    double mKnownCoordMax;

    double mKnownTemperatureMin;
    double mKnownTemperatureMax;
};

/**
 * Класс для вызова диалога построения кривой. Сохраняет/восстанавливает лимиты.
 */
class CurvePlotDialogSpawner : public QObject
{
    Q_OBJECT
public:
    CurvePlotDialogSpawner(QWidget *parent);

    CurvePlotDialog *execDialog(Block *block, Qt::Orientation orientation);

private slots:
    void saveMinMaxCoord(double min, double max);
    void saveMinMaxTemperature(double min, double max);

private:
    bool mSavedMinMaxCoord;
    double mMinCoord;
    double mMaxCoord;

    bool mSavedMinMaxTemperature;
    double mMinTemperature;
    double mMaxTemperature;
};

}

#endif // QFGUI_CURVEPLOTDIALOG_H
