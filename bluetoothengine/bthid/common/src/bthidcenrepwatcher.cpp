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
* Description: 
 *       This class handles intereted CenRep notification.
 *
*/


// INCLUDE FILES
#include <centralrepository.h>
#include "bthidcenrepwatcher.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================


// ---------------------------------------------------------
// CBtHidCenRepWatcher::NewL
// ---------------------------------------------------------
//
CBtHidCenRepWatcher* CBtHidCenRepWatcher::NewL(TUid aUid,
        MBtHidCenRepObserver& aObserver)
    {
    CBtHidCenRepWatcher* self = new (ELeave) CBtHidCenRepWatcher(aUid,
            aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CBtHidCenRepWatcher::~CBtHidCenRepWatcher
// ---------------------------------------------------------
//
CBtHidCenRepWatcher::~CBtHidCenRepWatcher()
    {
        TRACE_INFO((_L("[BTHID]\t CBtHidCenRepWatcher[0x%08x]::~CBtHidCenRepWatcher >>"),this));
    if (iNotifier)
        {
        iNotifier->StopListening();
        }
    delete iNotifier;
    delete iSession;
        TRACE_INFO(_L("[BTHID]\t CBtHidCenRepWatcher::~CBtHidCenRepWatcher <<"));
    }

// ---------------------------------------------------------
// CBtHidCenRepWatcher::CBtHidCenRepWatcher
// C++ default constructor can NOT contain any code that
// might leave.
// ---------------------------------------------------------
//
CBtHidCenRepWatcher::CBtHidCenRepWatcher(TUid aUid,
        MBtHidCenRepObserver& aObserver) :
    iUid(aUid), iObserver(aObserver)
    {
    }

// ---------------------------------------------------------
// CBtHidCenRepWatcher::ConstructL
// ---------------------------------------------------------
//
void CBtHidCenRepWatcher::ConstructL()
    {
        TRACE_INFO((_L("[BTHID]\t CBtHidCenRepWatcher[0x%08x]::ConstructL: Uid: 0x%08x"), this, iUid.iUid));
    iNotificationsEnabled = ETrue;
    iSession = CRepository::NewL(iUid);
    iNotifier = CCenRepNotifyHandler::NewL(*this, *iSession);
    iNotifier->StartListeningL();
        TRACE_INFO ((_L("[BTHID]\t CBtHidCenRepWatcher[0x%08x]::ConstructL, END"),this));
    }

// ---------------------------------------------------------
// CBtHidCenRepWatcher::HandleNotifyGeneric
//
// ---------------------------------------------------------
//
void CBtHidCenRepWatcher::HandleNotifyGeneric(TUint32 aId)
    {
        TRACE_INFO((_L("[BTHID]\t CBtHidCenRepWatcher[0x%08x]::HandleNotifyGeneric"),this));
    if (iNotificationsEnabled)
        iObserver.CenRepDataChanged(iUid, aId);
    }

// ---------------------------------------------------------
// CBtHidCenRepWatcher::HandleNotifyError
// ---------------------------------------------------------
//
void CBtHidCenRepWatcher::HandleNotifyError(TUint32 /*aId*/, TInt /*error*/,
        CCenRepNotifyHandler* /*aHandler*/)
    {
        TRACE_INFO((_L("[BTHID]\t CBtHidCenRepWatcher[0x%08x]::HandleNotifyError"),this));
    iNotifier->StopListening();
    }

// -----------------------------------------------------------------------------
// CBtHidCenRepWatcher::DisableNotifications
// -----------------------------------------------------------------------------
//
void CBtHidCenRepWatcher::DisableNotifications()
    {
        TRACE_INFO((_L("[BTHID]\t CBtHidCenRepWatcher::DisableNotifications")));
    iNotificationsEnabled = EFalse;
    }

// -----------------------------------------------------------------------------
// CBtHidCenRepWatcher::EnableNotifications
// -----------------------------------------------------------------------------
//
void CBtHidCenRepWatcher::EnableNotifications()
    {
        TRACE_INFO((_L("[BTHID]\t CBtHidCenRepWatcher::EnableNotifications")));
    iNotificationsEnabled = ETrue;
    }

//  End of File
