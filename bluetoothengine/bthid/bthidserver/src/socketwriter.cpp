/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the implementation of application class
 *
*/


#include "sockets.pan"
#include "socketwriter.h"
#include "socketobserver.h"
#include "timeouttimer.h"

// ----------------------------------------------------------------------

// Initial size of the buffers for data to write
const TInt KWriteBufferInitialSize = 20;

// 5 seconds socket write time-out
const TInt CSocketWriter::KTimeOut = 5000000;

CSocketWriter* CSocketWriter::NewL(TUint aSocketID,
        MSocketObserver& aObserver)
    {
    CSocketWriter* self = CSocketWriter::NewLC(aSocketID, aObserver);
    CleanupStack::Pop(self);
    return self;
    }

CSocketWriter* CSocketWriter::NewLC(TUint aSocketID,
        MSocketObserver& aObserver)
    {
    CSocketWriter* self = new (ELeave) CSocketWriter(aSocketID, aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CSocketWriter::CSocketWriter(TUint aSocketID, MSocketObserver& aObserver) :
    CActive(EPriorityStandard), iSocketID(aSocketID), iObserver(aObserver)
    {
    }

CSocketWriter::~CSocketWriter()
    {
    Cancel();
    delete iTimer;
    delete iWriteBuffer[0];
    delete iWriteBuffer[1];
    }

void CSocketWriter::DoCancel()
    {
    // Cancel asynchronous write request
    iSocket->CancelWrite();

    iTimer->Cancel();

    // Clear the buffers
    *iWriteBuffer[0] = KNullDesC8;
    *iWriteBuffer[1] = KNullDesC8;
    }

void CSocketWriter::ConstructL()
    {
    CActiveScheduler::Add(this);

    iTimer = CTimeOutTimer::NewL(CActive::EPriorityStandard, *this);

    iWriteBuffer[0] = HBufC8::NewL(KWriteBufferInitialSize);
    iWriteBuffer[1] = HBufC8::NewL(KWriteBufferInitialSize);
    iCurrentBuffer = 0;
    }

void CSocketWriter::TimerExpired()
    {
    Cancel();

    // Clear the buffers
    *iWriteBuffer[0] = KNullDesC8;
    *iWriteBuffer[1] = KNullDesC8;

    iObserver.HandleSocketError(iSocketID, EFalse, KErrTimedOut);
    }

void CSocketWriter::RunL()
    {
    iTimer->Cancel();

    // Remember the current status
    TInt status = iStatus.Int();

    // Clear the buffer that has been written
    *iWriteBuffer[iCurrentBuffer] = KNullDesC8;

    // If the other buffer has data, write it now
    if (iWriteBuffer[1 - iCurrentBuffer]->Length() > 0)
        {
        iCurrentBuffer = 1 - iCurrentBuffer;
        DoWrite();
        }

    // Handle status of completed write
    if (KErrNone == status)
        {
        iObserver.HandleWriteComplete(iSocketID);
        }
    else
        {
        // Error: pass it up to user interface
        iObserver.HandleSocketError(iSocketID, EFalse, status);
        }
    }

void CSocketWriter::Initialise(RSocket* aSocket)
    {
    __ASSERT_ALWAYS(!IsActive(),
            User::Panic(KPanicSocketsEngineWrite, ESocketsBadState));

    iSocket = aSocket;
    }

void CSocketWriter::IssueWriteL(const TDesC8& aData)
    {
    if (IsActive())
        {
        // Put the data in the other buffer
        StoreDataL(aData, 1 - iCurrentBuffer);
        }
    else
        {
        // Put the data in the current buffer and write immediately
        StoreDataL(aData, iCurrentBuffer);
        DoWrite();
        }
    }

void CSocketWriter::DoWrite()
    {
    // If we aren't in the correct state for a write, panic.
    __ASSERT_ALWAYS(!IsActive(),
            User::Panic(KPanicSocketsEngineWrite, ESocketsBadState));

    // Initiate actual write
    iSocket->Write(*iWriteBuffer[iCurrentBuffer], iStatus);

    // Request timeout
    if (!iTimer->IsActive())
        iTimer->After(KTimeOut);

    SetActive();
    }

void CSocketWriter::StoreDataL(const TDesC8& aData, TInt aBuffer)
    {
    __ASSERT_ALWAYS(iWriteBuffer[aBuffer]->Length() == 0,
            User::Panic(KPanicSocketsEngineWrite, ESocketsBadState));

    // Reallocate the buffer if it's too small
    if (aData.Length() > iWriteBuffer[aBuffer]->Des().MaxLength())
        {
        // Make sure we have a new buffer before deleting the old one
        HBufC8* newBuffer = HBufC8::NewL(aData.Length());

        delete iWriteBuffer[aBuffer];
        iWriteBuffer[aBuffer] = newBuffer;
        }

    *iWriteBuffer[aBuffer] = aData;
    }
