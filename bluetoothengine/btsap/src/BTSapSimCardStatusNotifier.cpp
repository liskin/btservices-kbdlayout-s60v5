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
*     This class handles SIM card status change notification
*
*
*/


// INCLUDE FILES
#include "BTSapSimCardStatusNotifier.h"
#include "BTSapServerState.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CBTSapSimCardStatusNotifier::CBTSapSimCardStatusNotifier(CBTSapServerState& aBTSapServerState)
    : CActive(EPriorityNormal), 
      iBTSapServerState(aBTSapServerState),
      iSubscriptionModule(aBTSapServerState.SubscriptionModule())
    {
    CActiveScheduler::Add(this);
    }

// Destructor.
CBTSapSimCardStatusNotifier::~CBTSapSimCardStatusNotifier()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  ~CBTSapSimCardStatusNotifier")));
	Cancel();
    }

// ---------------------------------------------------------
// NewL()
// ---------------------------------------------------------
//
CBTSapSimCardStatusNotifier* CBTSapSimCardStatusNotifier::NewL(CBTSapServerState& aBTSapServerState)
    {
    CBTSapSimCardStatusNotifier* self = new (ELeave) CBTSapSimCardStatusNotifier(aBTSapServerState);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// ConstructL
// ---------------------------------------------------------
//
void CBTSapSimCardStatusNotifier::ConstructL()
    {
    }

// ---------------------------------------------------------
// DoCancel
// ---------------------------------------------------------
//
void CBTSapSimCardStatusNotifier::DoCancel()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSimCardStatusNotifier: DoCancel")));
    iSubscriptionModule.CancelAsyncRequest(ECustomNotifySimCardStatusIPC);
    }

// ---------------------------------------------------------
// RunL
// ---------------------------------------------------------
//
void CBTSapSimCardStatusNotifier::RunL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSimCardStatusNotifier: RunL: %d, CardStatus: %d"), iStatus.Int(), iCardStatus));
    iBTSapServerState.SimCardStatusChanged(TCardStatus(iCardStatus));

    iSubscriptionModule.NotifySimCardStatus(iStatus, iCardStatus);
    SetActive();
    }

// ---------------------------------------------------------
// Start
// ---------------------------------------------------------
void CBTSapSimCardStatusNotifier::Start()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTSapSimCardStatusNotifier: Start")));

    iSubscriptionModule.NotifySimCardStatus(iStatus, iCardStatus);
    SetActive();
    }

//  End of File  
