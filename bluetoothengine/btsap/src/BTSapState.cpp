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
*     This class is a base state class for all BTSap states
*
*
*/



// INCLUDE FILES
#include <e32property.h>
#include <startupdomainpskeys.h>
#include "BTSapDomainPSKeys.h"

#include <btnotif.h>
#include "BTSapServerState.h"
#include "debug.h"

CBTSapServerState::TState::TState(CBTSapServerState& aServerState)
:   iSimStatus(0),
    iServerState(aServerState),
    iResponseMessage(aServerState.BTSapResponseMessage()),
    iStatus(NULL)
    {
    }

TInt CBTSapServerState::TState::AcceptSapConnection()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  TState: AcceptSapConnection: Not Ready#")));
    return KErrNotReady;
    }

TInt CBTSapServerState::TState::RejectSapConnection(TBTSapRejectReason /*aReason*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  TState: RejectSapConnection: Not Ready#")));
    return KErrNotReady;
    }

TInt CBTSapServerState::TState::DisconnectSapConnection(TBTSapDisconnectType /*aType*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  TState: DisconnectSapConnection: Not Ready#")));
    return KErrNotReady;
    }

void CBTSapServerState::TState::SimCardStatusChanged(TCardStatus /*aCardStatus*/)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  TState: SimCardStatusChanged: Not Ready#")));
    }

TInt CBTSapServerState::TState::ChangeState(TBTSapServerState& aNextState)
    {
    TInt retVal = KErrNone;

    if (aNextState != EStatePowerOff &&
        aNextState != EStateReset &&
        aNextState != EStateDisconnect)
        {
        aNextState = EStateIdle;
        retVal = KErrNotSupported;
        }

    return retVal;
    }

void CBTSapServerState::TState::NotifySapState(TSapConnectionState aState)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  NotifySapState: %d"), aState));

    TBool simPresent = IsSimPresent();

    switch(aState)
        {
        case ESapNotConnected:
            {
            // If the next state is ESapNotConnected, we need to verify the previous state
            TInt previousState;
            RProperty::Get(KPSUidBluetoothSapConnectionState, KBTSapConnectionState, previousState);

            // Only show NotConnected if the previous state was connecting (ie. connection failed)
            if(previousState == ESapConnecting)
                {
                if( simPresent )
                    {
                    StartBTNotifier(EBTSapFailed);
                    }
                else
                    {
                    // Notify the user of the specific reason that connection 
                    // could not be completed: there is no SIM present
                    StartBTNotifier(EBTSapNoSim);
                    }
                }
            break;
            }

        case ESapConnecting:
            {
            if( simPresent )
                {
                // Don't show the note if no SIM is present
                StartBTNotifier(EBTEnterSap);
                }
            break;
            }
        case ESapConnected:
            StartBTNotifier(EBTSapOk);
            break;

        default:
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  Invalid state!!! %d"), aState));
            break;
        }

    RProperty::Set(KPSUidBluetoothSapConnectionState, KBTSapConnectionState, aState);
    }

void CBTSapServerState::TState::StartBTNotifier(TInt note)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  StartBTNotifier: %d"), note));

    RNotifier notifier;
    TInt err = notifier.Connect();
    if(err != KErrNone)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  NotifySapState: Couldn't connect to notifier: %d"), err));
        }
    else
        {
        TBTGenericInfoNotiferParamsPckg type;
        type().iMessageType = (TBTGenericInfoNoteType) note;

        err = notifier.StartNotifier(KBTGenericInfoNotifierUid, type);

        if (err != KErrNone)
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  NotifySapState: Couldn't start notifier: err = %d"), err));
            }

        notifier.Close();
        }
    }

TBool CBTSapServerState::TState::IsSimPresent()
    {
#ifdef __WINS__

	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::TStateConnect::IsSimPresent ON THE WINS SIDE")));
	return ETrue;

#else   //__WINS__

    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapServerState::TStateConnect::IsSimPresent, SimStatus: %d"), iSimStatus));

    TInt err = KErrNone;
    if( iSimStatus == 0 )
        {
        err = RProperty::Get(KPSUidStartup, KPSSimStatus, iSimStatus);
        }
    TBool retVal = !(err || iSimStatus == ESimNotPresent || iSimStatus == ESimNotSupported);

    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  TStateConnect::IsSimPresent: err = %d, retVal = %d"), err, retVal));
    return retVal;

#endif  //__WINS__
    }
//  End of File  
