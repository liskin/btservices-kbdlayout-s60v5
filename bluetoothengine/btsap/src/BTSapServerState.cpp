/*
* Copyright (c) 2004-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     This class is a state machine of BTSap Server
*
*/



// INCLUDE FILES
#include <e32property.h>
#include "BTSapDomainPSKeys.h"

#include "BTSapServerState.h"
#include "BTSapSocketHandler.h"
#include "BTSapRequestHandler.h"
#include "BTSapSimCardStatusNotifier.h"
#include "debug.h"

_LIT(KPhoneTsy, "PhoneTsy") ;
_LIT(KDefaultPhone, "DefaultPhone");

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBTSapStatusObserver::CBTSapStatusObserver()
//----------------------------------------------------------
//
CBTSapStatusObserver::CBTSapStatusObserver(): CActive(CActive::EPriorityStandard)
    {                                
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------
// CBTSapStatusObserver::NewL()
//----------------------------------------------------------
// 
CBTSapStatusObserver* CBTSapStatusObserver::NewL() 
    {
    CBTSapStatusObserver* self = new(ELeave) CBTSapStatusObserver();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
 
// ---------------------------------------------------------
// CBTSapStatusObserver::ConstructL()
//----------------------------------------------------------
//   
void CBTSapStatusObserver::ConstructL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapStatusObserver::ConstructL")));
    
    User::LeaveIfError(iProperty.Attach( KPSUidBluetoothSapConnectionState,
                                         KBTSapConnectionState));
    }

// ---------------------------------------------------------
// CBTSapStatusObserver::~CBTSapStatusObserver
//----------------------------------------------------------
//
CBTSapStatusObserver::~CBTSapStatusObserver()
    {
    Cancel();
    iProperty.Close();
    }
    
// ---------------------------------------------------------
// CBTSapStatusObserver::StartObservingL()
//----------------------------------------------------------
//
void CBTSapStatusObserver::SubscribeSapStatusL(MSapStatusObserver* aObserver)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapStatusObserver::SubscribeSapStatusL")));
    
    ASSERT(aObserver);
    
    iObserver = aObserver;
    
    iProperty.Subscribe(iStatus);
    SetActive();
    }
    

// ---------------------------------------------------------
// CBTSapStatusObserver::DoCancel()
// ---------------------------------------------------------
//
void CBTSapStatusObserver::DoCancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapStatusObserver::DoCancel")));
    
    iProperty.Cancel();
    }

// ---------------------------------------------------------
// CBTSapStatusObserver::RunL()
// ---------------------------------------------------------
//
void CBTSapStatusObserver::RunL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapStatusObserver::RunL")));
    
    TInt btSapState;
    TInt err = iStatus.Int();
    
    iProperty.Subscribe(iStatus);
    SetActive();
    
    if (!err)
        {
        err = iProperty.Get(btSapState);
        if (!err)
            {
            iObserver->SapStatusChangedL(btSapState);
            }
        }
    }

// ---------------------------------------------------------
// CBTSapServerState::CBTSapServerState()
// ---------------------------------------------------------
//
CBTSapServerState::CBTSapServerState(CBTSapPlugin& aBTSapPlugin)
    : CActive(CActive::EPriorityStandard), 
      iBTSapPlugin(aBTSapPlugin),
      iCurrentState(EStateInit),
      iStatesCreated(EFalse)
    {
    CActiveScheduler::Add(this);

    _LIT_SECURITY_POLICY_C2(sapConnectionStateReadAndWritePolicy, ECapabilityLocalServices, ECapabilityReadDeviceData);
    //_LIT_SECURITY_POLICY_S0(sapConnectionStateWritePolicy, 0x10005950); // BtEngServer SID

    RProperty::Define(KPSUidBluetoothSapConnectionState, KBTSapConnectionState, RProperty::EInt,
                      sapConnectionStateReadAndWritePolicy, sapConnectionStateReadAndWritePolicy);
    }

