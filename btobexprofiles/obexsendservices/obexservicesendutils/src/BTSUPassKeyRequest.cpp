/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  CBTSUPasskeyRequest implementation
*
*/


// INCLUDE FILES
#include "BTSUPassKeyRequest.h"
#include "BTSUDebug.h"
#include "BTServiceUtils.h"

#include <utf.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBTSUPasskeyRequest::CBTSUPasskeyRequest
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBTSUPasskeyRequest::CBTSUPasskeyRequest() : CActive(EPriorityNormal)
    {
    CActiveScheduler::Add( this );
    }

// Destructor
CBTSUPasskeyRequest::~CBTSUPasskeyRequest()
    {
    FLOG(_L("[BTSU]\t CBTSUPasskeyRequest::~CBTSUPasskeyRequest"));

    Cancel(); // Make sure we're cancelled
    iNotif.Close();

    FLOG(_L("[BTSU]\t CBTSUPasskeyRequest:~CBTSUPasskeyRequest completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUPasskeyRequest::DoCancel()
// -----------------------------------------------------------------------------
//
void CBTSUPasskeyRequest::DoCancel()
    {
    FLOG(_L("[BTSU]\t CBTSUPasskeyRequest::DoCancel"));

    iNotif.CancelNotifier( KBTObexPasskeyQueryNotifierUid );
    iNotif.Close();

    FLOG(_L("[BTSU]\t CBTSUPasskeyRequest::DoCancel completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUPasskeyRequest::RunL()
// -----------------------------------------------------------------------------
//
void CBTSUPasskeyRequest::RunL()
    {
    FTRACE(FPrint(_L( "[BTSU]\t CBTSUPasskeyRequest::RunL status %d"), iStatus.Int() ));

    if ( iStatus.Int() == KErrNone )
        {
        TBuf<KBTObexPasskeyMaxLength> tempResultBuffer;
        CnvUtfConverter::ConvertToUnicodeFromUtf8( tempResultBuffer, iObexPasskey() );
        iObexClient->UserPasswordL( tempResultBuffer );

        FTRACE(FPrint(_L("[BTSU]\t CBTSUPasskeyRequest::RunL user entered '%S'"), &tempResultBuffer ));
        }
    else
        {
        iObexClient->Error( KErrCancel );
        }

    iNotif.CancelNotifier( KBTObexPasskeyQueryNotifierUid );
    iNotif.Close();

    FLOG(_L("[BTSU]\t CBTSUPasskeyRequest::RunL completed"));
    }

// -----------------------------------------------------------------------------
// CBTSUPasskeyRequest::RunError()
// -----------------------------------------------------------------------------
//
TInt CBTSUPasskeyRequest::RunError()
    {
    FLOG(_L("[BTSU]\t CBTSUPasskeyRequest::RunError"));

    iNotif.CancelNotifier( KBTObexPasskeyQueryNotifierUid );
    iNotif.Close();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBTSUPasskeyRequest::StartPassKeyRequestL()
// -----------------------------------------------------------------------------
//
void CBTSUPasskeyRequest::StartPassKeyRequestL( CObexClient* aObexClient )
    {
    FLOG(_L("[BTSU]\t CBTSUPasskeyRequest::StartPassKeyRequestL"));
    __ASSERT_DEBUG( aObexClient != NULL, BTSUPanic( EBTSUPanicNullPointer ) );

    User::LeaveIfError( iNotif.Connect() );
    TPtrC8 dummy( KNullDesC8 );

    // Starts notifier and gets the passkey for Obex
    iNotif.StartNotifierAndGetResponse( iStatus, KBTObexPasskeyQueryNotifierUid, dummy, iObexPasskey );
    iObexClient = aObexClient;
    SetActive();

    FLOG(_L("[BTSU]\t CBTSUPasskeyRequest::StartPassKeyRequestL completed"));
    }

//  End of File
