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

#ifndef USSDSENDMESSAGEIMPL_H
#define USSDSENDMESSAGEIMPL_H

#include <etelmm.h>

class MUSSDCallback;

/**
 *  Class for sending USSD data
 */
NONSHARABLE_CLASS( CUSSDSendMessageImpl ) : public CActive
{
public:
     static CUSSDSendMessageImpl* NewL(MUSSDCallback* aCallback, RMobileUssdMessaging& aUssdSession);
     ~CUSSDSendMessageImpl();

    TInt SendUSSDCmd(const TDesC8& aCmd, 
                     RMobileUssdMessaging::TMobileUssdAttributesV1& aUSDAttribute);

private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave
    
private:
    CUSSDSendMessageImpl(MUSSDCallback* aCallback, 
                          RMobileUssdMessaging& aUSSDMessagingSession);

private:
    MUSSDCallback* iCallback;        
    RMobileUssdMessaging& iUssdSession;
    };

#endif /* USSDSENDMESSAGEIMPL_H */
