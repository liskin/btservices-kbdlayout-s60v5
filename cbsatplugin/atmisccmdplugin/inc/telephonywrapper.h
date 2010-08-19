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
 * CTelephonyWrapper class declaration for wrapping CTelephony::GetPhoneId 
 * 
 */

#ifndef TELEPHONYWRAPPER_H
#define TELEPHONYWRAPPER_H

#include <etel3rdparty.h>

/**
 *  Synchronous wrapper for CTelephony::GetPhoneId 
 */
NONSHARABLE_CLASS (CTelephonyWrapper) : public CActive   
    {     
public:
    static CTelephonyWrapper* NewL();         
    virtual ~CTelephonyWrapper();     

    TInt SynchronousGetPhoneId(); 

    const TDesC8& GetPhoneModel();
    const TDesC8& GetPhoneSerialNum();
    const TDesC8& GetPhoneManufacturer();

private: // From CActive
    virtual void RunL();
    virtual void DoCancel();
    
private: // Constructors
    CTelephonyWrapper();
    void ConstructL();
    
private:                
    CActiveSchedulerWait        iWaiter;
    
    CTelephony*                 iTelephony;
    CTelephony::TPhoneIdV1      iPhoneIdV1;
    CTelephony::TPhoneIdV1Pckg  iPhoneIdV1Pckg;    
    
    TBufC8<CTelephony::KPhoneModelIdSize> iModel;
    TBufC8<CTelephony::KPhoneSerialNumberSize> iSn;
    TBufC8<CTelephony::KPhoneManufacturerIdSize> iManufacturer;
    
    };

#endif /* TELEPHONYWRAPPER_H */
