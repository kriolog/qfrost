/*
 * Copyright (C) 2010-2013  Denis Pesotsky, Maxim Torgonsky
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

#include "block.h"

#include <cmath>

#include <QtWidgets/QStyleOptionGraphicsItem>
#include <QtGui/QPainter>

#include <graphicsviews/arrow.h>
#include <graphicsviews/boundarypolygon.h>
#include <computations/computationthread.h>
#include <core/domain.h>
#include <core/heatsurface.h>
#include <qfrost.h>
#include <graphicsviews/scene.h>
#include <soils/soil.h>
#include <graphicsviews/view.h>
#include <graphicsviews/colorgenerator.h>

using namespace qfgui;

Block::Block(const QRectF &inRect,
             const ColorGenerator *colorGenerator,
             QGraphicsItem *parent)
    : QGraphicsItem(parent),
      mSize(inRect.size()),
      mZeroRect(QPointF(0, 0), mSize),
      mTextScaleFactor(qMin(mSize.width(), mSize.height()) / 75.0),
      mScaledZeroRect(QPointF(0, 0), mSize/mTextScaleFactor),
      mSceneRect(inRect),
      mMetersRect(QFrost::meters(mSceneRect)),
      mMetersCenter(mMetersRect.center()),
      mSoilBlock(mMetersRect.width(), mMetersRect.height()),
      mSoilBlockInDomainNum(),
      mSoil(),
      mColorGenerator(colorGenerator),
      mBrush(),
      mTemperatureBrush(mColorGenerator->colorFromTemperature(mSoilBlock)),
      mThawedPartBrush(mColorGenerator->colorFromThawedPart(mSoilBlock)),
      mConditionBrush(),
      mContactsTop(),
      mContactsBottom(),
      mContactsLeft(),
      mContactsRight(),
      mTopRect(mSceneRect.left(), mSceneRect.top() - QFrost::microSizeF,
               mSize.width(), QFrost::microSizeF / 2),
      mBottomRect(mSceneRect.left(), mSceneRect.bottom() + QFrost::microSizeF / 2,
                  mSize.width(), QFrost::microSizeF / 2),
      mLeftRect(mSceneRect.left() - QFrost::microSizeF, mSceneRect.top(),
                QFrost::microSizeF / 2, mSize.height()),
      mRightRect(mSceneRect.right() + QFrost::microSize / 2, mSceneRect.top(),
                 QFrost::microSizeF / 2, mSize.height()),
      mIsInDomain(),
      mNum(),
      mHasText(),
      mUndoReferenceCount(0)
{
    Q_ASSERT(inRect.isValid());
    Q_ASSERT(inRect.width() / 5 > QFrost::microSize);
    Q_ASSERT(inRect.height() / 5 > QFrost::microSize);
    setPos(inRect.topLeft());
    setFlags(ItemIsSelectable);
    setAcceptedMouseButtons(Qt::NoButton);

    updateConditionBrush();

    //setFlag(ItemClipsToShape);
}

QVariant Block::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSceneHasChanged) {
        if (value.value<QGraphicsScene *>() != NULL) {
            // восстанавливаем связь грунта с нами
            if (mSoil != NULL) {
                const_cast<Soil *>(mSoil)->addBlock(this);
            }
            // и обновляем кисть
            updateBrush();
        } else {
            // удаляем связь грунта с нами
            if (mSoil != NULL) {
                const_cast<Soil *>(mSoil)->removeBlock(this);
            }
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

void Block::paint(QPainter *painter,
                  const QStyleOptionGraphicsItem *option,
                  QWidget *widget)
{
    Q_UNUSED(widget)

    // Рассчитываем уровень детализации из уровня детализации сцены и с учётом
    // минимального из размеров блока
    const qreal lod = option->levelOfDetailFromTransform(painter->worldTransform()) *
                      qMin(mSize.width(), mSize.height());

    mHasText = (lod > 40.2);

    if (qfScene()->blocksNeedPen() && lod > 4.20) {
        painter->setPen(QPen(Qt::gray, 0));
    } else {
        painter->setPen(Qt::NoPen);
    }

    painter->setBrush(*mBrush);

    painter->drawRect(mZeroRect);

    if (mHasText) {
        //QFont font;
        //font.setPointSizeF(qMin(mSize.width(), mSize.height()) / 10.0);
        //painter->setFont(font);
        
        if (painter->font().pixelSize() != 10) {
            QFont font;
            font.setPixelSize(10);
            painter->setFont(font);
        }

        QString info;
        info += QString::number(mSoilBlock.temperature(), 'f', 2);
        info += "\302\260C";
        info += "\n" + QString::number(mSoilBlock.thawedPart() * 100.0, 'f', 0) + "%";
        info += "\n (" + QString::number(mMetersCenter.x(), 'f', 2) + "; ";
        info += QString::number(mMetersCenter.y(), 'f', 2) + ")";
        painter->setPen(QPen(Qt::black, 0, Qt::SolidLine));

        painter->scale(mTextScaleFactor, mTextScaleFactor);
        painter->drawText(mScaledZeroRect,
                          Qt::AlignCenter | Qt::TextWordWrap, 
                          info);
        painter->resetTransform();
    }
}

void Block::updateConditionBrush()
{
    Q_ASSERT(mSoilBlock.thawedPartIsOk());
    if (mSoilBlock.isAtPhaseTransition()) {
        mConditionBrush = mThawedPartBrush;
    } else {
        mConditionBrush = mTemperatureBrush;
    }
}

void Block::updateFromTemperature()
{
    mTemperatureBrush = QBrush(mColorGenerator->colorFromTemperature(mSoilBlock));
    updateFromThawedPart();
    if (mBrush == &mTemperatureBrush) {
        update();
    }
}

void Block::updateFromThawedPart()
{
    Q_ASSERT(mSoilBlock.thawedPartIsOk());
    mThawedPartBrush = QBrush(mColorGenerator->colorFromThawedPart(mSoilBlock));
    if (mBrush == &mThawedPartBrush || mBrush == &mConditionBrush) {
        update();
    }
    updateConditionBrush();
}

/// Координата центра @p block на перпендикулярной @p sliceOrientation оси.
double centerCoord(const Block *block, Qt::Orientation sliceOrientation)
{
    const QPointF center = block->rect().center();
    return (sliceOrientation == Qt::Horizontal)
           ? center.y()
           : center.x();
}

QList<Block *> Block::slice(Qt::Orientation orientation)
{
    QList<Block *> halfSliceBefore = halfSlice(orientation, true);
    QList<Block *> halfSliceAfter = halfSlice(orientation, false);

    QList<Block *> thisBlock;
    thisBlock << this;

    return halfSliceBefore + thisBlock + halfSliceAfter;
}

QList<Block *> Block::halfSlice(Qt::Orientation orientation, bool before)
{
    const double cCoord = centerCoord(this, orientation);

    QList<Block *> result;

    Block *prevBlock = this;
    while (prevBlock) {
        const QList<BlockContact> &contacts = orientation == Qt::Horizontal
                                              ? (before
                                                 ? prevBlock->mContactsLeft 
                                                 : prevBlock->mContactsRight)
                                              : (before
                                                 ? prevBlock->mContactsTop
                                                 : prevBlock->mContactsBottom);

        Block *nextBlock = NULL;
        if (contacts.isEmpty()) {
            break;
        } else if (contacts.size() == 1) {
            nextBlock = contacts.first().block();
        } else {
            foreach (const BlockContact &contact, contacts) {
                Block *const b = contact.block();
                if (!nextBlock
                    || qAbs(centerCoord(b, orientation) - cCoord) <
                       qAbs(centerCoord(nextBlock, orientation) - cCoord)) {
                    nextBlock = b;
                }
            }
        }
        if (nextBlock) {
            if (before) {
                result.prepend(nextBlock);
            } else {
                result.append(nextBlock);
            }
        }
        prevBlock = nextBlock;
    }

    return result;
}


void Block::moveDataToDomain(qfcore::Domain *domain, bool isAxiallySymmetric)
{
    mIsInDomain = true;

    Q_ASSERT(mSoilBlock.thawedPartIsOk());
    Q_ASSERT(mSoilBlock.transitionTemperature() == mSoil->block().transitionTemperature());

    mSoilBlock.prepareForComputation(mSoil->block());
    mSoilBlockInDomainNum = isAxiallySymmetric
                            ? domain->addSoilBlock(mSoilBlock, qAbs(mMetersCenter.x()))
                            : domain->addSoilBlock(mSoilBlock);
}

void Block::createHeatSurfaces(qfcore::Domain *domain,
                               const QList< BlockContact > &contacts,
                               short unsigned int axe,
                               bool isAxiallySymmetric) const
{
    for (QList<BlockContact>::ConstIterator it = contacts.constBegin();
            it != contacts.constEnd(); ++it) {
        if (!it->block()->mIsInDomain) {
            // сосед не в рассчётной области? нафиг его.
            continue;
        }

        if (isAxiallySymmetric && qFuzzyIsNull(it->r())) {
            // защита от дурака - осесимметричная задача, в которой блоки
            // контактируют на оси Y, будет работать как 2 различных области
            continue;
        }

        std::size_t block1Num = mSoilBlockInDomainNum;
        std::size_t block2Num = it->block()->mSoilBlockInDomainNum;
        Q_ASSERT(!qFuzzyIsNull(it->square(isAxiallySymmetric)));
        domain->addHeatSurface(axe,
                               it->square(isAxiallySymmetric),
                               block1Num, block2Num, isAxiallySymmetric);
    }
}

void Block::createTopAndLeftHeatSurfaces(qfcore::Domain *domain,
        bool isAxiallySymmetric) const
{
    Q_ASSERT(mIsInDomain);
    createHeatSurfaces(domain, mContactsTop, 1, isAxiallySymmetric);
    createHeatSurfaces(domain, mContactsLeft, 0, isAxiallySymmetric);
}

void Block::readFromBlockData(const qfgui::BlockData &data)
{
    mSoilBlock.setTemperature(data.temperature(), false);
    mSoilBlock.setThawedPart(data.thawedPart());
    updateFromTemperature();
    updateFromThawedPart();
}

void Block::showArrows()
{
    qDebug("Contacts for block with center in (%f; %f)...",
           mMetersCenter.x(),
           mMetersCenter.y());

    foreach(BlockContact contact, mContactsTop) {
        scene()->addItem(new Arrow(this, contact.block(), Qt::blue));
        qDebug(" - Top contact. Square = %f, R = %f",
               contact.square(),
               contact.r());
    }
    foreach(BlockContact contact, mContactsBottom) {
        scene()->addItem(new Arrow(this, contact.block(), Qt::red));
        qDebug(" - Bottom contact. Square = %f, R = %f",
               contact.square(),
               contact.r());
    }
    foreach(BlockContact contact, mContactsLeft) {
        scene()->addItem(new Arrow(this, contact.block(), Qt::magenta));
        qDebug(" - Left contact. Square = %f, R = %f",
               contact.square(),
               contact.r());
    }
    foreach(BlockContact contact, mContactsRight) {
        scene()->addItem(new Arrow(this, contact.block(), Qt::darkMagenta));
        qDebug(" - Right contact. Square = %f, R = %f",
               contact.square(),
               contact.r());
    }
}
