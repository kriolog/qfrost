/*
 * Copyright (C) 2010-2015  Denis Pesotsky, Maxim Torgonsky
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

#include "scene.h"

#include <cmath>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QUndoStack>
#include <QtCore/QTimer>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>
#include <QtCore/QPair>
#include <QGraphicsPixmapItem>

#include <graphicsviews/block.h>
#include <blockportable.h>
#include <boundarypolygonportable.h>
#include <graphicsviews/boundarypolygon.h>
#include <boundarypolygoncalc.h>
#include <graphicsviews/view.h>
#include <soils/soil.h>
#include <computations/computationthread.h>
#include <computations/blockslogger.h>
#include <mainwindow.h>
#include <plot/curveplotdialog.h>

#include <undo/addblockscommand.h>
#include <undo/removeblockscommand.h>
#include <undo/setblockstemperaturecommand.h>
#include <undo/setblocksthawedpartcommand.h>
#include <undo/setblockssoilcommand.h>
#include <undo/changeboundarypolygonscommand.h>
#include <undo/setboundaryconditionscommand.h>
#include <undo/readfromcomputationdatacommand.h>

#include <tools/anchor.h>
#include <tools/tool.h>
#include <tools/boundarypolygoncreator.h>
#include <tools/boundaryellipsecreator.h>
#include <tools/boundaryconditionsapplicator.h>
#include <tools/blockcreator.h>
#include <tools/polygonalselection.h>
#include <tools/rectangularselection.h>
#include <tools/ellipseselection.h>

#include <tools_panel/curveplottoolsettings.h>

#include <geometry/block_within_polygon.h>
#include <geometry/clip_polyline.h>
#include <units/units.h>

using namespace qfgui;

Scene::Scene(MainWindow *parent)
    : QGraphicsScene(parent)
    , mBlocksStyle(QFrost::blockShowsSoil)
    , mComputationThread(NULL)
    , mAnchor(new Anchor)
    , mTool()
    , mBlocksNeedPen(true)
    , mToolToCreate(QFrost::noTool)
    , mToolsSettings()
    , mOuterBoundaryPolygons()
    , mContainers()
    , mBlocksInDomain()
    , mIsGridded(true)
    , mIs1d(true)
    , mBlocks()
    , mBlocksConst()
    , mUpdateBlocksTimer(new QTimer(this))
    , mIsFillingSoil(false)
    , mSoilToFill(NULL)
    , mBackgroundItem(NULL)
    , mCurvePlotDialogSpawner(NULL)
{
    //setItemIndexMethod(NoIndex);
    int sceneHalfsize = QFrost::sceneHalfSize * 1.01;
    setSceneRect(QRect(-sceneHalfsize, -sceneHalfsize,
                       2 * sceneHalfsize, 2 * sceneHalfsize));
    // возможно, есть более элегантное решение? (может, уже есть ф-ция, которая
    //  вызывается при каждой смене выделения?..)
    connect(this, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));

    addItem(mAnchor);

    mUpdateBlocksTimer->setSingleShot(300);
    connect(mUpdateBlocksTimer, SIGNAL(timeout()), SLOT(updateBlocksInfo()));
}

const View *Scene::qfView() const
{
    return static_cast<const View *>(this->views().first());
}

MainWindow *Scene::mainWindow() const
{
    MainWindow *m = qobject_cast<MainWindow *>(parent());
    Q_ASSERT(m != NULL);
    return m;
}

View *Scene::qfView()
{
    return static_cast<View *>(this->views().first());
}

void Scene::addUndoCommand(QUndoCommand *c)
{
    if (c == NULL) {
        return;
    }
    MainWindow *m;
    m = qobject_cast<MainWindow *>(parent());
    Q_ASSERT(m != NULL);
    m->undoStack()->push(c);
}

void Scene::addBlocks(const QList<QList<QRectF> > &blocksRects,
                      bool mustChangeOuterPolygons)
{
    addUndoCommand(new AddBlocksCommand(this, blocksRects, mustChangeOuterPolygons));
}

QList<Block *> Scene::selectedBlocks() const
{
    QList<Block *> blocks;
    foreach(QGraphicsItem * item, selectedItems()) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block != NULL) {
            blocks.append(block);
        }
    }
    return blocks;
}

Block *Scene::block(const QPointF &pos) const
{
    Block *block;
    foreach(QGraphicsItem * item, items(pos, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder)) {
        block = qgraphicsitem_cast<Block *>(item);
        if (block != NULL) {
            return block;
        }
    }
    return NULL;
}

QList<Block *> Scene::blocks(const QRectF &rect, Qt::ItemSelectionMode mode) const
{
    QList<Block *> blocks;
    foreach(QGraphicsItem * item, items(rect, mode)) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block != NULL) {
            blocks.append(block);
        }
    }
    return blocks;
}

QList<Block *> Scene::blocks(const QPainterPath &path, Qt::ItemSelectionMode mode) const
{
    QList<Block *> blocks;
    foreach(QGraphicsItem * item, items(path, mode)) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block != NULL) {
            blocks.append(block);
        }
    }
    return blocks;
}

QList<BoundaryPolygon * > Scene::boundaryPolygons() const
{
    QList<BoundaryPolygon * > result;
    result = mOuterBoundaryPolygons;
    foreach(BoundaryPolygon * outerPolygon, mOuterBoundaryPolygons) {
        result.append(outerPolygon->childBoundaryPolygonItems());
    }
    return result;
}

QList<BoundaryPolygon *> Scene::boundaryPolygons(const QRectF &rect) const
{
    QList<BoundaryPolygon *> result;
    foreach(BoundaryPolygon * boundaryPolygon, boundaryPolygons()) {
        if (boundaryPolygon->intersects(rect)) {
            result.append(boundaryPolygon);
        }
    }
    return result;
}

QList< BoundaryPolygon *> Scene::boundaryPolygons(const QPolygonF &polygon,
        const QFrost::IntersectionType &intersectionType) const
{
    QList<BoundaryPolygon *> result;
    foreach(BoundaryPolygon * boundaryPolygon, boundaryPolygons()) {
        switch (intersectionType) {
        case QFrost::allVariants:
            if (boundaryPolygon->intersects(polygon)) {
                result.append(boundaryPolygon);
            }
            break;
        case QFrost::excludingOnlyPoints:
            if (boundaryPolygon->intersectsExcludingOnlyPoints(polygon)) {
                result.append(boundaryPolygon);
            }
            break;
        case QFrost::excludingBorder:
            if (boundaryPolygon->intersectsExcludingBorder(polygon)) {
                result.append(boundaryPolygon);
            }
            break;
        }
    }
    return result;
}

void Scene::slotApplyTemperatureToSelection(double t)
{
    addUndoCommand(SetBlocksTemperatureCommand::createCommand(selectedBlocks(), t));
}

void Scene::slotApplyTemperatureGradientToSelection(double t1, double t2)
{
    Q_ASSERT(!mTool.isNull());
    if (t1 == t2) {
        slotApplyTemperatureToSelection(t1);
        return;
    }
    const Tool *tool = mTool.data();
    QRectF toolRect = tool->boundingRect().translated(tool->pos());
    addUndoCommand(SetBlocksTemperatureCommand::createCommand(selectedBlocks(),
                   t1, t2,
                   toolRect.top(),
                   toolRect.bottom()));
}


void Scene::slotApplySoilToSelection(const Soil *soil, bool onlyClearBlocks)
{
    QList<Block *> blocks = selectedBlocks();
    if (blocks.isEmpty()) {
        qWarning("%s called with no blocks selected!", Q_FUNC_INFO);
        return;
    }
    if (onlyClearBlocks) {
        // FIXME: можно использовать QtConcurrent::blockingFilter
        QList<Block *>::Iterator it = blocks.begin();
        while (it != blocks.end()) {
            if ((*it)->hasSoil()) {
                it = blocks.erase(it);
            } else {
                ++it;
            }
        }
        if (blocks.isEmpty()) {
            qWarning("%s for clear blocks called with no clear blocks selected!",
                     Q_FUNC_INFO);
            return;
        }
    }
    addUndoCommand(new SetBlocksSoilCommand(blocks, soil));
    // теперь все выделенные блоки имеют грунт, оповестим об этом
    emit signalBlocksSelectionChanged(false, true);
}

void Scene::slotApplyThawedPartToSelection(double v)
{
    addUndoCommand(SetBlocksThawedPartCommand::createCommand(selectedBlocks(), v));
}

void Scene::slotStartComputation(const ComputationSettings &settings)
{
    Q_ASSERT(mComputationThread == NULL);
    Q_ASSERT(mBlocksInDomain.isEmpty());
    
    if (mIsFillingSoil) {
        stopSoilFillApply();
    }

    emit signalComputationStateChanged(true);
    views().first()->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    qDebug("creating thread");
    mComputationThread = new ComputationThread(this, settings);
    qDebug("creating thread done");
    qRegisterMetaType<ComputationData>();
    qRegisterMetaType<BlocksLogger>();
    connect(mComputationThread,
            SIGNAL(signalNewDataIsReady(qfgui::ComputationData)),
            SLOT(slotUpdateFromComputationData(qfgui::ComputationData)));
    connect(mComputationThread,
            SIGNAL(finished(qfgui::ComputationData)),
            SLOT(slotComputationFinished(qfgui::ComputationData)));
    connect(mComputationThread,
            SIGNAL(badStart(QString)),
            SLOT(slotComputationFinished(QString)));
    connect(mComputationThread,
            SIGNAL(dateChanged(QDate)), SIGNAL(computationDateChanged(QDate)));
    connect(mComputationThread, SIGNAL(loggerDataIsReady(qfgui::BlocksLogger)),
            mainWindow(), SLOT(onLoggerDataAvailable(qfgui::BlocksLogger)));

    // первые данные оттуда мы будем ожидать как минимум через ...
    QTimer::singleShot(kmUpdateInterval, this, SLOT(slotWantNewData()));
    qDebug("starting computations");
    mComputationThread->start();
    mComputationThread->setPriority(QThread::HighPriority);
}

void Scene::slotStopComputation()
{
    if (mComputationThread != NULL) {
        mComputationThread->stop();
    }
}

void Scene::slotUpdateFromComputationData(const ComputationData &data)
{
    Q_ASSERT(!mBlocksInDomain.isEmpty());
    Q_ASSERT(mBlocksInDomain.size() == data.numOfBlocks());
    qDebug("slotUpdateFromComputationData: *** start ***");
    // следующие данные мы будет ожидать как минимум через...
    QTimer::singleShot(kmUpdateInterval, this, SLOT(slotWantNewData()));
    addUndoCommand(new ReadFromComputationDataCommand(this, data));
    qDebug("slotUpdateFromComputationData: blocks readed data");
    /* HACK?: не знаю, возможно 2й аргумент можно было сделать и не 0, но
     *        QEvent::Paint точно не подошёл -- она не перерисовывается сразу
     *        после такого (а надо, чтобы перерисовывалась)... */
    QCoreApplication::sendPostedEvents(this, 0);
    qDebug("slotUpdateFromComputationData: processed events (e.g. redrawed)");
    slotReadyForRedraw();
    qDebug("slotUpdateFromComputationData: *** updated ***");
}

