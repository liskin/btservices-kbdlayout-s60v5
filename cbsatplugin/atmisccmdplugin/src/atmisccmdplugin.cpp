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
* Description:  Main handler for incoming requests
*
*/

#include "atmisccmdplugin.h"

#include "clckcommandhandler.h"
#include "cpwdcommandhandler.h"
#include "cpincommandhandler.h"
#include "cusdcommandhandler.h"
#include "cnumcommandhandler.h"
#include "cfuncommandhandler.h"
#include "cbccommandhandler.h"
#include "hvercommandhandler.h"
#include "cgsncommandhandler.h"
#include "cgmrcommandhandler.h"
#include "cgmicommandhandler.h"
#include "cmgwcommandhandler.h"
#include "cmgdcommandhandler.h"
#include "telephonywrapper.h"
#include "cgmmcommandhandler.h"
#include "scpbrcommandhandler.h"
#include "scpbwcommandhandler.h"

#include "atmisccmdpluginconsts.h"
#include "cmserror.h"
#include "debug.h"

#include <exterror.h>           // Additional RMobilePhone error code
#include <etelmmerr.h>          // ETelMM error code
#include <gsmerror.h>           // GSM error code

// +CME error code
_LIT8(KCMEMemoryFailure, "+CME ERROR: 23\r\n"); // Memory failure.\r\n
_LIT8(KCMEIncorrectPassword, "+CME ERROR: 16\r\n"); // Incorrect password.\r\n
_LIT8(KCMEPUKRequired, "+CME ERROR: 12\r\n"); // PUK required.\r\n
_LIT8(KCMENotAllowed, "+CME ERROR: 3\r\n"); // Operation not allowed.\r\n
_LIT8(KCMEPhoneError, "+CME ERROR: 0\r\n"); // Phone failure.\r\n
_LIT8(KCMEPhoneUnknown, "+CME ERROR: 100\r\n"); // unknown error
_LIT8(KCMESimNotInserted, "+CME ERROR: 10\r\n"); // SIM not inserted 
_LIT8(KCMEMemoryFull, "+CME ERROR: 20\r\n"); // Memory full
_LIT8(KCMEInvalidIndex, "+CME ERROR: 21\r\n"); // Invalid index 
_LIT8(KCMENotFound, "+CME ERROR: 22\r\n"); // Not found 
_LIT8(KCMEDialStringTooLong, "+CME ERROR: 26\r\n"); // Dial string too long 
_LIT8(KCMETextStringTooLong, "+CME ERROR: 24\r\n"); // Text string too long 
_LIT8(KCMEInvalidCharacters, "+CME ERROR: 27\r\n"); // Invalid characters in dial string



const TInt KErrorReplyLength = 9;  // CR+LF+"ERROR"+CR+LF
const TInt KEditorReplyLength = 4;  // CR+LF+'>'+' '

