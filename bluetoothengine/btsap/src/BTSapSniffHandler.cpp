/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
*     This class implements a handler, which sets the physical link to
*     sniff-mode when the SAP link is inactive for some time
*
*/


// INCLUDE FILES
#include "BTSapSniffHandler.h"
#include "debug.h"

// Sniff timer interval in microseconds
const TInt KSniffInterval = 3000000; // 3 seconds

CBTSapSniffHandler::CBTSapSniffHandler(RSocket& aSocket, RSocketServ& aSocketServer)
    : CTimer(EPriorityStandard),
      iSocket(aSocket),
      iSocketServer(aSocketServer),
      iIsLinkAdapterOpen(EFalse)
    {
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler")));
	
	CActiveScheduler::Add(this);
    }

CBTSapSniffHandler::~CBTSapSniffHandler()
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  ~CBTSapSniffHandler")));

    if(iIsLinkAdapterOpen)
        {
        // Now we can allow sniff-mode again
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  ~CBTSapSniffHandler Allowing LowPowerModes")));
        TInt err = iBTLinkAdapter.AllowLowPowerModes(ESniffMode);
        if(err != KErrNone)
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  ~CBTSapSniffHandler: Couldn't allow sniff-mode %d"), err));
            }
        iBTLinkAdapter.Close();
        }

    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  ~CBTSapSniffHandler Stopping the sniff-timer")));
	Cancel();
	}


// ---------------------------------------------------------
// NewL()
// ---------------------------------------------------------
//
CBTSapSniffHandler* CBTSapSniffHandler::NewL(RSocket& aSocket, RSocketServ& aSocketServer)
	{
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::NewL")));

	CBTSapSniffHandler* self = new (ELeave) CBTSapSniffHandler(aSocket, aSocketServer);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

// ---------------------------------------------------------
// Enable
// ---------------------------------------------------------
//
void CBTSapSniffHandler::Enable()
    {
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Enable")));
    TInt err;

    if(!iIsLinkAdapterOpen)
        {
        err = iBTLinkAdapter.Open(iSocketServer, iSocket);
        if(err != KErrNone)
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Enable: Couldn't open iBTLinkAdapter %d")));
            }
        else
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Enable: iBTLinkAdapter opened")));
            iIsLinkAdapterOpen = ETrue;
            }
        }

    if(iIsLinkAdapterOpen)
        {
        // Start the sniffer timer
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Enable: Starting the sniff timer %d microseconds"), KSniffInterval));
        Cancel();
        After(KSniffInterval);

        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Enable: Cancelling LowPowerModeRequester")));
        err = iBTLinkAdapter.CancelLowPowerModeRequester();
        if(err == KErrNone)
            {        
            // We don't want to go to sniff-mode while our timer is running, so we prevent low-power modes here
            BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Enable: Preventing LowPowerModes")));
            err = iBTLinkAdapter.PreventLowPowerModes(EAnyLowPowerMode);
            if(err != KErrNone)
                {
                BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Enable: Couldn't prevent low-power modes %d"), err));
                }
            }
        else
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Enable: Couldn't cancel sniff requester %d"), err));
            }
        }
    }

// ---------------------------------------------------------
// Disable
// ---------------------------------------------------------
//
void CBTSapSniffHandler::Disable()
    {
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Disable")));

    // Stop the timer
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Disable: Stopping the sniff-timer")));
    Cancel();

    // If the adapter is not open, we don't need to do anything
    if(iIsLinkAdapterOpen)
        {
        // When the sniffer is disabled, we should allow sniff-mode again
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Disable: Allowing LowPowerModes")));
        TInt err = iBTLinkAdapter.AllowLowPowerModes(ESniffMode);
        if(err != KErrNone)
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Disable: Couldn't allow sniff-mode %d"), err));
            }
        // And then close the physical link adapter
        iBTLinkAdapter.Close();
        iIsLinkAdapterOpen = EFalse;
        }
    }

// ---------------------------------------------------------
// Pause
// ---------------------------------------------------------
//
void CBTSapSniffHandler::Pause()
    {
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Pause")));
    TInt err;

    // Stop the timer
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Pause: Stopping the sniff-timer")));
    Cancel();

    if(iIsLinkAdapterOpen)
        {
        // When the sniffer is paused, we are probably handling some requests, so we prevent sniff-mode
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Pause: Preventing LowPowerModes")));
        err = iBTLinkAdapter.PreventLowPowerModes(EAnyLowPowerMode);
        if(err != KErrNone)
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::Pause: Couldn't prevent low-power modes %d"), err));
            }
        }
    }

// ---------------------------------------------------------
// RunL
// ---------------------------------------------------------
//
void CBTSapSniffHandler::RunL()
	{
	TInt status = iStatus.Int();
	BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler: RunL: %d"), status));

    TInt err;

    if(iIsLinkAdapterOpen)
        {
        // Now we can allow sniff-mode again
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::RunL: Allowing LowPowerModes")));
        err = iBTLinkAdapter.AllowLowPowerModes(ESniffMode);
        if(err == KErrNone)
            {
            // Start the sniff-mode requester
            BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::RunL: Activating SniffRequester")));
            err = iBTLinkAdapter.ActivateSniffRequester();
            if(err != KErrNone)
                {
                BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::RunL: Couldn't activate the sniff requester %d"), err));
                }
            }
        else
            {
            BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  CBTSapSniffHandler::RunL: Couldn't allow sniff-mode %d"), err));
            }
        }
	}

//  End of File  
