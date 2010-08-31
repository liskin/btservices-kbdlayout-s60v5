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
* Description:  device view item
*
*/


#ifndef BTCPUIMAINLISTVIEWITEM_H
#define BTCPUIMAINLISTVIEWITEM_H

#include <hblistviewitem.h>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <QtGui/QGraphicsGridLayout>
#include "btuimodelsortfilter.h"

class BtCpUiMainListViewItem : public HbListViewItem
{
    Q_OBJECT
    
public:
    BtCpUiMainListViewItem(QGraphicsItem * parent = 0);
    ~BtCpUiMainListViewItem();
    
    HbAbstractViewItem * createItem();
    void updateChildItems();
    
    void setModelSortFilter(BtuiModelSortFilter *filter);
    
private:
    QGraphicsItem *mParent;
    HbLabel *mDevTypeIconLabel;
    HbLabel *mDeviceNameLabel;
    HbLabel *mDevTypeTextLabel;
    BtuiModelSortFilter*    mBtuiModelSortFilter;
    
    QGraphicsGridLayout *mRow;
};

#endif /* BTCPUIMAINLISTVIEWITEM_H */
