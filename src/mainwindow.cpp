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

#include <mainwindow.h>

#include <QtCore/QSettings>
#include <QtCore/QTextStream>
#include <QtCore/QTimer>
#include <QtCore/QMimeData>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QUndoView>
#include <QtWidgets/QUndoStack>

#include <about.h>
#include <main.h>
#include <control_panel/controlpanel.h>
#include <qfrost.h>
#include <graphicsviews/ruler.h>
#include <graphicsviews/scene.h>
#include <graphicsviews/view.h>
#include <recentfilesmenu.h>
#include <tools/anchor.h>
#include <tools_panel/toolspanel.h>
#include <boundary_conditions/boundaryconditionspanel.h>
#include <boundary_conditions/boundaryconditionsmodel.h>
#include <boundary_conditions/boundarycondition.h>
#include <soils/soilspanel.h>
#include <soils/soilsmodel.h>
#include <control_panel/computationcontrol.h>
#include <control_panel/startingconditions.h>
#include <application.h>
#include <computations/blockslogger.h>
#include <graphicsviews/colorgenerator.h>
#include <graphicsviews/colorbar.h>
#include <units/unitsmenu.h>
#include <units/units.h>
#include <positionlabel.h>
#include <toolbar.h>
#include <blockscountlabel.h>
#include <welcomedialog.h>
#include <dialog.h>
#include <backgrounddialog.h>

#ifdef WIN32
#include <correctedstyle.h>
#endif

using namespace qfgui;

/// Магическое число, которым начинаются файлы нашего формата (мёртвая сиська)
static const quint32 kFilesMagicNumber = 0xD3ADB00B;
/// Версия формата файлов ".qfrost", в котором мы сохраняем и считываем
/* 1. первоначальная версия
 * 2. то же, что 1, но в конец дописывается кол-во шагов в сутках
 * 3. координаты в сцене в 10 раз больше (уменьшен QFrost::metersInUnit)
 * 4. температура ф.п. стала числом, добавились масштаб и позиция view
 * 5. переделана система сохранения грунтов и граничных условий (в связи с их
 *    переводом на qfgui::Item) + теперь сохраняется галочка осесимметричности
 * 6. в конец теперь дописываются системы измерения (qint32)
 * 7. QDataStream версии 4.8 вместо 5.0
 */
static const quint32 kFilesVersion = 7;
/// Версия настроек (QSettings)
static const int kSettingsVersion = 2;

static const QDataStream::Version kDataStreamVersion = QDataStream::Qt_5_0;

const QString MainWindow::settingsGroup = "mainwindow";
const QString MainWindow::kMainExt = ".qfrost";

inline QString strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

MainWindow::MainWindow()
{
    init();
    setCurrentFile("");

    mDialogToExecOnShow = new WelcomeDialog(this);
}

MainWindow::MainWindow(const QString &fileName)
{
    init();
    loadFile(fileName);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (mDontSave) {
        event->accept();
        return;
    }

    forceStopComputation();
    if (maybeSave()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}
void MainWindow::forceStopComputation()
{
    emit stopComputation();
    // это чтобы сцена приняла сигнал с последними данными
    QCoreApplication::processEvents();
}


void MainWindow::newFile()
{
    MainWindow *other = new MainWindow;
    other->move(x() + 40, y() + 40);
    other->show();
}

void MainWindow::open()
{
    const QString fileName = Dialog::getOpenFileName(this,
                                                     tr("Open File", "Dialog Title"),
                                                     QString(),
                                                     formats());
    open(fileName);
}

void MainWindow::open(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        MainWindow *existing = qfApp->findMainWindow(fileName);
        if (existing) {
            existing->show();
            existing->raise();
            existing->activateWindow();
            return;
        }

        if (mIsUntitled && !isWindowModified()) {
            loadFile(fileName);
            if (mIsUntitled) {
                // загрузка не удалась? создадим вместо нас другое окно,
                // чтобы не заморачиваться с очисткой
                MainWindow *other = new MainWindow;
                other->show();
                mDontSave = true;
                close();
                // FIXME надо туда переносить текущие настройки задачи и не
                //       открывать диалог приветствия
            }
        } else {
            MainWindow *other = new MainWindow(fileName);
            if (other->mIsUntitled) {
                other->mDontSave = true;
                other->close();
                return;
            }
            other->move(x() + 40, y() + 40);
            other->show();
        }
    }
}

bool MainWindow::save()
{
    forceStopComputation();
    if (mIsUntitled) {
        return saveAs();
    } else {
        return saveFile(mCurrentFilePath);
    }
}

QString MainWindow::formats()
{
    return tr("QFrost files") + QString(" (*%1)").arg(kMainExt);
}

bool MainWindow::saveAs()
{
    forceStopComputation();
    const QString fileName = Dialog::getSaveFileName(this,
                                                     tr("Save", "Dialog Title"),
                                                     mCurrentFilePath,
                                                     formats());
    if (fileName.isEmpty()) {
        return false;
    }

    return saveFile(fileName);
}

