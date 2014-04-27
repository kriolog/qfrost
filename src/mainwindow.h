/*
 * Copyright (C) 2010-2014  Denis Pesotsky
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

#ifndef QFGUI_MAINWINDOW_H
#define QFGUI_MAINWINDOW_H

#include <QtWidgets/QMainWindow>

QT_FORWARD_DECLARE_CLASS(QFile)
QT_FORWARD_DECLARE_CLASS(QAction)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QUndoView)
QT_FORWARD_DECLARE_CLASS(QUndoStack)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Units)
QT_FORWARD_DECLARE_CLASS(BlocksLogger)
QT_FORWARD_DECLARE_CLASS(ControlPanel)
QT_FORWARD_DECLARE_CLASS(Scene)
QT_FORWARD_DECLARE_CLASS(ToolsPanel)
QT_FORWARD_DECLARE_CLASS(BoundaryCondition)
QT_FORWARD_DECLARE_CLASS(View)
QT_FORWARD_DECLARE_CLASS(ColorGenerator)

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    MainWindow(const QString &fileName);
    ~MainWindow();

    View *qfView() const {
        return mView;
    }

    Scene *qfScene() const {
        return mScene;
    }

    QUndoStack *undoStack() const {
        return mUndoStack;
    }

    bool isUntitled() const {
        return mIsUntitled;
    }

    ControlPanel *controlPanel() const {
        return mControlPanel;
    }

    /// Главное используемое расширения файлов (начинается с точки)
    static const QString kMainExt;

    /// Сортированный список всех граничных условий (включая "пустое")
    QList<BoundaryCondition *> boundaryConditions() const;

    /// Путь к открытому файлу включая имя файла -- QFileInfo::canonicalFilePath
    const QString &currentFilePath() const {
        return mCurrentFilePath;
    }

    /// Имя открытого файла без расширения -- QFileInfo::completeBaseName
    const QString &currentFileBaseName() const {
        return mCurrentFileBaseName;
    }

    /// Путь к открытому файла включая имя файла без расширения
    QString currentFileBasePath() const {
        return mCurrentPath + "/" + mCurrentFileBaseName;
    }

    /// Раздел настроек главного окна (в QSettings)
    static const QString settingsGroup;

    const ColorGenerator *colorGenerator() const {
        return mColorGenerator;
    }

    Units *const units() {
        return mUnits;
    }

    QString settingsMenuText() const;

protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);

    /// Делает exec() у mDialogToExecOnShow (если тот не NULL)
    void showEvent(QShowEvent *event);

public slots:
    void open(const QString &fileName);

private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    bool exportData();
    bool exportDataForPlot();
    bool exportImage();
    void about();
    void documentWasModified();

private:
    void init();
    void createActions();
    void createUndo();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    /**
     * Пытается загрузить файл по имени @p fileName. Если не удалось, ругается
     * и делает @a mIsUntitled в true
     */
    void loadFile(const QString &fileName);
    /// Пытается загрузиться из @p file. Возвращает текст ошибки или QString().
    QString tryLoad(QFile &file);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    /// Испускает сигнал для остановки рассчётов и ждёт, пока они остановятся
    void forceStopComputation();

    View *mView;
    Scene *mScene;

    /// Путь к открытому файлу включая имя файла -- QFileInfo::canonicalFilePath
    QString mCurrentFilePath;
    /// Путь к открытому файлу без имени файла -- QFileInfo::canonicalPath
    QString mCurrentPath;
    /// Имя открытого файла без расширения -- QFileInfo::completeBaseName
    QString mCurrentFileBaseName;
    /// Имя открытого файла с расширением -- QFileInfo::fileName
    QString mCurrentFileName;

    bool mIsUntitled;

    QAction *mNewAct;
    QAction *mOpenAct;
    QAction *mSaveAct;
    QAction *mSaveAsAct;
    QAction *mExportAct;
    QAction *mExportDataForPlotAct;
    QAction *mExportImageAct;
    QAction *mCloseAct;
    QAction *mExitAct;
    QAction *mAboutAct;
    QAction *mAboutQtAct;
    QAction *mRemoveUnneededBlocksAct;
    QAction *mUndoAct;
    QAction *mRedoAct;
    QAction *mPlotAct;
    QAction *mSettingsAct;
    QAction *mFullScreenAct;
    QAction *mViewColorAct;
    QAction *mDiscretizeColors;

    ColorGenerator *mColorGenerator;

    ToolsPanel *mToolsPanel;
    ControlPanel *mControlPanel;

    //********************** UNDO **************************
    QUndoStack *mUndoStack;
    QUndoView *mUndoView;

    bool mComputationsAreActive;

    /// Нужно для загрузки файлов (см. open())
    bool mDontSave;

    Units *mUnits;

    /// Диалог, который мы должны показать сразу после нашего show() (или NULL)
    QDialog *mDialogToExecOnShow;

    /// Форматы файлов для нашей программы (так, как надо для QFileDialog)
    static QString formats();

    /**
     * Проверяет @p file. Если там при записи кончилось место,
     * пытается удалить этот файл. При любой ошибке, включая кончившееся место,
     * кидает QMessageBox::warning с заголовком @p errorTitle и текстом ошибки.
     * @return всё ли в порядке с файлом.
     * @warning файл должен быть открыт только на чтение!
     */
    bool checkFile(QFile &file, const QString &errorTitle);

    /**
     * Пытается открыть файл @p file с флагами @p flags. Если не удалось,
     * кидает QMessageBox::warning с заголовком @p errorTitle.
     * @return удалось ли открыть файл
     */
    bool tryOpen(QFile &file,
                 QIODevice::OpenMode flags,
                 const QString &errorTitle);

    /**
     * Сохраняет данные из @p logger в файл.
     * @return true, если получилось или пользователь не захотел сохранять
     */
    bool saveLoggerData(const BlocksLogger &logger);

private slots:
    /// Блокирует (разблокирует) вмешательство в вещи пользователем.
    void onComputationStateChanged(bool on);
    /** Обновляет mUndoAct и mRedoAct исходя из mComputationsAreActive.
     *  Нужно потому, что undo stack отменяет их disabled при новых действиях,
     *  что делает их доступными во время расчётов */
    void updateUndoActs();
    /// Изменяет статус изменённости документа.
    void setWindowNotModified(bool b);
    /// Открывает диалог, строящий текущие данные сцены (и сохраняющий картинки)
    void openPlotDialog();
    /// Открывает диалог изменения настроек приложения
    void openSettingsDialog();
    /// Пытается сохранить данные из @p logger в файл, пока это не получится
    void onLoggerDataAvailable(const qfgui::BlocksLogger &logger);
    /// Переключает полноэкранный режим
    void switchFullScreen();
    /// Обновляет mFullScreenAct (иконку и текст) исходя из его checked
    void updateFullScreenAction();

signals:
    /// Требование остановить рассчёты
    void stopComputation();
};

}

#endif // QFGUI_MAINWINDOW_H