void Scene::slotComputationFinished(const ComputationData &data)
{
    Q_ASSERT(!mBlocksInDomain.isEmpty());
    Q_ASSERT(mBlocksInDomain.size() == data.numOfBlocks());
    qDebug("slotComputationFinished: *** start ***");
    addUndoCommand(new ReadFromComputationDataCommand(this, data, true));
    qDebug("slotComputationFinished: blocks readed data");
    mComputationThread->onSceneReadyForRedraw();
    onComputationFinished();
}

void Scene::slotComputationFinished(const QString &errorText)
{
    mComputationThread->onSceneReadyForRedraw();
    onComputationFinished();
    if (!errorText.isEmpty()) {
        QMessageBox::warning(qfView(),
                             tr("Computations Start Unsuccessful"),
                             errorText);
    }
}

void Scene::onComputationFinished()
{
    // Дадим потоку немного времени на случай, если он не успел завершиться
    QTimer::singleShot(5000, mComputationThread, SLOT(deleteLater()));
    mComputationThread = NULL;
    emit signalComputationStateChanged(false);
    views().first()->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    mBlocksInDomain.clear();
}

void Scene::slotReadyForRedraw() const
{
    mComputationThread->onSceneReadyForRedraw();
}

void Scene::slotSetNeedBlocksRedrawing(bool needRedrawing)
{
    if (mComputationThread == NULL) {
        return;
    }
    mComputationThread->setNeedBlocksRedrawing(needRedrawing);
}

