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
*     This class is a BTSapServer's state for setting up connection with BTSap client

*
*/



// INCLUDE FILES
#include <e32property.h>
#include <ctsydomainpskeys.h>
#include <PSVariables.h>
#include "BTSapServerState.h"
#include "BTSapSocketHandler.h"
#include "BTSapSimCardStatusNotifier.h"
#include "debug.h"


CBTSapServerState::TStateConnect::TStateConnect(CBTSapServerState& aServerState)
    : TStateIdle(aServerState), iConnectRequestOK(EFalse), iCardStatus(ECardStatusReserved)
    {
    }

// ---------------------------------------------------------
// Enter
// ---------------------------------------------------------
void CBTSapServerState::TStateConnect:: Enter(TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateConnect: Enter")));

    iStatus = &aStatus;
    TConnectionStatus connectionStatus = EConnectionErrReject;
    if(!IsCallOngoing())
        {   // SAP cannot be accepted if a call is ongoing or if no SIM is present
        connectionStatus = EConnectionOK;
        }

    if (connectionStatus == EConnectionOK)
        {
        CheckMaxMsgSize(connectionStatus);
        }

    if (connectionStatus == EConnectionOK)
        {
        iConnectRequestOK = ETrue;

        // init value which is impossible to be received.
        // by checking the value, BTSap knows if status_ind is received
        iCardStatus = ECardStatusReserved;
        // listen to status_ind (card_reset is expected)
        iServerState.BTSapSimCardStatusNotifier().Start();

        NotifySapState(ESapConnecting);

        // Waiting for Accept/RejectSapConnection()
        aStatus = KRequestPending; 

        if(!IsSimPresent())
            {
            // If there is no SIM present, we bypass the accept/reject for the connection.
            // Instead this will be signalled in the connection reply, followed by disconnection.
            iCardStatus = ECardStatusRemoved;
            User::RequestComplete(iStatus, EUserAccepted);
            }
#ifdef __WINS__        

        //Doesn't wait for any confirmation (used in WINS)        
        BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateConnect: Enter Accept")));
        User::RequestComplete(iStatus, EUserAccepted); 
        
#endif //__WINS__

        }
    else
        {
        iConnectRequestOK = EFalse;

        iResponseMessage.SetMsgID(EConnectResponse);
        iResponseMessage.AddParameter(EParaConnectionStatus, connectionStatus);

        if (connectionStatus == EConnectionErrNotSupported)
            {
            iResponseMessage.AddParameter(EParaMaxMsgSize, KMaxMsgSize, 2);
            }

        iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());

        if (connectionStatus == EConnectionErrNotSupported)
            {
            // waiting for the client to send another conn_req with my KMaxMsgSize
            aStatus = KRequestPending; 
            }
        else
            {
            // too small or a phone call ongoing
            User::RequestComplete(iStatus, EConnectionError);
            }
        }
    }

// --------------------------------------------------------------------------------------------
// Complete
// set next state
// --------------------------------------------------------------------------------------------
//
TBTSapServerState CBTSapServerState::TStateConnect::Complete(TInt aReason)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_STM, BTSapPrintTrace(_L("[BTSap]  SM: TStateConnect: Complete")));

    TBTSapServerState nextState = EStateNotConnected;

    if (aReason == EUserAccepted || aReason == EUserRejected)
        {
        TInt connectionStatus = (aReason == EUserAccepted) ? EConnectionOK : EConnectionErrReject;

        iResponseMessage.SetMsgID(EConnectResponse);
        iResponseMessage.AddParameter(EParaConnectionStatus, connectionStatus);
        iServerState.BTSapSocketHandler().Send(iResponseMessage.Data());

        if (connectionStatus == EConnectionOK)
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  SM: TStateConnect: Complete: iCardStatus: %d"), iCardStatus));

            if (iCardStatus != ECardStatusReserved)
                {
                // send status_ind
                TStateIdle::SimCardStatusChanged(iCardStatus);
                }

            if(iCardStatus != ECardStatusRemoved)
                {
                // There is no SIM, so the connection will be disconnected by the client.
                // We therefore do not signal a connection complete.
                NotifySapState(ESapConnected);
                }
            nextState = EStateIdle;
            }
        }

    return nextState;
    }

void CBTSapServerState::TStateConnect::Cancel()
    {
    NotifySapState(ESapNotConnected);
    User::RequestComplete(iStatus, KErrCancel);
    }

TBool CBTSapServerState::TStateConnect::IsCallOngoing()
    {
#ifdef __WINS__

	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::TStateConnect::IsCallOngoing ON THE WINS SIDE")));
	return EFalse;
	
#else    
    
    TBool retVal = EFalse;
    
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::TStateConnect::IsCallOngoing")));

    TInt callState;
    // Try to get the call state property
    TInt err = RProperty::Get(KPSUidCtsyCallInformation, KCTsyCallState, callState);

    // Check if retrieving the property value succeeded
    if(err == KErrNone)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TStateConnect: callState: %d"), callState));

        // If callState is EPSTelephonyCallStateNone or EPSTelephonyCallStateUninitialized, there's no ongoing call
        retVal = (callState != EPSCTsyCallStateNone) && (callState != EPSCTsyCallStateUninitialized);
        }
    else
        {
        // Couldn't retrieve call state property
        BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  TStateConnect: Couldn't get callState!!! (err = %d)"), err));
        retVal = ETrue; // Assume there was an ongoing call to abort the SAP connection
        }

    return retVal;
#endif // __WINS__
    }

void CBTSapServerState::TStateConnect::CheckMaxMsgSize(TConnectionStatus& aMsgSizeStatus)
    {
    TInt msgSize;
    iServerState.BTSapRequestMessage().GetParameter(EParaMaxMsgSize, msgSize);
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  TStateConnect: msg size: %d"), msgSize));

    if (msgSize < KMinMsgSize)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  SM: TStateConnect: msg size too small ***")));
        aMsgSizeStatus = EConnectionErrTooSmall;
        }
    else if (msgSize > KMaxMsgSize)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  SM: TStateConnect: msg size too big ***")));
        aMsgSizeStatus = EConnectionErrNotSupported;
        }
    else
        {
        aMsgSizeStatus = EConnectionOK;
        }
    }

TInt CBTSapServerState::TStateConnect::AcceptSapConnection()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  SM: TStateConnect: AcceptSapConnection")));

    if (*iStatus == KRequestPending)
        {
        User::RequestComplete(iStatus, EUserAccepted);
        }

    return KErrNone;
    }

TInt CBTSapServerState::TStateConnect::RejectSapConnection(TBTSapRejectReason /*aReason*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  SM: TStateConnect: RejectSapConnection")));

    if (*iStatus == KRequestPending)
        {
        User::RequestComplete(iStatus, EUserRejected);
        }

    return KErrNone;
    }

TInt CBTSapServerState::TStateConnect::ChangeState(TBTSapServerState& aNextState)
    {
    TInt retVal = KErrNotSupported;
    TBTSapServerState nextState = EStateNotConnected;

    if ((aNextState == EStateConnect &&
        *iStatus == KRequestPending &&
        !iConnectRequestOK) ||
        aNextState == EStateDisconnect)
        {
        retVal = KErrNone;
        nextState = aNextState;
        }

    aNextState = nextState;
    return retVal;
    }

void CBTSapServerState::TStateConnect::SimCardStatusChanged(TCardStatus aCardStatus)
    {
    iCardStatus = aCardStatus;
    }

//  End of File  
