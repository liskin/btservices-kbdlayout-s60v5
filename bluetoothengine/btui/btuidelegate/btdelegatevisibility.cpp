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
* Description: Delegate class for setting visibility mode
*
*/


#include "btdelegatevisibility.h"
#include <btsettingmodel.h>
#include <btdevicemodel.h>
#include <bluetoothuitrace.h>
#include "btqtconstants.h"
#include <btengsettings.h>

const int MAX_TEMPORARY_VISIBILITY = 60;  // minutes, this value comes from the UI spec

/*!
    Constructor.
 */
BtDelegateVisibility::BtDelegateVisibility(
        QObject *parent )
    : BtAbstractDelegate( NULL, NULL, parent )
{
    TRAP_IGNORE( mBtengSettings = CBTEngSettings::NewL(this) );
    Q_CHECK_PTR( mBtengSettings );
    mActiveHandling = false;
}

/*!
    Destructor.
 */
BtDelegateVisibility::~BtDelegateVisibility()
{
    delete mBtengSettings;
}
/*!
 * executes visibility delegate functionality, ie. calls CBTEngSettings to set the visibility mode;
 * when operation completes, emits commandCompleted signal
 * Parameters:  Qlist<QVariant> where first item is VisibilityMode integer specifying operation;  
 *              for BtTemporary a 2nd parameter is give which signifies the number of minutes to stay visible.
 */
void BtDelegateVisibility::exec( const QVariant &params )
{
    int minutes, err = 0;

    if (mActiveHandling) {
        // complete command with error
        emit commandCompleted(KErrInUse);
        return;
    } 
    mActiveHandling = true;
    
    // read 1st parameter
    BTUI_ASSERT_X(params.toList().at(0).isValid(), "BtDelegateVisibility::exec", "invalid parameter");
    VisibilityMode btQtMode = (VisibilityMode)params.toList().at(0).toInt();
    mOperation = BtEngVisibilityMode(btQtMode);
    
    // verify that we are setting visibility to a new value, otherwise we won't get a callback
    TBTVisibilityMode visibilityMode( EBTVisibilityModeNoScans );
    err = mBtengSettings->GetVisibilityMode( visibilityMode );
    if (err) {
        mActiveHandling = false;
        emit commandCompleted(err);
        return;
    }
    if (visibilityMode == mOperation) {
        mActiveHandling = false;
        emit commandCompleted(KErrNone);
        return;
    }
    
    switch (mOperation) {
    case EBTVisibilityModeGeneral :
        err = mBtengSettings->SetVisibilityMode(mOperation, 0);
        break;
    case EBTVisibilityModeHidden:
        err = mBtengSettings->SetVisibilityMode(mOperation, 0);
        break;
    case EBTVisibilityModeTemporary:
        BTUI_ASSERT_X(params.toList().at(1).isValid(), "BtDelegateVisibility::exec", "invalid time parameter");
        minutes = params.toList().at(1).toInt();
        BTUI_ASSERT_X(((minutes >= 0 ) && (minutes <= MAX_TEMPORARY_VISIBILITY)), 
                "BtDelegateVisibility::exec", "invalid time parameter");
        err = mBtengSettings->SetVisibilityMode(mOperation, minutes);
        break;
    default:
        // error
        BTUI_ASSERT_X(false, "BtDelegateVisibility::exec", "invalid parameter");
    }
    if (err) {
        // complete command with error
        mActiveHandling = false;
        emit commandCompleted(err);
    }
}

void BtDelegateVisibility::PowerStateChanged( TBTPowerStateValue aState )
{
    Q_UNUSED( aState );
}

/*!
 * callback from BtEngine
 *    emits command complete with either: 
 *    1) KErrUnknown if something went wrong, or
 *    2) KErrNone if everything ok 
 */
void BtDelegateVisibility::VisibilityModeChanged( TBTVisibilityMode aState )
{
    if (mActiveHandling) {
        //Error handling has to be done, if value is not set properly.
        mActiveHandling = false;
        if (mOperation == aState) {
            emit commandCompleted(KErrNone);
        }
        else {
            emit commandCompleted(KErrUnknown);
        }
    }
}
