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

#ifndef QFGUI_SCENE_H
#define QFGUI_SCENE_H

#include <QtWidgets/QGraphicsScene>
#include <QtCore/QMap>
#include <QtCore/QPointer>

#include <qfrost.h>

QT_FORWARD_DECLARE_CLASS(QTimer)
QT_FORWARD_DECLARE_CLASS(QUndoCommand)
QT_FORWARD_DECLARE_CLASS(QTextStream)

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(BoundaryCondition)
QT_FORWARD_DECLARE_CLASS(Anchor)
QT_FORWARD_DECLARE_CLASS(MainWindow)
QT_FORWARD_DECLARE_CLASS(ComputationThread)
QT_FORWARD_DECLARE_CLASS(ComputationData)
QT_FORWARD_DECLARE_CLASS(Block)
QT_FORWARD_DECLARE_CLASS(BoundaryPolygon)
QT_FORWARD_DECLARE_CLASS(Vertex)
QT_FORWARD_DECLARE_CLASS(BoundaryPolyline)
QT_FORWARD_DECLARE_CLASS(Soil)
QT_FORWARD_DECLARE_CLASS(Tool)
QT_FORWARD_DECLARE_CLASS(View)
QT_FORWARD_DECLARE_CLASS(ToolSettings)
QT_FORWARD_DECLARE_CLASS(PointOnBoundaryPolygon)
QT_FORWARD_DECLARE_CLASS(ComputationSettings)

class Scene: public QGraphicsScene
{
    Q_OBJECT
public:
    Scene(MainWindow *parent);

    /**
     * Этот деструктор необходим для нормального завершения программы.
     * Деструктор QGraphicsScene делает clear(), что делает delete для всех
     * итемов в сцене. Но так как на итемы ссылаются команды Undo, нельзя
     * это позволить, ведь иначе при выходе из программы, если в сцене
     * имеются итемы, произойдёт segmentation fault.
     */
    //~Scene();

    /// Константный указатель на view
    const View *qfView() const;

    /// Указатель на главное окно
    MainWindow *mainWindow() const;

    /// Указатель на view (нужен для инструментов, дабы перемещать viewport).
    View *qfView();

    Anchor *anchor() {
        return mAnchor;
    }

    /**
     * Добавляет в сцену блоки с прямоугольниками @p blockRects (через Undo).
     * @warning в массива @p blockRects блоки должны идти слева направо, сверху
     *          вниз. То есть первый элемент первого массива -- верхний левый,
     *          первый элемент последнего массива -- нижний левый,
     *          последний элемент последнего массива -- нижний правый.
     */
    void addBlocks(const QList<QList<QRectF> > &blocksRects,
                   bool mustChangeOuterPolygons);

    /**
     * Прибавить @p polygon к полигонам граничных условий сцены (через Undo).
     * Действие делается через систему Undo.
     * @p text - название для команды Undo.
     */
    void uniteBoundaryPolygon(const QPolygonF &polygon, const QString &text);

    /**
     * Вычесть @p polygon из полигонов граничных условий сцены (через Undo).
     * @p text - название для команды Undo.
     */
    void subtractBoundaryPolygon(const QPolygonF &polygon, const QString &text);

    /// Список всех выделенных блоков сцены.
    QList<Block *> selectedBlocks() const;

    /// Список всех блоков в сцене
    const QList<Block *> &blocks() const;
    const QList<const Block *> &blocksConst() const;

    /**
     * Блок в данной точке.
     * @param scenePoint точка сцены, в которой ведётся поиск.
     * @return первый блок в @a pos
     */
    Block *block(const QPointF &pos) const;

    /**
     * Блоки в данном прямоугольнике (перегруженная функция).
     * @param scenePoint точка сцены, в которой ведётся поиск.
     * @param mode способ поиска (по умолчанию Qt::IntersectsItemBoundingRect)
     * @return список блоков в @a rect
     */
    QList<Block *> blocks(const QRectF &rect,
                          Qt::ItemSelectionMode mode = Qt::IntersectsItemBoundingRect) const;

