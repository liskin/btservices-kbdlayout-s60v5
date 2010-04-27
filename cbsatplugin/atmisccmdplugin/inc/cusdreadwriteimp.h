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

#ifndef CUSDREADWRITEIMP_H
#define CUSDREADWRITEIMP_H

#include <e32base.h>
#include <etelmm.h>

class MUSSDCallback;

NONSHARABLE_CLASS( CCUSSDReadMessageImpl ) : public CActive
{
public:
    static CCUSSDReadMessageImpl* NewL(MUSSDCallback* aCallback, RMobileUssdMessaging& aUssdSession);
    ~CCUSSDReadMessageImpl();

    TInt ReadUSSDMessage(TDes8& aReceivedData, 
                               RMobileUssdMessaging::TMobileUssdAttributesV1& aUSDAttribute );

private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave

private:
    CCUSSDReadMessageImpl(MUSSDCallback* aCallback, 
                          RMobileUssdMessaging& aUSSDMessagingSession);
private:
    MUSSDCallback* iCallback;
    RMobileUssdMessaging& iUssdSession;
    };

NONSHARABLE_CLASS( CCUSSDSendMessageImpl ) : public CActive
{
public:
     static CCUSSDSendMessageImpl* NewL(MUSSDCallback* aCallback, RMobileUssdMessaging& aUssdSession);
     ~CCUSSDSendMessageImpl();

    TInt HandleSendUSSDCmd(const TDesC8& aCmd, 
                           RMobileUssdMessaging::TMobileUssdAttributesV1& aUSDAttribute);

private: // methods from CActive
    virtual void RunL();
    virtual void DoCancel();
    // RunError not needed as RunL does not leave
    
private:
    CCUSSDSendMessageImpl(MUSSDCallback* aCallback, 
                          RMobileUssdMessaging& aUSSDMessagingSession);

private:
    MUSSDCallback* iCallback;        
    RMobileUssdMessaging& iUssdSession;
    };

#endif // CUSDREADWRITEIMP_H
