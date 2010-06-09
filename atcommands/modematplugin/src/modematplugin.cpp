/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Main handler for incoming requests
*
*/


#include "modematplugin.h"
#include "atcopscmd.h"
#include "debug.h"

const TInt KErrorReplyLength = 9;  // CR+LF+"ERROR"+CR+LF

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CModemAtPlugin* CModemAtPlugin::NewL()
    {
    CModemAtPlugin* self = new (ELeave) CModemAtPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CModemAtPlugin::~CModemAtPlugin()
	{
    TRACE_FUNC_ENTRY
	iHandlers.ResetAndDestroy();
	iHandlers.Close();
    iReplyBuffer.Close();
    TRACE_FUNC_EXIT
	}

// ---------------------------------------------------------------------------
// CModemAtPlugin::CModemAtPlugin
// ---------------------------------------------------------------------------
//
CModemAtPlugin::CModemAtPlugin() : CATExtPluginBase()
    {
    TRACE_FUNC_ENTRY
    iHandler = NULL;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// CModemAtPlugin::ConstructL
// ---------------------------------------------------------------------------
//
void CModemAtPlugin::ConstructL()
    {
    TRACE_FUNC_ENTRY
    CATCommandHandlerBase* handler = NULL;
    handler = CATCOPSCmd::NewL( this );
    CleanupStack::PushL( handler );
    iHandlers.AppendL( handler );
    CleanupStack::Pop( handler );
    TRACE_FUNC_EXIT
   	}

// ---------------------------------------------------------------------------
// Reports connection identifier name to the extension plugin.
// ---------------------------------------------------------------------------
//
void CModemAtPlugin::ReportConnectionName( const TDesC8& /*aName*/ )
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Reports the support status of an AT command. This is a synchronous API.
// ---------------------------------------------------------------------------
//
TBool CModemAtPlugin::IsCommandSupported( const TDesC8& aCmd )
    {
    TRACE_FUNC_ENTRY
    TInt i;
    TInt count = iHandlers.Count();
    for ( i=0; i<count; i++ )
        {
        CATCommandHandlerBase* handler = iHandlers[i]; 
        TBool supported = handler->IsCommandSupported( aCmd );
        if ( supported )
            {
            iHandler = handler;
            TRACE_FUNC_EXIT
            return ETrue;
            }
        }
    iHandler = NULL;
    TRACE_FUNC_EXIT
    return EFalse;
    }

// ---------------------------------------------------------------------------
// Handles an AT command. Cancelling of the pending request is done by
// HandleCommandCancel(). The implementation in the extension plugin should
// be asynchronous.
// ---------------------------------------------------------------------------
//
void CModemAtPlugin::HandleCommand( const TDesC8& aCmd,
                                     RBuf8& aReply,
                                     TBool aReplyNeeded )
	{
	TRACE_FUNC_ENTRY
	if ( iHandler )
	    {
        iHcCmd = &aCmd;
        iHcReply = &aReply;
	    iHandler->HandleCommand( aCmd, aReply, aReplyNeeded );
	    }
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Cancels a pending HandleCommand request.
// ---------------------------------------------------------------------------
//
void CModemAtPlugin::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
	if ( iHandler )
	    {
	    iHandler->HandleCommandCancel();
	    }
	TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Next reply part's length.
// The value must be equal or less than KDefaultCmdBufLength.
// When the reply from this method is zero, ATEXT stops calling
// GetNextPartOfReply().
// ---------------------------------------------------------------------------
//
TInt CModemAtPlugin::NextReplyPartLength()
    {
    TRACE_FUNC_ENTRY
    if ( iReplyBuffer.Length() < KDefaultCmdBufLength )
        {
        TRACE_FUNC_EXIT
        return iReplyBuffer.Length();
        }
    TRACE_FUNC_EXIT
    return KDefaultCmdBufLength;
    }

// ---------------------------------------------------------------------------
// Gets the next part of reply initially set by HandleCommandComplete().
// Length of aNextReply must be equal or less than KDefaultCmdBufLength.
// ---------------------------------------------------------------------------
//
TInt CModemAtPlugin::GetNextPartOfReply( RBuf8& aNextReply )
    {
    TRACE_FUNC_ENTRY
    TInt retVal = CreatePartOfReply( aNextReply );
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// Receives unsolicited results. Cancelling of the pending request is done by
// by ReceiveUnsolicitedResultCancel(). The implementation in the extension
// plugin should be asynchronous.
// ---------------------------------------------------------------------------
//
void CModemAtPlugin::ReceiveUnsolicitedResult()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Cancels a pending ReceiveUnsolicitedResult request.
// ---------------------------------------------------------------------------
//
void CModemAtPlugin::ReceiveUnsolicitedResultCancel()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Reports NVRAM status change to the plugins.
// ---------------------------------------------------------------------------
//
void CModemAtPlugin::ReportNvramStatusChange( const TDesC8& /*aNvram*/ )
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Reports about external handle command error condition.
// This is for cases when for example DUN decided the reply contained an
// error condition but the plugin is still handling the command internally.
// Example: in command line "AT+TEST;ATDT1234" was given. "AT+TEST" returns
// "OK" and "ATDT" returns "CONNECT". Because "OK" and "CONNECT" are
// different reply types the condition is "ERROR" and DUN ends processing.
// This solution keeps the pointer to the last AT command handling plugin
// inside ATEXT and calls this function there to report the error.
// It is to be noted that HandleCommandCancel() is not sufficient to stop
// the processing as the command handling has already finished.
// ---------------------------------------------------------------------------
//
void CModemAtPlugin::ReportExternalHandleCommandError()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------------------------
// Creates part of reply from the global reply buffer to the destination
// buffer. Used with APIs which need the next part of reply in multipart reply
// requests.
// ---------------------------------------------------------------------------
//
TInt CModemAtPlugin::CreatePartOfReply( RBuf8& aDstBuffer )
    {
    TRACE_FUNC_ENTRY
    if ( iReplyBuffer.Length() <= 0 )
        {
        TRACE_FUNC_EXIT
        return KErrGeneral;
        }
    TInt partLength = NextReplyPartLength();
    if ( iReplyBuffer.Length() < partLength )
        {
        TRACE_FUNC_EXIT
        return KErrNotFound;
        }
    aDstBuffer.Create( iReplyBuffer, partLength );
    iReplyBuffer.Delete( 0, partLength );
    if ( iReplyBuffer.Length() == 0 )
        {
        iReplyBuffer.Close();
        }
    TRACE_FUNC_EXIT
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Creates an AT command reply based on the reply type and completes the
// request to ATEXT. Uses iReplyBuffer for reply storage.
// ---------------------------------------------------------------------------
//
TInt CModemAtPlugin::CreateReplyAndComplete( TATExtensionReplyType aReplyType,
                                              const TDesC8& aSrcBuffer,
                                              TInt aError )
    {
    TRACE_FUNC_ENTRY
    iReplyBuffer.Close();
    if ( aError != KErrNone )
        {
        HandleCommandCompleted( aError, EReplyTypeUndefined );
        iHcCmd = NULL;
        iHcReply = NULL;
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    if ( !iHcReply )
        {
        TRACE_FUNC_EXIT
        return KErrGeneral;
        }
    switch ( aReplyType )
        {
        case EReplyTypeOther:
            iReplyBuffer.Create( aSrcBuffer );
            break;
        case EReplyTypeOk:
            CreateOkOrErrorReply( iReplyBuffer, ETrue );
            break;
        case EReplyTypeError:
            CreateOkOrErrorReply( iReplyBuffer, EFalse );
            break;
        default:
            TRACE_FUNC_EXIT
            return KErrGeneral;
        }
    CreatePartOfReply( *iHcReply );
    HandleCommandCompleted( KErrNone, aReplyType );
    iHcCmd = NULL;
    iHcReply = NULL;
    TRACE_FUNC_EXIT
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Creates a buffer for "OK" or "ERROR" reply based on the line settings
// ---------------------------------------------------------------------------
//
TInt CModemAtPlugin::CreateOkOrErrorReply( RBuf8& aReplyBuffer,
                                            TBool aOkReply )
    {
    TRACE_FUNC_ENTRY
    if ( iQuietMode )
        {
        TRACE_FUNC_EXIT
        return iReplyBuffer.Create( KNullDesC8 );
        }
    _LIT8( KErrorReplyVerbose, "ERROR" );
    _LIT8( KOkReplyVerbose,    "OK" );
    _LIT8( KErrorReplyNumeric, "4" );
    _LIT8( KOkReplyNumeric,    "0" );
    TBuf8<KErrorReplyLength> replyBuffer;
    if ( iVerboseMode )
        {
        replyBuffer.Append( iCarriageReturn );
        replyBuffer.Append( iLineFeed );
        if ( aOkReply )
            {
            replyBuffer.Append( KOkReplyVerbose );
            }
        else
            {
            replyBuffer.Append( KErrorReplyVerbose );
            }
        replyBuffer.Append( iCarriageReturn );
        replyBuffer.Append( iLineFeed );
        }
    else
        {
        if ( aOkReply )
            {
            replyBuffer.Append( KOkReplyNumeric );
            }
        else
            {
            replyBuffer.Append( KErrorReplyNumeric );
            }
        replyBuffer.Append( iCarriageReturn );
        }
    TInt retVal = aReplyBuffer.Create( replyBuffer );
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// From MLcCustomPlugin.
// Returns the array of supported commands
// ---------------------------------------------------------------------------
//
TInt CModemAtPlugin::GetSupportedCommands( RPointerArray<HBufC8>& aCmds )
    {
    TRACE_FUNC_ENTRY
    // Force superclass call here:
    TInt retVal = CATExtPluginBase::GetSupportedCommands( aCmds );
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// From MLcCustomPlugin.
// Returns plugin's character value settings (from CATExtPluginBase)
// ---------------------------------------------------------------------------
//
TInt CModemAtPlugin::GetCharacterValue( TCharacterTypes aCharType,
                                         TChar& aChar )
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNone;
    switch ( aCharType )
        {
        case ECharTypeCR:
            aChar = iCarriageReturn;
            break;
        case ECharTypeLF:
            aChar = iLineFeed;
            break;
        case ECharTypeBS:
            aChar = iBackspace;
            break;
        default:
            retVal = KErrNotFound;
            break;
        }
    TRACE_FUNC_EXIT
    return retVal;
    }

// ---------------------------------------------------------------------------
// From MLcCustomPlugin.
// Returns plugin's mode value settings (from CATExtPluginBase)
// ---------------------------------------------------------------------------
//
TInt CModemAtPlugin::GetModeValue( TModeTypes aModeType, TBool& aMode )
    {
    TRACE_FUNC_ENTRY
    TInt retVal = KErrNone;
    switch ( aModeType )
        {
        case EModeTypeQuiet:
            aMode = iQuietMode;
            break;
        case EModeTypeVerbose:
            aMode = iVerboseMode;
            break;
        default:
            retVal = KErrNotFound;
            break;
        }
    TRACE_FUNC_EXIT
    return retVal;
    }