bool MainWindow::exportData()
{
    static const QString exportFailedTitle = tr("Export Failed");
    static const QString txtExtension = ".txt";
    QString fileName = Dialog::getSaveFileName(this,
                                               tr("Export Data", "Dialog Title"),
                                               currentFileBasePath() + txtExtension,
                                               tr("Text files") + QString(" (*%1)").arg(txtExtension));

    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    if (!tryOpen(file,
                 QFile::WriteOnly | QFile::Text | QFile::Truncate,
                 exportFailedTitle)) {
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // FIXME вынести обёртку в функцию
    mScene->exportData(out);
    QApplication::restoreOverrideCursor();

    if (!checkFile(file, exportFailedTitle)) {
        return false;
    }

    statusBar()->showMessage(tr("Current data exported"), 2000);
    return true;
}

bool MainWindow::exportDataForPlot()
{
    static const QString exportFailedTitle = tr("Export Failed");
    static const QString txtExtension = ".txt";
    const QString fileName = Dialog::getSaveFileName(this,
                                                     tr("Export Data for Plot", "Dialog Title"),
                                                     currentFileBasePath() + txtExtension,
                                                     tr("Text files") + QString(" (*%1)").arg(txtExtension));
    
    if (fileName.isEmpty()) {
        return false;
    }
    
    QFile file(fileName);
    if (!tryOpen(file,
        QFile::WriteOnly | QFile::Text | QFile::Truncate,
        exportFailedTitle)) {
        return false;
    }
    
    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // FIXME вынести обёртку в функцию
    mScene->exportDataForPlot(out);
    QApplication::restoreOverrideCursor();
    
    if (!checkFile(file, exportFailedTitle)) {
        return false;
    }
    
    statusBar()->showMessage(tr("Current data exported"), 2000);
    return true;
}

bool MainWindow::exportImage()
{
    static const QString exportFailedTitle = tr("Saving Image Failed");
    static const QString format = "png";
    const QString fileName = Dialog::getSaveFileName(this,
                                                     tr("Save Image", "Dialog Title"),
                                                     currentFileBasePath() + "." + format,
                                                     tr("PNG files") + QString(" (*.%1)").arg(format));

    if (fileName.isEmpty()) {
        return false;
    }

    // FIXME вынести обёртку в функцию
    int oldFrameStyle = mView->frameStyle();
    mView->setFrameStyle(QFrame::NoFrame);
    Ruler::updateChildrenOffsets(centralWidget(), true);

    QPalette oldPalette = centralWidget()->palette();
    QPalette newPalette = oldPalette;
    newPalette.setBrush(QPalette::Window, Qt::white);
    newPalette.setBrush(QPalette::Text, Qt::black);
    centralWidget()->setPalette(newPalette);

    mScene->anchor()->hide();
    const QPixmap image = mView->viewport()->grab();

    mView->setFrameStyle(oldFrameStyle);
    centralWidget()->setPalette(oldPalette);
    mScene->anchor()->show();
    Ruler::updateChildrenOffsets(centralWidget(), false);

    QFile file(fileName);
    if (!tryOpen(file,
        QIODevice::WriteOnly | QIODevice::Truncate,
        exportFailedTitle)) {
        return false;
    }
    bool success = image.save(&file, qPrintable(format));
    if (!checkFile(file, exportFailedTitle)) {
        return false;
    }
    if (!success) {
        // Сюда мы вряд ли должны попасть - QPixmap::save не вернуло бы true
        QMessageBox::warning(this,
                             tr("Export Failed"),
                             tr("Cannot write file %1.")
                             .arg(locale().quoteString(fileName)));
        return false;
    }
    statusBar()->showMessage(tr("Current data exported"), 2000);
    return true;
}

bool MainWindow::saveLoggerData(const BlocksLogger &logger)
{
    static const QString exportFailedTitle = tr("Save Logger Data Failed");

    static const QString filterForCSV = tr("Comma-separated monthly temperature tables") + " (*.csv)";
    static const QString filterForLastBlockTXT = tr("Text files with bottom block data") + " (*.txt)";
    static const QString filterForScript = tr("Text files for isopleth plotter script") + " (*.txt)";

    QString selectedFilter;
    const QString fileName = Dialog::getSaveFileName(this,
                                                     tr("Save Logger Data", "Dialog Title"),
                                                     currentFileBasePath() + ".csv",
                                                     filterForCSV + ";;" + filterForLastBlockTXT + ";;" + filterForScript,
                                                     &selectedFilter);

    if (fileName.isEmpty() || selectedFilter.isEmpty()) {
        // юзер отменил диалог, значит ему эти данные не нужны
        return true;
    }

    QFile file(fileName);
    if (!tryOpen(file,
                 QFile::WriteOnly | QFile::Text | QFile::Truncate,
                 exportFailedTitle)) {
        return false;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    logger.exportData(out,
                      selectedFilter == filterForCSV
                      ? BlocksLogger::CSV
                      : selectedFilter == filterForLastBlockTXT
                      ? BlocksLogger::LastBlockTXT
                      : BlocksLogger::ForPlotterScript);
    QApplication::restoreOverrideCursor();

    if (!checkFile(file, exportFailedTitle)) {
        return false;
    }

    statusBar()->showMessage(tr("Logger data exported"), 2000);
    return true;
}

void MainWindow::onLoggerDataAvailable(const BlocksLogger &logger)
{
    while (!saveLoggerData(logger)) {}
}

void MainWindow::about()
{
    static const QString den = tr("Denis Pesotsky");
    static const QString max = tr("Maxim Torgonsky");
    About aboutDialog(tr("Temperature fields modeling and visualization"),
                      COPYRIGHT_YEARS, den + ", " + max, this);

    aboutDialog.addAuthor(den, tr("Main developer"),
                          "dev@qfrost.net");
    aboutDialog.addAuthor(max, tr("Developer, many ideas and algorythms"),
                          "kriolog@gmail.com");

    aboutDialog.addCredit(tr("L. N. Chrustalev"),
                          tr("Initial computation algorythm"));

    aboutDialog.addCredit(tr("G. P. Pustovoyt"),
                          tr("Explanation of theory and inspiration"));

    aboutDialog.addCredit(tr("S. N. Buldovich"),
                          tr("Help with designing user interface and more"));

    aboutDialog.addCredit(tr("L. V. Emelyanova"),
                          tr("Motivation and overall help"));

    aboutDialog.addCredit(tr("L.O.R community"),
                          tr("Help with C++ and Qt"),
                          "", "linux.org.ru");

    aboutDialog.addCredit(tr("CrossPlatfrom.RU community"),
                          tr("Help with Qt"),
                          "", "forum.crossplatform.ru");

    aboutDialog.exec();
}

void MainWindow::documentWasModified()
{
    setWindowModified(true);
}

bool MainWindow::openBackground()
{
    const QString fileName = Dialog::getOpenFileName(this,
                                                     tr("Open Background", "Dialog Title"),
                                                     QString(),
                                                     tr("Images") + " (*.png *.jpg *.jpeg *.jpe *.gif *.bmp)");

    if (fileName.isEmpty()) {
        return false;
    }
    
    QPixmap pixmap(fileName);
    
    if (pixmap.isNull()) {
        QMessageBox::warning(this,
                             tr("Open Background Failed"),
                             tr("Cannot open file %1.")
                             .arg(locale().quoteString(fileName)));
        return false;
    }
    
    BackgroundDialog *dialog = new BackgroundDialog(pixmap, this);
    dialog->exec();
    
    /*QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    item->setScale(100);
    item->setPos(1000, 1000);
    
    mScene->addItem(item);*/
}

void MainWindow::init()
{
#ifdef WIN32
    setStyle(new CorrectedStyle);
#endif
    mDialogToExecOnShow = NULL;
    mUnits = new Units(this);
    mColorGenerator = new ColorGenerator(this);

    QIcon appIcon;
    QStringList qfrostIconsFilter;
    qfrostIconsFilter << "qfrost_*x*.png";
    const QStringList qfrostIcons = QDir(":/").entryList(qfrostIconsFilter, QDir::Files);
    foreach(const QString & fileName, qfrostIcons) {
        appIcon.addFile(":/" + fileName);
    }
    QApplication::setWindowIcon(appIcon);

    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_QuitOnClose, true);
    //setWindowFlags(...);

    mComputationsAreActive = false;
    mDontSave = false;

    mScene = new Scene(this);
    connect(mColorGenerator, SIGNAL(changed()),
            mScene, SLOT(updateBlocksBrushes()));
    mView = new View(mScene, this);
    connect(mView, SIGNAL(mouseMoved(QPointF)),
            mScene, SIGNAL(mainViewMouseMoved(QPointF)));
    connect(mView, SIGNAL(scaleChanged(qreal)),
            mScene, SIGNAL(mainViewScaleChanged(qreal)));

    //setCentralWidget(mView);
    setCentralWidget(Ruler::createRulers(mView));

    createUndo();
    createActions();
    createToolBars();
    // Сначала тулбары, потом меню, иначе createPopupMenu() возвращает NULL
    createMenus();
    createStatusBar();

    readSettings();

    setUnifiedTitleAndToolBarOnMac(true);

    // Чтобы принимать все нажатия энтера в их виджетах...
    mControlPanel->installEventFilter(this);
    mToolsPanel->installEventFilter(this);

    setAcceptDrops(true);

    // Забираем драг-н-дроп у view...
    mView->installEventFilter(this); // у скроллбаров и рамки
    mView->viewport()->installEventFilter(this); // у основной части

    // WARN без этого трансформации (зум) во view не анкорится к курсору
    //      (плюс всякие слоты подсоединились и должны сработать)
    mToolsPanel->triggerDefaultTool();
}

void MainWindow::createUndo()
{
    mUndoStack = new QUndoStack(this);
    mUndoView = new QUndoView(mUndoStack);
    mUndoView->setWordWrap(true);
    mUndoView->setResizeMode(QListView::Adjust);
    mUndoView->setAlternatingRowColors(true);
    //TEST: mUndoStack->setUndoLimit(4);
    connect(mUndoStack, SIGNAL(cleanChanged(bool)),
            this, SLOT(setWindowNotModified(bool)));
}

void MainWindow::setWindowNotModified(bool b)
{
    setWindowModified(!b);
}

void MainWindow::createActions()
{
    mNewAct = new QAction(QIcon::fromTheme("document-new"),
                          tr("&New"), this);
    mNewAct->setShortcuts(QKeySequence::New);
    mNewAct->setStatusTip(tr("Create a new file"));
    mNewAct->setAutoRepeat(false);
    connect(mNewAct, SIGNAL(triggered()), this, SLOT(newFile()));

    mOpenAct = new QAction(QIcon::fromTheme("document-open"),
                           tr("&Open..."), this);
    mOpenAct->setShortcuts(QKeySequence::Open);
    mOpenAct->setStatusTip(tr("Open an existing file"));
    connect(mOpenAct, SIGNAL(triggered()), this, SLOT(open()));


    mSaveAct = new QAction(QIcon::fromTheme("document-save"),
                           tr("&Save"), this);
    mSaveAct->setShortcuts(QKeySequence::Save);
    mSaveAct->setStatusTip(tr("Save the document to disk"));
    connect(mSaveAct, SIGNAL(triggered()), this, SLOT(save()));

    mSaveAsAct = new QAction(QIcon::fromTheme("document-save-as"),
                             tr("Save &As..."), this);
    mSaveAsAct->setShortcuts(QKeySequence::SaveAs);
    mSaveAsAct->setStatusTip(tr("Save the document under a new name"));
    connect(mSaveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    mExportAct = new QAction(QIcon::fromTheme("document-export"),
                             tr("&Export..."), this);
    mExportAct->setStatusTip(tr("Export blocks data as text"));
    connect(mExportAct, SIGNAL(triggered()), this, SLOT(exportData()));
    
    mExportDataForPlotAct = new QAction(QIcon::fromTheme("document-export"),
                                        tr("Export for &plot..."), this);
    mExportDataForPlotAct->setStatusTip(tr("Export blocks data, polygons and "
                                           "additional bound points as text "
                                           "for contour plot"));
    connect(mExportDataForPlotAct, SIGNAL(triggered()), this, SLOT(exportDataForPlot()));

    mExportImageAct = new QAction(QIcon::fromTheme("image-x-generic"),
                                  tr("&Export Image..."), this);
    mExportImageAct->setStatusTip(tr("Export workspace to image"));
    connect(mExportImageAct, SIGNAL(triggered()), this, SLOT(exportImage()));

    mCloseAct = new QAction(QIcon::fromTheme("window-close"),
                            tr("&Close"), this);
    mCloseAct->setShortcut(tr("Ctrl+W"));
    mCloseAct->setStatusTip(tr("Close this window"));
    connect(mCloseAct, SIGNAL(triggered()), this, SLOT(close()));

    mExitAct = new QAction(QIcon::fromTheme("application-exit"),
                           tr("E&xit"), this);
    mExitAct->setShortcuts(QKeySequence::Quit);
    mExitAct->setStatusTip(tr("Exit the application"));
    connect(mExitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
    mExitAct->setMenuRole(QAction::QuitRole);

    // Используем windowIcon(), ибо так делают в приложениях для KDE
    mAboutAct = new QAction(windowIcon(), tr("&About"), this);
    mAboutAct->setStatusTip(tr("Show the application's About box"));
    connect(mAboutAct, SIGNAL(triggered()), this, SLOT(about()));
    mAboutAct->setMenuRole(QAction::AboutRole);

    mAboutQtAct = new QAction(QIcon(":/qt-logo"),
                              tr("About &Qt"), this);
    mAboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(mAboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    mAboutQtAct->setMenuRole(QAction::AboutQtRole);

    mRemoveUnneededBlocksAct = new QAction(QIcon::fromTheme("edit-clear"),
                                           tr("&Remove Excess Blocks"), this);
    mRemoveUnneededBlocksAct->setShortcut(QKeySequence::Delete);
    mRemoveUnneededBlocksAct->setToolTip(tr("Remove blocks outside domain"));
    mRemoveUnneededBlocksAct->setStatusTip(tr("Removes blocks that will not be included into domain"));
    connect(mRemoveUnneededBlocksAct, SIGNAL(triggered()),
            mScene, SLOT(removeUnneededBlocks()));

    mRedoAct = mUndoStack->createRedoAction(this);
    mRedoAct->setIcon(QIcon::fromTheme("edit-redo"));
    mRedoAct->setShortcut(QKeySequence::Redo);
    connect(mRedoAct, SIGNAL(changed()), SLOT(updateUndoActs()));

    mUndoAct = mUndoStack->createUndoAction(this);
    mUndoAct->setIcon(QIcon::fromTheme("edit-undo"));
    mUndoAct->setShortcut(QKeySequence::Undo);
    connect(mUndoAct, SIGNAL(changed()), SLOT(updateUndoActs()));

    mFullScreenAct = new QAction(this);
    mFullScreenAct->setCheckable(true);
    mFullScreenAct->setShortcut(QKeySequence("F11"));
    mFullScreenAct->setStatusTip(tr("Switch full screen mode"));
    mFullScreenAct->setAutoRepeat(false);
    connect(mFullScreenAct, SIGNAL(triggered()),
            SLOT(switchFullScreen()));

    mViewColorAct = new QAction(QIcon::fromTheme("fill-color"),
                                tr("&Light Background"), this);
    mViewColorAct->setCheckable(true);
    mViewColorAct->setStatusTip(tr("Switch color scheme"));
    connect(mViewColorAct, SIGNAL(toggled(bool)),
            mView, SLOT(setLightColorScheme(bool)));

    QIcon discretizeColorsIcon;
    discretizeColorsIcon.addFile(":/discretize-colors_16x16.png");
    discretizeColorsIcon.addFile(":/discretize-colors_22x22.png");
    discretizeColorsIcon.addFile(":/discretize-colors_32x32.png");
    discretizeColorsIcon.addFile(":/discretize-colors_48x48.png");
    mDiscretizeColors = new QAction(discretizeColorsIcon,
                                    tr("&Discretize Colors"), this);
    mDiscretizeColors->setCheckable(true);
    mDiscretizeColors->setChecked(mColorGenerator->discretizesColors());
    mDiscretizeColors->setStatusTip(tr("Discretize (sharpen) color of blocks"));
    connect(mDiscretizeColors, SIGNAL(toggled(bool)),
            mColorGenerator, SLOT(setDiscretizeColors(bool)));
    
    mOpenBackgroundAct = new QAction(QIcon::fromTheme("games-config-background"),
                                  tr("Open &Background"), this);
    mOpenBackgroundAct->setStatusTip(tr("Open background crosscut file"));
    connect(mOpenBackgroundAct, SIGNAL(triggered()), SLOT(openBackground())); 
}

void MainWindow::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(mNewAct);
    fileMenu->addSeparator();
    fileMenu->addAction(mOpenAct);
    RecentFilesMenu::addRecentFilesMenu(fileMenu);

    fileMenu->addSeparator();

    fileMenu->addAction(mSaveAct);
    fileMenu->addAction(mSaveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(mExportAct);
    fileMenu->addAction(mExportDataForPlotAct);
    fileMenu->addAction(mExportImageAct);
    fileMenu->addSeparator();
    fileMenu->addAction(mOpenBackgroundAct);
    fileMenu->addSeparator();
    fileMenu->addAction(mCloseAct);
    fileMenu->addAction(mExitAct);

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(mUndoAct);
    editMenu->addAction(mRedoAct);
    editMenu->addSeparator();
    editMenu->addAction(mRemoveUnneededBlocksAct);

    menuBar()->addSeparator();

    QMenu *settingsMenu = menuBar()->addMenu(tr("&Settings"));
    QMenu *popupMenu = createPopupMenu();
    Q_ASSERT(popupMenu != NULL);
    popupMenu->setTitle(tr("&Toolbars Shown"));
    popupMenu->setIcon(QIcon::fromTheme("configure-toolbars"));
    settingsMenu->addMenu(popupMenu);
    settingsMenu->addSeparator();
    settingsMenu->addAction(mFullScreenAct);
    settingsMenu->addAction(mViewColorAct);
    settingsMenu->addAction(mDiscretizeColors);
    settingsMenu->addSeparator();
    settingsMenu->addMenu(new UnitsMenu(this));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(mAboutAct);
    helpMenu->addAction(mAboutQtAct);
}

void MainWindow::createToolBars()
{
    ToolBar *toolbar = new ToolBar(tr("Main Toolbar"));
    addToolBar(toolbar);
    toolbar->setObjectName("Main Toolbar");

    toolbar->addAction(mNewAct);
    toolbar->addAction(mOpenAct);
    toolbar->addAction(mSaveAct);
    toolbar->addAction(mSaveAsAct);
    toolbar->addSeparator();

    toolbar->addAction(mUndoAct, true);
    toolbar->addAction(mRedoAct, true);
    toolbar->addAction(mRemoveUnneededBlocksAct, true);
    toolbar->addSeparator();

    toolbar->addAction(mFullScreenAct);
    toolbar->addAction(mViewColorAct, true);
    toolbar->addAction(mDiscretizeColors, true);

    mControlPanel = new ControlPanel(this);
    mControlPanel->setObjectName("Control Panel");
    addDockWidget(Qt::RightDockWidgetArea, mControlPanel);

    // Создаём только сейчас, им нужна контрольная панель
    ColorBar *bar1 = new ColorBar(mView, mColorGenerator, true);
    new ColorBar(mView, mColorGenerator, false, bar1);

    QDockWidget *undoViewDock = new QDockWidget(tr("Command List"), this);
    undoViewDock->setObjectName("Command List");
    undoViewDock->setWidget(mUndoView);
    undoViewDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, undoViewDock);

    connect(mControlPanel->startingConditions(),
            SIGNAL(signalApplyTemperature(double)),
            mScene, SLOT(slotApplyTemperatureToSelection(double)));

    connect(mControlPanel->startingConditions(),
            SIGNAL(signalApplyTemperatureGradient(double, double)),
            mScene, SLOT(slotApplyTemperatureGradientToSelection(double, double)));

    connect(mControlPanel->startingConditions(),
            SIGNAL(signalApplyThawedPart(double)),
            mScene, SLOT(slotApplyThawedPartToSelection(double)));

    connect(mControlPanel->soilsPanel(),
            SIGNAL(signalApplySoil(const Soil *, bool)),
            mScene,
            SLOT(slotApplySoilToSelection(const Soil *, bool)));

    connect(mControlPanel->soilsPanel(), 
            SIGNAL(signalBucketFillApply(const Soil*)),
            SLOT(startSoilFillApply(const Soil*)));

    connect (mScene, SIGNAL(soilFillApplyDone()), SLOT(stopSoilFillApply()));

    connect(mControlPanel->computationControl(),
            SIGNAL(signalStartComputation(ComputationSettings)),
            mScene, SLOT(slotStartComputation(ComputationSettings)));

    connect(mControlPanel->computationControl(),
            SIGNAL(signalStopComputation()),
            this, SIGNAL(stopComputation()));

    connect(this, SIGNAL(stopComputation()),
            mScene, SLOT(slotStopComputation()));

    connect(mControlPanel->computationControl(),
            SIGNAL(signalNeedBlocksRedrawing(bool)),
            mScene, SLOT(slotSetNeedBlocksRedrawing(bool)));

    connect(mScene, SIGNAL(signalBlocksSelectionChanged(bool, bool)),
            mControlPanel->soilsPanel(), SLOT(updateApplyButton(bool, bool)));

    connect(mScene, SIGNAL(signalBlocksSelectionChanged(bool)),
            mControlPanel->startingConditions(), SLOT(updateButtons(bool)));

    connect(mControlPanel, SIGNAL(signalChangeBlockStyle(QFrost::BlockStyle)),
            mScene, SLOT(slotSetBlocksStyle(QFrost::BlockStyle)));

    connect(mScene, SIGNAL(computationDateChanged(QDate)),
            mControlPanel->computationControl(),
            SLOT(slotComputationDateChanged(QDate)));

    mToolsPanel = new ToolsPanel(this);
    mToolsPanel->setObjectName("Tools Panel");
    addDockWidget(Qt::LeftDockWidgetArea, mToolsPanel);

    mScene->setToolsSettingsMap(mToolsPanel->toolsSettings());

    connect(mToolsPanel, SIGNAL(toolPicked(QFrost::ToolType)),
            mScene, SLOT(setTool(QFrost::ToolType)));

    connect(mScene, SIGNAL(signalComputationStateChanged(bool)),
            mToolsPanel, SLOT(slotBlockTools(bool)));

    connect(mScene, SIGNAL(signalComputationStateChanged(bool)),
            mControlPanel, SLOT(slotComputationStateChanged(bool)));

    connect(mScene, SIGNAL(signalComputationStateChanged(bool)),
            SLOT(onComputationStateChanged(bool)));
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Welcome to %1!")
                             .arg(QCoreApplication::applicationName()), 5000);

    mPermanentStatusText = new QLabel(this);
    statusBar()->addWidget(mPermanentStatusText);
    mPermanentStatusText->hide();

    QLabel *indicator1D = new QLabel(tr("[1D]"), this);
    indicator1D->setToolTip(tr("Are blocks placed one-dimensional (and vertically)?"));
    connect(mScene, SIGNAL(oneDimensionalityChanged(bool)),
            indicator1D, SLOT(setEnabled(bool)));

    QLabel *indicatorGrid = new QLabel(tr("[Grid]"), this);
    indicatorGrid->setText(tr("[Grid]"));
    indicatorGrid->setToolTip(tr("Are blocks gridded?\n"
                                 "Blocks are gridded if each\n"
                                 "has not more than one neighbor\n"
                                 "on each side and square of\n"
                                 "each contact fits it's dimension."));
    connect(mScene, SIGNAL(griddityChanged(bool)),
            indicatorGrid, SLOT(setEnabled(bool)));

    QWidget *indicators = new QWidget();
    QHBoxLayout *indicatorsLayout = new QHBoxLayout(indicators);
    indicatorsLayout->setContentsMargins(0, 0, 0, 0);
    indicatorsLayout->addWidget(new BlocksCountLabel(mScene, this));
    indicatorsLayout->addWidget(indicator1D);
    indicatorsLayout->addWidget(indicatorGrid);
    indicatorsLayout->addSpacing(3);
    statusBar()->addPermanentWidget(indicators);

    PositionLabel *cursorLabel = new PositionLabel(tr("Cursor"), this);
    statusBar()->addPermanentWidget(cursorLabel);
    connect(mView, SIGNAL(mouseMoved(QPointF)),
            cursorLabel, SLOT(updateText(QPointF)));

    PositionLabel *anchorLabel = new PositionLabel(tr("Anchor"), this);
    statusBar()->addPermanentWidget(anchorLabel);
    connect(mScene->anchor(), SIGNAL(signalPositionChanged(QPointF)),
            anchorLabel, SLOT(updateText(QPointF)));
    
    QSlider *slider = mView->createScaleSlider(Qt::Horizontal, this);
    slider->setMaximumWidth(150);
    statusBar()->addPermanentWidget(slider);
    
    slider->setValue(qRound(double(slider->maximum() + slider->minimum()) / 2.0 * 1.2));
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray(), kSettingsVersion);
    mViewColorAct->setChecked(settings.value("viewIsLight", false).toBool());
    mDiscretizeColors->setChecked(settings.value("discretizeColors",
                                  true).toBool());
    settings.endGroup();
    // Настройка полноэкранности подгрузилась только сейчас, обновим иконку
    updateFullScreenAction();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup(settingsGroup);
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState(kSettingsVersion));
    settings.setValue("viewIsLight", mViewColorAct->isChecked());
    settings.setValue("discretizeColors", mDiscretizeColors->isChecked());
    settings.endGroup();
}

