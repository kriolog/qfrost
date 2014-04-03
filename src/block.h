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

#ifndef QFGUI_BLOCK_H
#define QFGUI_BLOCK_H

#include <QtWidgets/QGraphicsItem>
#include <QtGui/QBrush>
#include <boost/math/constants/constants.hpp>

#include <scene.h>
#include <soils/soil.h>
#include <core/soilblock.h>
#include <qfrost.h>

namespace qfcore
{
QT_FORWARD_DECLARE_CLASS(SoilBlock)
QT_FORWARD_DECLARE_CLASS(Domain)
}

namespace qfgui
{

QT_FORWARD_DECLARE_CLASS(Soil)
QT_FORWARD_DECLARE_CLASS(Scene)
QT_FORWARD_DECLARE_CLASS(BlockData)
QT_FORWARD_DECLARE_CLASS(ColorGenerator)
QT_FORWARD_DECLARE_CLASS(Block)

class BlockContact
{
public:
    enum Side {
        Top,
        Bottom,
        Left,
        Right
    };

    /// Конструктор площадки между @p block1 и @p block2 (для @p block1).
    /// @param side где относительно @p block1 находится @p block2
    inline BlockContact(Block *block1,
                        Block *block2,
                        Side side);

    inline BlockContact(Block *block, double square, double r)
        : mBlock(block)
        , mSquare(square)
        , mRadialSquare(square * 2.0 * boost::math::constants::pi<double>() *r)
        , mR(r) {
        Q_ASSERT(square > 0);
        Q_ASSERT(r >= 0);
    }

    inline BlockContact createPairContact(Block *block) {
        BlockContact result = *this;
        result.mBlock = block;
        return result;
    }

    /// Блок, с которым идёт контакт
    inline Block *block() const {
        return mBlock;
    }

    /// Площадь контакта (для единичной ширины или же для обёрнутой вокруг Y)
    inline double square(bool isAxiallySymmetric = false) const {
        return isAxiallySymmetric ? mRadialSquare : mSquare;
    }

    /// Расстояние от оси Y до центра контакта
    inline double r() const {
        return mR;
    }

private:
    Block *mBlock;
    double mSquare;
    double mRadialSquare;
    double mR;
};

class Block: public QGraphicsItem
{
public:
    Block(const QRectF &inRect, const ColorGenerator *colorGenerator,
          QGraphicsItem *parent = NULL);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

    QRectF boundingRect() const {
        return mZeroRect;
    }

    /// Прямоугольник блока в координатах сцены
    const QRectF &rect() const {
        return mSceneRect;
    }

    /// Прямоугольник блока в координатах сцены, переведёный в метры
    const QRectF &metersRect() const {
        return mMetersRect;
    }

    /// Координаты центра блока в координатах сцены, переведёные в метры
    const QPointF &metersCenter() const {
        return mMetersCenter;
    }

    DEFINETYPE(BLOCKTYPE)

    bool isReady() const {
        return mSoil != NULL;
    }

    /**
     * Обновить mBrush. Вызывать при:
     * - смене стиля отображения сцены;
     * - смене температуры;
     * @warning Не делает update()
     */
    inline void updateBrush();

    /**
     * Создание в рассчётной области площадок теплопотока, связывающих этот
     * блок с соседними (если у них @a mIsInDomain == true).
     * Смотрит слева и сверху.
     */
    void createTopAndLeftHeatSurfaces(qfcore::Domain *domain,
                                      bool isAxiallySymmetric) const;

    bool isInDomain() const {
        return mIsInDomain;
    }

    /**
     * Создаёт в рассчётной области @p domain грунтовый блок.
     * Также придаёт @a mIsInDomain true и запоминает наш номер в ней.
     */
    void moveDataToDomain(qfcore::Domain *domain, bool isAxiallySymmetric);

    /**
     * Придаёт @a mIsInDomain false.
     * То есть говорит блоку, что он теперь не в рассчётной области.
     */
    void unsetInDomainFlag() {
        mIsInDomain = false;
    }

    /// Константный указатель на грунтовый блок, ассоциированный с нами.
    const qfcore::SoilBlock *soilBlock() const {
        return &mSoilBlock;
    }

    /// Создаёт стрелочки, указывающие на соседей блока.
    void showArrows();

    /// Порядковый номер грунтового блока в рассчётной области.
    std::size_t numInDomain() const {
        return mSoilBlockInDomainNum;
    }

    /// Порядковый номер блока (нужно для загрузки/сохранения).
    int num() const {
        return mNum;
    }

    /// Устанавливает порядковый номер блока (для загрузки/сохранения).
    void setNum(int num) {
        mNum = num;
    }

