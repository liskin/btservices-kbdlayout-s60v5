/* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
#include "cusdcommandhandler.h"

#include <exterror.h> 

#include "ussdreadmessageimpl.h"
#include "ussdsendmessageimpl.h"

#include "atmisccmdpluginconsts.h"
#include "debug.h"

const TUint8 KPhCltUssdCarriageReturn = 0x0d;
const TUint8 KPhCltUssdDcsNotSet  = 0x00; // not set

const TUint8 KQuarterShift( 2 );
const TUint8 KHighNibbleShift( 4 );
const TUint8 KDcsCharacterSet7Bit( 0x00 );
const TUint8 KDcsCharacterSet8Bit( 0x01 );
const TUint8 KDcsCharacterSet16Bit( 0x02 );
const TUint8 KDcsCharacterSet7Bit2( 0x00 );
const TUint8 KDcsCharacterSet16Bit2( 0x01 );

CCUSDCommandHandler* CCUSDCommandHandler::NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone)
    {
    TRACE_FUNC_ENTRY
    CCUSDCommandHandler* self = new (ELeave) CCUSDCommandHandler(aCallback, aATCmdParser, aPhone);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    TRACE_FUNC_EXIT
    return self;
    }

CCUSDCommandHandler::CCUSDCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone) :
    CATCmdSyncBase(aCallback, aATCmdParser, aPhone)
    {
    TRACE_FUNC_ENTRY
    iSendAttributes.iDcs = KPhCltUssdDcsNotSet;  
    iSendAttributes.iType = RMobileUssdMessaging::EUssdMORequest;
    iSendAttributes.iFlags = RMobileUssdMessaging::KUssdMessageType | RMobileUssdMessaging::KUssdMessageDcs | RMobileUssdMessaging::KUssdDataFormat;
    TRACE_FUNC_EXIT
    }

void CCUSDCommandHandler::ConstructL()
    {    
    TRACE_FUNC_ENTRY
    iReply.CreateL(KDefaultCmdBufLength);

    User::LeaveIfError(iUssdMessaging.Open(iPhone));
    
    iUSSDSender = CUSSDSendMessageImpl::NewL(this, iUssdMessaging);
    iUSSDReader = CUSSDReadMessageImpl::NewL(this, iUssdMessaging);
    
    //start a read session for network based ussd requests
    User::LeaveIfError(iUSSDReader->ReadUSSDMessage(iReceivedData, iReceiveAttributes));
    TRACE_FUNC_EXIT
    }

CCUSDCommandHandler::~CCUSDCommandHandler()
    {
    TRACE_FUNC_ENTRY
    delete iUSSDSender;
    delete iUSSDReader;
    
    iReply.Close();
    iUssdMessaging.Close();
    TRACE_FUNC_EXIT
    }

void CCUSDCommandHandler::HandleCommand( const TDesC8& /*aCmd*/,
                                         RBuf8& /*aReply*/,
                                         TBool /*aReplyNeeded*/ )
    {
    TRACE_FUNC_ENTRY
    TAtCommandParser::TCommandHandlerType cmdHandlerType = iATCmdParser.CommandHandlerType();
    iReply.Zero();

    switch (cmdHandlerType)
        {
        case (TAtCommandParser::ECmdHandlerTypeTest):
            {
            iReply.Append(KCRLF);
            iReply.Append(KCUSDSupportedCmdsList);
            iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply );
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeRead):
            {
            iReply.Append(KCRLF);
            iReply.Append(KAtCUSD);
            iReply.AppendNum(iDisplayRetMessage);
            iReply.Append(KCRLF);

            iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply );
            break;
            }
        case (TAtCommandParser::ECmdHandlerTypeSet):
            {         
            if(ParseCUSDCmd() == KErrNone)
               {
               if(iUSSDCommand.Length() == 0)
                   {
                   Trace(KDebugPrintD, "No USSD command: ", iUSSDCommand.Length());
                   
                   // the USSD command is not set then this is a set                   
                   iReply.Append( KOKCRLF );
                   iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply);
                   }
                else
                    {
                    if(iUSSDSender->SendUSSDCmd(iUSSDCommand, iSendAttributes) == KErrNone)
                        {
                        iSendCommandSent = ETrue;
                        }
                    else
                        {
                        iCallback->CreateReplyAndComplete(EReplyTypeError);
                        }                 
                    }
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

void CCUSDCommandHandler::HandleCommandCancel()
    {
    TRACE_FUNC_ENTRY   
    iUSSDSender->Cancel();
    iSendCommandSent = EFalse;
    TRACE_FUNC_EXIT
    }

TInt CCUSDCommandHandler::ParseCUSDCmd()
    {
    TRACE_FUNC_ENTRY
    
    TInt tempDispValue;
    TInt ret = iATCmdParser.NextIntParam(tempDispValue);
    
    
    if (ret == KErrNone && tempDispValue >= 0 && tempDispValue < 3)
        {
        Trace(KDebugPrintD, "tempDispValue: ", tempDispValue);
        iDisplayRetMessage = tempDispValue;
        }    
    else if (ret != KErrNotFound)
        {
        Trace(KDebugPrintD, "First NextIntParam failed: ", ret);
        TRACE_FUNC_EXIT
        return KErrGeneral;       
        }
    
    // now parse the USSD command
    TPtrC8 command;
    ret = iATCmdParser.NextTextParam(command);
    if (ret == KErrNone )
        {
        if(iUSSDCommand.Create(command.Length()) != KErrNone)
            {
            Trace(KDebugPrintD, "iUSSDCommand.Create() failed ", ret);
            TRACE_FUNC_EXIT
            return KErrGeneral;
            }
    
        Trace(KDebugPrintS, "command: ", &command);
        if(Pack(command, iUSSDCommand) < 0)
            {
            Trace(KDebugPrintS, "Pack() returned error, command: ", &command);
            TRACE_FUNC_EXIT
            return KErrGeneral;
            }    
        }
    else if (ret != KErrNotFound)
        {
        TRACE_FUNC_EXIT
        return KErrGeneral;       
        }
    TInt dcs = 0; // 3GPP TS 23.038 [25] Cell Broadcast Data Coding Scheme in integer format (default 0)
    ret = iATCmdParser.NextIntParam(dcs); 

    Trace(KDebugPrintD, "ret: ", ret);
    Trace(KDebugPrintD, "IsDcsValid(dcs): ", IsDcsValid(dcs));
    Trace(KDebugPrintD, "iATCmdParser.NextParam().Length(): ", iATCmdParser.NextParam().Length());
    if(ret != KErrNone && ret != KErrNotFound || !IsDcsValid(dcs) || 
            iATCmdParser.NextParam().Length() != 0)
        {
        Trace(KDebugPrintD, "Second NextIntParam failed: ", ret);
        TRACE_FUNC_EXIT
        return KErrGeneral;       
        }
    else
        {
        // dcs will have the default value if no parameter is given in the command line
        Trace(KDebugPrintD, "dcs ", dcs);   
        iSendAttributes.iDcs = dcs;
        }

    TRACE_FUNC_EXIT
    return KErrNone;
    }

void CCUSDCommandHandler::HandleSendMessageComplete(TInt aError)
    {
    TRACE_FUNC_ENTRY
    if (aError != KErrNone)
        {
        // Only respond from a Send USSD request if there has been an error
        iCallback->CreateReplyAndComplete(EReplyTypeError);
        }
    else if (iDisplayRetMessage == 0)
        {
        // if display mode is 0 then no response apart from OK should be displayed
        iCallback->CreateReplyAndComplete( EReplyTypeOther, KOKCRLF );
        }
   
    iUSSDCommand.Close();
    TRACE_FUNC_EXIT
    }

void CCUSDCommandHandler::HandleReadMessageComplete(TInt aError)
    {
    TRACE_FUNC_ENTRY
    if(aError == KErrNone)
        {
        if(iDisplayRetMessage)
            {
            FormatUSSDReceivedData();
            
            if(iSendCommandSent)
                {
                iCallback->CreateReplyAndComplete( EReplyTypeOther, iReply );
                }
             else
                {       
                iCallback->HandleUnsolicitedRequest(iReply);
                }
            }
        
        // as we have received the data initiated by the send USSD request we set
        // this parameter to EFalse
        iSendCommandSent = EFalse;
    
        // re-issue the read request
        iUSSDReader->ReadUSSDMessage(iReceivedData, iReceiveAttributes);
        }
    else
        {
        iCallback->CreateReplyAndComplete(EReplyTypeError);
        }
    TRACE_FUNC_EXIT
    }

TInt CCUSDCommandHandler::Pack(const TDesC8& aIn, TDes8& aOut)
    {
    TRACE_FUNC_ENTRY
    TInt startBit = 0; 
    
    // Ensure we've got the right length
    TInt packedOctetsRequired = PackedOctetsRequired( aIn.Length() );
    if (packedOctetsRequired > ( aOut.MaxLength() - aOut.Length() ) )
        {
        Trace(KDebugPrintD, "packedOctetsRequired: ", packedOctetsRequired);
        TRACE_FUNC_EXIT
        return KErrOverflow;
        }

    // Do the conversion
    TInt elementSizeInBits = ElementSizeInBits(TSmsDataCodingScheme::ESmsAlphabet7Bit);
    
    Trace(KDebugPrintD, "elementSizeInBits = : ", elementSizeInBits);
    if (elementSizeInBits == 8) // 8 bit data
        {
        // Straight copy here
        aOut.Append( aIn );
        }
    else if (elementSizeInBits == 7) // Need packing to 7-bit
        {
        Trace(KDebugPrintS, "aOut: ", &aOut);
        Trace(KDebugPrintS, "aIn: ", &aIn);

        // Get raw pointers and do packing
        TUint8* outPtr = ( TUint8* )aOut.Ptr() + aOut.Length();
        const TUint8* inPtr = aIn.Ptr();
    
        TInt inLength = aIn.Length();
        outPtr[0] = 0;
        
        Trace(KDebugPrintD, "inLength = : ", inLength);
        for (TInt i = 0; i < inLength; ++i)
           {
           TUint8 to = inPtr[i];
           *outPtr |= ( to << startBit );
            
           if (startBit)
               {
               ++outPtr;
               *outPtr = ( TUint8 ) ( to >> ( 8 - startBit ) );
               }

           startBit = (startBit + 7) % 8; // roll 0,1,2,3,4,5,6,7,0,1,2,...
           }
       
        Trace(KDebugPrintD, "startBit = : ", startBit);
        
        if ((inPtr[inLength - 1] == KPhCltUssdCarriageReturn && startBit == 0) || startBit == 1 )
            {
            //We have to add one CR more
            *outPtr |= ( KPhCltUssdCarriageReturn << startBit );

            if (startBit)
                {
                ++outPtr;
                *outPtr = (TUint8) (KPhCltUssdCarriageReturn >> (8 - startBit));
                }
            else
                {
                ++packedOctetsRequired;
                }
            startBit = (startBit + 7)%8;
            }
        
        // Increment the length for the packed data
        aOut.SetLength(aOut.Length() + packedOctetsRequired);
        
        Trace(KDebugPrintD, "aOut.Length() = : ", aOut.Length());
        }

    // Return number of bytes used
    TRACE_FUNC_EXIT
    return packedOctetsRequired;
    }

 TInt CCUSDCommandHandler::PackedOctetsRequired(TInt aNumUDUnits) const
    {
    TRACE_FUNC_ENTRY
    TInt startBit = 0; 

    TInt octetsRequired = 0;
    TInt elementSizeInBits = ElementSizeInBits(TSmsDataCodingScheme::ESmsAlphabet7Bit);
    if (elementSizeInBits == 8)
         {
        octetsRequired=aNumUDUnits;
         }
    else
        {
        octetsRequired =  (startBit + aNumUDUnits*elementSizeInBits + 7 )/8;    // Rounds up
        }
    TRACE_FUNC_EXIT
    return octetsRequired;
    }

 TBool CCUSDCommandHandler::IsDcsValid( const TUint8 aDcs ) const
     {
     TRACE_FUNC_ENTRY
     TBool isDcsValid(EFalse);
                                                                //      76543210
     TUint8 codingGroup  = ( aDcs & 0xF0 ) >> KHighNibbleShift; // bits XXXX____
     TUint8 characterSet = ( aDcs & 0x0C ) >> KQuarterShift;    // bits ____XX__
     TUint8 lowQuartet   = ( aDcs & 0x0F );                     // bits ____XXXX
  
     switch (codingGroup)
         {
         case 0x00:
         case 0x02:
         case 0x03:
         case 0x0F:
             {
             isDcsValid = ETrue;
             break;
             }

         case 0x01:
             {
             if ( ( KDcsCharacterSet7Bit2 == lowQuartet ) ||
                  ( KDcsCharacterSet16Bit2 == lowQuartet ) )
                 {
                 isDcsValid = ETrue;
                 }
             break;
             }

         case 0x04:
         case 0x05:
         case 0x06:
         case 0x07:
         case 0x09:
             {
             if ( ( KDcsCharacterSet7Bit == characterSet ) ||
                  ( KDcsCharacterSet8Bit == characterSet ) ||
                  ( KDcsCharacterSet16Bit == characterSet ) )
                 {
                 isDcsValid = ETrue;
                 }
             break;
             }
         default:
             {
             break;
             }
         }
     TRACE_FUNC_EXIT
     return isDcsValid;
     }
 
TInt CCUSDCommandHandler::ElementSizeInBits(TSmsDataCodingScheme::TSmsAlphabet aAlphabet) const
    {
    TRACE_FUNC_ENTRY
    TInt ret;
    switch (aAlphabet)
       {
       case TSmsDataCodingScheme::ESmsAlphabet7Bit:
           {
           ret = 7;
           break;
           }
       case TSmsDataCodingScheme::ESmsAlphabet8Bit:
       case TSmsDataCodingScheme::ESmsAlphabetUCS2:
           {
           ret = 8;
           break;
           }
       default:
           {
           ret = KErrGsmSMSDataCodingSchemeNotSupported;
           }
        }
    TRACE_FUNC_EXIT
    return ret;
    }

void CCUSDCommandHandler::FormatUSSDReceivedData()
    {
    TRACE_FUNC_ENTRY
    iReply.Zero();
    
    iReply.Append(KCRLF);
    iReply.Append(KAtCUSD);
    iReply.AppendNum(iDisplayRetMessage);
    iReply.Append(',');
    iReply.Append('"');
    
    TInt length = iReceivedData.Length();
    for(TInt i = 0; i < length; ++i)
       {       
       iReply.AppendNum(iReceivedData[i], EHex);
       }
    iReply.Append('"');
    iReply.Append(',');

    iReply.AppendNum(iReceiveAttributes.iDcs);
    iReply.Append(KCRLF);
    TRACE_FUNC_EXIT
    }