bool MainWindow::maybeSave()
{
    if (isWindowModified()) {
        QMessageBox::StandardButton ret;
        // FIXME: иконка у кнопки Discard должна быть как в kwrite
        ret = QMessageBox::warning(this, tr("Close Document"),
                                   tr("The document %1 has been modified.\n"
                                      "Do you want to save your changes or discard them?")
                                   .arg(locale().quoteString(mCurrentFileName)),
                                   QMessageBox::Save | QMessageBox::Discard
                                   | QMessageBox::Cancel, QMessageBox::Save);
        if (ret == QMessageBox::Save) {
            return save();
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName)
{
    static const QString loadFailedTitle = tr("Load Failed");

    QFile file(fileName);

    // по дефолту считаем, что загрузка не удалась
    setCurrentFile("");

    if (!tryOpen(file, QFile::ReadOnly, loadFailedTitle)) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QString loadErrorText = tryLoad(file);
    QApplication::restoreOverrideCursor();

    if (!loadErrorText.isNull()) {
        QMessageBox::warning(isHidden() ? NULL : this,
                             loadFailedTitle,
                             tr("Cannot load file %1.")
                             .arg(locale().quoteString(strippedName(fileName)))
                             + "\n\n" + loadErrorText);
        return;
    } else {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File loaded"), 2000);
    }
}

template <class T>
static QList<const T *> castedList(QList<const Item *> items)
{
    QList<const T *> result;
    foreach(const Item * item, items) {
        Q_ASSERT(item != NULL);
        result << qobject_cast<const T * >(item);
        Q_ASSERT(result.last() != NULL);
    }
    return result;
}

QString MainWindow::tryLoad(QFile &file)
{
    static const QString badFormatError = tr("File is corrupted or is in bad format.");

    Q_ASSERT(file.isReadable());
    Q_ASSERT(file.isOpen());
    Q_ASSERT(file.openMode() == QIODevice::ReadOnly);

    QDataStream fin(&file);
    /*********************** проверяем магическое число ***********************/
    quint32 magic;
    fin >> magic;
    if (magic != kFilesMagicNumber) {
        qWarning("Load failed: bad magic number!");
        return badFormatError;
    }

    if (fin.status() != QDataStream::Ok) {
        qWarning("Load failed: cannot read magic number!");
        return badFormatError;
    }

    QByteArray data;
    /************* считываем остатки в byte array и разжимаем их **************/
    fin >> data;
    if (fin.status() != QDataStream::Ok) {
        qWarning("Load failed: cannot read raw data!");
        return badFormatError;
    }

    data = qUncompress(data);
    if (data.isEmpty()) {
        qWarning("Load failed: cannot uncompress data!");
        return badFormatError;
    }

    QDataStream in(&data, QIODevice::ReadOnly);
    in.setVersion(kDataStreamVersion);

    /************************* проверяем версию ***************************/
    quint32 version;
    in >> version;
    if (in.status() != QDataStream::Ok) {
        qWarning("Load failed: cannot read version!");
        return badFormatError;
    }
    if (version != kFilesVersion) {
        QString error;
        if (version < kFilesVersion) {
            qWarning("Load failed: version of file is too low: %d!", version);
            error = tr("File is for older version of %1.").arg(QCoreApplication::applicationName());
        } else {
            qWarning("Load failed: version of file is too high: %d!", version);
            error = tr("File is for newer version of %1.").arg(QCoreApplication::applicationName());
        }
        error += "\n";
        error += tr("Provided version ID: %1. Wanted version ID: %2.")
                 .arg(version).arg(kFilesVersion);
        return error;
    }

    /*********************** СЧИТЫВАЕМ ВСЮ ФИГНЮ ******************************/
    try {
        /****************************** грунты ********************************/
        QList<const Soil *> soils;
        soils = castedList<Soil>(mControlPanel->soilsPanel()->model()->load(in));
        /************************* граничные условия **************************/
        QList<const BoundaryCondition *> boundaryConditions;
        boundaryConditions = castedList<BoundaryCondition>(mControlPanel->boundaryConditionsPanel()
                             ->model()->load(in));
        /****************************** даты **********************************/
        mControlPanel->computationControl()->load(in);
        /***************************** сцену **********************************/
        mScene->load(in, soils, boundaryConditions);
        /***************************** view ***********************************/
        mView->load(in);
        /**************************** units ***********************************/
        mUnits->load(in);
    } catch (...) {
        /* Мы сюда никак не должны попасть: если magic number в порядке,
         * версия сооветствует и qUncompress прошло успешно,
         * (а все это проделано выше, иначе бы мы сюда не попали),
         * то данные ну никак не могут быть плохими */
        qWarning("Wrong input, but version, magic number was ok and uncompress "
                 "passed well. Check save/load algorithms!");
        return badFormatError;
    }

    return QString();
}

bool MainWindow::saveFile(const QString &fileName)
{
    static const QString saveFailedTitle = tr("Save Failed");

    QFile file(fileName);
    if (!tryOpen(file, QFile::WriteOnly | QFile::Truncate, saveFailedTitle)) {
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Записываем в начало файла магическое число
    QDataStream fout(&file);
    fout.setVersion(kDataStreamVersion);
    fout << kFilesMagicNumber;

    // Создаём поток с записью в QByteArray, чтобы потом это сжать и записать
    QByteArray rawSave;
    QDataStream out(&rawSave, QIODevice::WriteOnly);
    out.setVersion(kDataStreamVersion);

    out << kFilesVersion;
    mControlPanel->soilsPanel()->model()->save(out);
    mControlPanel->boundaryConditionsPanel()->model()->save(out);
    mControlPanel->computationControl()->save(out);
    mScene->save(out);
    mView->save(out);
    mUnits->save(out);

    // И записываем данные в файл, сжав их
    fout << qCompress(rawSave, 9);

    QApplication::restoreOverrideCursor();
    ////////////////////////////////////////////////////////////////////////////

    if (!checkFile(file, saveFailedTitle)) {
        return false;
    }

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

bool MainWindow::checkFile(QFile &file, const QString &errorTitle)
{
    Q_ASSERT(file.openMode().testFlag(QIODevice::WriteOnly));
    // если во время записи возникла ошибка.
    // например, место кончилось или флешку, куда писалось, вдруг выдернули...
    if (file.error()) {
        QString errorString = tr("Cannot write file %1.")
                              .arg(locale().quoteString(file.fileName()))
                              + "\n\n" + file.errorString();
        if (file.error() == QFile::ResourceError) {
            // на устройстве кончилось место, пытаемя удалить недописанный файл
            if (file.remove()) {
                errorString += "\n" + tr("Unfinished file has been removed.");
            } else {
                errorString += "\n" + tr("Unfinished file removal failed, "
                                         "please remove it manually.");
            }
        }
        QMessageBox::warning(isHidden() ? NULL : this,
                             errorTitle, errorString);
        return false;
    }
    return true;
}

bool MainWindow::tryOpen(QFile &file,
                         QIODevice::OpenMode flags,
                         const QString &errorTitle)
{
    if (!file.open(flags)) {
        QString errorText;
        if (flags.testFlag(QIODevice::ReadOnly)) {
            errorText = tr("Cannot read file %1.");
        } else if (flags.testFlag(QIODevice::WriteOnly)) {
            errorText = tr("Cannot write file %1.");
        } else {
            errorText = tr("Cannot open file %1.");
        }
        QMessageBox::warning(isHidden() ? NULL : this, errorTitle, errorText
                             .arg(locale().quoteString(strippedName(file.fileName())))
                             + "\n\n" + file.errorString());
        return false;
    }
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    mIsUntitled = fileName.isEmpty();

    const QString unnamedString = tr("unnamed");
    if (mIsUntitled) {
        mCurrentFileBaseName = unnamedString;
        mCurrentPath = QString();
        mCurrentFileName = mCurrentFileBaseName + kMainExt;
        mCurrentFilePath = mCurrentFileName;
    } else {
        QFileInfo info(fileName);
        mCurrentFileBaseName = info.completeBaseName();
        mCurrentPath = info.canonicalPath();
        mCurrentFileName = info.fileName();
        mCurrentFilePath = info.canonicalFilePath();

        RecentFilesMenu::prependFile(fileName);
    }

    setWindowFilePath(mIsUntitled ? mCurrentFileBaseName : mCurrentFileName);
    mUndoStack->setClean();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    int key = event->key();
    if (key == Qt::Key_Escape) {
        mToolsPanel->checkedAction()->trigger();
        return;
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (mDialogToExecOnShow) {
        mDialogToExecOnShow->setModal(true);
        /* show, а не exec, ибо иначе может быть падение (особенно, в valgrind).
         * И по таймеру, чтобы мы успели показаться. Можно и по нулевому, но
         * через небольшой промежуток красивее. */
        QTimer::singleShot(500, mDialogToExecOnShow, SLOT(show()));
        mDialogToExecOnShow = NULL;
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    QEvent::Type t = event->type();
    if (watched != mView && watched != mView->viewport()) {
        if (t == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();
            // Все энтеры передаём в сцену
            if (key == Qt::Key_Enter || key == Qt::Key_Return) {
                QCoreApplication::sendEvent(mScene, keyEvent);
                return true;
            }
            // А ескейпы забираем себе
            if (key == Qt::Key_Escape) {
                keyPressEvent(keyEvent);
                return true;
            }
        }
    } else {
        // Перетаскивания также забираем себе
        if (t == QEvent::DragEnter || t == QEvent::DragMove
                || t == QEvent::DragLeave || t == QEvent::Drop) {
            return true;
        }
    }
    // А остальное -- как обычно
    return QMainWindow::eventFilter(watched, event);
}

MainWindow::~MainWindow()
{
    // Некоторые команды undo в деструкторе используют сцену, модель грунтов
    // и т.п., так что эти команды надо удалить до прочих объектов.
    delete mUndoView;
    delete mUndoStack;
}

QList<BoundaryCondition *> MainWindow::boundaryConditions() const
{
    return mControlPanel->boundaryConditionsPanel()->boundaryConditionsSorted();
}

void MainWindow::onComputationStateChanged(bool on)
{
    mComputationsAreActive = on;

    if (on) {
        QApplication::setOverrideCursor(Qt::BusyCursor);
        // Первое изменение в undo stack произойдёт позже, так что заранее
        // отметить, что произведены изменения
        setWindowModified(true);
    } else {
        QApplication::restoreOverrideCursor();
        // Могли ткнуть отмену, пока оно не приступило к расчётам
        setWindowNotModified(mUndoStack->isClean());
    }

    mUndoView->setDisabled(on);
    mRemoveUnneededBlocksAct->setDisabled(on);

    updateUndoActs();
}

void MainWindow::updateUndoActs()
{
    bool canUndo = !mComputationsAreActive && mUndoStack->canUndo();
    bool canRedo = !mComputationsAreActive && mUndoStack->canRedo();
    mUndoAct->setEnabled(canUndo);
    mRedoAct->setEnabled(canRedo);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    foreach(QUrl url, event->mimeData()->urls()) {
        open(url.toLocalFile());
    }
    QWidget::dropEvent(event);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    QWidget::dragEnterEvent(event);

    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void MainWindow::switchFullScreen()
{
    setWindowState(windowState() ^ Qt::WindowFullScreen);
    updateFullScreenAction();

}

void MainWindow::updateFullScreenAction()
{
    if (isFullScreen()) {
        mFullScreenAct->setIcon(QIcon::fromTheme("view-restore"));
        if (mFullScreenAct->icon().isNull()) {
            mFullScreenAct->setIcon(QIcon::fromTheme("view-fullscreen"));
        }
        mFullScreenAct->setText(tr("Exit &Full Screen Mode"));
        mFullScreenAct->setChecked(true);
    } else {
        mFullScreenAct->setIcon(QIcon::fromTheme("view-fullscreen"));
        mFullScreenAct->setText(tr("&Full Screen Mode"));
        mFullScreenAct->setChecked(false);
    }
}

QString MainWindow::settingsMenuText() const
{
    return tr("&Settings").replace('&', "");
}

void MainWindow::startSoilFillApply(const Soil *soil)
{
    Q_ASSERT(soil);
    if (!mScene->isFillingSoil()) {
        mToolsPanel->slotBlockTools(true);
        QApplication::setOverrideCursor(Qt::CrossCursor);
    }
    setPermanentStatusText(tr("Filling with soil %1...")
                           .arg(locale().quoteString(soil->name())));
    mScene->startSoilFillApply(soil);
}

void MainWindow::stopSoilFillApply()
{
    mToolsPanel->slotBlockTools(false);
    QApplication::restoreOverrideCursor();
    setPermanentStatusText("");
}

void MainWindow::setPermanentStatusText(const QString &text)
{
    if (text.isEmpty()) {
        mPermanentStatusText->clear();
        mPermanentStatusText->hide();
    } else {
        mPermanentStatusText->setText(text);
        mPermanentStatusText->show();
    }
}
