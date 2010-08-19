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

#include "cmgwcommandhandler.h"
#include <mmretrieve.h>
#include <mmlist.h> 
#include <exterror.h>

#include "atmisccmdpluginconsts.h"
#include "debug.h"

const TUint KEOT = 26;    // End of Transmission
const TUint KESC = 27;   // Escape

const TUint8 KSCATonBitMask = 0x70;
const TUint8 KSCANpiBitMask = 0x0F;

CCMGWCommandHandler* CCMGWCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCMGWCommandHandler* self = new (ELeave) CCMGWCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCMGWCommandHandler::CCMGWCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdAsyncBase(aCallback, aATCmdParser, aPhone),
    iEntryPckg(iEntry)
    {
    TRACE_FUNC_ENTRY
    TRACE_FUNC_EXIT
    }

void CCMGWCommandHandler::ConstructL()
    {
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);
    iTPDU.CreateL(KDefaultCmdBufLength);
    
    User::LeaveIfError( iMobileSmsMessaging.Open(iPhone) );

    TInt err = iMobileSmsStore.Open(iMobileSmsMessaging, KETelIccSmsStore);
    if (err != KErrNone)
        {
        iState = ECMGWStateSimStoreNotSupported;
        }
    iRetrieveMobilePhoneSmspList = CRetrieveMobilePhoneSmspList::NewL(iMobileSmsMessaging);

    TRACE_FUNC_EXIT
    }

CCMGWCommandHandler::~CCMGWCommandHandler()
    {
    TRACE_FUNC_ENTRY
    Cancel();
    delete iRetrieveMobilePhoneSmspList;
    delete iMobilePhoneSmspList;
    iMobileSmsStore.Close();
    iMobileSmsMessaging.Close();
    iReply.Close();
    iTPDU.Close();
    TRACE_FUNC_EXIT
    }

/**
 * Set message format
 * 0: PDU mode
 * 1: Text mode - not supported
 */
void CCMGWCommandHandler::SetMessageFormat(TInt aFormat)
    {
    TRACE_FUNC_ENTRY
    if (aFormat == 0 || aFormat == 1)
        {
        iMsgFormat = aFormat;
        }
    Trace(_L("Message format: %d"), iMsgFormat);
    TRACE_FUNC_ENTRY
    }

void CCMGWCommandHandler::HandleCommand(const TDesC8& aCmd, RBuf8& /*aReply*/, TBool /*aReplyNeeded*/)
    {
    TRACE_FUNC_ENTRY
    
    if (iMsgFormat == 1 || iState == ECMGWStateSimStoreNotSupported )
        {
        // Reply "ERROR" if text mode is set
        // Reply "ERROR" if SIM store is not supported
        iCallback->CreateReplyAndComplete( EReplyTypeError );
        TRACE_FUNC_EXIT
        return;
        }
    
    TInt err = KErrNone;
    
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    
    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            iCallback->CreateReplyAndComplete( EReplyTypeOk );
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeSet): 
            {
            switch(iState)
                {
                case ECMGWStateIdle:
                    {
                    // Parse parameters
                    err = ParseParameters();
                    if (err == KErrNone)
                        {
                        Trace(_L("Parse parameters OK."));
                        Trace(_L("Length = %d"), iTPDULength);
                        Trace(_L("stat = %d"), iTPDUStat);
                        
                        iTPDU.Zero();
                        iState = ECMGWStateEditMode;
                        iCallback->CreateReplyAndComplete( EReplyTypeEditor );
                        }
                    else
                        {
                        // Syntax error
                        Trace(_L("Syntax error. err = %d"), err);
                        iState = ECMGWStateIdle;
                        iCallback->CreateReplyAndComplete(EReplyTypeError);
                        }
                    break;
                    }
                case ECMGWStateEditMode:   // Edit state
                    {
                    HandleEditModeCommand(aCmd);
                    break;
                    }
                default:    // Other states
                    {
                    Cancel();
                    iCallback->CreateReplyAndComplete( EReplyTypeError );
                    }
                }
            break;
            }
        default:
            {
            iCallback->CreateReplyAndComplete( EReplyTypeError );
            break;
            }
        }
    
    TRACE_FUNC_EXIT
    }

