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

#include "clckcommandhandler.h"

#include <startupdomainpskeys.h>
#include <e32property.h>

#include "securitycodeverifier.h"
#include "securityeventhandler.h"
#include "cbsettinghandler.h"

#include "atmisccmdpluginconsts.h"
#include "debug.h"

CCLCKCommandHandler* CCLCKCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCLCKCommandHandler* self = new (ELeave) CCLCKCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCLCKCommandHandler::CCLCKCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdAsyncBase(aCallback, aATCmdParser, aPhone),
    iLockInfoPckg(iLockInfo)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCLCKCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);
    
    // initialise AOs
    iCBRetrieve = CRetrieveMobilePhoneCBList::NewL(iPhone);
    iCBSettingHandler = CCBSettingHandler::NewL(iPhone);
    iSecurityEventHandler = CSecurityEventHandler::NewL(this, iPhone);
    iSecurityCodeVerifier = CSecurityCodeVerifier::NewL(this, iPhone);

    TRACE_FUNC_EXIT
    }

CCLCKCommandHandler::~CCLCKCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    
    delete iCBRetrieve;
    delete iCBSettingHandler;
    delete iSecurityEventHandler;
    delete iSecurityCodeVerifier;
    
    iPassword.Close();
    iReply.Close();
    TRACE_FUNC_EXIT
    }