void Scene::slotWantNewData() const
{
    // Возможно, потока уже нет, поэтому нужна проверка.
    if (mComputationThread != NULL) {
        mComputationThread->onSceneWantsNewData();
    }
}

QRectF Scene::blocksBoundingRect() const
{
    QRectF blockBoundingRect;
    foreach(Block * block, blocks()) {
        blockBoundingRect = blockBoundingRect.united(block->rect());
    }
    return blockBoundingRect;
}

void Scene::slotSelectionChanged()
{
    const bool hasSelection = !selectedItems().isEmpty();

    emit signalBlocksSelectionChanged(!hasSelection);
    emit signalBlocksSelectionChanged(!hasSelection, hasSelection
                                                     ? !hasClearBlocksSelected()
                                                     : true);
}

void Scene::startSoilFillApply(const Soil *soil)
{
    mSoilToFill = soil;
    mIsFillingSoil = true;
}

void Scene::stopSoilFillApply(QPointF scenePoint)
{
    Q_ASSERT(mIsFillingSoil);
    if (scenePoint != QFrost::noPoint) {
        Q_ASSERT(mSoilToFill != NULL);
        Block *firstBlock = block(scenePoint);
        if (firstBlock && firstBlock->soil() != mSoilToFill) {
            const Soil *soil = firstBlock->soil();
            clearSelection();
            QSet<Block *> blocksToProcess;
            blocksToProcess << firstBlock;
            // FIXME: алгоритм медленный. Оптимизировать (хотя бы для сеток).
            while (!blocksToProcess.isEmpty()) {
                QSet<Block *> newBlocksToProcess;
                foreach(Block *block, blocksToProcess) {
                    if (block->isSelected()) {
                        continue;
                    }
                    foreach (const BlockContact &contact, block->contactsLeft()) {
                        Block *newBlock = contact.block();
                        if (!newBlock->isSelected() && newBlock->soil() == soil) {
                            newBlocksToProcess << newBlock;
                        }
                    }
                    foreach (const BlockContact &contact, block->contactsRight()) {
                        Block *newBlock = contact.block();
                        if (!newBlock->isSelected() && newBlock->soil() == soil) {
                            newBlocksToProcess << newBlock;
                        }
                    }
                    foreach (const BlockContact &contact, block->contactsTop()) {
                        Block *newBlock = contact.block();
                        if (!newBlock->isSelected() && newBlock->soil() == soil) {
                            newBlocksToProcess << newBlock;
                        }
                    }
                    foreach (const BlockContact &contact, block->contactsBottom()) {
                        Block *newBlock = contact.block();
                        if (!newBlock->isSelected() && newBlock->soil() == soil) {
                            newBlocksToProcess << newBlock;
                        }
                    }
                    block->setSelected(true);
                }
                blocksToProcess = newBlocksToProcess;
            }
            QList<Block *> blocksToFill = selectedBlocks();
            if (!blocksToFill.isEmpty()) {
                clearSelection();
                addUndoCommand(new SetBlocksSoilCommand(blocksToFill, mSoilToFill));
            }
        }
    }
    mIsFillingSoil = false;
    emit soilFillApplyDone();
}