// ---------------------------------------------------------
// CBTSapServerState::~CBTSapServerState()
// ---------------------------------------------------------
//
CBTSapServerState::~CBTSapServerState()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::~CBTSapServerState")));
    // Make sure we've cancelled.
    Cancel();
    
    if (IsSapConnected())
        {
        iStateArray[iCurrentState]->NotifySapState(ESapNotConnected);
        }

    iStateArray.ResetAndDestroy();

    delete iSocketHandler;
    delete iSimCardStatusNotifier;
    delete iRequestHandler;
    delete iStatusObserver;

    iSubscriptionModule.Close();
    iPhone.Close();
    iTelServer.UnloadPhoneModule(KPhoneTsy);
    iTelServer.Close();

    RProperty::Delete(KPSUidBluetoothSapConnectionState, KBTSapConnectionState);
    }

// ---------------------------------------------------------
// CBTSapServerState::NewL()
// ---------------------------------------------------------
//
CBTSapServerState* CBTSapServerState::NewL(CBTSapPlugin& aBTSapPlugin) 
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::NewL")));
    
    CBTSapServerState* self = new (ELeave) CBTSapServerState(aBTSapPlugin);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CBTSapServerState::ConstructL()
// ---------------------------------------------------------
//
void CBTSapServerState::ConstructL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::ConstructL")));
    
    OpenSubscriptionModuleL();
    iRequestHandler = CBTSapRequestHandler::NewL(*this);
    iSocketHandler = CBTSapSocketHandler::NewL(*this, *iRequestHandler);
    iSimCardStatusNotifier = CBTSapSimCardStatusNotifier::NewL(*this);
    iStatusObserver = CBTSapStatusObserver::NewL();
    
	TState* state = new (ELeave) TStateInit(*this);
	CleanupStack::PushL(state);
    User::LeaveIfError(iStateArray.Append(state));
	CleanupStack::Pop(state);
    
	ChangeState(EStateInit);
    }

// ---------------------------------------------------------
// CBTSapServerState::StartL()
// ---------------------------------------------------------
//
void CBTSapServerState::StartL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::StartL")));
       
    // the order matters
	TState* state = new (ELeave) TStateNotConnected(*this);
    CleanupStack::PushL(state);
	User::LeaveIfError(iStateArray.Append(state));
	CleanupStack::Pop(state);
    
    // The SAP connection is accepted/rejected via a P&S key (KPSUidBluetoothSapConnectionState)
    // iStatusObserver monitors P&S key changes and calls AcceptSapConnection or
    // RejectSapConnection accordingly.
    iStatusObserver->SubscribeSapStatusL(this);
    
    ChangeState(EStateNotConnected);
    }

// ---------------------------------------------------------
// CBTSapServerState::DoCancel()
// ---------------------------------------------------------
//
void CBTSapServerState::DoCancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::DoCancel")));

    iStateArray[iCurrentState]->Cancel();
    }

// ---------------------------------------------------------
// CBTSapServerState::ChangeState
// ---------------------------------------------------------
void CBTSapServerState:: ChangeState(const TBTSapServerState aNextState)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState:: ChangeState: %d"), aNextState));

    Cancel();
    iCurrentState = aNextState;
    iStateArray[iCurrentState]->Enter(iStatus);  
    SetActive();
    }

// ---------------------------------------------------------
// CBTSapServerState::RunL()
// ---------------------------------------------------------
//
void CBTSapServerState::RunL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::RunL: %d >>"), iStatus.Int()));

    TBTSapServerState nextState = iStateArray[iCurrentState]->Complete(iStatus.Int());

    // Enter new state
    ChangeState(nextState);

    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: RunL complete <<")));
    }

// ---------------------------------------------------------
// CBTSapServerState::CurrentState()
// ---------------------------------------------------------
TBTSapServerState CBTSapServerState:: CurrentState()
    {
    return iCurrentState;
    }

// ---------------------------------------------------------
// CBTSapServerState::AcceptSapConnection()
// ---------------------------------------------------------
//
TInt CBTSapServerState::AcceptSapConnection()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap] CBTSapServerState::AcceptSapConnection")));
    
    return iStateArray[iCurrentState]->AcceptSapConnection();
    }