void CCMGWCommandHandler::HandleEditModeCommand( const TDesC8& aCmd )
    {
    TRACE_FUNC_ENTRY
    
    TInt err = KErrNone;
    TUint8 cmdCharVal = 0;
    if (aCmd.Length())
        {
        cmdCharVal = aCmd[0];
        }

    switch ( cmdCharVal ) 
        {
        case KEOT:  // End of Transmission: Now write the message
            {
            // Extract SCA fro PDU
            err = ExtractSCA();
            if (err == KErrNotFound)
                {
                // SCA not provided by client
                if (iMobileSmspEntry.iServiceCentre.iTelNumber.Length() == 0)
                    {
                    // Retrieve SMS parameter list
                    iRetrieveMobilePhoneSmspList->Start(iStatus);
                    iState = ECMGWStateRetrieveSCA;
                    SetActive();
                    }
                else
                    {
                    // Got the SCA from SIM params already - self complete
                    iServiceCentre = iMobileSmspEntry.iServiceCentre;
                    
                    TRequestStatus* status = &iStatus;
                    User::RequestComplete(status, KErrNone);
                    iState = ECMGWStatePreparePDU;
                    SetActive();
                    }
                }
            else if( err == KErrNone )
                {
                // Got the SCA from client (in iService Centre) - self complete
                TRequestStatus* status = &iStatus;
                User::RequestComplete(status, KErrNone);
                iState = ECMGWStatePreparePDU;
                SetActive();            
                }
            else
                {
                // Extract SCA failed
                iState = ECMGWStateIdle;
                iCallback->CreateCMSReplyAndComplete(KErrGsmSMSInvalidPDUModeParameter);
                }
             break;
            }
        case KESC:  // Escape
            {
            iState = ECMGWStateIdle;
            iCallback->CreateReplyAndComplete( EReplyTypeOk );
            break;
            }
        default:    // Still entering PDU data
            {
            iTPDU.Append( aCmd );
            iCallback->CreateReplyAndComplete( EReplyTypeEditor );
            break;
            }
        }
    
    TRACE_FUNC_EXIT
    }

void CCMGWCommandHandler::RunL()
    {
    TRACE_FUNC_ENTRY
    
    iReply.Zero();
    TInt err = iStatus.Int();
    Trace(_L("State = %d, err = %d"), iState, err);
    
    if (err == KErrNone)
        {
        switch (iState)
            {
            case ECMGWStateRetrieveSCA:
                {
                // Got SCA from SIM params - update iServiceCentre
                iMobilePhoneSmspList = iRetrieveMobilePhoneSmspList->RetrieveListL();
                iMobileSmspEntry = iMobilePhoneSmspList->GetEntryL(0);
                iServiceCentre = iMobileSmspEntry.iServiceCentre;
                
                // Complete self to send PDU in next state
                TRequestStatus* status = &iStatus;
                User::RequestComplete(status, KErrNone);
                iState = ECMGWStatePreparePDU;
                SetActive();
                }
                break;
            case ECMGWStatePreparePDU:
                {
                // Create an SMS entry from PDU
                iEntry.iServiceCentre = iServiceCentre;
                
                err = CreateSmsEntry();
                if (err == KErrNone)
                    {
                    Trace(_L("Create SMS entry OK."));
                    Trace(_L("Service center: %S"),
                            &iEntry.iServiceCentre.iTelNumber);
                    Trace(_L("Type of number: %d"),
                            iEntry.iServiceCentre.iTypeOfNumber);
                    Trace(_L("Number plan: %d"),
                            iEntry.iServiceCentre.iNumberPlan);
                    Trace(_L("Message status: %d"), iEntry.iMsgStatus);

                    // Start to write PDU
                    iEntry.iIndex = -1;
                    iMobileSmsStore.Write(iStatus, iEntryPckg);
                    iState = ECMGWStateWritePDU;
                    SetActive();
                    }
                else
                    {
                    // Create failed
                    iState = ECMGWStateIdle;
                    iCallback->CreateCMSReplyAndComplete(KErrGsmSMSInvalidPDUModeParameter);
                    }                
                break;
                }
            case ECMGWStateWritePDU:
                {
                Trace(_L("Write successful. Index = %d"), iEntry.iIndex);
                
                iReply.Append(KCRLF);
                iReply.Append(KAtCMGW);
                iReply.AppendNum(iEntry.iIndex);
                iState = ECMGWStateIdle;
                iCallback->CreateReplyAndComplete(EReplyTypeOk, iReply);
                }
                break;
            default:
                iState = ECMGWStateIdle;
                iCallback->CreateReplyAndComplete(EReplyTypeError);
                break;
            }
        }
    else
        {
        iState = ECMGWStateIdle;
        iCallback->CreateCMSReplyAndComplete(err);
        }
    
    TRACE_FUNC_EXIT
    }