void Scene::slotSetBlocksStyle(QFrost::BlockStyle style)
{
    if (mBlocksStyle != style) {
        mBlocksStyle = style;

        mBlocksNeedPen = (mBlocksStyle != QFrost::blockShowsTemperatureField
                          && mBlocksStyle != QFrost::blockShowsThawedPartField
                          && mBlocksStyle != QFrost::blockShowsConditionField
                          && mBlocksStyle != QFrost::blockShowsTemperatureDiffField);

        foreach(Block * block, blocks()) {
            block->updateBrush();
        }

        // перерисовываемся, ибо updateBrush это не делает
        update();
    }
}

void Scene::updateBlocksBrushes()
{
    foreach(Block * block, blocks()) {
        block->updateFromTemperature();
        block->updateFromThawedPart();
    }
}

void Scene::setBackground(const QPixmap &pixmap, const QTransform &transform)
{
    const bool hadBackgroundItem = mBackgroundItem;
    if (!hadBackgroundItem) {
        mBackgroundItem = new QGraphicsPixmapItem(pixmap);
        addItem(mBackgroundItem);
        mBackgroundItem->setZValue(QFrost::BackgroundZValue);
        mBackgroundItem->setOpacity(0.3);
    } else {
        mBackgroundItem->setPixmap(pixmap);
    }

    mBackgroundItem->setTransform(transform);

    emit backgroundChanged(true);
    if (!hadBackgroundItem) {
        emit backgroundAdded();
    } else {
        setBackgroundVisible(true); // оно само проверит, видима ли она уже
    }
}

void Scene::removeBackground()
{
    if (!mBackgroundItem) {
        return;
    }
    removeItem(mBackgroundItem);
    delete mBackgroundItem;
    mBackgroundItem = NULL;
    
    emit backgroundChanged(false);
    emit backgroundRemoved();
}

void Scene::setBackgroundVisible(bool visible)
{
    Q_ASSERT(mBackgroundItem);
    if (mBackgroundItem->isVisible() == visible) {
        return;
    }
    mBackgroundItem->setVisible(visible);
    emit backgroundVisibilityChanged(visible);
}

void Scene::openCurvePlotDialog()
{
    Q_ASSERT(mToolToCreate == QFrost::curvePlot);

    if (mAnchor->pos() == QFrost::noPoint) {
        return;
    }

    Block *const selectedBlock = block(mAnchor->pos());
    if (!selectedBlock) {
        qWarning("%s called with anchor outside of block!", Q_FUNC_INFO);
        return;
    }

    CurvePlotToolSettings *settings = qobject_cast<CurvePlotToolSettings*>(mToolsSettings[mToolToCreate]);
    Q_ASSERT(settings);

    if (!mCurvePlotDialogSpawner) {
        mCurvePlotDialogSpawner = new CurvePlotDialogSpawner(qfView());
    }

    mCurvePlotDialogSpawner->execDialog(selectedBlock, settings->orientation());
}