// ---------------------------------------------------------
// CBTSapServerState::RejectSapConnection()
// ---------------------------------------------------------
//
TInt CBTSapServerState:: RejectSapConnection(TBTSapRejectReason aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap] CBTSapServerState::RejectSapConnection")));
    
    return iStateArray[iCurrentState]->RejectSapConnection(aReason);
    }

// ---------------------------------------------------------
// CBTSapServerState::DisconnectSapConnection()
// ---------------------------------------------------------
//
TInt CBTSapServerState::DisconnectSapConnection(TBTSapDisconnectType aType)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::DisconnectSapConnection: %d"), aType));
    TInt retVal = KErrNotReady;

    if (IsSapConnected())
        {
        Cancel(); // Cancel current state
        retVal = iStateArray[iCurrentState]->DisconnectSapConnection(aType);
        ChangeState(aType == EDisconnectImmediate ? EStateNotConnected : EStateIdle);
        }

    return retVal;
    }

// ---------------------------------------------------------
// CBTSapServerState::IsSapConnected()
// ---------------------------------------------------------
//
TBool CBTSapServerState::IsSapConnected()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::IsSapConnected")));
    TBool ret = EFalse;
    if (iSocketHandler)
        {
        ret = iSocketHandler->IsSapConnected();
        }
    return ret;
    }

// ---------------------------------------------------------
// CBTSapServerState::GetRemoteBTAddress()
// ---------------------------------------------------------
//
TInt CBTSapServerState::GetRemoteBTAddress(TBTDevAddr& aBTDevAddr)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap] CBTSapServerState::GetRemoteBTAddress")));
    
    return (iSocketHandler != NULL) ? iSocketHandler->GetRemoteBTAddress(aBTDevAddr) : KErrDisconnected;
    }

// ---------------------------------------------------------
// CBTSapServerState::SimCardStatusChanged()
// ---------------------------------------------------------
//
void CBTSapServerState::SimCardStatusChanged(TCardStatus aCardStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  SapSimCardStatusChanged: %d"), aCardStatus));

    if (iCurrentState == EStateConnect ||
        iCurrentState == EStatePowerOn ||
        iCurrentState == EStateReset)
        {
        iStateArray[iCurrentState]->SimCardStatusChanged(aCardStatus);
        }
    else
        {
        // real indication
        Cancel();
        iStateArray[iCurrentState]->SimCardStatusChanged(aCardStatus);
        ChangeState(EStateIdle);
        }
    }
    
