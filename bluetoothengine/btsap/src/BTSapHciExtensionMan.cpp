/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     This class holds the HCI Extension Conduit singleton 
*
*/


// INCLUDE FILES
#include <e32cmn.h>
#include "BTSapHciExtensionMan.h"
#include "debug.h"

// CONSTANT

/**
* A prefix patch in all commands for compatible reason with stack
*/
const TUint8 KHciExtensionCmdPatch[] = {0xFC, 0x00, 0x00};

/**
* Common prerfix of additional functionality command
*/
const TUint8 KHciExtensionCmdCommomPrefix[] = {0x00, 0xFC};

/**
* Command Channel ID (Type of command)
*/
// Channel Id of additional functionality command
const TUint KHciExtensionFunctionalityChannelID = 0xF0;

/**
* Command opcode and parameter total length
*/
// ENCRYPTION_KEY_LENGTH_READ_CMD
const TUint8 KEncryptionKeyLengthReadOpcode = 0x07;

const TInt KEncryptionKeyLengthReadCmdParamLen = 8;

/**
* Event constants
*/
const TInt KHciExtensionEventIndexOfOpcode = 2;

const TInt KEncryptionKeyLengthReadEventLen = 6;

// DATA TYPES

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBTHciExtensionMan::NewL()
// ---------------------------------------------------------
//
CBTHciExtensionMan* CBTHciExtensionMan::NewL()
    {
    CBTHciExtensionMan* self = new (ELeave) CBTHciExtensionMan();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// Destructor.
CBTHciExtensionMan::~CBTHciExtensionMan()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::~CBTHciExtensionMan >>")));
    if (iConduit)
        {
        iConduit->StopWaitingForEvent();
        delete iConduit;        
        }
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::~CBTHciExtensionMan <<")));   
    }

// ---------------------------------------------------------
// CBTHciExtensionMan::HandleReqeustL
// ---------------------------------------------------------
//
void CBTHciExtensionMan::GetEncryptionKeyLengthL(const TBTDevAddr& aBTDevAddr, TRequestStatus& aStatus)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::HandleRequestL >>")));
    
    TInt err = KErrNone; 
    
    CBTHciExtensionCmd* cmd = NULL;
    TBTDevAddrPckgBuf pckg;
    pckg = aBTDevAddr;

    cmd = CBTHciExtensionCmd::NewLC(KEncryptionKeyLengthReadOpcode); 

    cmd->SetParamL(pckg().Des());

    iRequestOpcode = cmd->Opcode();
    TPtrC8 ptr = cmd->DesC();
    TUint16 requestOpcode = ptr[0] << 8 | ptr[1];
    err = iConduit->IssueCommandL(requestOpcode, ptr.Mid(sizeof(KHciExtensionCmdPatch)));
    if (err)
        {
        BTSAP_TRACE_OPT(KBTSAP_TRACE_ERROR, BTSapPrintTrace(_L("[BTSap]  iConduit->IssueCommandL err %d"), err));
        User::Leave(err);
        }

    aStatus = KRequestPending;
    iStatus = &aStatus;    
    
    CleanupStack::PopAndDestroy(cmd);

    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::HandleRequestL <<")));
    }

// ---------------------------------------------------------
// CBTHciExtensionMan::GetResultL
// ---------------------------------------------------------
//
TInt CBTHciExtensionMan::GetResultL(TUint8& aKeyLength)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap] CBTHciExtensionMan::GetResultL: enc key length: %d"),iKeyLength ));
    if ( iStatus && iStatus->Int() == KRequestPending )
        {
        return KErrNotReady;
        }
    
    aKeyLength = iKeyLength;
    return KErrNone;
    }
   
// ---------------------------------------------------------
// CBTHciExtensionMan::CancelRequest
// ---------------------------------------------------------
//   
void CBTHciExtensionMan::CancelRequest()
    {
    iConduit->StopWaitingForEvent();
    }
    
// ---------------------------------------------------------
// CBTHciExtensionMan::CommandCompleted
// ---------------------------------------------------------
//
void CBTHciExtensionMan::CommandCompleted(TInt aError)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::CommandCompleted(%d)"), aError));
    if(aError == KErrNone)
        {
        aError = iConduit->WaitForEvent();
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::CommandCompleted, iConduit->WaitForEvent %d"), aError));
        }
    if (aError != KErrNone)
        {
        User::RequestComplete(iStatus, aError);
        }
    }

