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
#include "socketreader.h"
#include "socketobserver.h"
#include "debug.h"

CSocketReader* CSocketReader::NewL(TUint aSocketID,
        MSocketObserver& aObserver, TInt aInitialBufSize)
    {
    CSocketReader* self = CSocketReader::NewLC(aSocketID, aObserver,
            aInitialBufSize);
    CleanupStack::Pop(self);
    return self;
    }

CSocketReader* CSocketReader::NewLC(TUint aSocketID,
        MSocketObserver& aObserver, TInt aInitialBufSize)
    {
    CSocketReader* self = new (ELeave) CSocketReader(aSocketID, aObserver);
    CleanupStack::PushL(self);
    self->ConstructL(aInitialBufSize);
    return self;
    }

CSocketReader::CSocketReader(TUint aSocketID, MSocketObserver& aObserver) :
    CActive(EPriorityStandard), iSocketID(aSocketID), iObserver(aObserver),
            iBufferPtr(0, 0)
    {
    }

CSocketReader::~CSocketReader()
    {
    Cancel(); // Causes DoCancel

    delete iBuffer;
    }

void CSocketReader::ConstructL(TInt aInitialBufSize)
    {
    iBuffer = HBufC8::NewL(aInitialBufSize);
    iBufferPtr.Set(iBuffer->Des());

    CActiveScheduler::Add(this);
    }

void CSocketReader::DoCancel()
    {
    // Cancel asynchronous read request
    iSocket->CancelRead();
    }

void CSocketReader::RunL()
    {
        // Active object request complete handler
        TRACE_INFO((_L("CSocketReader::RunL(), iStatus.Int()=%d, socketID=%d"),iStatus.Int(), iSocketID));

    switch (iStatus.Int())
        {
        case KErrNone:
            {
            // Data has been read from socket
            // Inform the observer and issue another read if we want
            // to continue listening
            if (iObserver.HandleDataReceived(iSocketID, iBufferPtr))
                {
                IssueRead();
                }
            break;
            }

        default:
            {
            // A read error has occurred. Inform the observer the connection is lost
            iObserver.HandleSocketError(iSocketID, ETrue, iStatus.Int());
            break;
            }
        }
    }

void CSocketReader::IssueRead()
    {
    // Initiate a new read from socket into iBuffer
    iSocket->Read(iBufferPtr, iStatus);
    SetActive();
    }

void CSocketReader::StartReadingL(RSocket* aSocket, TInt aMTU)
    {
    // Initiate a new read from socket into iBuffer
    __ASSERT_DEBUG(!IsActive(),
            User::Panic(KPanicSocketsEngineRead, ESocketsBadState));

    if (aMTU > iBufferPtr.MaxLength())
        {
        // Realloc the buffer as it isn't large enough.
        iBufferPtr.Set(0, 0, 0);
        delete iBuffer;
        iBuffer = 0;
        iBuffer = HBufC8::NewL(aMTU);
        iBufferPtr.Set(iBuffer->Des());
        }

    iSocket = aSocket;
    IssueRead();
    }
