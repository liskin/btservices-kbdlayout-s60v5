/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BtCpUiDeviceDetailsView declaration
*/

#ifndef BTCPUIDEVICEDETAILSVIEW_H
#define BTCPUIDEVICEDETAILSVIEW_H

#include <hbview.h>
#include <qglobal.h>
#include <hbdataform.h>
#include <hbgroupbox.h>
#include <hbdocumentloader.h>
#include <cpbasesettingview.h>
#include <QGraphicsLinearLayout>

class BtCpUiDeviceDetailsView : public CpBaseSettingView
{
    Q_OBJECT

public:
    
    explicit BtCpUiDeviceDetailsView(QGraphicsItem *parent = 0);
    
    virtual ~BtCpUiDeviceDetailsView();

    void setDeviceName(const QString &deviceName);
    void addItem(HbDataForm *item);
    void removeItem(HbDataForm *item);

    
protected:
    HbDocumentLoader *mLoader;
    HbAction        *mSoftKeyBackAction;

    QGraphicsLinearLayout *mLayout;
    HbGroupBox      *mGroupBox;
    
    Q_DISABLE_COPY(BtCpUiDeviceDetailsView)
};

#endif /* BTCPUIDEVICEDETAILSVIEW_H */