// ---------------------------------------------------------
// CBTHciExtensionMan::ReceiveEvent
// ---------------------------------------------------------
//
TBool CBTHciExtensionMan::ReceiveEvent(TDesC8& aEvent, TInt aError)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::ReceiveEvent(aError %d) >>"), aError));
    if (aError != KErrNone)
        {
        User::RequestComplete(iStatus, aError);
        return EFalse;
        }
        
    CBTHciExtensionCmdEvent* event = NULL;
    TInt err;
    TRAP(err, event = CBTHciExtensionCmdEvent::NewL(aEvent));
    if (!event)
        {
        User::RequestComplete(iStatus, err);
        return EFalse;
        }
    if (event->Opcode() != iRequestOpcode)
        {
        User::RequestComplete(iStatus, KErrArgument);
		delete event;
        return EFalse;
        }
    switch (iRequestOpcode)
        {
        case KEncryptionKeyLengthReadOpcode:
            {
            TPtrC8 desptr = event->DesC();
            iKeyLength = desptr[desptr.Length() - 1];
            err = KErrNone;
            break;
            }
        default:
            {
            err = KErrNotSupported;
            break;
            }
        }
    delete event;
    User::RequestComplete(iStatus, err);
    
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::ReceiveEvent %d <<"), err));    
    return EFalse;
    }

// ---------------------------------------------------------
// CBTHciExtensionMan::ConstructL
// ---------------------------------------------------------
//
void CBTHciExtensionMan::ConstructL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::ConstructL >>")));
	iConduit = CHciExtensionConduit::NewL(*this);
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionMan::ConstructL, iConduit %d <<"), iConduit));
    }

// ---------------------------------------------------------
// CBTHciExtensionMan::CBTHciExtensionMan
// ---------------------------------------------------------
//
CBTHciExtensionMan::CBTHciExtensionMan(): iKeyLength(0)
    {
    }

// ---------------------------------------------------------
// CBTHciExtensionCmd::NewL
// ---------------------------------------------------------
//
CBTHciExtensionCmd* CBTHciExtensionCmd::NewL(TUint8 aOpcode)
    {
    CBTHciExtensionCmd* self = CBTHciExtensionCmd::NewLC(aOpcode);
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CBTHciExtensionCmd::NewLC
// ---------------------------------------------------------
//
CBTHciExtensionCmd* CBTHciExtensionCmd::NewLC(TUint8 aOpcode)
    {
    CBTHciExtensionCmd* self = new (ELeave) CBTHciExtensionCmd(aOpcode);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }
    
// Destructor.
CBTHciExtensionCmd::~CBTHciExtensionCmd()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::~CBTHciExtensionCmd >>")));
    delete iCmdDes;
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::~CBTHciExtensionCmd <<")));
    }

// ---------------------------------------------------------
// CBTHciExtensionCmd::Opcode
// ---------------------------------------------------------
//    
TUint8 CBTHciExtensionCmd::Opcode() const
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::Opcode %d >>"), iOpcode));
    return iOpcode;
    }

// ---------------------------------------------------------
// CBTHciExtensionCmd::CmdDesC
// ---------------------------------------------------------
//
TPtrC8 CBTHciExtensionCmd::DesC() const
    {
    return iCmdDes->Des();
    }

// ---------------------------------------------------------
// CBTHciExtensionCmd::SetParamL
// ---------------------------------------------------------
//  
void CBTHciExtensionCmd::SetParamL(const TDesC8& aParam)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::SetParamL param len %d >>"), aParam.Length()));
    TInt length = aParam.Length();
    if (length != iParamTotalLength - 2)
        {
        User::Leave(KErrArgument);
        }
    iCmdDes->Des().Replace(iCmdLength - length, length, aParam);
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::SetParamL <<")));
    }

// ---------------------------------------------------------
// CBTHciExtensionCmd::CBTHciExtensionCmd
// ---------------------------------------------------------
//  
CBTHciExtensionCmd::CBTHciExtensionCmd(TUint8 aOpcode)
    : iOpcode(aOpcode)
    {
    }