Qt::Orientations Scene::toolChangesOrientations()
{
    if (mTool.isNull()) {
        return 0;
    } else {
        return mTool.data()->changesOrientations();
    }
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (mIsFillingSoil) {
            stopSoilFillApply(event->scenePos());
            return;
        }
        if (!mAnchor->posOnPolygon().isNull()) {
            emit mousePressed(mAnchor->posOnPolygon());
        } else if (mAnchor->pos() != QFrost::noPoint) {
            emit mousePressed(mAnchor->pos());
        } else {
            Block *b = block(event->scenePos());
            if (b != NULL) {
                b->showArrows(); // TMP
            }
            // если анкора нема, делать нечего
            return;
        }
        bool justCreatedTool = false;
        switch (mToolToCreate) {
        case QFrost::noTool:
            break;
        case QFrost::blockCreator:
            mTool = new BlockCreator(mToolsSettings[mToolToCreate]);
            justCreatedTool = true;
            break;
        case QFrost::rectangularSelection:
            mTool = new RectangularSelection(mToolsSettings[mToolToCreate]);
            justCreatedTool = true;
            break;
        case QFrost::ellipseSelection:
            mTool = new EllipseSelection(mToolsSettings[mToolToCreate]);
            justCreatedTool = true;
            break;
        case QFrost::boundaryEllipseCreator:
            mTool = new BoundaryEllipseCreator(mToolsSettings[mToolToCreate]);
            justCreatedTool = true;
            break;
        case QFrost::curvePlot:
            openCurvePlotDialog();
            break;
        case QFrost::boundaryPolygonCreator:
        case QFrost::boundaryConditionsCreator:
        case QFrost::polygonalSelection:
            break;
        default:
            Q_ASSERT(false);
        }
        if (justCreatedTool) {
            mTool.data()->setPos(anchor()->pos());
            addItem(mTool.data());
            // После этого обязательно запустить основной обработчик нажатий!
        }
        if (mToolToCreate != QFrost::curvePlot) {
            mToolToCreate = QFrost::noTool;
        }
    }
    // HACK: временно врубаем у view ScrollHandDrag, тогда не будет deselect'а
    QGraphicsView::DragMode oldDragMode = qfView()->dragMode();
    qfView()->setDragMode(QGraphicsView::ScrollHandDrag);
    QGraphicsScene::mousePressEvent(event);
    qfView()->setDragMode(oldDragMode);
}

void Scene::keyPressEvent(QKeyEvent *event)
{
    if (!mTool.isNull() && mTool.data()->changesOrientations() == 0) {
        int key = event->key();
        if (key == Qt::Key_Enter || key == Qt::Key_Return) {
            mTool.data()->apply(event->modifiers() != Qt::NoModifier);
            event->accept();
            return;
        } else if (key == Qt::Key_Backspace) {
            mTool.data()->cancelLastChange();
        } else if (mIsFillingSoil && key == Qt::Key_Escape) {
            stopSoilFillApply();
        }
    }
    QGraphicsScene::keyPressEvent(event);
}

void Scene::removeUnneededBlocks()
{
    QList<Block *> blocks = this->blocks();

    QList<Block *>::Iterator blockIt;
    QList<BoundaryPolygon *>::Iterator polygonIt;
    for (polygonIt = mOuterBoundaryPolygons.begin();
            polygonIt != mOuterBoundaryPolygons.end(); ++polygonIt) {
        for (blockIt = blocks.begin(); blockIt != blocks.end(); ++blockIt) {
            if (BlockWithinPolygon::blockIntersectsPolygon(*blockIt, *polygonIt)) {
                /* если блок попал в рассчётную область, удалять не надо,
                 * как и проверять на пересечение с другими полигонами */
                blockIt = blocks.erase(blockIt) - 1;
            }
        }
    }
    if (!blocks.isEmpty()) {
        addUndoCommand(new RemoveBlocksCommand(this, blocks));
    }
}

void Scene::setTool(QFrost::ToolType toolType)
{
    if (!mTool.isNull()) {
        // удаляем инструмент со сцены и планируем его удаление (как объекта)
        removeItem(mTool.data());
        mTool.data()->deleteLater();
        mTool.clear();
    }
    switch (toolType) {
    case QFrost::noTool:
    case QFrost::blockCreator:
    case QFrost::rectangularSelection:
    case QFrost::boundaryEllipseCreator:
    case QFrost::ellipseSelection:
        break;
    case QFrost::boundaryPolygonCreator:
        mTool = new BoundaryPolygonCreator;
        addItem(mTool.data());
        break;
    case QFrost::boundaryConditionsCreator:
        mTool = new BoundaryConditionsApplicator;
        addItem(mTool.data());
        break;
    case QFrost::polygonalSelection:
        mTool = new PolygonalSelection;
        addItem(mTool.data());
        break;
    case QFrost::curvePlot:
        break;
    default:
        Q_ASSERT(false);
    }

    mToolToCreate = toolType;

    mAnchor->setTool(toolType);
}

Tool *Scene::activeTool() const
{
    return mTool.data();
}

void Scene::addItems(const QList<BoundaryPolygon *> &boundaryPolygons)
{
    if (boundaryPolygons.isEmpty()) {
        return;
    }
    foreach(BoundaryPolygon * boundaryPolygon, boundaryPolygons) {
        Q_ASSERT(boundaryPolygon->parentItem() == NULL);
        addItem(boundaryPolygon);
        mOuterBoundaryPolygons.append(boundaryPolygon);
    }
}

