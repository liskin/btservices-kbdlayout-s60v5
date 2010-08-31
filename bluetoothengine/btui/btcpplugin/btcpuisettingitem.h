/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  
 *   
 */

#ifndef BTCPUISETTINGITEM_H
#define BTCPUISETTINGITEM_H

#include <cpsettingformentryitemdata.h>
#include <btsettingmodel.h>
#include <btdevicemodel.h>

#include "btcpuimainview.h"

class HbTranslator;

class BtCpUiSettingItem : public CpSettingFormEntryItemData
{
	Q_OBJECT
public:
	explicit BtCpUiSettingItem(CpItemDataHelper &itemDataHelper);	 
	virtual ~BtCpUiSettingItem();
private slots:
	void onLaunchView();
	void handleCloseView();
	void handleDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
private:
	virtual CpBaseSettingView *createSettingView() const;
	void updateStatus();
	void loadTranslators();
private:
	HbMainWindow* mMainWindow;
	
	BtCpUiMainView *mBtMainView;
	
	//Owns this model.
	BtSettingModel *mSettingModel;
	BtDeviceModel *mDeviceModel;
	
	HbView *mCpView;
	
	HbTranslator *mViewTranslator;
	HbTranslator *mDialogTranslator;
	
};

#endif //BTCPUISETTINGITEM_H
