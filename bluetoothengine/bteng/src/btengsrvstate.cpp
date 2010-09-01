/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Engine server state machine class implementation.
*
*/



#include <ecom/ecomresolverparams.h>

#include "btengsrvstate.h"
#include "btengserver.h"
#include "btengsrvpluginmgr.h"
#include "btengsrvsettingsmgr.h"
#include "debug.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSrvState::CBTEngSrvState( CBTEngServer* aServer )
:   iState( EInit ),
    iOperation( ESrvOpIdle ),
    iServer( aServer )
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSrvState::ConstructL()
    {
    TRACE_FUNC_ENTRY
    // Create the active object handling asynchronous state transitions.
    // The state transitions need to be done fast, as it can be initiated 
    // through user interaction (i.e. the user is waiting).
    const TCallBack cb(&ChangeStateCb, this);
    iAsyncCallback = new(ELeave) CAsyncCallBack(cb, CActive::EPriorityHigh);
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSrvState* CBTEngSrvState::NewL( CBTEngServer* aServer )
    {
    CBTEngSrvState* self = new( ELeave ) CBTEngSrvState( aServer );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSrvState::~CBTEngSrvState()
    {
    delete iAsyncCallback;
    }


// ---------------------------------------------------------------------------
// Returns the current state machine operation.
// ---------------------------------------------------------------------------
//
CBTEngSrvState::TBTEngSrvOperation CBTEngSrvState::CurrentOperation()
    {
    return iOperation;
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvState::StartStateMachineL( TBTPowerState aState )
    {
    TRACE_FUNC_ENTRY
    if( aState == EBTOn )
        {
        // Power on; starting state is initializing the stack
        iState = EInitBTStack;
        iOperation = EPowerOn;
        }
    else
        {
        // Power off, starting state is to disconnect the plug-ins
        iState = EDisconnectPlugins;
        iOperation = EPowerOff;
        iServer->SettingsManager()->UpdateCenRepPowerKeyL( EBTOff );
        iServer->SettingsManager()->SetUiIndicatorsL();
        }
    ChangeState();
    }


// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvState::ChangeState()
    {
    TRACE_FUNC_ENTRY
    iAsyncCallback->CallBack();
    }

// ---------------------------------------------------------------------------
// ?implementation_description
// ---------------------------------------------------------------------------
//
TInt CBTEngSrvState::ChangeStateCb(TAny* aThis)
    {
    TRAPD(err, static_cast<CBTEngSrvState*>(aThis)->RequestCompletedL());
    if(err != KErrNone)
        {
        static_cast<CBTEngSrvState*>(aThis)->HandleError(err);
        }
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// ?implementation_description
// ---------------------------------------------------------------------------
//
void CBTEngSrvState::RequestCompletedL()
    {
    switch( iState )
        {
        case EInitBTStack:
            {
            iState = ELoadDIService;
            iServer->SettingsManager()->InitBTStackL();
            }
            break;
        case ELoadDIService:
            {
            iState = ELoadPlugins;
            iServer->ManageDIServiceL( ETrue );
            }
            break;
        case ELoadPlugins:
            {
            iState = ELoadNextPlugin;
            TEComResolverParams params;
            iServer->PluginManager()->LoadProfilePluginsL( params );
            }
            break;
        case ELoadNextPlugin:
            {
                // Only change the state when all plug-ins are loaded
            if( iServer->PluginManager()->LoadPluginL() <= 0 )
                {
                iState = EIdle;
                }
            }
            break;
        case EStopBTStack:
            {
            iState = EWaitingForPowerOff;
            iServer->SettingsManager()->StopBTStackL();
            }
            break;
        case EUnloadDIService:
            {
            iState = EStopBTStack;
            iServer->ManageDIServiceL( EFalse );
            }
            break;
        case EUnloadPlugins:
            {
            iState = EUnloadDIService;
            iServer->PluginManager()->UnloadProfilePlugins();
            }
            break;
        case EDisconnectPlugins:
            {
            iState = EUnloadPlugins;
            // This is the starting state; the disconnect command has 
            // already been sent. Just one more loop to give more time
            // for disconnecting.
            }
            break;
        case EWaitingForPowerOff:
            {
            // Disconnecting all connections completed and 
            // called us back; we are done.
            iState = EIdle;
            }   // Fall through
        case EIdle:
            {
            if( iOperation == EPowerOn )
                {
                iServer->SettingsManager()->UpdateCenRepPowerKeyL( EBTOn );
                iServer->SettingsManager()->SetUiIndicatorsL();
                }
            iOperation = ESrvOpIdle;
            iServer->CheckIdle();
            }
        case EInit:
        default:
            break;
        }
    // In state EWaitingForPowerOff, we change the state 
    // through a callback in the server.
    if( iOperation != ESrvOpIdle && iState != EWaitingForPowerOff )
        {
        ChangeState();
        }
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// From class MBTEngActiveObserver.
// Handles a leave in RunL/RequestCompletedL.
// ---------------------------------------------------------------------------
//
void CBTEngSrvState::HandleError(TInt aError)
    {
    TRACE_FUNC_ARG( ( _L( "error: %d" ), aError ) )
    
    (void) aError;
    // All critical operations are done in turning power on; all other
    // operations are recoverable i.e. we can do without loading/unloading
    // of services. So simply change the state to handle next state change.
    ChangeState();
    }