void Scene::addItems(const QList< Block *> &blocks)
{
    if (blocks.isEmpty()) {
        return;
    }
    QList<QGraphicsItem *> addedContainers;
    QGraphicsItem *container = NULL;
    int i = 0;
    foreach(QGraphicsItem * block, blocks) {
        if (i-- == 0) {
            container = new QGraphicsRectItem(0, 0, 0, 0);
            addedContainers.append(container);
            i = 1000;
        }
        block->setParentItem(container);
    }
    foreach(QGraphicsItem * item, addedContainers) {
        mContainers.append(item);
        addItem(item);
    }

    mUpdateBlocksTimer->start();
}

void Scene::removeItems(const QList<BoundaryPolygon *> &boundaryPolygons)
{
    if (boundaryPolygons.isEmpty()) {
        return;
    }
    foreach(BoundaryPolygon * boundaryPolygon, boundaryPolygons) {
        Q_ASSERT(mOuterBoundaryPolygons.count(boundaryPolygon) == 1);
        removeItem(boundaryPolygon);
        mOuterBoundaryPolygons.removeOne(boundaryPolygon);
    }
}

void Scene::removeItems(const QList<Block *> &blocks)
{
    if (blocks.isEmpty()) {
        return;
    }
    foreach(QGraphicsItem * block, blocks) {
        removeItem(block);
    }
    fixContainers();

    mUpdateBlocksTimer->start();
}

void Scene::updateBlocksInfo()
{
    mBlocks.clear();
    mBlocksConst.clear();
    foreach(QGraphicsItem * item, items()) {
        Block *block = qgraphicsitem_cast<Block *>(item);
        if (block != NULL) {
            mBlocks.append(block);
            mBlocksConst.append(block);
        }
    }
    bool is_1d = true;
    bool isGridded = true;

    if (!mBlocks.isEmpty()) {
        foreach(Block * b, mBlocks) {
            if (is_1d) {
                if (!b->contactsLeft().isEmpty() || !b->contactsRight().isEmpty()) {
                    is_1d = false;
                }
            }
            if (isGridded) {
                isGridded = b->isGridPart();
            }
        }
    }

    if (mIs1d != is_1d) {
        mIs1d = is_1d;
        emit oneDimensionalityChanged(mIs1d);
    }

    if (mIsGridded != isGridded) {
        mIsGridded = isGridded;
        emit griddityChanged(mIsGridded);
    }

    emit blocksCountChanged(mBlocks.size());
}

void Scene::replaceBlocks(const QList<Block *> &removed,
                          const QList<Block *> &added)
{
    removeItems(removed);
    addItems(added);
}

const QList< Block *> &Scene::blocks() const
{
    if (mUpdateBlocksTimer->isActive()) {
        const_cast<Scene *>(this)->mUpdateBlocksTimer->stop();
        const_cast<Scene *>(this)->updateBlocksInfo();
    }
    return mBlocks;
}

const QList< const Block * > &Scene::blocksConst() const
{
    if (mUpdateBlocksTimer->isActive()) {
        const_cast<Scene *>(this)->mUpdateBlocksTimer->stop();
        const_cast<Scene *>(this)->updateBlocksInfo();
    }
    return mBlocksConst;
}

void Scene::fixContainers()
{
    foreach(QGraphicsItem * item, mContainers) {
        item->childItems();
        if (item->childItems().isEmpty()) {
            removeItem(item);
            delete item;
            Q_ASSERT(mContainers.count(item) == 1);
            mContainers.removeOne(item);
        }
    }
}

void Scene::uniteBoundaryPolygon(const QPolygonF &polygon, const QString &text)
{
    BoundaryPolygonCalc calc(this);
    QPair<BoundaryPolygonList, BoundaryPolygonList> diff;
    diff = calc.uniteOperation(polygon);
    if (!diff.first.isEmpty() || !diff.second.isEmpty()) {
        addUndoCommand(new ChangeBoundaryPolygonsCommand(this, diff, text));
    }
}

void Scene::subtractBoundaryPolygon(const QPolygonF &polygon, const QString &text)
{
    BoundaryPolygonCalc calc(this);
    QPair<BoundaryPolygonList, BoundaryPolygonList> diff;
    diff = calc.subtractOperation(polygon);
    if (!diff.first.isEmpty() || !diff.second.isEmpty()) {
        addUndoCommand(new ChangeBoundaryPolygonsCommand(this, diff, text));
    }
}

void Scene::setBoundaryConditions(BoundaryPolygon *polygon, QList<Vertex> corners)
{
    addUndoCommand(new SetBoundaryConditionsCommand(polygon, corners));
}

