/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares Registry watcher class for BTUI.
*
*/


// INCLUDE FILES

#include "BTUI.h"
#include "BTUIRegistryWatcher.h"
#include <bt_subscribe.h>

#include "BTDevModel.h"  //for Log printing Macro

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBTUIRegistryWatcherAO::CBTUIRegistryWatcherAO
// C++ default constructor can NOT contain any code, that
// might leave.
// ---------------------------------------------------------
//
CBTUIRegistryWatcherAO::CBTUIRegistryWatcherAO( MBTUIRegistryObserver* aParent )
    : CActive(EPriorityNormal), iParent ( aParent )
    {
    TRACE_FUNC_ENTRY

    CActiveScheduler::Add(this);

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// Destructor.
// ---------------------------------------------------------
//
CBTUIRegistryWatcherAO::~CBTUIRegistryWatcherAO()
    {
    TRACE_FUNC_ENTRY

    Cancel();
    iProperty.Close();

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CBTUIRegistryWatcherAO::WatchL
// ---------------------------------------------------------
//
void CBTUIRegistryWatcherAO::WatchL()
    {
    TRACE_FUNC_ENTRY

    if( !iProperty.Handle() )
        {
		User::LeaveIfError( iProperty.Attach( KPropertyUidBluetoothCategory,
                                              KPropertyKeyBluetoothRegistryTableChange ) );
        }

    if( !IsActive() )
        {
        iProperty.Subscribe(iStatus);
        SetActive();    
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CBTUIRegistryWatcherAO::DoCancel
// ---------------------------------------------------------
//
void CBTUIRegistryWatcherAO::DoCancel()
    {
    TRACE_FUNC

    iProperty.Cancel();
    }

// ---------------------------------------------------------
// CBTUIRegistryWatcherAO::RunL
// ---------------------------------------------------------
//
void CBTUIRegistryWatcherAO::RunL()
    {
    TRACE_FUNC_ENTRY

    if( iStatus.Int()==KErrNone )
        {
        __ASSERT_DEBUG(iParent, PANIC(EBTPanicClassMemberVariableIsNull));

        // Inform observer
        //
        iParent->RegistryContentChangedL();
        
        // Continue watching
        //
        WatchL();
        }

    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CBTUIRegistryWatcherAO::RunError
// ---------------------------------------------------------
//
TInt CBTUIRegistryWatcherAO::RunError( TInt aError )
    {
    TRACE_INFO((_L("RunError: %d"), aError))

    return aError;
    }

// ---------------------------------------------------------
// CBTUIRegistryWatcherAO::StopWatching
// ---------------------------------------------------------
//
void CBTUIRegistryWatcherAO::StopWatching()
    {
    TRACE_FUNC

    Cancel();
    }

// End of File
