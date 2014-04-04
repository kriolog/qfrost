/*
 * Copyright (C) 2010-2014  Denis Pesotsky, Maxim Torgonsky
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

#include <scene.h>

#include <cmath>

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QUndoStack>
#include <QtCore/QTimer>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>
#include <QtCore/QPair>

#include <block.h>
#include <blockportable.h>
#include <boundarypolygonportable.h>
#include <boundarypolygon.h>
#include <boundarypolygoncalc.h>
#include <view.h>
#include <soils/soil.h>
#include <computations/computationthread.h>
#include <computations/blockslogger.h>
#include <mainwindow.h>

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

#include <geometry/block_within_polygon.h>
#include <geometry/clip_polyline.h>

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


void Scene::slotApplySoilToSelection(const Soil *soil)
{
    addUndoCommand(new SetBlocksSoilCommand(selectedBlocks(), soil));
}

void Scene::slotApplyThawedPartToSelection(double v)
{
    addUndoCommand(SetBlocksThawedPartCommand::createCommand(selectedBlocks(), v));
}

void Scene::slotStartComputation(const ComputationSettings &settings)
{
    Q_ASSERT(mComputationThread == NULL);
    Q_ASSERT(mBlocksInDomain.isEmpty());

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
    emit signalBlocksSelectionChanged(selectedItems().isEmpty());
}

void Scene::slotSetBlocksStyle(QFrost::BlockStyle style)
{
    if (mBlocksStyle != style) {
        mBlocksStyle = style;

        mBlocksNeedPen = (mBlocksStyle != QFrost::blockShowsTemperatureField
                          && mBlocksStyle != QFrost::blockShowsThawedPartField
                          && mBlocksStyle != QFrost::blockShowsConditionField);

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

Qt::Orientations Scene::toolChangesOrientations()
{
    if (mTool.isNull()) {
        return 0;
    } else {
        return mTool.data()->changesOrientations();
    }
}

#include <QDebug>
void Scene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
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
        case QFrost::boundaryEllipseCreator:
            mTool = new BoundaryEllipseCreator(mToolsSettings[mToolToCreate]);
            justCreatedTool = true;
            break;
        case QFrost::boundaryPolygonCreator:
        case QFrost::boundaryConditionsCreator:
        case QFrost::polygonalSelection:
            break;
        }
        if (justCreatedTool) {
            mTool.data()->setPos(anchor()->pos());
            addItem(mTool.data());
            // После этого обязательно запустить основной обработчик нажатий!
        }
        mToolToCreate = QFrost::noTool;
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
        const bool firstBlockLeftSideIsAtLeftHalfPlane = (blocks().first()->rect().left() < 0);

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

void Scene::exportData(QTextStream &out,
                       int tDecimals,
                       int vDecimals) const
{
    QList<const Block *> blocks = blocksConst();
    if (blocks.isEmpty()) {
        return;
    }
    qSort(blocks.begin(), blocks.end(), xzLessThan);

    out << "x" << "\t"
        << "y" << "\t"
        << "t" << "\t"
        << "v"
        << "\n";
    foreach(const Block * block, blocks) {
        QPointF center = QFrost::meters(block->rect().center());
        out << center.x() << "\t"
            << center.y() << "\t"
            << QString::number(block->soilBlock()->temperature(), 'f', tDecimals) << "\t"
            << QString::number(block->soilBlock()->thawedPart() * 100, 'f', vDecimals)  << "\t"
            << "\n";
    }
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

static void printPoint(double x, double y,
                       QTextStream &out,
                       const QString &delim = "\t",
                       bool newLine = true)
{
    printPoint(QPointF(x, y), out, delim, newLine);
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
    exportData(out);
    
    const QList<BoundaryPolygon *> &outerPolygons = outerBoundaryPolygons();

    out << "Outer Polygons\n";
    foreach(const BoundaryPolygon * outerPolygon, outerPolygons) {
        foreach(const Vertex &corner, outerPolygon->corners()) {
            printPoint(QFrost::meters(corner.point), out);
        }
    }
    
    bool hasInnerPolygons = false;
    out << "Inner Polygons\n";
    foreach(const BoundaryPolygon * outerPolygon, outerPolygons) {
        foreach (const BoundaryPolygon * innerPolygon, outerPolygon->childBoundaryPolygonItems()) {
            hasInnerPolygons = true;
            foreach(const Vertex &corner, innerPolygon->corners()) {
                printPoint(QFrost::meters(corner.point), out);
            }
        }
    }
    
    out << "Bounds\n";
    foreach(const Block * block, blocks()) {
        // По контактам определим, какие из сторон блока находятся на границе
        enum Side {
            Top = 0x1,
            Bottom = 0x2,
            Left = 0x4,
            Right = 0x8
        };
        Q_DECLARE_FLAGS(Sides, Side)
        
        Sides edgeSides;
        if (block->contactsLeft().isEmpty()) {
            edgeSides |= Left;
        }
        if (block->contactsRight().isEmpty()) {
            edgeSides |= Right;
        }
        if (block->contactsTop().isEmpty()) {
            edgeSides |= Top;
        }
        if (block->contactsBottom().isEmpty()) {
            edgeSides |= Bottom;
        }

        if (edgeSides == 0) {
            // Блок со всех сторон имеет соседей - он нас не интересует
            continue;
        } else {
            // Блок на границе (пока не знаем, на внешней или на внутренней)
            if (blockIntersectsPolygonSide(block, outerPolygons)) {
                // Блок находится на границе, выведем ключевые внешние точки
                if (edgeSides.testFlag(Top)) {
                    printPoint(block->metersCenter().x(), 
                               block->metersRect().top(),
                               out);
                }
                if (edgeSides.testFlag(Bottom)) {
                    printPoint(block->metersCenter().x(), 
                               block->metersRect().bottom(),
                               out);
                }
                if (edgeSides.testFlag(Left)) {
                    printPoint(block->metersRect().left(),
                               block->metersCenter().y(),
                               out);
                }
                if (edgeSides.testFlag(Right)) {
                    printPoint(block->metersRect().right(),
                               block->metersCenter().y(),
                               out);
                }

                if (edgeSides.testFlag(Top) && edgeSides.testFlag(Left)) {
                    printPoint(block->metersRect().topLeft(), out);
                }
                if (edgeSides.testFlag(Bottom) && edgeSides.testFlag(Left)) {
                    printPoint(block->metersRect().bottomLeft(), out);
                }
                if (edgeSides.testFlag(Top) && edgeSides.testFlag(Right)) {
                    printPoint(block->metersRect().topRight(), out);
                }
                if (edgeSides.testFlag(Bottom) && edgeSides.testFlag(Right)) {
                    printPoint(block->metersRect().bottomRight(), out);
                }
            }
        }
    }
}