// ---------------------------------------------------------
// CBTSapServerState::CreateStatesL()
// ---------------------------------------------------------
//
TInt CBTSapServerState::CreateStatesL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::CreateStatesL")));
    
    if (!iStatesCreated)
        {
        // Create SAP states. TStateInit and TStateNotConnected have already been created.
		TState* state = new (ELeave) TStateConnect(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
		
		state = new (ELeave) TStateDisconnect(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
		
		state = new (ELeave) TStateIdle(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
		
		state = new (ELeave) TStateAPDU(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
		
		state = new (ELeave) TStateATR(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
		
		state = new (ELeave) TStatePowerOff(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
		
		state = new (ELeave) TStatePowerOn(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
		
		state = new (ELeave) TStateReset(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
		
		state = new (ELeave) TStateCardReaderStatus(*this);
		CleanupStack::PushL(state);
        User::LeaveIfError(iStateArray.Append(state));
		CleanupStack::Pop(state);
        }
    
    iStatesCreated = ETrue;
    return KErrNone;
    }
    
// ---------------------------------------------------------
// CBTSapServerState::ReleaseStatesL()
// ---------------------------------------------------------
//
TInt CBTSapServerState::ReleaseStatesL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::ReleaseStatesL")));
    // Delete SAP states, all except TStateInit and TStateNotConnected.
    for(TInt i = iStateArray.Count() - 1; i >= 2; i--)
        {
        delete iStateArray[i];
        iStateArray[i] = NULL;
        iStateArray.Remove(i);
        }
        
    iStatesCreated = EFalse;
    return KErrNone;
    }

// ---------------------------------------------------------
// CBTSapServerState::DisconnectCompleteL()
// ---------------------------------------------------------
//  
TInt CBTSapServerState::DisconnectCompleteL(TInt aErr)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::DisconnectCompleteL")));
    iBTSapPlugin.OperationCompletedL(MSapAsyncObserver::EDisconnect, aErr);
    return KErrNone;
    }

// ---------------------------------------------------------
// CBTSapServerState::HandleStateChangeRequest()
// ---------------------------------------------------------
//
TInt CBTSapServerState::HandleStateChangeRequest(TBTSapServerState& aNextState)
    {
     return iStateArray[iCurrentState]->ChangeState(aNextState);
    }
    
// ---------------------------------------------------------
// CBTSapServerState::SendErrorResponse()
// ---------------------------------------------------------
//
void CBTSapServerState::SendErrorResponse()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap] CBTSapServerState::SendErrorResponse")));
    
    // set response: error_response
    iResponseMessage.SetMsgID(EErrorResponse);
    iSocketHandler->Send(iResponseMessage.Data());
    }

// ---------------------------------------------------------
// CBTSapServerState::OpenSubscriptionModuleL()
// ---------------------------------------------------------
//
void CBTSapServerState::OpenSubscriptionModuleL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap] CBTSapServerState::OpenSubscriptionModuleL")));
    
    User::LeaveIfError(iTelServer.Connect());
    User::LeaveIfError(iTelServer.LoadPhoneModule(KPhoneTsy));   // Load Custom API Extension
    User::LeaveIfError(iPhone.Open( iTelServer, KDefaultPhone));
    User::LeaveIfError(iSubscriptionModule.Open(iPhone));
    }

// ---------------------------------------------------------
// CBTSapServerState::BTSapSocketHandler()
// ---------------------------------------------------------
//
CBTSapSocketHandler& CBTSapServerState::BTSapSocketHandler()
    {
    return *iSocketHandler;
    }

// ---------------------------------------------------------
// CBTSapServerState::BTSapSimCardStatusNotifier()
// ---------------------------------------------------------
//
CBTSapSimCardStatusNotifier& CBTSapServerState::BTSapSimCardStatusNotifier()
    {
    return *iSimCardStatusNotifier;
    }

// ---------------------------------------------------------
// CBTSapServerState::SubscriptionModule()
// ---------------------------------------------------------
//
RMmCustomAPI& CBTSapServerState::SubscriptionModule()
    {
    return iSubscriptionModule;
    }

// ---------------------------------------------------------
// CBTSapServerState::BTSapRequestMessage()
// ---------------------------------------------------------
//
TBTSapMessage& CBTSapServerState::BTSapRequestMessage()
    {
    return iRequestMessage;
    }

// ---------------------------------------------------------
// CBTSapServerState::BTSapResponseMessage()
// ---------------------------------------------------------
//
TBTSapMessage& CBTSapServerState::BTSapResponseMessage()
    {
    return iResponseMessage;
    }
    
// ---------------------------------------------------------
// CBTSapServerState::SapStatusChangedL()
// ---------------------------------------------------------
//    
void CBTSapServerState::SapStatusChangedL(TInt aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap] CBTSapServerState::SapStatusChangedL")));
    
    switch (aStatus)
        {
        case EBTSapNotConnected:
        case EBTSapConnecting:
            {
            // no actions
            break; 
            }
        case EBTSapConnected:
            {
            // Inform observers
            iBTSapPlugin.ConnectComplete();
            break;
            }
        case EBTSapAccepted:
            {
            AcceptSapConnection();
            break;
            }
        case EBTSapRejected:
            {
            RejectSapConnection(ERejectGeneralError);
            break;
            }
        default:
            {
            // An error or invalid state
            RejectSapConnection(ERejectGeneralError);
            break;
            }
        }
    }


//  End of File  