TInt CCMGWCommandHandler::RunError(TInt aError)
    {
    TRACE_FUNC_ENTRY
    
    delete iMobilePhoneSmspList;
    iMobilePhoneSmspList = NULL;
    iState = ECMGWStateIdle;
    iCallback->CreateCMSReplyAndComplete(aError);
    
    TRACE_FUNC_EXIT
    return KErrNone;
    }

void CCMGWCommandHandler::DoCancel() 
    {
    TRACE_FUNC_ENTRY
    
    switch (iState)
        {
        case ECMGWStateRetrieveSCA:
            {
            iRetrieveMobilePhoneSmspList->Cancel();
            break;
            }
        case ECMGWStateWritePDU:
            {
            iMobileSmsStore.CancelAsyncRequest(EMobilePhoneStoreWrite);
            break;
            }
        }
    iState = ECMGWStateIdle;
    
    TRACE_FUNC_EXIT
    }
/**
 * Parse parameters of +CMGW=<length>,<stat>
 */
TInt CCMGWCommandHandler::ParseParameters()
    {
    TRACE_FUNC_ENTRY
    
    TInt ret = KErrNone;
    iTPDULength = 0;
    iTPDUStat = 0; // default value
    TInt otherParams = 0;
    // Get length
    TInt retLength = iATCmdParser.NextIntParam(iTPDULength);
    // Get status
    TInt retStat = iATCmdParser.NextIntParam(iTPDUStat);
    // Get other parameters
    TInt retOther = iATCmdParser.NextIntParam(otherParams);
    // syntax error happens if
    // a)there is no param 1
    // b)there are 3 params
    // c)param 2 is not 0,1,2 or 3
    TBool noParam1 = (retLength != KErrNone);
    TBool badParam2 = (retStat == KErrGeneral);
    TBool tooManyParams = (retOther != KErrNotFound);
    
    if (noParam1 || badParam2 || tooManyParams)
        {
        ret = KErrArgument;
        }
    else
        {
        switch (iTPDUStat)
            {
            case 0:
                // to receive unread message
                iMessageStatus = RMobileSmsStore::EStoredMessageUnread;
                break;
            case 1:
                // to receive read message
                iMessageStatus = RMobileSmsStore::EStoredMessageRead;
                break;
            case 2:
                // Unsent is not supported in this version
                ret = KErrNotSupported;
                break;
            case 3:
                // Sent is not supported in this version
                ret = KErrNotSupported;
                break;
            default:
                ret = KErrArgument;
                break;
            }
        iEntry.iMsgStatus = iMessageStatus;
        }
    
    TRACE_FUNC_EXIT
    return ret;
    }

