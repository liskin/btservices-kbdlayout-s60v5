/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  CObexSMPasskey definition
*
*/


#ifndef _OBEXSMPASSKEY_H
#define _OBEXSMPASSKEY_H

#include <btnotif.h>

class CObexServer;

/**
 *  Class to handle BT Obex Passkey events
 */
class CObexSMPasskey : public CActive
    {
public:
    static CObexSMPasskey* NewL();
    ~CObexSMPasskey();
    
    void StartPassKeyRequestL(CObexServer* aObexServer);
    
private:
    CObexSMPasskey();
    void Cleanup();

private: // from CActive
    void DoCancel();
    void RunL();
    TInt RunError(TInt aError);

private:
    RNotifier iNotif;               // For Obex passkey notifier
    TObexPasskeyBuf iObexPasskey;   // Obex passkey
    CObexServer* iObexServer;       // Obex Server
    };

#endif

// End of File
