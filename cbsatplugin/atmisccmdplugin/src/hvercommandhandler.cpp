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

#include "hvercommandhandler.h"
#include <sysutil.h>

#include "atmisccmdpluginconsts.h"
#include "debug.h"

const TUint KCommaValue = 44;
const TUint KSpaceValue = 32;

CHVERCommandHandler* CHVERCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CHVERCommandHandler* self = new (ELeave) CHVERCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CHVERCommandHandler::CHVERCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdSyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CHVERCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iTelError = KErrNone;
    iSWVersion.CreateL(KSysUtilVersionTextLength);
    iReply.CreateL(KDefaultCmdBufLength);
    TRACE_FUNC_EXIT
    }

CHVERCommandHandler::~CHVERCommandHandler()
    {
    TRACE_FUNC_ENTRY
    iSWVersion.Close();
    iReply.Close();
    TRACE_FUNC_EXIT
    }

void CHVERCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY
    
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    if (cmdHandlerType != TAtCommandParser::ECmdHandlerTypeBase)
        {
        iCallback->CreateReplyAndComplete(EReplyTypeError);
        }
    else if(iTelError == KErrNone)
        {
        if (iReply.Length() == 0)
            {
            // Get model
            iReply.Append( KCRLF );
            iReply.Append( iModel );
    
            TChar commaValue = KCommaValue;
            TChar spaceValue = KSpaceValue;            
            iReply.Append( commaValue );
            iReply.Append( spaceValue );
            
            // Get software version (RM-xxx)
            TBuf<KSysUtilVersionTextLength> swVersion;
            TInt errorCode = SysUtil::GetSWVersion( swVersion );
    
            if (KErrNone == errorCode)
                {
                ParseSWVersion(swVersion.Collapse());
                iReply.Append(iSWVersion);
                }
            iReply.Append( KCRLF );
            }
        iCallback->CreateReplyAndComplete( EReplyTypeOk, iReply);
        }
    else
        {
        iCallback->CreateCMEReplyAndComplete(iTelError);
        }
   
    TRACE_FUNC_EXIT
    }

void CHVERCommandHandler::ParseSWVersion(const TDesC8& aSWVersion)
    {  
    TRACE_FUNC_ENTRY
    if (iSWVersion.Length() != 0)
        {
        TRACE_FUNC_EXIT
        return;
        }
    
    TInt start = aSWVersion.Find(KHVERModelString);
    
    if ( KErrNotFound != start )
        {
        TLex8 lex(aSWVersion);
        lex.Inc(start);
        lex.Mark(); 
        lex.SkipCharacters();
        TPtrC8 token = lex.MarkedToken(); 
    
        iSWVersion.Append(token);
        }
    TRACE_FUNC_EXIT
    }

void CHVERCommandHandler::SetHWVersion(const TDesC8& aHWVersion)
    {
    TRACE_FUNC_ENTRY
    iModel.Zero();
    if (aHWVersion.Length() <= CTelephony::KPhoneModelIdSize)
        {
        iModel.Copy(aHWVersion);
        }
    else
        {
        iModel.Copy(aHWVersion.Left(CTelephony::KPhoneModelIdSize));
        }    
    TRACE_FUNC_EXIT
    }



void CHVERCommandHandler::SetTelephonyError(TInt aTelError)
    {
    TRACE_FUNC_ENTRY
    iTelError = aTelError;
    TRACE_FUNC_EXIT
    }