CATMiscCmdPlugin* CATMiscCmdPlugin::NewL()
    {
    CATMiscCmdPlugin* self = new (ELeave) CATMiscCmdPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CATMiscCmdPlugin::~CATMiscCmdPlugin()
	{
    iReplyBuffer.Close();
    
    delete iCLCKHandler;
    delete iCPWDHandler;
    delete iCPINHandler;
    delete iCUSDHandler;
    delete iCNUMHandler;
    delete iCFUNHandler;
    delete iCBCHandler;
	delete iHVERHandler;
	delete iCGSNHandler;
    delete iCGMRHandler;
    delete iCGMIHandler;
    delete iCMGWHandler;
    delete iCMGDHandler;
    delete iCGMMHandler;
	delete iSCPBRHandler;
    delete iSCPBWHandler;  
	
    iPhone.Close();
    iTelServer.Close();
	}

CATMiscCmdPlugin::CATMiscCmdPlugin() : 
    CATExtPluginBase()
    {
    }

void CATMiscCmdPlugin::ConstructL()
    {
    TRACE_FUNC_ENTRY
    ConnectToEtelL(iTelServer, iPhone);
    
    iCLCKHandler = CCLCKCommandHandler::NewL(this, iCommandParser, iPhone);
    iCPWDHandler = CCPWDCommandHandler::NewL(this, iCommandParser, iPhone);
    iCUSDHandler = CCUSDCommandHandler::NewL(this, iCommandParser, iPhone);
    iCPINHandler = CCPINCommandHandler::NewL(this, iCommandParser, iPhone);
    iCNUMHandler = CCNUMCommandHandler::NewL(this, iCommandParser, iPhone, iTelServer);
    iCFUNHandler = CCFUNCommandHandler::NewL(this, iCommandParser, iPhone);
    iCBCHandler = CCBCCommandHandler::NewL(this, iCommandParser, iPhone);
	iHVERHandler = CHVERCommandHandler::NewL(this, iCommandParser, iPhone);
    iCGSNHandler = CCGSNCommandHandler::NewL(this, iCommandParser, iPhone);
    iCGMRHandler = CCGMRCommandHandler::NewL(this, iCommandParser, iPhone);
    iCGMIHandler = CCGMICommandHandler::NewL(this, iCommandParser, iPhone);
    iCMGWHandler = CCMGWCommandHandler::NewL(this, iCommandParser, iPhone);
    iCMGDHandler = CCMGDCommandHandler::NewL(this, iCommandParser, iPhone); 
    iCGMMHandler = CCGMMCommandHandler::NewL(this, iCommandParser, iPhone);
	iSCPBRHandler = CSCPBRCommandHandler::NewL(this, iCommandParser, iPhone);
    iSCPBWHandler = CSCPBWCommandHandler::NewL(this, iCommandParser, iPhone);
    
   
    // Get telephony information - Model, IMEI, Manufacturer
    CTelephonyWrapper* telephonyWrapper = CTelephonyWrapper::NewL();
    TInt result = telephonyWrapper->SynchronousGetPhoneId();

    if (KErrNone == result)
        {
        static_cast<CHVERCommandHandler*>(iHVERHandler)->SetHWVersion(telephonyWrapper->GetPhoneModel());
        static_cast<CCGSNCommandHandler*>(iCGSNHandler)->SetSerialNum(telephonyWrapper->GetPhoneSerialNum());
        static_cast<CCGMICommandHandler*>(iCGMIHandler)->SetManufacturer(telephonyWrapper->GetPhoneManufacturer());
        static_cast<CCGMMCommandHandler*>(iCGMMHandler)->SetManufacturer(telephonyWrapper->GetPhoneManufacturer());
        static_cast<CCGMMCommandHandler*>(iCGMMHandler)->SetModelID(telephonyWrapper->GetPhoneModel());
        }
    else // The result is used to determine whether to display CME error or not
        {
        static_cast<CHVERCommandHandler*>(iHVERHandler)->SetTelephonyError(result);
        static_cast<CCGSNCommandHandler*>(iCGSNHandler)->SetTelephonyError(result);
        static_cast<CCGMICommandHandler*>(iCGMIHandler)->SetTelephonyError(result);
        static_cast<CCGMMCommandHandler*>(iCGMMHandler)->SetTelephonyError(result);
        }
    delete telephonyWrapper;
    
    TRACE_FUNC_EXIT
   	}

/**
 * @see CATExtPluginBase::ReportConnectionName
 */
void CATMiscCmdPlugin::ReportConnectionName( const TDesC8& /*aName*/ )
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

/**
 * @see CATExtPluginBase::IsCommandSupported
 */
TBool CATMiscCmdPlugin::IsCommandSupported( const TDesC8& aCmd )
    {
    TRACE_FUNC_ENTRY
    iHcCmd = NULL;
    iHcReply = NULL;
    iCurrentHandler = NULL;
    TBool supported = ETrue;
    
    Trace(KDebugPrintS, "aCmd: ", &aCmd);
    
    iCommandParser.ParseAtCommand(aCmd);

    Trace(KDebugPrintD, "Command: ", iCommandParser.Command());
    // TODO: check if the type is supported?
    // iCommandParser.CommandHandlerType() != TAtCommandParser::ECmdHandlerTypeUndefined
    switch (iCommandParser.Command())
        {
        case (TAtCommandParser::ECmdAtClck):
            {
            iCurrentHandler = iCLCKHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCpwd):
            {
            iCurrentHandler = iCPWDHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCpin):
            {
            iCurrentHandler = iCPINHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCusd):
            {
            iCurrentHandler = iCUSDHandler;
            break;
            }   
        case (TAtCommandParser::ECmdAtCnum):
            {
            iCurrentHandler = iCNUMHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCfun):
            {
            iCurrentHandler = iCFUNHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCbc):
            {
            iCurrentHandler = iCBCHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCmee):
            {
            iCurrentHandler = NULL;	
            break;
			}
        case (TAtCommandParser::ECmdAtHver):
            {
            iCurrentHandler = iHVERHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCgsn): // intentional fall through
        case (TAtCommandParser::ECmdAtGsn):
        case (TAtCommandParser::ECmdAtI1):
            {
            iCurrentHandler = iCGSNHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCgmr): // intentional fall through
        case (TAtCommandParser::ECmdAtGmr):
        case (TAtCommandParser::ECmdAtI2):
        case (TAtCommandParser::ECmdAtI4):
            {
            iCurrentHandler = iCGMRHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCgmi): // intentional fall through
        case (TAtCommandParser::ECmdAtGmi):
        case (TAtCommandParser::ECmdAtI):
        case (TAtCommandParser::ECmdAtI0):
            {
            iCurrentHandler = iCGMIHandler;
            break;
            }
        case (TAtCommandParser::ECmdAtCmgw):
            {
            iCurrentHandler = iCMGWHandler;
            break;
            }
		case (TAtCommandParser::ECmdAtCmgd):
            {
            iCurrentHandler = iCMGDHandler;
            break;
            }
		case (TAtCommandParser::ECmdAtCmgf):
            {
            iCurrentHandler = NULL;
            break;
            }
		case (TAtCommandParser::ECmdAtCgmm): // intentional fall through
		case (TAtCommandParser::ECmdAtGmm):
		case (TAtCommandParser::ECmdAtI3):
            {
            iCurrentHandler = iCGMMHandler;
            break;
            }
		case (TAtCommandParser::ECmdAtScpbr):
            {
            iCurrentHandler = iSCPBRHandler;
            break;
            }    
		case (TAtCommandParser::ECmdAtScpbw):
            {
            iCurrentHandler = iSCPBWHandler;
            break;
            }
        case (TAtCommandParser::EUnknown):
        default:
            {
            supported = EFalse;
            break;
            }
        }
    Trace(KDebugPrintD, "supported: ", supported);
    TRACE_FUNC_EXIT
    return supported;
	}

/**
 * @see CATExtPluginBase::HandleCommand
 */
void CATMiscCmdPlugin::HandleCommand( const TDesC8& aCmd,
                                     RBuf8& aReply,
                                     TBool aReplyNeeded )
	{
	TRACE_FUNC_ENTRY
	
	if(iCommandParser.Command() == TAtCommandParser::ECmdAtCmee)
	    {
        HandleCMEECommand();
        HandleCommandCompleted( KErrNone, EReplyTypeOk);
	    }
	else if (iCommandParser.Command() == TAtCommandParser::ECmdAtCmgf)
	    {
        HandleCMGFCommand();
        HandleCommandCompleted( KErrNone, EReplyTypeOk);
	    }
	else if (iCurrentHandler != NULL)
	    {
	    iHcCmd = &aCmd;
	    iHcReply = &aReply;
	    // No need to parse the command again as assumed that
	    // it is always called from CATMiscCmdPlugin::IsCommandSupported()
	    iCurrentHandler->HandleCommand( aCmd, aReply, aReplyNeeded );
	    }
	TRACE_FUNC_EXIT
    }

/**
 * @see CATExtPluginBase::HandleCommandCancel
 */
void CATMiscCmdPlugin::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
    if (iCurrentHandler != NULL)
	    {
	    iCurrentHandler->HandleCommandCancel();
	    }
	TRACE_FUNC_EXIT
    }

