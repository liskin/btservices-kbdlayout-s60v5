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
// This file contains the implementation of the AT+CPWD command
// 
//


#include <mmretrieve.h>         // AO

#include "cpwdcommandhandler.h"

#include "debug.h"


// password types
_LIT8(KATCPWDPS, "PS"); // Phone lock
_LIT8(KATCPWDP2, "P2"); // PIN2
_LIT8(KATCPWDSC, "SC"); // PIN
_LIT8(KATCPWDAB, "AB"); // All Barring code



// strings for debugging trace
_LIT8(KDbgStr, "+CPWD: %s \r\n");
_LIT8(KDbgTDes, "+CPWD: %s%S\r\n");

// constant for Set All barring code service - originally defined in mw/PSetConstants.h
const TInt KPsetAllSSServices = 0;
// Max buffer length for an MD5 digest - originally defined in SCPServerInterface.h
const TInt KSCPMaxHashLength( 32 );



CCPWDCommandHandler* CCPWDCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCPWDCommandHandler* self = new (ELeave) CCPWDCommandHandler(aCallback, aATCmdParser, aPhone);
    TRACE_FUNC_EXIT
    return self;
    }

CCPWDCommandHandler::CCPWDCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
        CATCmdAsyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

CCPWDCommandHandler::~CCPWDCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    TRACE_FUNC_EXIT
    }

void CCPWDCommandHandler::HandleCommand( const TDesC8& /*aCmd*/,
                                   RBuf8& /*aReply*/,
                                   TBool /*aReplyNeeded*/ )
    {
    TRACE_FUNC_ENTRY
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            // Get supported passwords with max. length of them. AT+CPWD=?
            // return hardcoded supported password with max. length
            _LIT8(KCPWDTestValue, "\r\n+CPWD: (\"PS\",10),(\"SC\",8),(\"AB\",4),(\"P2\",8)\r\n\r\nOK\r\n");
            iCallback->CreateReplyAndComplete( EReplyTypeOther, KCPWDTestValue );
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeSet):
            {
            // Change password. AT+CPWD="nn","xxxx","yyyy"
            ChangePassword();
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            }
        }
    TRACE_FUNC_EXIT
    }

void CCPWDCommandHandler::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    TRACE_FUNC_EXIT
    }


void CCPWDCommandHandler::ChangePassword()
    {
    TRACE_FUNC_ENTRY

    // Get parameters from AT command
    TInt ret1;
    TInt ret2; 
    TInt ret3;
    TPtrC8 passwordType = iATCmdParser.NextTextParam(ret1);
    TPtrC8 oldPassword = iATCmdParser.NextTextParam(ret2);
    TPtrC8 newPassword = iATCmdParser.NextTextParam(ret3);
    
    if(ret1 != KErrNone || ret2 != KErrNone || ret3 != KErrNone
            || iATCmdParser.NextParam().Compare(KNullDesC8) != 0)
        {
        Trace(KDbgStr, "invalid arguments");
        iCallback->CreateReplyAndComplete( EReplyTypeError);
        TRACE_FUNC_EXIT
        return;
        }
    
    if(passwordType.Compare(KATCPWDPS) == 0) // Phone lock
        {
    // "PS" PH-SIM (lock PHone to SIM/UICC card) (MT asks password when other than current SIM/UICC card
    // inserted; MT may remember certain amount of previously used cards thus not requiring password when they
    // are inserted)
        RMobilePhone::TMobilePhonePasswordChangeV1 passwordChange;
        
        // Phone lock password is hashed in RSPClient and NokiaTSY. See CSCPServer::HashISACode() for details.
        TBuf8<KSCPMaxHashLength> hashOldPwd;
        TBuf8<KSCPMaxHashLength> hashNewPwd;
        iATCmdParser.HashSecurityCode(oldPassword, hashOldPwd);
        iATCmdParser.HashSecurityCode(newPassword, hashNewPwd);

        ChangeSecurityCode(RMobilePhone::ESecurityCodePhonePassword, hashOldPwd, hashNewPwd);
        }
    else if(passwordType.Compare(KATCPWDSC) == 0) // SIM pin
        {
        // "SC" SIM (lock SIM/UICC card) (SIM/UICC asks password in MT power-up and when this lock command
        // issued)
        
        // todo: if it is required to return +CME code according to SIM Lock status,
        // should check SIM Lock is on here and return an error if it is off.
        ChangeSecurityCode(RMobilePhone::ESecurityCodePin1, oldPassword, newPassword);
        }
    else if(passwordType.Compare(KATCPWDP2) == 0) // SIM pin2
        {
        ChangeSecurityCode(RMobilePhone::ESecurityCodePin2, oldPassword, newPassword);
        }
    else if(passwordType.Compare(KATCPWDAB) == 0) // All Barring services
        {
        // "AB" All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
        RMobilePhone::TMobilePhonePasswordChangeV2 passwordChange;
        passwordChange.iOldPassword.Copy(oldPassword);
        passwordChange.iNewPassword.Copy(newPassword);
        passwordChange.iVerifiedPassword.Copy(newPassword);
        RMobilePhone::TMobilePhonePasswordChangeV2Pckg pwdPckg ( passwordChange );
        // The current S60 use SetSSPassword for All Barring services code.
        // see CPsetCallBarring::ChangePasswordL for details.
        iPhone.SetSSPassword(iStatus, pwdPckg, KPsetAllSSServices);
        iPendingEvent = EMobilePhoneSetSSPassword;
        SetActive();

        }
    else    // other unsupported arguments
        {
        Trace(KDbgTDes, "unknown password type:", &passwordType);
        iCallback->CreateCMEReplyAndComplete(KErrUnknown);
        }
    
    TRACE_FUNC_EXIT
    return;
    }

void CCPWDCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY

    if (iStatus.Int() == KErrNone)
        { // complete without error
        Trace(KDbgStr, "complete OK." );
        iCallback->CreateReplyAndComplete( EReplyTypeOk );
        }
    else
        { // if there is an error
        iCallback->CreateCMEReplyAndComplete(iStatus.Int());   
        }
    TRACE_FUNC_EXIT
    }

void CCPWDCommandHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    Trace(KDbgStr, "Request cancelled." );
    iPhone.CancelAsyncRequest(iPendingEvent);
    TRACE_FUNC_EXIT
    }


void CCPWDCommandHandler::ChangeSecurityCode(RMobilePhone::TMobilePhoneSecurityCode aType, TDesC8& aOldPassword, TDesC8& aNewPassword)
    {
    RMobilePhone::TMobilePhonePasswordChangeV1 passwordChange;
    passwordChange.iOldPassword.Copy(aOldPassword);
    passwordChange.iNewPassword.Copy(aNewPassword);
    iPhone.ChangeSecurityCode(iStatus, aType, passwordChange);
    iPendingEvent = EMobilePhoneChangeSecurityCode;
    SetActive();
    }
