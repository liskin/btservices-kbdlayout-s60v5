// Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// This file contains the implementation of the AT+CPIN command
// 
//

#include "cpincommandhandler.h"

#include <mmretrieve.h>         // define AO wrapper
#include <EXTERROR.H>           // Additional RMobilePhone error code

#include "debug.h"

CCPINCommandHandler* CCPINCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCPINCommandHandler* self = new (ELeave) CCPINCommandHandler(aCallback, aATCmdParser, aPhone);
    TRACE_FUNC_EXIT
    return self;
    }

CCPINCommandHandler::CCPINCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdAsyncBase(aCallback, aATCmdParser, aPhone),
    iLockInfoPckg(iLockInfo)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

CCPINCommandHandler::~CCPINCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    TRACE_FUNC_EXIT
    }

void CCPINCommandHandler::HandleCommand(const TDesC8& /*aCmd*/,
                                   RBuf8& /*aReply*/,
                                   TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY
   
    switch (iATCmdParser.CommandHandlerType())
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
                // AT+CPIN=?  - simply return OK.
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOk);
            break;
            }
        
        case(TAtCommandParser::ECmdHandlerTypeRead):
            // AT+CPIN?  - Get PIN require status.  return +CPIN: READY/SIM PUK
        case (TAtCommandParser::ECmdHandlerTypeSet):
            // AT+CPIN="puk","pin"  - Change password when PUK is required. 
            {
            // call GetLockInfo to check if PUK is required
            iPhone.GetLockInfo(iStatus, RMobilePhone::ELockICC, iLockInfoPckg);
            iPendingEvent = EMobilePhoneGetLockInfo;
            SetActive();
            break;
            }
        default:
            {
            // unsupported command
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            break;
            }
        }
    
    TRACE_FUNC_EXIT
    }

void CCPINCommandHandler::ChangePassword()
    {
    TRACE_FUNC_ENTRY
    TPtrC8 pukCode8;
    TPtrC8 pinCode8;
    
    // Get parameters from AT command
    TInt ret1 = iATCmdParser.NextTextParam(pukCode8);
    TInt ret2 = iATCmdParser.NextTextParam(pinCode8);
    
    if(ret1 != KErrNone || ret2 != KErrNone
            || iATCmdParser.NextParam().Length() != 0)
        {
        iCallback->CreateReplyAndComplete( EReplyTypeError);
        }
    else
        {
        RMobilePhone::TMobilePassword pukCode;
        RMobilePhone::TMobilePassword pinCode;
        pukCode.Copy(pukCode8);
        pinCode.Copy(pinCode8);
        iPhone.VerifySecurityCode(iStatus, RMobilePhone::ESecurityCodePuk1, pinCode, pukCode);
        // re-use the AO for VerifySecurityCode 
        iPendingEvent = EMobilePhoneVerifySecurityCode; 
        SetActive();
        }
    
    TRACE_FUNC_EXIT
    }

void CCPINCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    if( iPendingEvent == EMobilePhoneGetLockInfo)
        { // after calling GetLockInfo
        HandleGetLockInfo();
        }
    else
        { // after calling VerifySecurityCode
        ASSERT(iPendingEvent == EMobilePhoneVerifySecurityCode);
        HandleVerifySecurityCode();
        }
    TRACE_FUNC_EXIT
    }

void CCPINCommandHandler::HandleGetLockInfo()
    {
    TRACE_FUNC_ENTRY
    TAtCommandParser::TCommandHandlerType handleType = iATCmdParser.CommandHandlerType();
    
    switch(iStatus.Int())
        {
        case KErrNone:
            {
            // log lock info
            _LIT8(KDbgPhoneLockInfo, "GetLockInfo: Status %d, Lock %d, Setting %d");
            Trace(KDbgPhoneLockInfo, iStatus.Int(), iLockInfo.iStatus, iLockInfo.iSetting);
            
            if(handleType == TAtCommandParser::ECmdHandlerTypeRead )
                {
                // complete without error
                _LIT8(KCPINReady, "\r\n+CPIN: READY\r\n\r\nOK\r\n");
                iCallback->CreateReplyAndComplete( EReplyTypeOther, KCPINReady);
                }
            else 
                {
                ASSERT( handleType== TAtCommandParser::ECmdHandlerTypeSet );
                // PUK/PIN is not required
                iCallback->CreateCMEReplyAndComplete(KErrGsm0707OperationNotAllowed);
                }
            break;
            }
        case KErrAccessDenied:
            {
            if(handleType == TAtCommandParser::ECmdHandlerTypeRead )
                {
                // reply PUK is required
                _LIT8(KCPINSimPuk, "\r\n+CPIN: SIM PUK\r\n\r\nOK\r\n");
                iCallback->CreateReplyAndComplete( EReplyTypeOther, KCPINSimPuk);
                }
            else
                
                {
                // change PIN with a PUK code
                ChangePassword();
                }
            break;
            }
        default:
            {
            // any other errors, such as KErrGeneral when SIM card is not available
            Trace(KDebugPrintD, "Complete CPIN with error ", iStatus.Int());
            iCallback->CreateReplyAndComplete( EReplyTypeError );
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

void CCPINCommandHandler::HandleVerifySecurityCode()
    {
    TRACE_FUNC_ENTRY

    // check error after change PUK
    if (iStatus.Int() == KErrNone)
        { // complete without error
        iCallback->CreateReplyAndComplete( EReplyTypeOk );
        }
    else
        { // if there is an error
        iCallback->CreateCMEReplyAndComplete(iStatus.Int());   
        }

    TRACE_FUNC_EXIT
    }

void CCPINCommandHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    iPhone.CancelAsyncRequest(iPendingEvent);
    TRACE_FUNC_EXIT
    }