    QList<Block *> blocks(const QPainterPath &path,
                          Qt::ItemSelectionMode mode = Qt::IntersectsItemBoundingRect) const;

    /**
     * Окаймляющий все существующие в сцене блоки прямоугольник.
     * Аналог QGraphicsScene::itemsBoundingRect().
     */
    QRectF blocksBoundingRect() const;

    inline QFrost::BlockStyle blocksStyle() const {
        return mBlocksStyle;
    }

    /**
     * Все полигоны граничных условий сцены.
     * @return список всех полигонов граничных условий в сцене
     */
    QList<BoundaryPolygon *> boundaryPolygons() const;

    /**
     * Полигоны граничных условий, пересекающиеся с данным прямоугольником (!)по
     * границе (перегруженная функция).
     * @param rect прямоугольник в координатах сцены, для которого ведётся поиск.
     * @return список полигонов граничных условий, пересекающихся с @a rect
     */
    QList<BoundaryPolygon *> boundaryPolygons(const QRectF &rect) const;

    /**
     * Полигоны граничных условий, пересекающиеся с данным полигоном
     * (перегруженная функция).
     *
     * @param polygon полигон в координатах сцены, для которого ведётся поиск.
     *
     * @param intersectionType тип пересечения:
     *            allVariants -- результатом пересечения может быть точка,
     *                             отрезок, фигура или их комбинация
     *            excludingOnlyPoints -- результатом пересечения не может быть
     *                               множество изолированных точек
     *            excludingBorder -- результатом пересечения должен включать
     *                               хотя бы одну фигуру.
     */
    QList<BoundaryPolygon *> boundaryPolygons(const QPolygonF &polygon,
            const QFrost::IntersectionType &intersectionType
            = QFrost::allVariants) const;

    Qt::Orientations toolChangesOrientations();

    bool blocksNeedPen() const {
        return mBlocksNeedPen;
    }

    const QList<Block *> &blocksInDomain() const {
        return mBlocksInDomain;
    }

    /// Сохраняет блоки и граничные полигоны в @p out
    void save(QDataStream &out) const;
    
    /**
     * Загружает блоки и граничные полигоны из @p in,
     * используя грунты @p soils и граничные условия @p boundaryConditions
     */
    void load(QDataStream &in,
              const QList<const Soil *> &soils,
              const QList<const BoundaryCondition *> &boundaryConditions);

    /** 
     * Записывает в @p out данные в формате x z t, с сортировкой по x и z
     * Формат:
     *  x y t v
     *  x1 z1 t1 v1
     *  x2 z2 t2 v2
     * (V в процентах)
     */
    void exportData(QTextStream &out,
                    int tDecimals = 1,
                    int vDecimals = 0) const;

    /**
     * Записывает в @p out данные для построения изолиний.
     *
     * Формат:
     *  Regular Grid / Irregular grid                             [вид разбивки]
     *  x y t v                         [координаты центров блоков, T и V в них]
     *  x1 z1 t1 v1
     *  x2 z2 t2 v2
     *  ...
     *  Outer Polygons                                        [внешние полигоны]
     *  x1 y1
     *  x2 y2
     *  ...
     *  Outer Polygons Start Points  [номера точек, начинающих внешние полигоны]
     *  N1
     *  N2
     *  ...
     *  Inner Polygons                                                   [дырки]
     *  x1 y1
     *  x2 y2
     *  ...
     *  Inner Polygons Start Points             [номера точек, начинающих дырки]
     *  N1
     *  N2
     *  ...
     *  Hull                [внешние мнимые точки и T и V соседних к ним блоков]
     *  x1 y1 t1 v1
     *  X2 y2 t2 v2
     *  ...
     * 
     * Информация по t и v идёт с сортировкой по x и z. Все полигоны замкнутые.
     * Направление полигонов (по часовой стрелке или против) не определено.
     * 
     * Hull - это информация, необходимая для того, чтобы достроить построение
     * от середин крайних блоков до краёв расчётной области: все внешние углы
     * и середины сторон блоков + t и v этих блоков.
     * 
     * FIXME одномерные задачи сохранять в другом формате. М.б. и сеточные тоже.
     */
    void exportDataForPlot(QTextStream &out) const;