/**
 * @see CATExtPluginBase::NextReplyPartLength
 */
TInt CATMiscCmdPlugin::NextReplyPartLength()
    {
    TRACE_FUNC_ENTRY
    TInt length = iReplyBuffer.Length();
    if ( length >= KDefaultCmdBufLength )
        {
        length = KDefaultCmdBufLength;
        }
    TRACE_FUNC_EXIT
    return length;
    }

/**
 * @see CATExtPluginBase::GetNextPartOfReply
 */
TInt CATMiscCmdPlugin::GetNextPartOfReply( RBuf8& aNextReply )
    {
    TRACE_FUNC_ENTRY
    TInt retVal = CreatePartOfReply( aNextReply );
    TRACE_FUNC_EXIT
    return retVal;
    }

/**
 * @see CATExtPluginBase::ReceiveUnsolicitedResult
 */
void CATMiscCmdPlugin::ReceiveUnsolicitedResult()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

/**
 * @see CATExtPluginBase::ReceiveUnsolicitedResultCancel
 */
void CATMiscCmdPlugin::ReceiveUnsolicitedResultCancel()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

/**
 * @see CATExtPluginBase::ReportNvramStatusChange
 */
void CATMiscCmdPlugin::ReportNvramStatusChange( const TDesC8& /*aNvram*/ )
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

