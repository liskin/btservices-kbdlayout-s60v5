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

#ifndef USSDREADMESSAGEIMPL_H
#define USSDREADMESSAGEIMPL_H

#include <etelmm.h>

class MUSSDCallback;

/**
 *  Class for receiving USSD data
 */
NONSHARABLE_CLASS( CUSSDReadMessageImpl ) : public CActive
{
public:
    static CUSSDReadMessageImpl* NewL(MUSSDCallback* aCallback, RMobileUssdMessaging& aUssdSession);
    ~CUSSDReadMessageImpl();

    TInt ReadUSSDMessage(TDes8& aReceivedData, 
                               RMobileUssdMessaging::TMobileUssdAttributesV1& aUSDAttribute );

private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave

private:
    CUSSDReadMessageImpl(MUSSDCallback* aCallback, 
                          RMobileUssdMessaging& aUSSDMessagingSession);
private:
    MUSSDCallback* iCallback;
    RMobileUssdMessaging& iUssdSession;
    };

#endif /* USSDREADMESSAGEIMPL_H */