void Scene::load(QDataStream &in,
                 const QList<const Soil *> &soils,
                 const QList<const BoundaryCondition *> &boundaryConditions)
{
    Q_ASSERT(blocks().isEmpty());
    Q_ASSERT(boundaryPolygons().isEmpty());
    Q_ASSERT(in.status() == QDataStream::Ok);
    /**************************** Загружаем блоки *****************************/
    {
        QList<BlockPortable> blocksPortable;
        // пытаемся загрузить блоки
        try {
            in >> blocksPortable;
        } catch (...) {
            // если не вышло, ругаемся и кидаем исключение
            qWarning("Load failed: cannot read blocks!");
            throw false;
        }

        // создаём блоки в памяти (и нумеруем их)
        QList<Block *> blocks;
        for (int i = 0; i < blocksPortable.size(); ++i) {
            Block *b = blocksPortable[i].createBlock(soils,
                       mainWindow()->colorGenerator());
            blocks << b;
            b->setNum(i);
        }
        // заполняем у них массивы соседей
        for (int i = 0; i < blocksPortable.size(); ++i) {
            blocksPortable[i].fillBlockNeighbors(blocks);
        }
        // и добавляем все блоки в сцену
        replaceBlocks(QList<Block *>(), blocks);
    }
    /**************************************************************************/
    Q_ASSERT(in.status() == QDataStream::Ok);
    /*************** Загружаем полигоны граничных условий *********************/
    {
        QList<BoundaryPolygonPortable> boundaryPolygonsPortable;
        // пытаемся загрузить граничные полигоны
        try {
            in >> boundaryPolygonsPortable;
        } catch (...) {
            // если не вышло, ругаемся и кидаем исключение
            qWarning("Load failed: cannot read boundary polygons!");
            throw false;
        }
        QList<BoundaryPolygon *> outerBoundaryPolygons;
        // создаём полигоны в памяти
        foreach(BoundaryPolygonPortable bp, boundaryPolygonsPortable) {
            outerBoundaryPolygons << bp.createPolygon(boundaryConditions);
        }
        // и добавляем все полигоны в сцену
        addItems(outerBoundaryPolygons);
    }
    /**************************************************************************/
    Q_ASSERT(in.status() == QDataStream::Ok);
}

bool Scene::hasClearBlocksSelected() const
{
    foreach (QGraphicsItem *item, selectedItems()) {
        Q_ASSERT(qgraphicsitem_cast<Block *>(item) != NULL);
        if (!static_cast<Block *>(item)->hasSoil()) {
            return true;
        }
    }
    return false;
}

void Scene::save(QDataStream &out) const
{
    /**************************** Сохраняем блоки *****************************/
    {
        // будем сохранять все блоки
        QList<Block *> blocks = this->blocks();
        // нумеруем их (чтобы они потом могли записать соседей по номерам)
        for (int i = 0; i < blocks.size(); ++i) {
            blocks[i]->setNum(i);
        }
        // создаём и заполняем массив переносных версий блоков
        QList<BlockPortable> blocksPortable;
        foreach(Block * block, blocks) {
            blocksPortable << BlockPortable(block);
        }
        // записывем этот массив
        out << blocksPortable;
    }
    /**************************************************************************/
    /*************** Сохраняем полигоны граничных условий *********************/
    {
        // создаём и заполняем массив полигонов граничных условий
        QList<BoundaryPolygonPortable> boundaryPolygonsPortable;
        foreach(BoundaryPolygon * outerPolygon, mOuterBoundaryPolygons) {
            boundaryPolygonsPortable << BoundaryPolygonPortable(outerPolygon);
        }
        // записываем этот массив
        out << boundaryPolygonsPortable;
    }
    /**************************************************************************/
}

void Scene::exportData(QTextStream &out) const
{
    QList<const Block *> blocks = blocksConst();
    if (blocks.isEmpty()) {
        return;
    }
    qSort(blocks.begin(), blocks.end(), xzLessThan);

    out << "#X\tZ\tT\tVth\tTbf\n";

    const int temperatureDecimals = Units::decimals(Temperature);
    foreach (const Block *block, blocks) {
        const QPointF &center = block->metersCenter();
        out << center.x() << "\t"
            << center.y() << "\t"
            << QString::number(block->soilBlock()->temperature(),
                               'f', temperatureDecimals) << "\t"
            << qRound(block->soilBlock()->thawedPart() * 100.0) << "\t"
            << QString::number(block->soilBlock()->transitionTemperature(),
                               'f', temperatureDecimals) << "\n";
    }
}

static void printPoint(const QPointF &p, double t, double v, QTextStream &out,
                       const QString &delim = "\t",
                       bool newLine = true)
{
    out << p.x() << delim << p.y() << delim << t << delim << v;
    if (newLine) {
        out << "\n";
    }
}

static void printPoint(double x, double y, double t, double v,
                       QTextStream &out,
                       const QString &delim = "\t",
                       bool newLine = true)
{
    printPoint(QPointF(x, y), t, v, out, delim, newLine);
}

static void printPoint(const QPointF &p, QTextStream &out, 
                       const QString &delim = "\t",
                       bool newLine = true)
{
    out << p.x() << delim << p.y();
    if (newLine) {
        out << "\n";
    }
}