/**
 * @see CATExtPluginBase::ReportExternalHandleCommandError
 */
void CATMiscCmdPlugin::ReportExternalHandleCommandError()
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

/**
 * Creates part of reply from the global reply buffer to the destination
 * buffer. Used with APIs which need the next part of reply in multipart
 * reply requests.
 *
 * @param aBuffer Destination buffer; the next part of reply is stored to
 *                   this buffer.
 * @return None
 */
TInt CATMiscCmdPlugin::CreatePartOfReply( RBuf8& aBuffer )
    {
    TRACE_FUNC_ENTRY
    TInt ret = KErrNone;
    if ( iReplyBuffer.Length() <= 0 )
        {
        ret = KErrGeneral;
        }
    else
        {
    	TInt partLength = NextReplyPartLength();
        if ( iReplyBuffer.Length() < partLength )
            {
            ret =  KErrNotFound;
            }
        else if (ret == KErrNone)
			{
			aBuffer.Create( iReplyBuffer, partLength );
			iReplyBuffer.Delete( 0, partLength );
			if ( iReplyBuffer.Length() == 0 )
				{
				iReplyBuffer.Close();
				}
			}
        }

    Trace(KDebugPrintD, "ret: ", ret);
    TRACE_FUNC_EXIT
    return ret;
    }

/**
 * @see MATMiscCmdPlugin::CreateReplyAndComplete
 */