// ---------------------------------------------------------
// CBTHciExtensionCmd::ConstructL
// ---------------------------------------------------------
//
void CBTHciExtensionCmd::ConstructL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::ConstructL >>")));
    switch (iOpcode)
        {
        case KEncryptionKeyLengthReadOpcode:
            {
            iChannelID = KHciExtensionFunctionalityChannelID;
            iParamTotalLength = KEncryptionKeyLengthReadCmdParamLen;
            iCmdLength = sizeof(KHciExtensionCmdPatch) +
                sizeof(KHciExtensionCmdCommomPrefix) + iParamTotalLength + 1;
            break;
            }
        default:
            User::Leave(KErrArgument);
        }
    InitializeCmdDesL();
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::ConstructL <<")));
    }

// ---------------------------------------------------------
// CBTHciExtensionCmd::InitializeCmdDesL
// ---------------------------------------------------------
//
void CBTHciExtensionCmd::InitializeCmdDesL()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::InitializeCmdDesL >>")));
    iCmdDes = HBufC8::NewL(iCmdLength);
    TPtr8 cmdPtr = iCmdDes->Des();
    cmdPtr.Append(KHciExtensionCmdPatch, sizeof(KHciExtensionCmdPatch));
    cmdPtr.Append(KHciExtensionCmdCommomPrefix, sizeof(KHciExtensionCmdCommomPrefix));
    cmdPtr.Append(iParamTotalLength);
    cmdPtr.Append(iChannelID);
    cmdPtr.Append(iOpcode);
    cmdPtr.AppendFill(0, iParamTotalLength - 2);
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmd::InitializeCmdDesL, len %d <<"), cmdPtr.Length()));
    }

CBTHciExtensionCmdEvent* CBTHciExtensionCmdEvent::NewL(const TDesC8& aEvent)
    {
    CBTHciExtensionCmdEvent* self = CBTHciExtensionCmdEvent::NewLC(aEvent);
    CleanupStack::Pop();
    return self;    
    }

CBTHciExtensionCmdEvent* CBTHciExtensionCmdEvent::NewLC(const TDesC8& aEvent)
    {
    CBTHciExtensionCmdEvent* self = new (ELeave) CBTHciExtensionCmdEvent();
    CleanupStack::PushL(self);
    self->ConstructL(aEvent);
    return self;
    }
    
CBTHciExtensionCmdEvent::~CBTHciExtensionCmdEvent()
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmdEvent::~CBTHciExtensionCmdEvent >>")));
    delete iEventDes;
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmdEvent::~CBTHciExtensionCmdEvent <<")));
    }
    
TUint8 CBTHciExtensionCmdEvent::Opcode() const
    {
    return (*iEventDes)[KHciExtensionEventIndexOfOpcode];
    }

TPtrC8 CBTHciExtensionCmdEvent::DesC() const
    {
    return iEventDes->Des();
    }

CBTHciExtensionCmdEvent::CBTHciExtensionCmdEvent()
    {
    }

void CBTHciExtensionCmdEvent::ConstructL(const TDesC8& aEvent)
    {
    BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmdEvent::ConstructL len %d >>"), aEvent.Length()));
    if (aEvent.Length() > KHciExtensionEventIndexOfOpcode)
        {
        TInt desLength = 0;
        // Get the opcode of this event
        TUint8 opcode = aEvent[KHciExtensionEventIndexOfOpcode];
        BTSAP_TRACE_OPT(KBTSAP_TRACE_INFO, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmdEvent::ConstructL opcode %d >>"), opcode));  
        switch (opcode)
            {
            case KEncryptionKeyLengthReadOpcode:
                {
                if (aEvent.Length() != KEncryptionKeyLengthReadEventLen)
                    {
                    User::Leave(KErrArgument);
                    }
                desLength = KEncryptionKeyLengthReadEventLen;
                break;
                }
            default:
                User::Leave(KErrNotSupported);
            }
        iEventDes = HBufC8::NewL(desLength);
        TPtr8 ptr = iEventDes->Des();
        ptr.Append(aEvent);        
        }
    else
        {
        User::Leave(KErrArgument);
        }
    BTSAP_TRACE_OPT(KBTSAP_TRACE_FUNCTIONS, BTSapPrintTrace(_L("[BTSap]  CBTHciExtensionCmdEvent::ConstructL <<")));
    }

//  End of File
