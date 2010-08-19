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
 * 
 * Description :
 * 
 * SCPBWCommandHandler class declaration for AT^SCPBW command
 * 
 */

#ifndef SCPBWCOMMANDHANDLER_H
#define SCPBWCOMMANDHANDLER_H

#include "atcmdasyncbase.h"

#include <etelmm.h>
#include <rmmcustomapi.h>

class CPhoneBookBuffer;

/**
 *  AT^SCPBW command handler implementation class
 */
NONSHARABLE_CLASS( CSCPBWCommandHandler ) : public CATCmdAsyncBase
    {
private:
    enum TSCPBWState
        {
        ESCPBWStateIdle,
        ESCPBWStateGetPhonebookInfo,
        ESCPBWStateGet3GPBInfo,
        ESCPBWStateWrite,
        ESCPBWStateDelete,
        ESCPBWStateSimStoreNotSupported 
        };
    
public:
    static CSCPBWCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CSCPBWCommandHandler();
        
private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();

private: // methods from CATCmdAsyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    
private:
    CSCPBWCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();

    TInt ParseParameters( TBool& aIsDeleteRequest );
    TInt CreateContactEntry();
    void ResetParameters();    
    void SetBuffer(TDes& aDest, const TDesC8& aSource);
    
private:

    RMobilePhoneBookStore   iPhoneBookStore;
    CPhoneBookBuffer*       iPhoneBookBuffer;
    RMobilePhoneBookStore::TMobilePhoneBookInfoV1 iPhoneBookStoreInfo;
    RMmCustomAPI    iMmCustomAPI;
    RMmCustomAPI::T3GPBInfo i3GPBInfo;
    
    TInt    iIndex;
    RBuf    iNum1;
    TInt    iType1;
    RBuf    iNum2;
    TInt    iType2;
    RBuf    iNum3;
    TInt    iType3;
    RBuf    iNum4;
    TInt    iType4;
    RBuf    iText;
    TInt    iCoding;
    RBuf    iEmail;
    
    TInt    iNumLength;
    TInt    iTextLength;
    TInt    iEmailLength;
    TInt    iMaxEntries;

    RBuf8   iPbData;
    
    TSCPBWState iState;
    
    RBuf8 iReply;
    
    };

#endif /* SCPBWCOMMANDHANDLER_H */