    /// Блоки, примыкающие к этому блоку сверху.
    inline const QList<BlockContact> contactsTop() const {
        return mContactsTop;
    }
    /// Блоки, примыкающие к этому блоку снизу.
    inline const QList<BlockContact> contactsBottom() const {
        return mContactsBottom;
    }
    /// Блоки, примыкающие к этому блоку слева.
    inline const QList<BlockContact> contactsLeft() const {
        return mContactsLeft;
    }
    /// Блоки, примыкающие к этому блоку справа.
    inline const QList<BlockContact> contactsRight() const {
        return mContactsRight;
    }

    /**
     * Является ли блок частью сетки.
     * Блок является частью сетки, если с каждой стороны граничит не более чем
     * с одним блоком и площадь контакта равна высоте/ширине.
     */
    inline bool isGridPart() const;

    const Block *blockAtBottom() const {
        Q_ASSERT(mContactsBottom.size() <= 1);
        return mContactsBottom.isEmpty() ? NULL : mContactsBottom.first().block();
    }

    const Block *blockAtRight() const {
        Q_ASSERT(mContactsRight.size() <= 1);
        return mContactsRight.isEmpty() ? NULL : mContactsRight.first().block();
    }

    /// Обновляет заливки, зависящие от температуры и от кол-ва незамёрзшей воды
    void updateFromTemperature();

    /// Обновляет заливки, зависящие от кол-ва незамёрзшей воды
    void updateFromThawedPart();

    /**
     * Увеличивает счётик кол-во ссылающихся на этот блок undo-команд.
     * Этот счётчик нужен для того, чтобы блок сам удалялся, если на него
     * больше не ссылается ни одной undo-команды, и при этом он не находится в
     * сцене. @sa decreaseUndoReferenceCount */
    inline void increaseUndoReferenceCount() {
        ++mUndoReferenceCount;
    }

    /**
     * Уменьшает счётик кол-ва ссылающихся на этот блок undo-команд и удаляет
     * этот блок, если счётчик стал равен нулю и при этом мы не находимся в
     * сцене. @sa increaseUndoReferenceCount */
    inline void decreaseUndoReferenceCount() {
        Q_ASSERT(mUndoReferenceCount > 0);
        --mUndoReferenceCount;
        if (mUndoReferenceCount == 0 && scene() == NULL) {
            delete this;
        }
    }

protected:
    /**
     * Тут мы отслеживаем смену сцены, чтобы обновляться исходя из scale
     */
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    /**
     * Размер блока.
     */
    const QSizeF mSize;

    /**
     * Прямоугольник с позицией (0,0) и размером, равным @a mSize.
     */
    const QRectF mZeroRect;
    
    /**
     * Коэффициент масштабирования для вмещения в нас текста
     */
    const qreal mTextScaleFactor;
    
    /**
     * Отмасштабированный с учётом @a mTextScaleFactor @a mZeroRect
     */
    const QRectF mScaledZeroRect;

    /**
     * Прямоугольник с позицией pos() и размером, равным @a mSize.
     */
    const QRectF mSceneRect;

    const QRectF mMetersRect;

    const QPointF mMetersCenter;

    /**
     * Грунтовый блок, ассоциированный с этим объектом.
     *
     * В нём для блока важны лишь температура и относительный объём талой фазы.
     * Также в актуальном (соответствующем классу грунта) состоянии должна
     * поддерживаться температура фазового перехода, ведь она важна при попытках
     * изменить количество относительного объёма талой фазы.
     */
    qfcore::SoilBlock mSoilBlock;

    /**
     * Номер грунтового блока, используемый для получения данных из рассчётов.
     * @warning Не во время рассчётов не пользоваться.
     */
    std::size_t mSoilBlockInDomainNum;

    /// Указатель на грунт (элемент SoilModel), к которому принадлежим.
    const Soil *mSoil;

    /// Указатель на генератор цветов (в главном окне)
    const ColorGenerator *mColorGenerator;

    /// Указатель на заливку, используемую при рисовании.
    const QBrush *mBrush;

    /// Заливка, соответствующая температуре.
    QBrush mTemperatureBrush;

    /// Заливка, соответствующая количеству незамёрзшей воды.
    QBrush mThawedPartBrush;

    /// Заливка, соответствующая температуре и количеству незамёрзшей воды.
    QBrush mConditionBrush;

    /// Обновляет заливку, завязанную на температуру и кол-во незамёрзшей воды.
    void updateConditionBrush();

    inline Scene *qfScene() const {
        return static_cast<Scene *>(scene());
    }

