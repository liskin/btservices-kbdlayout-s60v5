/*
* Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  BTUISearchListViewItem implementation
*
*/


#include "btcpuimainlistviewitem.h"
#include "btdevicemodel.h"
#include "btuiiconutil.h"
#include <QGraphicsGridLayout>

BtCpUiMainListViewItem::BtCpUiMainListViewItem(QGraphicsItem * parent) :
    HbListViewItem(parent), mParent(parent)
{
    mDeviceNameLabel = 0;
    mDevTypeIconLabel = 0;
    mDevTypeTextLabel = 0;
    mBtuiModelSortFilter = ((BtCpUiMainListViewItem *)parent)->mBtuiModelSortFilter;
    
    mRow = 0;   
}

BtCpUiMainListViewItem::~BtCpUiMainListViewItem()
{
//    delete mDeviceNameLabel;
//    delete mDevTypeIconLabel;
//    delete mDevTypeTextLabel;
}

/*
 * This method is called by the HbListView when it needs a new
 * view item element.
 *
 */
HbAbstractViewItem * BtCpUiMainListViewItem::createItem()
{
    return new BtCpUiMainListViewItem(*this); 
}

/*!
   UpdateChildItem updates the item graphics.
   Screen elements are created once if not already done.  This may increase the overall memory 
   consumption of the application, however, this is deemed inconsequential.  There might be a small 
   performance improvement with current style.
 */
void BtCpUiMainListViewItem::updateChildItems()
{
    QModelIndex index;
    
    // Get device name from model
    if (mBtuiModelSortFilter)
        index = mBtuiModelSortFilter->mapToSource(modelIndex());
    else
        index = modelIndex();

    // create new icon label if needed
    if (!mDevTypeIconLabel) {
        mDevTypeIconLabel = new HbLabel();
        mDevTypeIconLabel->setPreferredSize(53.5260, 53.5260); //8un x 8un
        mDevTypeIconLabel->setMinimumWidth(53.5260);
    }
    // create new label if needed
    if (!mDeviceNameLabel) {
        mDeviceNameLabel = new HbLabel();
        mDeviceNameLabel->setPreferredSize(250, 26.763);
    }
    // create new label if needed
    if (!mDevTypeTextLabel) {
        mDevTypeTextLabel = new HbLabel();
        mDevTypeTextLabel->setPreferredSize(250, 26.763);
    }
    // create layout if needed
    if ( !mRow ) {
        // Still need to create the actual layout
        mRow = new QGraphicsGridLayout();
        mRow->addItem(mDevTypeIconLabel,0,0,2,1);
        mRow->addItem(mDeviceNameLabel,0,1,1,1);
        mRow->addItem(mDevTypeTextLabel,1,1,1,1);
        setLayout(mRow);
    }
        
    QString data = index.data(Qt::DisplayRole).toString();
    int cod = (index.data(BtDeviceModel::CoDRole)).toInt();
    int majorProperty = (index.data(BtDeviceModel::MajorPropertyRole)).toInt();
    
    // ToDo:  remove clear() once Orbit bug is fixed
    mDeviceNameLabel->clear();
    mDeviceNameLabel->setPlainText(data);
    mDevTypeIconLabel->clear();
    mDevTypeTextLabel->clear();
    mDevTypeTextLabel->setPlainText( getDeviceTypeString( cod ));
    HbIcon icon = 
    getBadgedDeviceTypeIcon( cod, majorProperty, 
            BtuiBottomLeft | BtuiBottomRight | BtuiTopLeft | BtuiTopRight );
    mDevTypeIconLabel->setIcon(icon);
}


void BtCpUiMainListViewItem::setModelSortFilter(BtuiModelSortFilter *filter)
{
    mBtuiModelSortFilter = filter;
}