static bool blockIntersectsPolygonSide(const Block *block,
                                       const BoundaryPolygon *polygon)
{
    // FIXME использовать BoundaryPolygon::simplified (когда будет реализована)
    QPolygonF p;
    foreach(const Vertex &corner, polygon->corners()) {
            p << corner.point;
    }

    return ClipPolyline::clips(block, p);
}

static bool blockIntersectsPolygonSide(const Block *block,
                                       const QList<BoundaryPolygon *> &polygons)
{
    foreach(const BoundaryPolygon * polygon, polygons) {
        if (blockIntersectsPolygonSide(block, polygon)) {
            return true;
        }
    }
    return false;
}

void Scene::exportDataForPlot(QTextStream &out) const
{
    out << (isGridded() ? "Regular Grid" : "Irregular Grid") << "\n";

    QList<const Block *> blocks = blocksConst();
    qSort(blocks.begin(), blocks.end(), xzLessThan);
    out << "x" << "\t" << "y" << "\t" << "t" << "\t" << "v" << "\n";
    foreach(const Block * block, blocks) {
        printPoint(block->metersCenter(),
                   block->soilBlock()->temperature(),
                   block->soilBlock()->thawedPart(),
                   out);
    }

    const QList<BoundaryPolygon *> &outerPolygons = outerBoundaryPolygons();

    QList<int> startPoints;

    out << "Outer Polygons\n";
    int n = 0;
    foreach(const BoundaryPolygon * outerPolygon, outerPolygons) {
        startPoints << n;
        foreach(const Vertex &corner, outerPolygon->corners()) {
            printPoint(QFrost::meters(corner.point), out);
            ++n;
        }
    }

    out << "Outer Polygons Start Points\n";
    foreach(int n, startPoints) {
        out << n << "\n";
    }

    out << "Inner Polygons\n";
    n = 0;
    startPoints.clear();
    foreach(const BoundaryPolygon * outerPolygon, outerPolygons) {
        foreach (const BoundaryPolygon * innerPolygon, outerPolygon->childBoundaryPolygonItems()) {
            startPoints << n;
            foreach(const Vertex &corner, innerPolygon->corners()) {
                printPoint(QFrost::meters(corner.point), out);
                ++n;
            }
        }
    }

    out << "Inner Polygons Start Points\n";
    foreach(int n, startPoints) {
        out << n << "\n";
    }

    out << "Hull\n";
    foreach(const Block * block, blocks) {
        // По контактам определим, какие из сторон блока находятся на границе        
        Qt::Edges edgeSides;
        if (block->contactsLeft().isEmpty()) {
            edgeSides |= Qt::LeftEdge;
        }
        if (block->contactsRight().isEmpty()) {
            edgeSides |= Qt::RightEdge;
        }
        if (block->contactsTop().isEmpty()) {
            edgeSides |= Qt::TopEdge;
        }
        if (block->contactsBottom().isEmpty()) {
            edgeSides |= Qt::BottomEdge;
        }

        if (edgeSides == 0) {
            // Блок со всех сторон имеет соседей - он нас не интересует
            continue;
        } else {
            // Блок на границе (пока не знаем, на внешней или на внутренней)
            if (blockIntersectsPolygonSide(block, outerPolygons)) {
                // Блок находится на границе, выведем ключевые внешние точки
                const double t = block->soilBlock()->temperature();
                const double v = block->soilBlock()->thawedPart();
                
                if (edgeSides.testFlag(Qt::TopEdge)) {
                    printPoint(block->metersCenter().x(), 
                               block->metersRect().top(),
                               t, v, out);
                }
                if (edgeSides.testFlag(Qt::BottomEdge)) {
                    printPoint(block->metersCenter().x(), 
                               block->metersRect().bottom(),
                               t, v, out);
                }
                if (edgeSides.testFlag(Qt::LeftEdge)) {
                    printPoint(block->metersRect().left(),
                               block->metersCenter().y(),
                               t, v, out);
                }
                if (edgeSides.testFlag(Qt::RightEdge)) {
                    printPoint(block->metersRect().right(),
                               block->metersCenter().y(),
                               t, v, out);
                }

                if (edgeSides.testFlag(Qt::TopEdge) && edgeSides.testFlag(Qt::LeftEdge)) {
                    printPoint(block->metersRect().topLeft(), t, v, out);
                }
                if (edgeSides.testFlag(Qt::BottomEdge) && edgeSides.testFlag(Qt::LeftEdge)) {
                    printPoint(block->metersRect().bottomLeft(), t, v, out);
                }
                if (edgeSides.testFlag(Qt::TopEdge) && edgeSides.testFlag(Qt::RightEdge)) {
                    printPoint(block->metersRect().topRight(), t, v, out);
                }
                if (edgeSides.testFlag(Qt::BottomEdge) && edgeSides.testFlag(Qt::RightEdge)) {
                    printPoint(block->metersRect().bottomRight(), t, v, out);
                }
            }
        }
    }
}
