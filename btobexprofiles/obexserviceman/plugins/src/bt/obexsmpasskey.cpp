/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Class to handle Obexpasskey queries
*
*/



// INCLUDE FILES
#include "obexsmpasskey.h"
#include <obex.h>
#include <utf.h>
#include "debug.h"


CObexSMPasskey::CObexSMPasskey()
    : CActive(CActive::EPriorityStandard)
    {
    CActiveScheduler::Add(this);
    }

CObexSMPasskey* CObexSMPasskey::NewL()
    {
    CObexSMPasskey* self = new (ELeave) CObexSMPasskey;
    return self;
    }

CObexSMPasskey::~CObexSMPasskey()
    {
    Cancel();
    iNotif.Close();
    FLOG( _L( "[SRCS] CObexSMPasskey: ~CObexSMPasskey" ) );
    }

void CObexSMPasskey::Cleanup()
    {
    iNotif.CancelNotifier( KBTObexPasskeyQueryNotifierUid );
    iNotif.Close();
    }

// ---------------------------------------------------------
// DoCancel()
// Purpose: Cancels ObexPasskey notifier
// Parameters:
// Return value:
// ---------------------------------------------------------
//
void CObexSMPasskey::DoCancel()
    {
    Cleanup();
    FLOG( _L( "[SRCS] CObexSMPasskey: DoCancel" ) );
    }

// ---------------------------------------------------------
// RunL()
// Purpose: Handles ObexPasskey notifiers return value
// Parameters:
// Return value:
// ---------------------------------------------------------
//
void CObexSMPasskey::RunL()
    {
    if ( iStatus.Int() != KErrNone )
        {
        FLOG( _L( "[SRCS] CObexSMPasskey: RunL::Cancel\t" ) );
        User::Leave(KErrCancel);
        }
    
    TBuf<KBTObexPasskeyMaxLength> tempResultBuffer;
    CnvUtfConverter::ConvertToUnicodeFromUtf8( tempResultBuffer, iObexPasskey() );
    iObexServer->UserPasswordL( tempResultBuffer );
    FLOG( _L( "[SRCS] CObexSMPasskey: RunL ObexPasskey returned\t" ) );
    
    Cleanup();
    }

// ---------------------------------------------------------
// RunError( TInt aError )
// Purpose: Handle error cases
// Parameters: TInt
// Return value: error value
// ---------------------------------------------------------
//
TInt CObexSMPasskey::RunError( TInt aError )
    {
    FTRACE(FPrint(_L("[SRCS] CObexSMPasskey: RunError: aError = %d"), aError));
    Cleanup();
    iObexServer->Error(aError); // Ensure that OBEX is kept informed of the problem.
    return KErrNone;
    }

// ---------------------------------------------------------
// StartPassKeyRequestL( CObexServer* aObexServer )
// Purpose: Starts ObexPasskey notifier
// Parameters: CObexServer*
// Return value:
// ---------------------------------------------------------
//
void CObexSMPasskey::StartPassKeyRequestL( CObexServer* aObexServer )
    {
    FLOG( _L( "[SRCS] CObexSMPasskey: StartPassKeyRequest" ) );
    User::LeaveIfError( iNotif.Connect() );
    TPtrC8 dummy( KNullDesC8 );
    // Starts notifier and gets the passkey for Obex
    iNotif.StartNotifierAndGetResponse( iStatus, KBTObexPasskeyQueryNotifierUid, dummy, iObexPasskey );
    iObexServer = aObexServer;
    SetActive();
    FLOG( _L( "[SRCS] CObexSMPasskey: SetActive" ) );
    }

//  End of File
