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
* Description:  Declares disk status watcher class for ObexUtils.
*
*/


// INCLUDE FILES

#include "obexutilspropertynotifier.h"
#include "obexutilsdebug.h"

// ================= MEMBER FUNCTIONS =======================


EXPORT_C CObexUtilsPropertyNotifier* CObexUtilsPropertyNotifier::NewL(
    MObexUtilsPropertyNotifyHandler* aHandler,
    TMemoryPropertyCheckType aCheckType )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::NewL()"));
    CObexUtilsPropertyNotifier* self=new(ELeave) CObexUtilsPropertyNotifier(aHandler, aCheckType);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::NewL() completed"));
    return self;
    }

// ---------------------------------------------------------
// CObexUtilsPropertyNotifier::CObexUtilsPropertyNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ---------------------------------------------------------
//
CObexUtilsPropertyNotifier::CObexUtilsPropertyNotifier(
    MObexUtilsPropertyNotifyHandler* aHandler,
    TMemoryPropertyCheckType aCheckType )
    : CActive( EPriorityNormal ), iHandler ( aHandler ), iCheckType ( aCheckType )
    {
    CActiveScheduler::Add(this);
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::CObexUtilsPropertyNotifier() completed"));
    }

// ---------------------------------------------------------
// Destructor.
// ---------------------------------------------------------
//
CObexUtilsPropertyNotifier::~CObexUtilsPropertyNotifier()
    {
    Cancel();
    iProperty.Close();
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::~CObexUtilsPropertyNotifier() completed"));
    }

// ---------------------------------------------------------
// CObexUtilsPropertyNotifier::ConstructL
// ---------------------------------------------------------
//
void CObexUtilsPropertyNotifier::ConstructL()
    {   
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::ConstructL()"));
    if (iCheckType == ECheckPhoneMemory)
        {
        FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::ConstructL() Phone"));
        User::LeaveIfError(iProperty.Attach(KPSUidUikon,KUikFFSFreeLevel));
        }
    else
        {
        FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::ConstructL() MMC"));
        User::LeaveIfError(iProperty.Attach(KPSUidUikon,KUikMmcFFSFreeLevel));
        }
    Subscribe();
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::ConstructL() completed"));
    }

// ---------------------------------------------------------
// CObexUtilsPropertyNotifier::Subscribe
// ---------------------------------------------------------
//
void CObexUtilsPropertyNotifier::Subscribe()
    {
    iProperty.Subscribe(iStatus);
    SetActive();
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::Subscribe() completed"));
    }

// ---------------------------------------------------------
// CObexUtilsPropertyNotifier::DoCancel
// ---------------------------------------------------------
//
void CObexUtilsPropertyNotifier::DoCancel()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::DoCancel()"));
    iProperty.Cancel();
    }

// ---------------------------------------------------------
// CObexUtilsPropertyNotifier::RunL
// ---------------------------------------------------------
//
void CObexUtilsPropertyNotifier::RunL()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::RunL()"));
    
    Subscribe();
    
    TInt memorystatus;
    if ((iProperty.Get(memorystatus)!=KErrNotFound) && (memorystatus!=0))
        {
            FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::RunL() notify"));
            TRAPD(retTrap, iHandler->HandleNotifyL( iCheckType ));
            retTrap = retTrap;
            FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::RunL() notify completed"));
        }
    FLOG(_L("[OBEXUTILS]\t CObexUtilsPropertyNotifier::RunL() completed"));
    }
