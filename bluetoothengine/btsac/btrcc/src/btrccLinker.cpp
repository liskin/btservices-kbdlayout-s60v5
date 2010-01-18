/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  State Machine of BTRCC
*
*/

// INCLUDE FILES
#include <remconaddress.h>
#include <remconbeareravrcp.h> //	KRemConBearerAvrcpImplementationUid = 0x1020685f
#include <btaccObserver.h>
#include <remconinterfaceselector.h>  
#include <remconaddress.h> 
#include <remconcoreapitarget.h>
#include <apacmdln.h>
#include <apgcli.h>
#include "btaudioremconpskeys.h"
#include "btrccLegacyVolumeLevelController.h"
#include "btrccAbsoluteVolumeLevelController.h"
#include "btrccLinker.h"
#include "btrccplayerstarter.h"
#include "btrccBrowsingAdapter.h"
#include "debug.h"
#include <btnotif.h>
#include <mmf/server/sounddevice.h>

#include "prjconfig.h"

#ifdef PRJ_MODULETEST_BUILD
const TInt KAvrcpBearerUid = 0xEEEEAAAA;
#else
const TInt KAvrcpBearerUid = KRemConBearerAvrcpImplementationUid;
#endif

const TInt KMaxRetries = 3;
// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCLinker::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTRCCLinker* CBTRCCLinker::NewL(MBTAccObserver& aAccObserver)
    {
    CBTRCCLinker* self = new (ELeave) CBTRCCLinker(aAccObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::CBTRCCLinker
// C++ constructor.
// -----------------------------------------------------------------------------
//
CBTRCCLinker::CBTRCCLinker(MBTAccObserver& aAccObserver)
    : CActive(EPriorityNormal), iAccObserver(aAccObserver)
	{
	CActiveScheduler::Add(this);
	}

// -----------------------------------------------------------------------------
// CBTRCCLinker::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::ConstructL()
	{
    TRACE_FUNC_ENTRY

    iInterfaceSelector = CRemConInterfaceSelector::NewL();
    iVolController = NULL;
    iRegisterVolumeChangeNotificationCounter = 0;
    if (iAccObserver.IsAvrcpVolCTSupported())
        {
        iAbsoluteVolController = CBTRCCAbsoluteVolumeLevelController::NewL(*iInterfaceSelector, *this);
        iLegacyVolController = CBTRCCLegacyVolumeLevelController::NewL(*iInterfaceSelector, *this);
        }
    else 
   		{
        // If volume control is not supported, we'll need another interface selector session for disconnecting. 
        iInterfaceSelectorForDisconnectingTargetSession = CRemConInterfaceSelector::NewL(); 
    	}
    
    iPlayerStarter = CPlayerStarter::NewL();
    iCoreTarget = CRemConCoreApiTarget::NewL(*iInterfaceSelector, *iPlayerStarter);
    iPlayerStarter->SetCoreTarget(*iCoreTarget);

    iRemConBatteryTgt = CRemConBatteryApiTarget::NewL(*iInterfaceSelector, *this);

//    iBrowsingAdapter = CBTRCCBrowsingAdapter::NewL(*iInterfaceSelector); 
    
    if (iAccObserver.IsAvrcpVolCTSupported()) 
        {
        iInterfaceSelector->OpenControllerL();
        }
    iInterfaceSelector->OpenTargetL(); 

    // The insert order matters
    LEAVE_IF_ERROR(iStateArray.Insert(new (ELeave) TStateIdle(*this), EStateIndexIdle));
    LEAVE_IF_ERROR(iStateArray.Insert(new (ELeave) TStateConnecting(*this), EStateIndexConnecting));
    LEAVE_IF_ERROR(iStateArray.Insert(new (ELeave) TStateConnected(*this), EStateIndexConnected));
    LEAVE_IF_ERROR(iStateArray.Insert(new (ELeave) TStateDisconnect(*this), EStateIndexDisconnect));
    ChangeState(EStateIndexIdle);
	TRACE_FUNC_EXIT
    }

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTRCCLinker::~CBTRCCLinker()
	{
	TRACE_FUNC_ENTRY
    if (iClientRequest)
        User::RequestComplete(iClientRequest, KErrAbort);
	
    delete iAbsoluteVolController;
    delete iLegacyVolController;
	delete iPlayerStarter;
	Cancel();
    iStateArray.ResetAndDestroy();
    iStateArray.Close();
	delete iInterfaceSelector;
	delete iInterfaceSelectorForDisconnectingTargetSession; 
	TRACE_FUNC_EXIT
	}

// -----------------------------------------------------------------------------
// CBTRCCLinker::Connect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::Connect(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    if ( !iAccObserver.IsAvrcpVolCTSupported() )
        {
        aStatus = KRequestPending;
        TRequestStatus* ptr = &aStatus;
        User::RequestComplete( ptr, KErrNotSupported );
        }
    else
        {
        TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
        iStateArray[iCurrentStateIndex]->Connect(aAddr, aStatus);
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::CancelConnect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::CancelConnect(const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
    iStateArray[iCurrentStateIndex]->CancelConnect(aAddr);
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::Disconnect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::Disconnect(TRequestStatus& aStatus, const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
    iStateArray[iCurrentStateIndex]->Disconnect(aStatus, aAddr);
   	}

// -----------------------------------------------------------------------------
// CBTRCCLinker::ActivateRemoteVolumeControl
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::ActivateRemoteVolumeControl()
    {
	TRACE_FUNC
    TRACE_ASSERT(iAccObserver.IsAvrcpVolCTSupported(), EBtrccPanicAvrcpVolCTNotSupported)
    if (!iRvcActivated)
        {
        iRvcActivated = ETrue;
        TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
        iStateArray[iCurrentStateIndex]->UpdateRemoteVolumeControling(iRvcActivated);
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::DeActivateRemoteVolumeControl
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::DeActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    TRACE_ASSERT(iAccObserver.IsAvrcpVolCTSupported(), EBtrccPanicAvrcpVolCTNotSupported)
    if (iRvcActivated)
        {
        iRvcActivated = EFalse;
        TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
        iStateArray[iCurrentStateIndex]->UpdateRemoteVolumeControling(iRvcActivated);
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::RunL
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::RunL()
	{
	TRACE_INFO((_L("CBTRCCLinker::RunL, %d"), iStatus.Int()))
    TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
    iStateArray[iCurrentStateIndex]->RemConRequestCompleted(iStatus.Int());
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::RunError
// -----------------------------------------------------------------------------
//
TInt CBTRCCLinker::RunError(TInt aError)
    {
    TRACE_INFO((_L("CBTRCCLinker::RunError, %d"), aError))
    (void) aError;
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::DoCancel
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::DoCancel()
    {
    TRACE_FUNC
    TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
    iStateArray[iCurrentStateIndex]->DoCancel();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::AccObserver
// -----------------------------------------------------------------------------
//
MBTAccObserver& CBTRCCLinker::AccObserver()
    {
    return iAccObserver;
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::DoConnect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::DoConnect()
    {
    TRACE_FUNC
	TRACE_ASSERT(!IsActive(), EBtrccPanicAOIsActive)
	DoRemConOrientation();
	if (iRemConOriented)
		{
    	iInterfaceSelector->ConnectBearer(iStatus);
    	SetActive();
		}
	else
	    {
        TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
        iStateArray[iCurrentStateIndex]->RemConRequestCompleted(KErrCouldNotConnect);	    
	    }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::DoSubscribeConnetionStatus
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::DoSubscribeConnetionStatus()
    {
    TRACE_FUNC
	TRACE_ASSERT(!IsActive(), EBtrccPanicAOIsActive)
	iInterfaceSelector->NotifyConnectionsChange(iStatus);
	SetActive();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::DoCancelSubscribe
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::DoCancelSubscribe()
    {
    TRACE_FUNC
    Cancel();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::DoDisconnect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::DoDisconnect()
    {
    TRACE_FUNC
	TRACE_ASSERT(!IsActive(), EBtrccPanicAOIsActive)
	TInt err = KErrNone;
    
    // if AVRCP RVC is supported, we use InterfaceSelector to do disconnect.
	if (iAccObserver.IsAvrcpVolCTSupported())
	    {
	    err = DoRemConOrientation();
	    }
	else
	    {
	    // if AVRCP RVC is not supported, we must be a RemCon Controller for the 
	    // disconnected. Use inner RemCon Controller here for not disturbing 
	    // interfaceSelector.
        TRAP(err, iInterfaceSelectorForDisconnectingTargetSession->OpenControllerL()); 
        TRACE_INFO((_L("open iInterfaceSelectorForDisconnectingTargetSession controller %d"), err))
        if (!err)
            {
            TRemConAddress addr;
            addr.BearerUid() = TUid::Uid( KAvrcpBearerUid );
            addr.Addr() = iRemoteAddr.Des();
            TRAP(err, iInterfaceSelectorForDisconnectingTargetSession->GoConnectionOrientedL(addr));
            TRACE_INFO((_L("InterfaceSelectorForDisconnectingTargetSession GoConnectionOrientedL %d"), err))
            }    
	    }
	
	if (!err)
		{
		if (iAccObserver.IsAvrcpVolCTSupported())
		    {
		    iInterfaceSelector->DisconnectBearer(iStatus);
		    }
		else
		    {
            iInterfaceSelectorForDisconnectingTargetSession->DisconnectBearer(iStatus);
		    }
    	SetActive();
		}
	else
	    {
        TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
        iStateArray[iCurrentStateIndex]->RemConRequestCompleted(KErrCouldNotConnect);	    
	    }
   	}

// -----------------------------------------------------------------------------
// CBTRCCLinker::ChangeState
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::ChangeState(TBTRCCStateIndex aNextState)
    {
	TRACE_INFO((_L("CBTRCCLinker::ChangeState, STATE from %d to %d"), iCurrentStateIndex, aNextState))
    iCurrentStateIndex = aNextState;
	TRACE_ASSERT(iCurrentStateIndex < iStateArray.Count(), EBtrccPanicOutOfRangeState);
    iStateArray[iCurrentStateIndex]->Enter();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::DoRemConOrientation
// -----------------------------------------------------------------------------
//
TInt CBTRCCLinker::DoRemConOrientation()
    {
    TRACE_FUNC
    TInt err(KErrNone);
	// the oriented remote might be a different device than the currently requested,
    // Go connectionless first.
	if (iRemConOriented)
	    {
	    TRAP(err, iInterfaceSelector->GoConnectionlessL());
    	iRemConOriented = EFalse;
	    if (err)
	        {
	        TRACE_INFO((_L("GoConnectionless ret %d"), err))
            return err;
	        }
	    }
	TRemConAddress addr;
	addr.BearerUid() = TUid::Uid( KAvrcpBearerUid );
	addr.Addr() = iRemoteAddr.Des();
    TRAP(err, iInterfaceSelector->GoConnectionOrientedL(addr));
    TRACE_INFO((_L("GoConnectionOriented ret %d"), err))
    if (!err)
        {
        iRemConOriented = ETrue;
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::DoGetRemConConnectionStatus
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::DoGetRemConConnectionStatus(RArray<TBTDevAddr>& aConnects)
    {
	TRACE_FUNC
	TSglQue<TRemConAddress> connections;
	aConnects.Reset();
	if (iInterfaceSelector->GetConnections(connections) == KErrNone)
		{
		TSglQueIter<TRemConAddress> iter(connections);
		TRemConAddress* addr;
		while((addr=iter++) != NULL)
			{
			if(addr->BearerUid() == TUid::Uid( KAvrcpBearerUid ) )
				{
				// We have a BT Connection 
				if ((addr->Addr().Length() == KBTDevAddrSize) && (TBTDevAddr(addr->Addr()) != TBTDevAddr()))
				    {
    				TRACE_INFO(_L("RemCon bearer found from connections list"));
    				aConnects.Append(TBTDevAddr(addr->Addr()));
				    }
				}
			// We've copied the information we need, so delete
			delete addr;
			}
		}
	else
		{
		TRACE_INFO(_L("GetNotificationStatus(): FAILED !!!"));
		}
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::StartRemoteVolumeControl
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::StartRemoteVolumeControl()
    {
    TRACE_FUNC
    if (iAccObserver.IsAvrcpVolCTSupported())
        {
        // Choose based on SDP result whether to create 
        // absolute controller or legacy controller.
        if(!iVolController)
            {
            if (iAccObserver.IsAbsoluteVolumeSupported(iRemoteAddr))
                {
                iVolController = iAbsoluteVolController;
                }
            else 
                {
                iVolController = iLegacyVolController;
                }
            }
        }
    if (iVolController)
        {
        iVolController->Start();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::StopRemoteVolumeControl
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::StopRemoteVolumeControl()
    {
    TRACE_FUNC
    if (iVolController)
        {
        iVolController->Stop();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::ResetVolmeControlSetting
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::ResetVolmeControlSetting()
    {
    TRACE_FUNC
    if (iVolController)
        {
        iVolController->Reset();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::VolumeControlError
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::VolumeControlError(TInt aError)
    {
    // This is called if there's an error with sending the volume commands out.
    switch(aError)
        {
        case ERegisterNotificationsFailed:
            if (iVolController && iRegisterVolumeChangeNotificationCounter<KMaxRetries)
                {
                iRegisterVolumeChangeNotificationCounter++;
                iVolController->RegisterVolumeChangeNotification();
                }
            else
                {
                iRegisterVolumeChangeNotificationCounter = 0;
                }
            break;
        case EVolumeAdjustmentFailed:
        default:
            {
            StopRemoteVolumeControl();
            StartRemoteVolumeControl();
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::MrcbstoBatteryStatus
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::MrcbstoBatteryStatus(TControllerBatteryStatus& aBatteryStatus)
	{
	RDebug::Printf("CBmbPlugin::MrcbstoBatteryStatus(), aBatteryStatus = %d",aBatteryStatus);
	TBool showBatteryNote(EFalse);

	TBTGenericInfoNotiferParamsPckg pckg;
	pckg().iRemoteAddr.Copy(iRemoteAddr.Des());
	switch(aBatteryStatus)
		{
		case EWarning:
		    pckg().iMessageType = ECmdShowBtBatteryLow;
		    showBatteryNote = ETrue;
		    break; 
		case ECritical:
		    pckg().iMessageType = ECmdShowBtBatteryCritical;
		    showBatteryNote = ETrue;
		    break; 
		default: 
		    break; 
		}

	if (showBatteryNote)
	    {
		RNotifier notifier;
		TInt err = notifier.Connect();	
		if (!err )
		    {
		    TRequestStatus status;
		    notifier.StartNotifierAndGetResponse(status, KBTGenericInfoNotifierUid, pckg, pckg);
		    User::WaitForRequest(status);
		    notifier.Close();
		    }
	    }
	}

// ================= TState MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCLinker::TState::TState
// -----------------------------------------------------------------------------
//
CBTRCCLinker::TState::TState(CBTRCCLinker& aParent)
:   iParent(aParent)
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TState::DoCancel
// -----------------------------------------------------------------------------
//  
void CBTRCCLinker::TState::DoCancel()
    {
    TRACE_INFO((_L("TState::DoCancel #Default# state %d"), iParent.iCurrentStateIndex))
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TState::Connect
// -----------------------------------------------------------------------------
//  
void CBTRCCLinker::TState::Connect(const TBTDevAddr& /*aAddr*/, TRequestStatus& aStatus)
    {
    TRACE_INFO((_L("TState::Connect #Default# state %d"), iParent.iCurrentStateIndex))
    TRequestStatus* status = &aStatus;
    aStatus = KRequestPending;
    User::RequestComplete(status, KErrInUse);
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TState::CancelConnect
// -----------------------------------------------------------------------------
//  
void CBTRCCLinker::TState::CancelConnect(const TBTDevAddr& /*aAddr*/)
    {
    TRACE_INFO((_L("TState::CancelConnect #Default# state %d"), iParent.iCurrentStateIndex))
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TState::Disconnect
// -----------------------------------------------------------------------------
//  
void CBTRCCLinker::TState::Disconnect(TRequestStatus& aStatus, const TBTDevAddr& /*aAddr*/)
    {
    TRACE_INFO((_L("TState::Disconnect #Default# state %d"), iParent.iCurrentStateIndex))
    // We assume disconnect is always succeeded and complete this request immediately,
    // because a) disconnect is not able to be cancelled, and
    //         b) we can do nothing if disconnect fails
    TRequestStatus* status = &aStatus;
    aStatus = KRequestPending;
    User::RequestComplete(status, KErrNone);
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TState::RemConRequestCompleted
// -----------------------------------------------------------------------------
//  
void CBTRCCLinker::TState::RemConRequestCompleted(TInt /*aErr*/)
    {
    TRACE_INFO((_L("TState::RemConRequestCompleted #Default# state %d"), iParent.iCurrentStateIndex))
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TState::UpdateRemoteVolumeControling
// -----------------------------------------------------------------------------
//  
void CBTRCCLinker::TState::UpdateRemoteVolumeControling(TBool /*aActivated*/)
    {
    TRACE_INFO((_L("TState::UpdateRemoteVolumeControling #Default# state %d"), iParent.iCurrentStateIndex))
    }

// ================= TStateIdle MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateIdle::TStateIdle
// -----------------------------------------------------------------------------
//
CBTRCCLinker::TStateIdle::TStateIdle(CBTRCCLinker& aParent)
    : TState(aParent)
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateIdle::Enter
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateIdle::Enter()
    {
    TRACE_FUNC
    iParent.iRemoteAddr = TBTDevAddr();
    iParent.DoSubscribeConnetionStatus();
    iParent.ResetVolmeControlSetting();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateIdle::DoCancel
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateIdle::DoCancel()
    {
    TRACE_FUNC
    iParent.iInterfaceSelector->NotifyConnectionsChangeCancel();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateIdle::Connect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateIdle::Connect(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    iParent.iRemoteAddr = aAddr;
    aStatus = KRequestPending;
    iParent.iClientRequest = &aStatus;
    iParent.DoCancelSubscribe();
    iParent.ChangeState(EStateIndexConnecting);
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateIdle::RemConRequestCompleted
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateIdle::RemConRequestCompleted(TInt aErr)
    {
    TRACE_FUNC
    if (!aErr)
        {
        RArray<TBTDevAddr> connects;
        iParent.DoGetRemConConnectionStatus(connects);
        
        // There can only be maximum one AVRCP connection at this time, as BTRCC is running 
        // as long as BT is ON. 
        if (connects.Count())
            {
            iParent.iRemoteAddr = connects[0];
            if (iParent.iAccObserver.IsAvrcpVolCTSupported())
                {
                iParent.DoRemConOrientation();
                }
            iParent.ChangeState(EStateIndexConnected);
            // This function call is safe after state transition because the state machine
            // keeps all state instances in memory.
            // AVRCP Controller initiates connection with AVRCP Traget. Thus we assume the remote
            // is a CT.
            iParent.AccObserver().NewAccessory(iParent.iRemoteAddr, ERemConCT);
            }
        else
            {
            // Remain in this state, re-subsrcibe
            iParent.DoSubscribeConnetionStatus();
            }
        connects.Close();
        }
    else if (aErr != KErrServerTerminated)
        {
        // some error returned in subscribe, redo if the reason is other than server termination.
        iParent.DoSubscribeConnetionStatus();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnecting::TStateConnecting
// -----------------------------------------------------------------------------
//
CBTRCCLinker::TStateConnecting::TStateConnecting(CBTRCCLinker& aParent)
    : TState(aParent)
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnecting::Enter
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateConnecting::Enter()
    {
    TRACE_FUNC
    iConnectCanceled = EFalse;
    iParent.DoConnect();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnecting::DoCancel
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateConnecting::DoCancel()
    {
    TRACE_FUNC
    iParent.iInterfaceSelector->ConnectBearerCancel();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnecting::CancelConnect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateConnecting::CancelConnect(const TBTDevAddr& aAddr)
    {
    TRACE_INFO(_L("TStateConnecting::CancelConnect"))
    if (aAddr == iParent.iRemoteAddr)
        {
        // RemCon FW doesn't bring down AVRCP linking in ConnectBearerCancel(), so
        // we set the flag which will be checked when the completion of connecting request.
        iConnectCanceled = ETrue;
        if (iParent.iClientRequest)
            User::RequestComplete(iParent.iClientRequest, KErrCancel);
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnecting::RemConRequestCompleted
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateConnecting::RemConRequestCompleted(TInt aErr)
    {
    TRACE_FUNC
    TBTRCCStateIndex nextState = (aErr) ? EStateIndexIdle : EStateIndexConnected;
    if (!aErr && iConnectCanceled)
        {
        nextState =  EStateIndexDisconnect;
        aErr = KErrCancel;
        }
    if (iParent.iClientRequest)
        User::RequestComplete(iParent.iClientRequest, aErr);
    iParent.ChangeState(nextState);
    }

// ================= TStateConnected MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnected::TStateConnected
// -----------------------------------------------------------------------------
//
CBTRCCLinker::TStateConnected::TStateConnected(CBTRCCLinker& aParent)
    : TState(aParent)
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnected::Enter
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateConnected::Enter()
    {
    TRACE_FUNC
    iParent.DoSubscribeConnetionStatus();
    if (iParent.iRvcActivated)
        {
        iParent.StartRemoteVolumeControl();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnected::DoCancel
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateConnected::DoCancel()
    {
    TRACE_FUNC
    iParent.iInterfaceSelector->NotifyConnectionsChangeCancel();
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnected::Connect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateConnected::Connect(const TBTDevAddr& aAddr, TRequestStatus& aStatus)
    {
    TRACE_FUNC
    TInt err = KErrNone;
    if (iParent.iRemoteAddr != aAddr)
        {
        err = KErrInUse;
        }
    TRequestStatus* status = &aStatus;
    aStatus = KRequestPending;
    User::RequestComplete(status, err);
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnected::Disconnect
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateConnected::Disconnect(TRequestStatus& aStatus, const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TState::Disconnect(aStatus, aAddr);
    if (iParent.iRemoteAddr == aAddr)
        {
        iParent.DoCancelSubscribe();
   	    iParent.StopRemoteVolumeControl();
    	iParent.ChangeState(EStateIndexDisconnect);
        }
    }

// -----------------------------------------------------------------------------
// Find
// -----------------------------------------------------------------------------
//
TBool Find(const RArray<TBTDevAddr>& aList, const TBTDevAddr& aAddr)
    {
    TInt count = aList.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (aList[i] == aAddr)
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnected::RemConRequestCompleted
// -----------------------------------------------------------------------------
//    
void CBTRCCLinker::TStateConnected::RemConRequestCompleted(TInt aErr)
    {
    TRACE_FUNC
    if (!aErr)
        {
        RArray<TBTDevAddr> connects;
        iParent.DoGetRemConConnectionStatus(connects);
        
        // There might be more than one AVRCP connections at this time, 
        // We only check if the connection maintained by BTRCC exists or not.
        // For connection with other devices, we are not interested since currently
        // BT audio design only allows single HFP/A2DP/AVRCP connection.
        if (!Find(connects, iParent.iRemoteAddr))
            {
            iParent.AccObserver().AccessoryDisconnected(iParent.iRemoteAddr, EAnyRemConProfiles);
            iParent.StopRemoteVolumeControl();
            iParent.ChangeState(EStateIndexIdle);
            }
        else 
            {
            // not interested connection status change, remain in this state and re-subscribe
            iParent.DoSubscribeConnetionStatus();
            }
        connects.Close();
        }
    else if (aErr == KErrServerTerminated || aErr == KErrCommsBreak)
        {
        // Serious error, requires a restart. 
        // ToDo: Check and possibly redesign the connection/disconnection
        // To enable it from this class.

        // Must at least inform the parent that there was a disconnection, 
        // Or should we just connect back silently?

        // iRemoteAddr stores the address so this is possible in theory. 
        }
    else 
        {
        // Other error, subscribe again. 
        iParent.DoSubscribeConnetionStatus();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateConnected::UpdateRemoteVolumeControling
// -----------------------------------------------------------------------------
//    
void CBTRCCLinker::TStateConnected::UpdateRemoteVolumeControling(TBool aActivated)
    {
    TRACE_FUNC
    if (aActivated)
        {
        iParent.StartRemoteVolumeControl();
        }
    else
        {
        iParent.StopRemoteVolumeControl();
        }    
    }


// ================= TStateDisconnect MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateDisconnect::TStateDisconnect
// -----------------------------------------------------------------------------
//
CBTRCCLinker::TStateDisconnect::TStateDisconnect(CBTRCCLinker& aParent)
    : TState(aParent)
    {
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateDisconnect::Enter
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateDisconnect::Enter()
    {
    TRACE_FUNC
    iParent.DoDisconnect();
    }
    
// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateDisconnect::DoCancel
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateDisconnect::DoCancel()
    {
    TRACE_FUNC
    if (iParent.iAccObserver.IsAvrcpVolCTSupported())
        {    
        iParent.iInterfaceSelector->DisconnectBearerCancel();
        }
    else
        {
        iParent.iInterfaceSelectorForDisconnectingTargetSession->DisconnectBearerCancel();
        }
    }

// -----------------------------------------------------------------------------
// CBTRCCLinker::TStateDisconnect::RemConRequestCompleted
// -----------------------------------------------------------------------------
//
void CBTRCCLinker::TStateDisconnect::RemConRequestCompleted(TInt /*aErr*/)
    {
    TRACE_FUNC
    iParent.ChangeState(EStateIndexIdle);
    }

//  End of File  
