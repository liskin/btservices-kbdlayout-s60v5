/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of BTEng server session handling
*
*/



#include <e32base.h>
#include "btengsrvsession.h"
#include "btengserver.h"
#include "btengsrvbbconnectionmgr.h"
#include "btengsrvsettingsmgr.h"
#include "btengclientserver.h"
#include "debug.h"


/**  The message argument which holds the client event package. */
const TInt KBTEngEventSlot = 2;

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBTEngSrvSession::CBTEngSrvSession()
:   CSession2()
    {
    }


// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBTEngSrvSession::ConstructL()
    {
    TRACE_FUNC_ENTRY
    
    //create queue for storing multiple connection events
    iConnectionEventQueue = new (ELeave) CDesC8ArraySeg(1);
    }


// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
CBTEngSrvSession* CBTEngSrvSession::NewL()
    {
    CBTEngSrvSession* self = new( ELeave ) CBTEngSrvSession();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CBTEngSrvSession::~CBTEngSrvSession()
    {
    TRACE_FUNC_ENTRY
    if( !iNotifyConnMessage.IsNull() )
        {
        iNotifyConnMessage.Complete( KErrCancel );
        }
    if( Server() )
        {
        Server()->RemoveSession(this, iAutoSwitchOff );
        }
    delete iConnectionEventQueue;
    TRACE_FUNC_EXIT
    }


// ---------------------------------------------------------------------------
// Complete the construction of the session.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSession::CreateL()
    {
    TRACE_FUNC_ENTRY
    Server()->AddSession();
    }


// ---------------------------------------------------------------------------
// Notify the client of the connection event.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSession::NotifyConnectionEvent(const TBTDevAddr aAddr, 
    TBTEngConnectionStatus aConnStatus, RBTDevAddrArray* aArray, TInt aErr)
    {
    TRACE_FUNC_ENTRY

    TRACE_INFO((_L("[BTEng]\t Informing client %d"), iNotifyConnMessage.Handle()));
    
    //package up connection event to send to the client
    TBTEngEventPkg pkg;
    pkg().iAddr = aAddr;
    pkg().iConnEvent = aConnStatus;
    pkg().iConflictsBuf.Zero();
    if (aArray)
        {
        TBTDevAddrPckgBuf addr;
        
        for (TInt i = 0; i < aArray->Count(); i++)
            {
            addr = (*aArray)[i];
            pkg().iConflictsBuf.Append(addr);
            }
        }
    
    TInt err = KErrNotFound;
    
    //if a client message is outstanding
    if (!iNotifyConnMessage.IsNull())
        {
        //complete the message prioritising the main error
        err = iNotifyConnMessage.Write(KBTEngEventSlot, pkg);
        aErr = (aErr != KErrNone) ? aErr : err;
        iNotifyConnMessage.Complete(aErr);
        }
    
    //if there is no client message outstanding or the write failed
    if (err != KErrNone)
        {
        //if the append fails we can't do anything with the error so ignore
        //it and let the event get dropped
        TRAP_IGNORE(iConnectionEventQueue->AppendL(pkg));
        }
    }

// ---------------------------------------------------------------------------
// From class CSession2
// Handles servicing of a client request that has been passed to the server.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSession::ServiceL( const RMessage2& aMessage )
    {
    TRAPD( err, DispatchMessageL( aMessage ) );
    if( !aMessage.IsNull() &&
        ( err || 
          ( aMessage.Function() != EBTEngNotifyConnectionEvents && 
            aMessage.Function() != EBTEngSetPowerState ) ) )
        {
            // Return the error code to the client.
        aMessage.Complete( err );
        }
    }


// ---------------------------------------------------------------------------
// Returns a handle to CBTEngServer.
// ---------------------------------------------------------------------------
//
CBTEngServer* CBTEngSrvSession::Server()
    {
    return (CBTEngServer*) CSession2::Server();
    }


// ---------------------------------------------------------------------------
// Handles a client request that has been passed to the server.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSession::DispatchMessageL( const RMessage2& aMessage )
    {
    TRACE_FUNC_ENTRY
    TInt opcode = aMessage.Function();
   
    switch( opcode )
        {
        case EBTEngSetPowerState:
            {
            // Store auto off information
            iAutoSwitchOff = (TBool) aMessage.Int1();
            Server()->SetPowerStateL( aMessage );
            }
            break;
        case EBTEngSetVisibilityMode:
            {
            Server()->SettingsManager()->SetVisibilityModeL(
                        (TBTVisibilityMode) aMessage.Int0(),aMessage.Int1() );
            }
            break;
        case EBTEngNotifyConnectionEvents:
            {
            //ensure the message contains the correctly sized descriptor
            if (aMessage.GetDesMaxLengthL(KBTEngEventSlot) != sizeof(TBTEngEventMsg))
                {
                User::Leave(KErrBadDescriptor);
                }
            
            if(!iNotifyConnMessage.IsNull())
                {
                User::Leave(KErrInUse);
                }
            //save the client message
            iNotifyConnMessage = RMessage2(aMessage);
            
            //if there is an existing connection event that the client does
            //not yet know about
            if (iConnectionEventQueue->Count() > 0)
                {
                //complete straight away with package from queue
                TInt err = iNotifyConnMessage.Write(KBTEngEventSlot, (*iConnectionEventQueue)[0]);
                if (err == KErrNone)
                    {
                    iConnectionEventQueue->Delete(0);
                    }
                iNotifyConnMessage.Complete(err);
                }
            }
            break;
        case EBTEngCancelEventNotifier:
            {
            if( !iNotifyConnMessage.IsNull() )
                {
                iNotifyConnMessage.Complete( KErrCancel );
                }
            }
            break;
        case EBTEngConnectDevice:
        case EBTEngCancelConnectDevice:
        case EBTEngDisconnectDevice:
        case EBTEngIsDeviceConnected:
        case EBTEngGetConnectedAddresses:
            {
            CheckPowerOnL();
            // Simply forward it to the plug-in manager
            Server()->DispatchPluginMessageL( aMessage );
            }
            break;    
        case EBTEngIsDeviceConnectable:
            {
            Server()->DispatchPluginMessageL( aMessage );
            }
            break;
        case EBTEngPrepareDiscovery:
            {
            aMessage.Complete( KErrNone );  // Client does not have to wait.
            Server()->BasebandConnectionManager()->ManageTopology( ETrue );
            }
            break;
        default:
            {
            TRACE_INFO( ( _L( "[BTENG]\t DispatchMessageL: bad request (%d)" ), aMessage.Function() ) )
            User::Leave( KErrArgument );
            }
            break;
        }
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Check if power is on, and leave if it is not.
// ---------------------------------------------------------------------------
//
void CBTEngSrvSession::CheckPowerOnL()
    {
    TBTPowerState power = EBTOff;
    (void) Server()->SettingsManager()->GetHwPowerState( power );
    if ( power == EBTOff )
        {
        User::Leave( KErrNotReady );
        }
    }