    /// Берёт температуру и кол-во незамёрзшей воды из @p data и обновляет цвет.
    void readFromBlockData(const BlockData &data);

    /// Создаёт в @p Domain площадки теплопотока из списка контактов @p сontact
    /// для оси @p axe (0 - x, 1 - y). @param isAxiallySymmetric использовать ли
    /// формулы площади для осесимметричной задачи
    void createHeatSurfaces(qfcore::Domain *domain,
                            const QList<BlockContact> &contacts,
                            short unsigned int axe,
                            bool isAxiallySymmetric) const;

    QList<BlockContact> mContactsTop;
    QList<BlockContact> mContactsBottom;
    QList<BlockContact> mContactsLeft;
    QList<BlockContact> mContactsRight;

    /* Прямоугольники в QFrost::microSizeF от соотв. сторон и имеющие соотв.
     * размер, равный QFrost::microSizeF */
    const QRectF mTopRect;
    const QRectF mBottomRect;
    const QRectF mLeftRect;
    const QRectF mRightRect;

    /// Находится ли блок в рассчётной области.
    /// Актуально только во время счёта!
    bool mIsInDomain;

    /// Порядковый номер блока (нужно для загрузки/сохранения)
    int mNum;

    /// Нарисован ли на этом блоке текст
    bool mHasText;

    /// Кол-во ссылающихся на этот блок undo-команд
    ushort mUndoReferenceCount;

    /// Создаёт у соответствующих ячеек связи с нами
    inline void restoreBottomLinks(const QList<Block *> &blocksNotToTouch = QList<Block *>());
    inline void restoreTopLinks(const QList<Block *> &blocksNotToTouch = QList<Block *>());
    inline void restoreLeftLinks(const QList<Block *> &blocksNotToTouch = QList<Block *>());
    inline void restoreRightLinks(const QList<Block *> &blocksNotToTouch = QList<Block *>());

    /// Удаляет запись о нас из @p list
    inline void destroyLink(QList<BlockContact> &list);

    /// Удаляет у соответствующих ячеек связи с нами
    inline void destroyBottomLinks(const QList<Block *> &blocksNotToTouch = QList<Block *>());
    inline void destroyTopLinks(const QList<Block *> &blocksNotToTouch = QList<Block *>());
    inline void destroyLeftLinks(const QList<Block *> &blocksNotToTouch = QList<Block *>());
    inline void destroyRightLinks(const QList<Block *> &blocksNotToTouch = QList<Block *>());

