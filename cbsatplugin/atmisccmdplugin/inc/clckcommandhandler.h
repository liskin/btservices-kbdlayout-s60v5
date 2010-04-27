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
 * CCLCKCommandHandler class declaration for AT+CLCK command
 */

#ifndef CLCKCOMMANDHANDLER_H
#define CLCKCOMMANDHANDLER_H

#include <mmretrieve.h>

#include "atmisccmdplugin.h"

class CSecurityCodeVerifier;
class CSecurityEventHandler;
class CCBSettingHandler;

enum TInfoClassMask
    {
    EInfoClassVoice      = 1,   // voice (telephony) 
    EInfoClassData       = 2,   // data (refers to all bearer services; with <mode>=2
                                // this may refer only to some bearer service if TA does
                                // not support values 16, 32, 64 and 128)
    EInfoClassFax        = 4,   // fax (facsimile services)
    EInfoClassSMS        = 8,   // short message service
    EInfoClassSyncData   = 16,  // data circuit sync
    EInfoClassASyncData  = 32,  // data circuit async
    EInfoClassPacketData = 64,  // dedicated packet access
    EInfoClassPadAccess  = 128  // dedicated PAD access
    };

/**
 *  AT+CLCK command handler implementation class
 */
NONSHARABLE_CLASS( CCLCKCommandHandler ) : public CATCmdAsyncBase
    {
public:
    static CCLCKCommandHandler* NewL(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    ~CCLCKCommandHandler();
    
    // Callback methods for helper objects
    void HandlePasswordVerification(TInt aError);
    void HandleSecurityEvent(TInt aError, RMobilePhone::TMobilePhoneSecurityEvent aSecurityEvent);
    
private:
    /**
     * States for lock setting
     */
    enum TLockSettingState
        {
        ELockSettingIdle = 0x01,
        ELockSettingRequested = 0x02,
        ELockSettingPasswordRequested = 0x03
        };
    
    enum TCLCKCommand
        {
        ECLCKUndefined  = KErrNotFound,
        ECLCKLockSet    = 0x01,
        ECLCKLockGet    = 0x02,
        ECLCKBarringSet = 0x03,
        ECLCKBarringGet = 0x04
        };
    
    enum TCmdFacilityType
        {
        ECmdFacilityTypeUnknown = 0,
        ECmdFacilityTypeLock,
        ECmdFacilityTypeBarring,
        ECmdFacilityTypeAllBarring
        };
    
private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave

private: // methods from CATCmdAsyncBase    
    virtual void HandleCommand( const TDesC8& aCmd, RBuf8& aReply, TBool aReplyNeeded );
    virtual void HandleCommandCancel();
    
private:
    CCLCKCommandHandler(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);
    void ConstructL();
    
    TInt ParseCCLCKCommand();
    void IssueCLCKCommand();
    TInt ReceiveCBList();

private:
    CCBSettingHandler* iCBSettingHandler;
    CSecurityEventHandler* iSecurityEventHandler;
    CSecurityCodeVerifier* iSecurityCodeVerifier; 
    CRetrieveMobilePhoneCBList* iCBRetrieve;

    RBuf8 iReply;

    TCLCKCommand iCLCKCommandType;
    RBuf8 iPassword;
    TInt iInfoClass;
    
    TLockSettingState iLockSettingState;
    RMobilePhone::TMobilePhoneLock iLockType;
    RMobilePhone::TMobilePhoneLockSetting iLockChange;
    RMobilePhone::TMobilePhoneSecurityCode iSecurityCode;
    
    RMobilePhone::TMobilePhoneLockInfoV1 iLockInfo;
    RMobilePhone::TMobilePhoneLockInfoV1Pckg iLockInfoPckg;
    
    RMobilePhone::TMobilePhoneCBCondition iCondition;
    RMobilePhone::TMobilePhoneCBChangeV1 iCBInfo;
    };

#endif // CLCKCOMMANDHANDLER_H
