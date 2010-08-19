/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 * Description :
 *
 */

#include "telephonywrapper.h"
#include "debug.h"

CTelephonyWrapper* CTelephonyWrapper::NewL()    
    {
    TRACE_FUNC_ENTRY
    CTelephonyWrapper* self = new (ELeave) CTelephonyWrapper();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    } 

CTelephonyWrapper::CTelephonyWrapper() :
        CActive(CActive::EPriorityStandard), 
        iPhoneIdV1Pckg(iPhoneIdV1)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    } 
    
void CTelephonyWrapper::ConstructL()    
    {
    TRACE_FUNC_ENTRY
    CActiveScheduler::Add(this);
    iTelephony = CTelephony::NewL();
    TRACE_FUNC_EXIT
    } 
    
CTelephonyWrapper::~CTelephonyWrapper()    
    {
    TRACE_FUNC_ENTRY
    Cancel();
    delete iTelephony;
    TRACE_FUNC_EXIT
    }    
    
TInt CTelephonyWrapper::SynchronousGetPhoneId()    
    {
    TRACE_FUNC_ENTRY
    
    if (!IsActive())
        {
        iTelephony->GetPhoneId(iStatus, iPhoneIdV1Pckg);
        SetActive();
        iWaiter.Start();
        
        if (KErrNone == iStatus.Int())
            {
            iModel = iPhoneIdV1.iModel.Collapse();
            iSn = iPhoneIdV1.iSerialNumber.Collapse();
            iManufacturer = iPhoneIdV1.iManufacturer.Collapse();
            }
        }
    else
        {
        TRACE_FUNC_EXIT
        return KErrInUse;
        }
       
    TRACE_FUNC_EXIT
    return iStatus.Int();    
    } 

const TDesC8& CTelephonyWrapper::GetPhoneModel()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    return iModel;
    }

const TDesC8& CTelephonyWrapper::GetPhoneSerialNum()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    return iSn;
    }

const TDesC8& CTelephonyWrapper::GetPhoneManufacturer()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    return iManufacturer;
    }

void CTelephonyWrapper::RunL()    
    {
    TRACE_FUNC_ENTRY
    iWaiter.AsyncStop();
    TRACE_FUNC_EXIT
    } 

void CTelephonyWrapper::DoCancel()    
    {
    TRACE_FUNC_ENTRY
    iTelephony->CancelAsync(CTelephony::EGetPhoneIdCancel);
    iWaiter.AsyncStop();
    TRACE_FUNC_EXIT
    }