TInt CATMiscCmdPlugin::CreateReplyAndComplete( TATExtensionReplyType aReplyType,
                                              const TDesC8& aSrcBuffer,
											  TInt aError )
    {
    TRACE_FUNC_ENTRY
    iReplyBuffer.Close();
    Trace(KDebugPrintD, "aError: ", aError);
    if ( aError != KErrNone )
        {
        HandleCommandCompleted( aError, EReplyTypeUndefined );
        iHcCmd = NULL;
        iHcReply = NULL;
        iCurrentHandler = NULL;
        TRACE_FUNC_EXIT
        return KErrNone;
        }
    
    Trace(KDebugPrintS, "iHcReply: ", &iHcReply);
    if (iHcReply == NULL)
        {
        TRACE_FUNC_EXIT
        return KErrGeneral;
        }
    
    Trace(KDebugPrintD, "iQuietMode: ", iQuietMode);
    if ( iQuietMode )
        {
        iReplyBuffer.Create( KNullDesC8 );
        }
    else
        {
        iReplyBuffer.Create( aSrcBuffer );
        }
    
    Trace(KDebugPrintD, "aReplyType: ", aReplyType);
    switch ( aReplyType )
        {
        case EReplyTypeOther:
            break;
        case EReplyTypeEditor:
            CreateEditModeBuffer( iReplyBuffer );
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
    if ( EReplyTypeEditor != aReplyType )
        {
        iHcCmd = NULL;
        iHcReply = NULL;
        iCurrentHandler = NULL;
        }
    TRACE_FUNC_EXIT
    return KErrNone;
    }

/**
 * @see MATMiscCmdPlugin::CreateOkOrErrorReply
 */
TInt CATMiscCmdPlugin::CreateOkOrErrorReply( RBuf8& aReplyBuffer,
                                            TBool aOkReply )
    {
    TRACE_FUNC_ENTRY
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

    aReplyBuffer.ReAlloc(aReplyBuffer.Length() + replyBuffer.Length());
    aReplyBuffer.Append( replyBuffer );
    TRACE_FUNC_EXIT
    return KErrNone;
    }

/**
 * @see MATMiscCmdPlugin::CreateEditModeBuffer
 */
TInt CATMiscCmdPlugin::CreateEditModeBuffer( RBuf8& aReplyBuffer )
    {
    TRACE_FUNC_ENTRY
    _LIT8( KReplyPromptAndSpace, "> " );
    TBuf8<KEditorReplyLength> replyBuffer;
    replyBuffer.Append( iCarriageReturn );
    replyBuffer.Append( iLineFeed );
    replyBuffer.Append( KReplyPromptAndSpace );

    TInt err = aReplyBuffer.ReAlloc( aReplyBuffer.Length() + replyBuffer.Length() );
    if (KErrNone != err)
        {
        return err;
        }
    aReplyBuffer.Append( replyBuffer );
    TRACE_FUNC_EXIT
    return KErrNone;
    }

/**
 * @see MATMiscCmdPlugin::GetCharacterValue
 */
TInt CATMiscCmdPlugin::GetCharacterValue( TCharacterTypes aCharType,
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

/**
 * @see MATMiscCmdPlugin::GetModeValue
 */
TInt CATMiscCmdPlugin::GetModeValue( TModeTypes aModeType, TBool& aMode )
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

void CATMiscCmdPlugin::CreateCMEReplyAndComplete(TInt aError)
    {
    TRACE_FUNC_ENTRY
    
    // currently only support +CME error level 0 and 1
    ASSERT(iErrorLevel == 0 || iErrorLevel == 1 );
    
    // Log error code
    Trace(KDebugPrintD, "complete with error ", aError);
    
    if(iErrorLevel == 1 && !iQuietMode)
        {
        // return error code to AT client
        RBuf8 response;
        response.Create(KDefaultCmdBufLength);
        response.Append(KCRLF);
        switch(aError)
            {
            case KErrGsm0707IncorrectPassword:
            case KErrAccessDenied:
                {    
                // code was entered erroneously
                response.Append(KCMEIncorrectPassword);
                break;
                }
            case KErrGsmSSPasswordAttemptsViolation:
            case KErrLocked:
                {
                // Pin blocked 
                response.Append(KCMEPUKRequired);
                break;
                }
            case KErrGsm0707OperationNotAllowed:
                {
                // not allowed with this sim
                response.Append(KCMENotAllowed);
                break;
                }
			case KErrGsmMMImeiNotAccepted:
	            {
	            // Memory failure
	            response.Append(KCMEMemoryFailure);
	            break;
	            }
            case KErrUnknown:
            case KErrGsmSimServAnrFull:
                {
                // unknown error
                response.Append(KCMEPhoneUnknown);
                break;
                }
            case KErrNotFound:
                {
                response.Append(KCMENotFound);
                break;
                }
            case KErrInUse:
            case KErrGsmMMServiceOptionTemporaryOutOfOrder:
                {
                // SIM not inserted
                response.Append(KCMESimNotInserted);
                break;
                }
            case KErrArgument:
            case KErrGsm0707InvalidIndex:
            case KErrGsm0707NotFound:
                {
                // Invalid index
                response.Append(KCMEInvalidIndex);
                break;
                }
            case KErrGsm0707TextStringTooLong:
                {
                // Text string too long
                response.Append(KCMETextStringTooLong);
                break;
                }
            case KErrGsm0707DialStringTooLong:
                {
                // Dial string too long
                response.Append(KCMEDialStringTooLong);
                break;
                }
            case KErrGsmCCUnassignedNumber:
            case KErrGsm0707InvalidCharsInDialString:
                {
                // Invalid characters in dial string
                response.Append(KCMEInvalidCharacters);
                break;
                }
            case KErrMMEtelMaxReached:
                {
                // Memory full
                response.Append(KCMEMemoryFull);
                break;
                }
            default:
                {
                response.Append(KCMEPhoneError);
                break;
                }
            }
        CreateReplyAndComplete( EReplyTypeError, response );
        response.Close();
        }
    else
        {
        CreateReplyAndComplete( EReplyTypeError);
        }

    TRACE_FUNC_EXIT
    }

void CATMiscCmdPlugin::CreateCMSReplyAndComplete(TInt aError)
    {
    TRACE_FUNC_ENTRY
    
    if(iQuietMode)
        {
        CreateReplyAndComplete(EReplyTypeError);
        }
    else 
        {
        // return error code to AT client
        RBuf8 response;
        if (KErrNone != response.Create(KDefaultCmdBufLength))
            {
            CreateReplyAndComplete(EReplyTypeError);
            return;
            }
       
        // return error code to AT client
        response.Append(KCRLF);
        response.Append(KCMSErr);

        switch(aError)
            {
            case KErrGsmSMSReserved:
                {
                // Other application cause SMS interface being reserved
                response.AppendNum(EATCMSErr301);
                break;
                }
            case KErrGsmSMSInvalidPDUModeParameter:
                {
                // Under the mode of PDU, PDU parameter error 
                response.AppendNum(EATCMSErr304);
                break;
                }
            case KErrGsm0707SimFailure:
            case KErrGsmMMServiceOptionTemporaryOutOfOrder:
                {    
                // SIM card not inserted
                response.AppendNum(EATCMSErr310);
                break;
                }
            case KErrGsmSMSSimPin1Required:
                {
                // PIN request by SIM card
                response.AppendNum(EATCMSErr311);
                break;
                }
            case KErrGsmSMSPhoneToSimLockRequired:
                {
                // PH-(U) SIM PIN request by SIM card
                response.AppendNum(EATCMSErr312);
                break;
                }
            case KErrGsmSMSSimPuk1Required:
                {
                // PUK request by SIM card
                response.AppendNum(EATCMSErr316);
                break;
                }
            case KErrGsmSMSMemoryFailure:
                {
                // Memory error
                response.AppendNum(EATCMSErr320);
                break;
                }
            case KErrPathNotFound:
            case KErrGsmSMSInvalidMemoryIndex:
                {
                // Invalid Memory index number 
                response.AppendNum(EATCMSErr321);
                break;
                }
            case KErrOverflow:
            case KErrGsmSMSMemoryFull:
                {
                // Memory is full
                response.AppendNum(EATCMSErr322);
                break;
                }
            default:
				{
				response.AppendNum(EATCmsErrGeneral);
				break;
				}
            }
        CreateReplyAndComplete( EReplyTypeError, response );
        response.Close();
        }
    
    TRACE_FUNC_EXIT
    }

TInt CATMiscCmdPlugin::HandleUnsolicitedRequest(const TDesC8& aAT )
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    return SendUnsolicitedResult(aAT);
    }

void CATMiscCmdPlugin::HandleCMEECommand()
    {
    TRACE_FUNC_ENTRY
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iCommandParser.CommandHandlerType();
    
    if (cmdHandlerType == TAtCommandParser::ECmdHandlerTypeSet)
        {
        // +CMEE=n
        TInt errLevel;
        TInt ret = iCommandParser.NextIntParam(errLevel);
        TPtrC8 param = iCommandParser.NextParam();
        _LIT8(KCMEDbg, "CME : NextIntParam returned %d, NextParam Length is %d, errLevel %d\r\n");
        Trace(KCMEDbg, ret, param.Length(), errLevel);
        if (ret == KErrNone && param.Length() == 0 &&
           (errLevel == 0 || errLevel == 1))
            {
            iErrorLevel = errLevel;
            _LIT8(KCMEErrorLevel, "CME error level: %d\r\n");
            Trace(KCMEErrorLevel, iErrorLevel);
            }
        }
    TRACE_FUNC_EXIT
    }


void CATMiscCmdPlugin::HandleCMGFCommand()
    {
    TRACE_FUNC_ENTRY
    
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iCommandParser.CommandHandlerType();
    
    if (cmdHandlerType == TAtCommandParser::ECmdHandlerTypeSet)
        {
        TInt msgFormat = 0;
        TInt ret = iCommandParser.NextIntParam(msgFormat);
        if(ret == KErrNone && iCMGWHandler)
            {
            static_cast<CCMGWCommandHandler*> (iCMGWHandler)->SetMessageFormat(msgFormat);
            }
        }
    
    TRACE_FUNC_EXIT
    }
  

void CATMiscCmdPlugin::ConnectToEtelL(RTelServer& aTelServer, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    User::LeaveIfError(aTelServer.Connect());

    // get the name of the first available phone
    TInt phoneCount;
    RTelServer::TPhoneInfo info;

    User::LeaveIfError(aTelServer.EnumeratePhones(phoneCount));  
    if (phoneCount < 1)
        {
        User::Leave(KErrNotFound);
        }
    User::LeaveIfError(aTelServer.GetPhoneInfo(0, info));
    User::LeaveIfError(aPhone.Open(aTelServer, info.iName));
    
    if (iTelServer.SetExtendedErrorGranularity(RTelServer::EErrorExtended)!=KErrNone)
        {
        User::LeaveIfError(iTelServer.SetExtendedErrorGranularity(RTelServer::EErrorBasic));
        }

    TRACE_FUNC_EXIT
    }

