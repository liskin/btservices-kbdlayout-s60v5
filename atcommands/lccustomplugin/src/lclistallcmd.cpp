/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Handles the commands "AT+CLAC?" and "AT+CLAC"
*
*/

#include "lclistallcmd.h"
#include "debug.h"

const TInt KCrLfLength    = 2;  // CR+LF
const TInt KOkReplyLength = 6;  // CR+LF+"OK"+CR+LF

_LIT8( KClacCmd, "AT+CLAC" );

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CLcListAllCmd* CLcListAllCmd::NewL( MLcCustomPlugin* aCallback )
    {
    CLcListAllCmd* self = new (ELeave) CLcListAllCmd( aCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CLcListAllCmd::~CLcListAllCmd()
    {
    }

// ---------------------------------------------------------------------------
// CLcListAllCmd::CLcListAllCmd
// ---------------------------------------------------------------------------
//
CLcListAllCmd::CLcListAllCmd( MLcCustomPlugin* aCallback ) :
    iCallback( aCallback )
    {
    iCmdHandlerType = ECmdHandlerTypeUndefined;
    }

// ---------------------------------------------------------------------------
// CLcListAllCmd::ConstructL
// ---------------------------------------------------------------------------
//
void CLcListAllCmd::ConstructL()
    {
    if ( !iCallback )
        {
        User::Leave( KErrGeneral );
        }
    }

// ---------------------------------------------------------------------------
// Checks command types
// ---------------------------------------------------------------------------
//
TBool CLcListAllCmd::CheckCommand( const TDesC8& aCmd )
    {
    TRACE_FUNC_ENTRY
    // The AT+CLAC command supports two types: base and test
    iCmdHandlerType = iCallback->CheckCommandType( KClacCmd, aCmd );
    if ( iCmdHandlerType==ECmdHandlerTypeBase ||
         iCmdHandlerType==ECmdHandlerTypeTest )
        {
        iDetectedCmd = EDetectedCmdCLAC;
        TRACE_FUNC_EXIT
        return ETrue;
        }
    TRACE_FUNC_EXIT
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Reports the support status of an AT command. This is a synchronous API.
// ---------------------------------------------------------------------------
//
TBool CLcListAllCmd::IsCommandSupported( const TDesC8& aCmd )
    {
    TRACE_FUNC_ENTRY
    // Set all to undefined if either the command or its type is unknown
    // HandleCommand() should be round only when both are set
    TBool cmdUnderstood = CheckCommand( aCmd );
    if ( cmdUnderstood )
        {
        TRACE_FUNC_EXIT
        return ETrue;
        }
    iCmdHandlerType = ECmdHandlerTypeUndefined;
    iDetectedCmd = EDetectedCmdUndefined;
    TRACE_FUNC_EXIT
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles an AT command. Cancelling of the pending request is done by
// HandleCommandCancel(). The implementation in the extension plugin should
// be asynchronous.
// ---------------------------------------------------------------------------
//
void CLcListAllCmd::HandleCommand( const TDesC8& /*aCmd*/,
                                   RBuf8& /*aReply*/,
                                   TBool aReplyNeeded )
    {
    TRACE_FUNC_ENTRY
    if ( !aReplyNeeded )
        {
        TRACE_FUNC_EXIT
        return;
        }
    if ( iCmdHandlerType == ECmdHandlerTypeTest )
        {
        iCallback->CreateReplyAndComplete( EReplyTypeOk );
        TRACE_FUNC_EXIT
        return;
        }
    // Else here means ECmdHandlerTypeBase
    // First check the quiet mode and verbose mode.
    // These are handled in CreateReplyAndComplete()
    TInt retTemp;
    TBool quietMode = EFalse;
    retTemp  = iCallback->GetModeValue( EModeTypeQuiet, quietMode );
    if ( retTemp != KErrNone )
        {
        iCallback->CreateReplyAndComplete( EReplyTypeError );
        TRACE_FUNC_EXIT
        return;
        }
    RBuf8 reply;
    TBool error = CreateSupportedList( reply );
    if ( error )
        {
        iCallback->CreateReplyAndComplete( EReplyTypeError );
        reply.Close();
        TRACE_FUNC_EXIT
        return;
        }
    RBuf8 okReply;
    iCallback->CreateOkOrErrorReply( okReply, ETrue );
    reply.Append( okReply );
    okReply.Close();
    iCallback->CreateReplyAndComplete( EReplyTypeOther, reply );
    reply.Close();
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Cancels a pending HandleCommand request.
// ---------------------------------------------------------------------------
//
void CLcListAllCmd::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Creates a linearized list of supported commands
// ---------------------------------------------------------------------------
//
TBool CLcListAllCmd::CreateSupportedList( RBuf8& aReply )
    {
    TRACE_FUNC_ENTRY
    // First get the unsorted list from ATEXT
    RPointerArray<HBufC8> commands;
    iCallback->GetSupportedCommands( commands );
    // Next linearize the list for a reply
    if ( commands.Count() <= 0 )
        {
        commands.Close();
        TRACE_FUNC_EXIT
        return EFalse;
        }
    TInt i;
    TInt linearSize = KOkReplyLength;
    TInt count = commands.Count();
    for ( i=0; i<count; i++ )
        {
        linearSize += (*commands[i]).Length();
        if ( i < count-1 )
            {
            linearSize += KCrLfLength;
            }
        }
    linearSize += KCrLfLength;
    // Now we have the length of the linear region,
    // use that to create the reply
    TChar carriageReturn;
    TChar lineFeed;
    TInt retTemp;
    retTemp  = aReply.Create( linearSize );
    retTemp |= iCallback->GetCharacterValue( ECharTypeCR, carriageReturn );
    retTemp |= iCallback->GetCharacterValue( ECharTypeLF, lineFeed );
    if ( retTemp != KErrNone )
        {
        commands.ResetAndDestroy();
        commands.Close();
        TRACE_FUNC_EXIT
        return ETrue;
        }
    for ( i=0; i<count; i++ )
        {
        aReply.Append( *commands[i] );
        if ( i < count-1 )
            {
            aReply.Append( carriageReturn );
            aReply.Append( lineFeed );
            }
        }
    aReply.Append( carriageReturn );
    aReply.Append( lineFeed );
    // Delete the array as it is no longer needed
    commands.ResetAndDestroy();
    commands.Close();
    TRACE_FUNC_EXIT
    return EFalse;
    }
