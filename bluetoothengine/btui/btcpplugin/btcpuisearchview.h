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

#ifndef	BTCPUISEARCHVIEW_H
#define	BTCPUISEARCHVIEW_H

#include <cpbasesettingview.h>
#include <hbaction.h>
#include <hbtoolbar.h>
#include "btcpuibaseview.h"
#include "btuimodelsortfilter.h"

class HbLabel;
class HbPushButton;
class HbIcon;
class HbDocumentLoader;
class HbListView;
class BtAbstractDelegate;
class HbSelectionDialog;
class HbGroupBox;
class HbDataForm;

class BtCpUiSearchView : public BtCpUiBaseView
{
    Q_OBJECT
    
public:

    explicit BtCpUiSearchView(
            BtSettingModel &settingModel, 
            BtDeviceModel &deviceModel, 
            QGraphicsItem *parent = 0);
    virtual ~BtCpUiSearchView();
    virtual void activateView( const QVariant& value, bool fromBackButton );
    virtual void deactivateView();
    virtual void setSoftkeyBack();
    
public slots:
    virtual void switchToPreviousView();
    void changeOrientation( Qt::Orientation orientation );
    void stopSearching();
    void retrySearch();
    void viewByDeviceTypeDialog();
    void viewByDialogClosed(HbAction* action);
    void searchDelegateCompleted(int error);
    void deviceSearchCompleted(int error);
    void deviceSelected(const QModelIndex& modelIndex);
    
private:
    void startSearchDelegate();
    
private:
    enum devTypeSelectionList {
        BtUiDevAudioDevice = 0,
        BtUiDevComputer,
        BtUiDevInputDevice,
        BtUiDevPhone,
        BtUiDevOtherDevice
    };
private:
    HbDocumentLoader *mLoader;
    HbLabel *mDeviceIcon;
    HbDataForm *mDataForm;
    HbLabel *mLabelSearching;        
    HbListView *mDeviceList;
    QStringList mDevTypeList;
    HbSelectionDialog *mQuery;
    
    Qt::Orientation mOrientation;
    
    HbMainWindow*           mMainWindow;
    BtCpUiBaseView*         mMainView;
    HbAction *              mSoftKeyBackAction;
    HbAction*               mViewBy;
    HbAction*               mStop;
    HbAction*               mRetry;

    //pointer to abstract delegate, and it is instantiated at runtime
    BtAbstractDelegate*     mAbstractDelegate;
    BtuiModelSortFilter*    mBtuiModelSortFilter;
};

#endif//	BTCPUISEARCHVIEW_H