/**
 * Create an SMS entry from the PDU string
 */
TInt CCMGWCommandHandler::CreateSmsEntry()
    {
    TRACE_FUNC_ENTRY
    
    TInt err = KErrNone;
    
    // Check the length
    if (iTPDU.Length() != (iSCALength+iTPDULength+1)*2)
        {
        TRACE_FUNC_EXIT
        return KErrArgument;
        }
    
    RBuf8 buf;
    err = buf.Create(iTPDULength);
    if (err != KErrNone)
        {
        TRACE_FUNC_EXIT
        return err;
        }
    // Convert to binary format
    for(TInt i=(iSCALength+1)*2; i< iTPDU.Length(); i+=2)
        {
        TLex8 lex(iTPDU.Mid(i, 2));
        TUint8 val = 0;
        err = lex.Val(val, EHex);
        if (err != KErrNone)
            {
            buf.Close();
            TRACE_FUNC_EXIT
            return err;
            }
        buf.Append(val);
        }
    iEntry.iMsgData.Copy(buf);
    
    buf.Close();
    TRACE_FUNC_EXIT
    return KErrNone;
    }

/**
 * Extract the SMS service center address from the head of PDU string
 */
TInt CCMGWCommandHandler::ExtractSCA()
    {
    TRACE_FUNC_ENTRY
    
    TInt err = KErrNone;
    TLex8 lex;
    RMobilePhone::TMobileAddress sca;
    // SCA length
    lex.Assign(iTPDU.Left(2));
    err = lex.Val(iSCALength, EHex); 
    if (err != KErrNone)
        {
        TRACE_FUNC_EXIT
        return err;
        }
    TInt length = iTPDU.Length();
    if (iSCALength == 0)
        {
        // Service center is not found in PDU
        err = KErrNotFound;
        }
    else if (iSCALength > (length-2)/2)
        {
        // Service certer length error
        err = KErrArgument;
        }
    else
        {
        // SCA is given
        // Parse SCA TON and NPI
        TUint8 val = 0;
        lex.Assign(iTPDU.Mid(2,2));
        err = lex.Val(val, EHex);
        if (err != KErrNone)
            {
            TRACE_FUNC_EXIT
            return err;
            }
        TUint8 ton = (val&KSCATonBitMask)>>4;
        TUint8 npi = val&KSCANpiBitMask;
        switch (ton) // TON
            {
            case 0: // 000
                sca.iTypeOfNumber = RMobilePhone::EUnknownNumber;
                break;
            case 1: // 001
                sca.iTypeOfNumber = RMobilePhone::EInternationalNumber;
                sca.iTelNumber.Append('+');
                break;
            case 2: // 010
                sca.iTypeOfNumber = RMobilePhone::ENationalNumber;
                break;
            default: 
                // CMCC doesn't support other types
                TRACE_FUNC_EXIT
                return KErrArgument;
            }
        switch (npi) // NPI
            {
            case 0: // 0000
                sca.iNumberPlan = RMobilePhone::EUnknownNumberingPlan;
                break;
            case 1: // 0001
                sca.iNumberPlan = RMobilePhone::EIsdnNumberPlan;
                break;
            default:
                // CMCC doesn't support other number plans
                TRACE_FUNC_EXIT
                return KErrArgument;
            }
        // Extract SCA number
        for (TInt i=4; i<(iSCALength+1)*2; i+=2)
            {
            sca.iTelNumber.Append(iTPDU[i+1]);
            sca.iTelNumber.Append(iTPDU[i]);
            }
        if(sca.iTelNumber[sca.iTelNumber.Length()-1] == 'F' 
                || sca.iTelNumber[sca.iTelNumber.Length()-1] == 'f')
            {
            sca.iTelNumber.Delete(sca.iTelNumber.Length()-1, 1);
            }
        iServiceCentre = sca;
        }
    TRACE_FUNC_EXIT
    return err;
    }