    /**
     * Являются ли представленные данные сеточными.
     * Данные называется сеточными, если у каждого блока с каждой стороны
     * имеется не более одного контакта с другими блоками и соседствующие
     * блоки имеют соответствующие равные измерения (ширину/высоту).
     */
    bool isGridded() const {
        return mIsGridded;
    }

    /**
     * Являются ли представленные данные одномерными.
     * Данные называется одномерными, если все блоки не имеют контактов с
     * блоками слева и справа (а только сверху и снизу).
     */
    bool is1D() const {
        return mIs1d;
    }

    /// Находится ли в сцене одна рассчётная область.
    bool isSingle() const {
        /* Подразумевается, что рассчётная область одна, если имеется
         * единственный внешний граничный полигон. При этом не
         * проверяется принадлежность всех существующих блоков к этому
         * полигону, то есть если у пользователя имеется островок блоков
         * вне полигона, то область всё равно будет считаться одной,
         * а он сам себе злобный буратино*/
        return mOuterBoundaryPolygons.size() == 1;
    }

    /**
     * Удаляет блоки @p removed и добавляет @p added.
     * @p warning этот метод -- для Undo.
     */
    void replaceBlocks(const QList<Block *> &removed,
                       const QList<Block *> &added);

public slots:
    void slotSetBlocksStyle(QFrost::BlockStyle);

    void slotApplyTemperatureToSelection(double t);
    void slotApplyTemperatureGradientToSelection(double t1, double t2);
    void slotApplyThawedPartToSelection(double v);
    void slotApplySoilToSelection(const Soil *soil);
    void slotStartComputation(const ComputationSettings &settings);
    void slotSetNeedBlocksRedrawing(bool needRedrawing);
    void slotStopComputation();

    void slotSelectionChanged();

    /// Слот, приуроченный к не успешному завершению расчётов
    void slotComputationFinished(const qfgui::ComputationData &data);
    /// Слот, приуроченный к успешному завершению расчётов
    void slotComputationFinished(const QString &errorText);
    /// Слот, приуроченный к очередному получению данных из рассчётов
    void slotUpdateFromComputationData(const qfgui::ComputationData &data);

    /** Извещает поток рассчётов о том, что мы хотим новые данные
     * (отщёлкал таймер).
     */
    void slotWantNewData() const;

    /** Извещает поток рассчётов о том, что мы можем принять новые данные
     * (отрисовали предыдущие).
     */
    void slotReadyForRedraw() const;

    /**
     * Меняет инструмент с удалением старого.
     */
    void setTool(QFrost::ToolType toolType);

    Tool *activeTool() const;

    void setToolsSettingsMap(QMap< QFrost::ToolType, ToolSettings * > map) {
        mToolsSettings = map;
    }

    /// Делает углы у @p polygon равными @p corners через систему Undo.
    void setBoundaryConditions(BoundaryPolygon *polygon, QList<Vertex> corners);

    /// Удаляет (через undo) блоки, которые не попадут в рассчётную область.
    void removeUnneededBlocks();

    /// Запоминает блоки @p blocks как блоки, участвующие в рассчётах
    void setBlocksInDomain(const QList<Block *> blocks) {
        mBlocksInDomain = blocks;
    }

    /// Все внешние граничные полигоны
    const QList<BoundaryPolygon *> &outerBoundaryPolygons() const {
        return mOuterBoundaryPolygons;
    }

