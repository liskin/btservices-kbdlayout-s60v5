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
* Description:  The base active class definition 
*
*/


#include "basrvactivenotifier.h"
#include "debug.h"

// ======== MEMBER FUNCTIONS ========

CBasrvActiveNotifier* CBasrvActiveNotifier::NewL(MBasrvActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId, const TBTDevAddr& aAddr, TBTGenericInfoNoteType aNote)
    {
    CBasrvActiveNotifier* self = new (ELeave) CBasrvActiveNotifier(
        aObserver, aPriority, aRequestId, aAddr, aNote);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBasrvActiveNotifier::~CBasrvActiveNotifier()
    {
    Cancel();
    iNotifier.Close();
    TRACE_FUNC
    }

void CBasrvActiveNotifier::GoActive()
    {
    }

void CBasrvActiveNotifier::DoCancel()
    {
    iNotifier.CancelNotifier(KBTGenericInfoNotifierUid);
    TRACE_INFO((_L("Service %d cancelled"), RequestId()))
    }
    
CBasrvActiveNotifier::CBasrvActiveNotifier(MBasrvActiveObserver& aObserver, 
    CActive::TPriority aPriority, TInt aRequestId, const TBTDevAddr& aAddr, TBTGenericInfoNoteType aNote)
    : CBasrvActive(aObserver, aPriority, aRequestId)
    {
    TRACE_FUNC
    iPckg().iRemoteAddr.Copy( aAddr.Des() );
    iPckg().iMessageType = aNote;
    }

void CBasrvActiveNotifier::ConstructL()
    {
    LEAVE_IF_ERROR(iNotifier.Connect());
    iNotifier.StartNotifierAndGetResponse(iStatus, KBTGenericInfoNotifierUid, iPckg, iNoResult);
    SetActive();
    }