void CCLCKCommandHandler::DoCancel()
    {
    TRACE_FUNC_ENTRY
    switch (iCLCKCommandType)
        {
        case (CCLCKCommandHandler::ECLCKLockGet):
            {
            iPhone.CancelAsyncRequest(EMobilePhoneGetLockInfo);
            break;
            }
        case (CCLCKCommandHandler::ECLCKLockSet):
            {
            iPhone.CancelAsyncRequest(EMobilePhoneSetLockSetting);
            iSecurityCodeVerifier->Cancel();
            iSecurityEventHandler->Cancel();
            break;
            }
        case (CCLCKCommandHandler::ECLCKBarringGet):
            {
            iCBRetrieve->Cancel();
            break;
            }
        case (CCLCKCommandHandler::ECLCKBarringSet):
            {
            iCBSettingHandler->Cancel();
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

void CCLCKCommandHandler::HandleCommand( const TDesC8& /*aCmd*/, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    Trace(KDebugPrintD, "cmdHandlerType: ", cmdHandlerType);
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            iCallback->CreateReplyAndComplete(EReplyTypeOther, KCLCKSupportedCmdsList);
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeSet):
            {
            if (ParseCCLCKCommand() == KErrNone)
                {
                IssueCLCKCommand();
                }
            else
                {
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                }
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

void CCLCKCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    Trace(KDebugPrintD, "iStatus.Int(): ", iStatus.Int());
    TInt err = iStatus.Int();
    if (err == KErrNone)
        {
        switch ( iCLCKCommandType )
            {
            case (CCLCKCommandHandler::ECLCKLockGet):
                {
                iReply.Append(KAtCLCK);
                if (iLockInfo.iStatus == RMobilePhone::EStatusUnlocked)
                    {
                    iReply.AppendNum(0);
                    iReply.Append(KCRLF);
                    }
                else if (iLockInfo.iStatus == RMobilePhone::EStatusLocked)
                    {
                    iReply.AppendNum(1); 
                    iReply.Append(KCRLF);
                    }
                else
                    {
                    err = KErrArgument;
                    }
                break;
                }
            case(CCLCKCommandHandler::ECLCKBarringGet):
                {
                err = ReceiveCBList();
                break;
                }
            case (CCLCKCommandHandler::ECLCKLockSet):
                {
                iLockSettingState = ELockSettingIdle;
                iSecurityCodeVerifier->Cancel();
                iSecurityEventHandler->Cancel();
                break;
                }
            case(CCLCKCommandHandler::ECLCKBarringSet):
            default:
                {
                // no action required
                break;
                }
            }
        }

    if (err != KErrNone)
        {
        if (iCLCKCommandType == CCLCKCommandHandler::ECLCKLockSet)
            {
            iLockSettingState =ELockSettingIdle;
            iSecurityCodeVerifier->Cancel();
            iSecurityEventHandler->Cancel();
            }
        iCallback->CreateCMEReplyAndComplete(err);
        }
    else
        {
        iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply );
        }
    
    TRACE_FUNC_EXIT
    }

TInt CCLCKCommandHandler::ReceiveCBList()
    {
    TRACE_FUNC_ENTRY
    iInfoClass = 0;
    CMobilePhoneCBList* callBarringList=NULL;
    TRAPD(leaveCode, callBarringList=iCBRetrieve->RetrieveListL(););
    if (leaveCode != KErrNone) 
        {
        TRACE_FUNC_EXIT
        return leaveCode;
        }

    TInt count = callBarringList->Enumerate();
    while (count-- > 0)
        {
        RMobilePhone::TMobilePhoneCBInfoEntryV1 entry;
        TRAP(leaveCode, entry = callBarringList->GetEntryL(count););
        if (leaveCode != KErrNone) 
            {
            TRACE_FUNC_EXIT
            return leaveCode;
            }
        if (entry.iCondition == iCondition &&
            entry.iStatus == RMobilePhone::ECallBarringStatusActive)
            {
            switch (entry.iServiceGroup)
                {
                case(RMobilePhone::ETelephony):
                    {
                    iInfoClass |= EInfoClassVoice;
                    break;   
                    }
                case(RMobilePhone::EAllBearer):
                    {
                    iInfoClass |= EInfoClassData;
                    break;   
                    }
                case(RMobilePhone::EFaxService):
                    {
                    iInfoClass |= EInfoClassFax;
                    break;   
                    }
                case(RMobilePhone::EShortMessageService):
                    {
                    iInfoClass |= EInfoClassSMS;
                    break;   
                    }
                case(RMobilePhone::ESyncData):
                    {
                    iInfoClass |= EInfoClassSyncData;
                    break;   
                    }
                case(RMobilePhone::EAsyncData):
                    {
                    iInfoClass |= EInfoClassASyncData;
                    break;   
                    }
                case(RMobilePhone::EPacketData):
                    {
                    iInfoClass |= EInfoClassPacketData;
                    break;   
                    }
                case(RMobilePhone::EPadAccess):
                    {
                    iInfoClass |= EInfoClassPadAccess;
                    break;   
                    }
                }
            }
        }
    
    // There are no services with barring active - therefore report
    // - status = 0 (inactive)
    // - class = 7 (default value; voice, fax and data)
    TInt status;
    if (iInfoClass == 0)
        {
        status = 0;
        iInfoClass = 7; // Default value, see ETSI TS 127 007 V6.9.0 (2007-06)
        }
    else
        {
        // There is at least one service with barring active - report status 1 (active)
        status = 1;
        }
    
    iReply.Append(KAtCLCK);
    iReply.AppendNum(status);
    iReply.Append(','); 
    iReply.AppendNum(iInfoClass);
    iReply.Append(KCRLF);
    
    delete callBarringList;
    TRACE_FUNC_EXIT
    return KErrNone;
    }

void CCLCKCommandHandler::IssueCLCKCommand()
    {
    TRACE_FUNC_ENTRY
    Trace(KDebugPrintD, "iCLCKCommandType: ", iCLCKCommandType);
    iReply.Zero();
    
    switch (iCLCKCommandType)
        {
        case (CCLCKCommandHandler::ECLCKLockGet):
            {
            iPhone.GetLockInfo(iStatus, iLockType, iLockInfoPckg);
            SetActive();
            break;
            }
        case (CCLCKCommandHandler::ECLCKLockSet):
            {
            if (iPassword.Length() == 0)
                {
                iCallback->CreateReplyAndComplete( EReplyTypeError);
                }
            else
                {
                // Set the property to ignore security events in other clients
                // it allows to avoid GUI promt for security code
                TInt  ret = KErrNone;
                if (iLockType == RMobilePhone::ELockICC)
                    {
                    ret = RProperty::Set(KPSUidStartup, KIgnoreSecurityEvent, EPSIgnoreSecurityEventEPin1Required);
                    }
                else
                    {
                    ret = RProperty::Set(KPSUidStartup, KIgnoreSecurityEvent, EPSIgnoreSecurityEventEPhonePasswordRequired);
                    }
                Trace(KDebugPrintD, "RProperty::Set: ", ret);
                if (ret == KErrNone)
                    {
                    // Start security event handler - this will notify whether a 
                    // password is required to complete the set lock request.
                    iSecurityEventHandler->Start();
                
                    Trace(KDebugPrintD, "SetLockSetting iLockType: ", iLockType);
                    Trace(KDebugPrintD, "SetLockSetting iLockChange: ", iLockChange);
                    iPhone.SetLockSetting(iStatus, iLockType, iLockChange);
                    iLockSettingState = ELockSettingRequested;
                    SetActive();
                    }
                else
                    {
                    iCallback->CreateReplyAndComplete( EReplyTypeError);
                    }
                }
            break;
            }
        case (CCLCKCommandHandler::ECLCKBarringGet):
            {
            iCBRetrieve->Start(iStatus, iCondition, RMobilePhone::RMobilePhone::EInfoLocationNetwork);
            SetActive();
            break;
            }
        case (CCLCKCommandHandler::ECLCKBarringSet):
            {
            if (iPassword.Length() == 0)
                {
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                }
            else
                {
                iCBSettingHandler->Start(iStatus, iInfoClass, iCondition, &iCBInfo);
                SetActive();
                }
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete(EReplyTypeError);
            break;
            }
        }   
    TRACE_FUNC_EXIT
    }

void CCLCKCommandHandler::HandlePasswordVerification(TInt aError)
    {
    TRACE_FUNC_ENTRY
    
    if (aError != KErrNone)
        {
        iCallback->CreateCMEReplyAndComplete(aError);
        Cancel();
        TRACE_FUNC_EXIT
        return;
        }
    
    switch (iLockSettingState)
         {
         case (ELockSettingIdle):
             {
             // Security code setting request has already been completed
             iSecurityEventHandler->Cancel();
             break;
             }
         case (ELockSettingPasswordRequested):
             {
             // after password has been verified go back to the previous state
             iLockSettingState = ELockSettingRequested;
             break;
             }
         case (ELockSettingRequested):
         default:
             {
             // should never be in this state
             __ASSERT_DEBUG(EFalse,  User::Invariant());
             break;
             }
         }

    TRACE_FUNC_EXIT
    }

void CCLCKCommandHandler::HandleSecurityEvent(TInt aError, RMobilePhone::TMobilePhoneSecurityEvent aSecurityEvent)
    {
    TRACE_FUNC_ENTRY
    if (aError != KErrNone)
        {
        iCallback->CreateCMEReplyAndComplete(aError);
        Cancel();
        TRACE_FUNC_EXIT
        return;
        }
    
    Trace(KDebugPrintD, "iLockSettingState: ", iLockSettingState);
    Trace(KDebugPrintD, "aSecurityEvent: ", aSecurityEvent);
    switch (iLockSettingState)
        {
        case (ELockSettingIdle):
            {
            // Set Lock request has already been completed
            iSecurityCodeVerifier->Cancel();
            break;
            }
        case (ELockSettingRequested):
            {
            switch (aSecurityEvent)
                 {
                 case(RMobilePhone::EPin1Required):
                 case(RMobilePhone::EPhonePasswordRequired):
                     {
                     iSecurityEventHandler->Start();
                     if( iSecurityCode == RMobilePhone::ESecurityCodePin1 &&
                         aSecurityEvent == RMobilePhone::EPin1Required ||
                         iSecurityCode == RMobilePhone::ESecurityCodePhonePassword &&
                         aSecurityEvent == RMobilePhone::EPhonePasswordRequired)
                         {
                         // security code request has been triggered, reissue the security notification
                         // request and provide the code if this is expected
                         iSecurityCodeVerifier->Start(iPassword, iSecurityCode);
                         iLockSettingState =ELockSettingPasswordRequested;
                         }
                     break;
                     }
                 case(RMobilePhone::ENoICCFound):
                 case(RMobilePhone::EICCTerminated):
                     {
                     // No SIM present or it is unusable
                     iCallback->CreateCMEReplyAndComplete(aError);
                     Cancel();
                     break;
                     }
                 default:
                     {
                     // other processes may trigger various security events, ignore them
                     // if not related and reissue the notification request
                     iSecurityEventHandler->Start();
                     break;
                     }
                 }
            break;
            }
        case (ELockSettingPasswordRequested):
            {
            switch (aSecurityEvent)
                 {
                 case(RMobilePhone::EPin1Verified):
                     {
                     // PIN1 has been verified, ignore if not applicable
                     // otherwise security event handler and  security code verifier
                     // no longer needed 
                     // (note that another client could have provided the PIN1)
                     if (iSecurityCode == RMobilePhone::ESecurityCodePin1)
                         {
                         iLockSettingState =ELockSettingRequested;
                         }
                     break;
                     }
                 case(RMobilePhone::EPhonePasswordVerified):
                     {
                     if (iSecurityCode == RMobilePhone::ESecurityCodePhonePassword)
                         {
                         iLockSettingState =ELockSettingRequested;
                         }
                     break;
                     }
                 case(RMobilePhone::ENoICCFound):
                 case(RMobilePhone::EICCTerminated):
                     {
                     // No SIM present or it is unusable, terminate the operation
                     Cancel();
                     iCallback->CreateCMEReplyAndComplete(aError);
                     break;
                     }
                 default:
                     {
                     // other processes may trigger various security events, ignore them if not related
                     // and reissue the notification request
                     iSecurityEventHandler->Start();
                     break;
                     }
                 }
            break;
            }
        default:
            {
            // lock setting state value is out of boundies, complete with error
            iCallback->CreateCMEReplyAndComplete(aError);
            Cancel();
            break;
            }
        }
    TRACE_FUNC_EXIT
    }

TInt CCLCKCommandHandler::ParseCCLCKCommand()
    {
    TRACE_FUNC_ENTRY
    TPtrC8 command;
    TInt ret = iATCmdParser.NextTextParam(command);
    if (ret != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    TInt mode = 0;
    ret = iATCmdParser.NextIntParam(mode);
    if (ret != KErrNone)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    TPtrC8 tmpPwd;
    ret = iATCmdParser.NextTextParam(tmpPwd);
    if (ret != KErrNone && ret != KErrNotFound)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    iPassword.Create(tmpPwd);
    if (iPassword.Length() != 0)
        {
        ret = iATCmdParser.NextIntParam(iInfoClass);;
        if (ret == KErrNotFound)
            {
            iInfoClass = 7; // Default value, see ETSI TS 127 007 V6.9.0 (2007-06)
            }
        else if (ret != KErrNone || iATCmdParser.NextParam().Length() != 0)
            {
            TRACE_FUNC_EXIT
            return KErrArgument;
            }
        }
    
    TCmdFacilityType facilityType = ECmdFacilityTypeUnknown;
    
    if (command.CompareF(KATCLCKPS) == 0)
        {
        // Lock phone to SIM on/off
        iSecurityCode = RMobilePhone::ESecurityCodePhonePassword;
        iLockType = RMobilePhone::ELockPhoneToICC;
        facilityType = ECmdFacilityTypeLock;
        }
    else if (command.CompareF(KATCLCKSC) == 0)
        {
        // PIN on/off
        iSecurityCode = RMobilePhone::ESecurityCodePin1;
        iLockType = RMobilePhone::ELockICC;
        facilityType = ECmdFacilityTypeLock;
        }
    else if (command.CompareF(KATCLCKAO) == 0)
        {
        iCondition = RMobilePhone::EBarAllOutgoing;
        facilityType = ECmdFacilityTypeBarring;
        }
    else if (command.CompareF(KATCLCKOI) == 0)
        {
        iCondition = RMobilePhone::EBarOutgoingInternational;
        facilityType = ECmdFacilityTypeBarring;
        }
    else if (command.CompareF(KATCLCKOX) == 0)
        {
        iCondition = RMobilePhone::EBarOutgoingInternationalExHC;
        facilityType = ECmdFacilityTypeBarring;
        }
    else if (command.CompareF(KATCLCKAI) == 0)
        {
        iCondition = RMobilePhone::EBarAllIncoming;
        facilityType = ECmdFacilityTypeBarring;
        }
    else if (command.CompareF(KATCLCKIR) == 0)
        {
        iCondition = RMobilePhone::EBarIncomingRoaming;
        facilityType = ECmdFacilityTypeBarring;
        }
    else if (command.CompareF(KATCLCKAB) == 0)
        {
        iCondition = RMobilePhone::EBarAllCases;
        facilityType = ECmdFacilityTypeAllBarring;
        }
    else if (command.CompareF(KATCLCKAG) == 0)
        {
        iCondition = RMobilePhone::EBarAllOutgoingServices;
        facilityType = ECmdFacilityTypeAllBarring;
        }
    else if (command.CompareF(KATCLCKAC) == 0)
        {
        iCondition = RMobilePhone::EBarAllIncomingServices;
        facilityType = ECmdFacilityTypeAllBarring;
        }
    else 
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    switch (facilityType)
        {
        case (ECmdFacilityTypeLock):
            {
            switch (mode)
                {
                case 0: // e.g. AT+CLCK="SC",0,"0000"
                    {
                    iCLCKCommandType = ECLCKLockSet;
                    iLockChange = RMobilePhone::ELockSetDisabled;
                    break;
                    }
                case 1: // e.g. AT+CLCK="SC",1
                    {
                    iCLCKCommandType = ECLCKLockSet;
                    iLockChange = RMobilePhone::ELockSetEnabled;
                    break;
                    }
                case 2: // e.g. AT+CLCK="SC",2
                    {
                    iCLCKCommandType = ECLCKLockGet;
                    break;
                    }
                default:
                    {
                    TRACE_FUNC_EXIT
                    return KErrArgument;
                    }
                }
            break;
            }
        case (ECmdFacilityTypeBarring):
            {
            iCBInfo.iPassword.Copy(iPassword);
            switch (mode)
                {
                case 0: // AT+CLCK="AO",0
                    {
                    iCLCKCommandType = ECLCKBarringSet;
                    iCBInfo.iAction = RMobilePhone::EServiceActionDeactivate;
                    break;
                    }
                case 1: // AT+CLCK="AO",1,"1919",1
                    {
                    iCLCKCommandType = ECLCKBarringSet;
                    iCBInfo.iAction = RMobilePhone::EServiceActionActivate;
                    break;
                    }
                case 2: // AT+CLCK="AO",2
                    {
                    iCLCKCommandType = ECLCKBarringGet;
                    break;
                    }
                default:
                    {
                    TRACE_FUNC_EXIT
                    return KErrArgument;
                    }
                }
            break;
            }
        case (ECmdFacilityTypeAllBarring):
            {
            iCBInfo.iPassword.Copy(iPassword);
            if (iInfoClass == 0)
                {
                iInfoClass = 7; // Default value, see ETSI TS 127 007 V6.9.0 (2007-06)
                }
            
            if ( mode == 0)
                {
                iCLCKCommandType = ECLCKBarringSet;
                iCBInfo.iAction = RMobilePhone::EServiceActionDeactivate;
                }
            else
                // only deactivation is supported
                {
                TRACE_FUNC_EXIT
                return KErrArgument;
                }
            break;
            }
        default:
            {
            TRACE_FUNC_EXIT
            return KErrArgument;
            }
        }
    
    // if phone password is required it needs to be hashed before verification
    if (iSecurityCode == RMobilePhone::ESecurityCodePhonePassword 
                    && iCLCKCommandType == ECLCKLockSet)
        {
        TBuf8<KSCPMaxHashLength> hashedPwd;
        iATCmdParser.HashSecurityCode(iPassword, hashedPwd);
        if (hashedPwd.Length() > iPassword.MaxLength())
            {
            TInt ret = iPassword.ReAlloc(hashedPwd.Length());
            if (ret != KErrNone)
                {
                TRACE_FUNC_EXIT
                return ret;
                }
            }
        iPassword = hashedPwd;
        }
            
    TRACE_FUNC_EXIT
    return KErrNone;
    }