    /**
     * Обновляет зависимые от температуры и незамёрзшей воды заливки в блоках.
     * Вызывать при изменении ColorGenerator.
     */
    void updateBlocksBrushes();

private slots:
    /// Обновляет mIsGridded, mIs1D, mBlocks и mBlocksConst и испускает сигналы
    /// об изменении кол-ва блоков, одномерности и сеточности.
    void updateBlocksInfo();

signals:
    void signalBlocksSelectionChanged(bool selectionIsEmpty);

    void signalComputationStateChanged(bool computationIsNowOn);

    /**
     * Сигнал о том, что позиция курсора в главном view изменилась.
     * @param newPosition новая позиция.
     */
    void mainViewMouseMoved(const QPointF &newPosition);

    /**
     * Сигнал о том, что масштаб в главном view изменился.
     * @param newScale новый масштаб.
     */
    void mainViewScaleChanged(qreal newScale);

    /**
     * Сигнал о том, что поймано нажатие (для инструментов).
     * @param acnhorPos позиция анкора при этом нажатии.
     */
    void mousePressed(const QPointF &anchorPos);

    /**
     * Сигнал о том, что поймано нажатие (для инструментов).
     * @param acnhorPos позиция анкора при этом нажатии.
     */
    void mousePressed(const PointOnBoundaryPolygon &anchorPos);

    /// Изменена дата в потоке рассчётов (нужно для отображения прогресса)
    void computationDateChanged(const QDate &date);

    /// Изменена одномерность блоков
    void oneDimensionalityChanged(bool is_1d);

    /// Изменена сеточность блоков
    void griddityChanged(bool isGridded);

    /// Изменено количество блоков здесь
    void blocksCountChanged(int blocksNum);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    QFrost::BlockStyle mBlocksStyle;

    ComputationThread *mComputationThread;

    Anchor *mAnchor;

    QPointer<Tool> mTool;

    /// Нужна ли обводка для блоков
    bool mBlocksNeedPen;

    /// Тип инструмента, который будет создан по левому клику мыши.
    QFrost::ToolType mToolToCreate;

    QMap<QFrost::ToolType, ToolSettings *>  mToolsSettings;

    void addUndoCommand(QUndoCommand *c);

    /// Внешние граничные полигоны в сцене.
    QList<BoundaryPolygon *> mOuterBoundaryPolygons;

    /// Контейнеры -- пустые итемы, которые нужны для балансировки bsp-деревьев
    QList<QGraphicsItem *> mContainers;

    /**
     * Список блоков, которые задействованы в рассчётной области
     * @warning Не во время рассчётов не пользоваться.
     */
    QList <Block *> mBlocksInDomain;

    /// Минимальный интервал обновления сцены во время рассчётов
    static const int kmUpdateInterval = 300;

    /// Удаляет пустые (не содержащие детей) контейнеры
    void fixContainers();

    /// Добавляет в сцену @p boundaryPolygons.
    void addItems(const QList<BoundaryPolygon *> &boundaryPolygons);
    /// Добавляет в сцену @p blocks.
    void addItems(const QList<Block *> &blocks);

    /// Удаляет из сцены @p boundaryPolygons.
    void removeItems(const QList<BoundaryPolygon *> &boundaryPolygons);
    /// Удаляет из сцены @p blocks.
    void removeItems(const QList<Block *> &blocks);

    /// Это надо совершать после любого завершения рассчётов (успешного или нет)
    void onComputationFinished();

    /// @see isGridded()
    bool mIsGridded;
    /// @see is1D()
    bool mIs1d;

    /// Все грунтовые блоки, находящиеся здесь
    QList<Block *> mBlocks;
    QList<const Block *> mBlocksConst;

    /// Надо ли нам обновить mBlocks и mBlocksConst при следующем запросе
    /// одного из этих массивов
    QTimer *mUpdateBlocksTimer;

    friend class ChangeBoundaryPolygonsCommand;
    friend class ReadFromComputationDataCommand;
};

}

#endif // QFGUI_SCENE_H