    friend class AddBlocksCommand;
    friend class BlockWithOldThawedPart;
    friend class BlockWithOldTemperature;
    friend class BlockWithOldSoil;
    friend class BlockWithOldTransitionParameters;
    friend class ReadFromComputationDataCommand;
    friend class SetBlocksSoilCommand;
    friend class RemoveBlocksCommand;
    friend class BlockPortable;
};

void Block::restoreBottomLinks(const QList<Block *> &blocksNotToTouch)
{
    QList<BlockContact>::Iterator it;
    for (it = mContactsBottom.begin(); it != mContactsBottom.end(); ++it) {
        if (!blocksNotToTouch.contains(it->block())) {
            it->block()->mContactsTop.append(it->createPairContact(this));
        }
    }
}

void Block::restoreTopLinks(const QList<Block *> &blocksNotToTouch)
{
    QList<BlockContact>::Iterator it;
    for (it = mContactsTop.begin(); it != mContactsTop.end(); ++it) {
        if (!blocksNotToTouch.contains(it->block())) {
            it->block()->mContactsBottom.append(it->createPairContact(this));
        }
    }
}

void Block::restoreLeftLinks(const QList<Block *> &blocksNotToTouch)
{
    QList<BlockContact>::Iterator it;
    for (it = mContactsLeft.begin(); it != mContactsLeft.end(); ++it) {
        if (!blocksNotToTouch.contains(it->block())) {
            it->block()->mContactsRight.append(it->createPairContact(this));
        }
    }
}

void Block::restoreRightLinks(const QList<Block *> &blocksNotToTouch)
{
    QList<BlockContact>::Iterator it;
    for (it = mContactsRight.begin(); it != mContactsRight.end(); ++it) {
        if (!blocksNotToTouch.contains(it->block())) {
            it->block()->mContactsLeft.append(it->createPairContact(this));
        }
    }
}

void Block::destroyLink(QList<BlockContact> &list)
{
    Q_ASSERT(!list.isEmpty());

    QList<BlockContact>::Iterator it = list.begin();
    while (it->block() != this) {
        ++it;
        Q_ASSERT(it != list.end());
    }
    list.erase(it);
}

void Block::destroyBottomLinks(const QList<Block *> &blocksNotToTouch)
{
    QList<BlockContact>::Iterator it;
    for (it = mContactsBottom.begin(); it != mContactsBottom.end(); ++it) {
        if (!blocksNotToTouch.contains(it->block())) {
            destroyLink(it->block()->mContactsTop);
        }
    }
}

void Block::destroyTopLinks(const QList<Block *> &blocksNotToTouch)
{
    QList<BlockContact>::Iterator it;
    for (it = mContactsTop.begin(); it != mContactsTop.end(); ++it) {
        if (!blocksNotToTouch.contains(it->block())) {
            destroyLink(it->block()->mContactsBottom);
        }
    }
}

void Block::destroyLeftLinks(const QList<Block *> &blocksNotToTouch)
{
    QList<BlockContact>::Iterator it;
    for (it = mContactsLeft.begin(); it != mContactsLeft.end(); ++it) {
        if (!blocksNotToTouch.contains(it->block())) {
            destroyLink(it->block()->mContactsRight);
        }
    }
}

void Block::destroyRightLinks(const QList<Block *> &blocksNotToTouch)
{
    QList<BlockContact>::Iterator it;
    for (it = mContactsRight.begin(); it != mContactsRight.end(); ++it) {
        if (!blocksNotToTouch.contains(it->block())) {
            destroyLink(it->block()->mContactsLeft);
        }
    }
}

void Block::updateBrush()
{
    Q_ASSERT(scene() != NULL);
    static const QBrush whiteBrush = QBrush(Qt::white);
    switch (qfScene()->blocksStyle()) {
    case QFrost::blockShowsSoil:
        mBrush = (mSoil ? mSoil->brush() : &whiteBrush);
        break;
    case QFrost::blockShowsBoundaryConditions:
        mBrush = &whiteBrush;
        break;
    case QFrost::blockShowsTemperature:
    case QFrost::blockShowsTemperatureField:
        mBrush = &mTemperatureBrush;
        break;
    case QFrost::blockShowsThawedPartField:
        mBrush = &mThawedPartBrush;
        break;
    case QFrost::blockShowsConditionField:
        mBrush = &mConditionBrush;
        break;
    default:
        Q_ASSERT(false);
    }
}

bool Block::isGridPart() const
{
    if (!mContactsBottom.isEmpty()) {
        if (mContactsBottom.size() != 1) {
            return false;
        }
        if (mContactsBottom.first().square() != mMetersRect.width()) {
            return false;
        }
    }

    if (!mContactsTop.isEmpty()) {
        if (mContactsTop.size() != 1) {
            return false;
        }
        if (mContactsTop.first().square() != mMetersRect.width()) {
            return false;
        }
    }

    if (!mContactsLeft.isEmpty()) {
        if (mContactsLeft.size() != 1) {
            return false;
        }
        if (mContactsLeft.first().square() != mMetersRect.height()) {
            return false;
        }
    }

    if (!mContactsRight.isEmpty()) {
        if (mContactsRight.size() != 1) {
            return false;
        }
        if (mContactsRight.first().square() != mMetersRect.height()) {
            return false;
        }
    }

    return true;
}

inline static bool xzLessThan(const Block *b1, const Block *b2)
{
    const QPointF &p1 = b1->metersCenter();
    const QPointF &p2 = b2->metersCenter();
    if (p1.x() == p2.x()) {
        return p1.y() < p2.y();
    } else {
        return p1.x() < p2.x();
    }
}

BlockContact::BlockContact(Block *block1,
                           Block *block2,
                           BlockContact::Side side)
    : mBlock(block2)
    , mSquare()
    , mR()
{
    if (side == Top || side == Bottom) {
        double x1 = qMin(block1->metersRect().right(), block2->metersRect().right());
        double x2 = qMax(block1->metersRect().left(), block2->metersRect().left());
        mSquare = x1 - x2;
        mR = qAbs((x1 + x2) / 2.0);
    } else {
        double y1 = qMin(block1->metersRect().bottom(), block2->metersRect().bottom());
        double y2 = qMax(block1->metersRect().top(), block2->metersRect().top());
        mSquare = y1 - y2;
        mR = qAbs((side == Left)
                  ? block1->metersRect().left()
                  : block1->metersRect().right());
    }
    mRadialSquare = mSquare * 2.0 * boost::math::constants::pi<double>() * mR;
    Q_ASSERT(mSquare > 0);
    Q_ASSERT(mR >= 0);
    Q_ASSERT(!qFuzzyIsNull(mSquare));
}

}

#endif // QFGUI_BLOCK_H
