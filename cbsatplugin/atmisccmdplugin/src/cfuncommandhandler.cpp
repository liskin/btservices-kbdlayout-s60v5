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

#include "cfuncommandhandler.h"

#include <MProfileEngine.h>
#include <starterclient.h>

#include <Profile.hrh>

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCFUNCommandHandler* CCFUNCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCFUNCommandHandler* self = new (ELeave) CCFUNCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCFUNCommandHandler::CCFUNCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdAsyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCFUNCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);
    iProfileEngine = CreateProfileEngineL();
    TRACE_FUNC_EXIT
    }

CCFUNCommandHandler::~CCFUNCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    if (iProfileEngine != NULL)
        {
        iProfileEngine->Release();
        }
    TRACE_FUNC_EXIT
    }

void CCFUNCommandHandler::HandleCommand(const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    TInt ret = KErrNone;
    iReply.Zero();
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOther, KCFUNSupportedCmdsList);
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeRead):
            {
            TInt profileId = iProfileEngine->ActiveProfileId();
            
            iReply.Append( KAtCFUN );
    
            if( profileId == EProfileOffLineId )
                {
                iReply.Append( '0' );
                }
            else
                {
                iReply.Append( '1' );
                }
            iReply.Append( KOKCRLF );
            iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply);
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeSet):
            {
            TInt func = 0;
            TInt reset = 0; // default 0 - do not reset the MT before setting it to <fun> power level
            
            ret = iATCmdParser.NextIntParam(func);
            if (ret != KErrNone && ret != KErrNotFound)
                {
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                TRACE_FUNC_EXIT
                return;
                }
            ret = iATCmdParser.NextIntParam(reset);
            
            // second parameter is optional, but only 0 and 1 are valid if it is specified
            if (ret != KErrNone && ret != KErrNotFound && (reset != 0 || reset != 1))
                {
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                TRACE_FUNC_EXIT
                return;
                }
            ret = ActivateProfile(func, reset);
           
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            }
        }
    
    if (ret != KErrNone)
        {
        iCallback->CreateReplyAndComplete(EReplyTypeError);
        }
    else
        {
        iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply );
        }
    TRACE_FUNC_EXIT
    }

void CCFUNCommandHandler::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
    // no asyc requests are made in when using AT+CFUN
    TRACE_FUNC_EXIT
    }


void CCFUNCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    // no asyc requests are made in when using AT+CFUN
    TRACE_FUNC_EXIT
    }

void CCFUNCommandHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    // no asyc requests are made in when using AT+CFUN
    TRACE_FUNC_EXIT
    }

TInt CCFUNCommandHandler::ActivateProfile(TInt aFunc, TInt aReset)
    {
    TInt err = KErrNone;
    
    switch (aFunc)
        {
        case (0):
        case (4):
            {
            err = SetActiveProfile(KOfflineProfileId);
            break;
            }
        case (1):
            {
            err = SetActiveProfile(KGeneralProfileId);
            break;
            }
        default:
            {
            err = KErrArgument;
            break;
            }
        }
    
    if (err == KErrNone && aReset == 1)
        {
        err = RestartDevice();
        }
    
    return err;
    }

TInt CCFUNCommandHandler::SetActiveProfile(TInt aProfileId)
    {
    TInt err = KErrNone;

    if(iProfileEngine)
        {
        TRAP(err, iProfileEngine->SetActiveProfileL( aProfileId ));
        }

    return err;
    }

TInt CCFUNCommandHandler::RestartDevice()
    {
    RStarterSession session;
    TInt err = session.Connect();

    if( err == KErrNone )
        {
        session.Reset(RStarterSession::EUnknownReset);
        session.Close();
        }

    return err;
    }
