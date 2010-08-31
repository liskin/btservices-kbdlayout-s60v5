/*
* Copyright (c) 2007-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Obexserviceman client handling for usbobex api
*
*/



#ifndef CUSBCLIENTWRAPPER_H
#define CUSBCLIENTWRAPPER_H


#include <e32base.h>
#include "obexsmclient.h"


/**
 *   Obexserviceman client handling for usbobex api
 * 
 * 
 *  @since S60 S60 v3.2
 */
NONSHARABLE_CLASS (CUSBClientWrapper) : public CBase
    {


public:

    static CUSBClientWrapper* NewL();          

    /**
    * Destructor.
    */
    virtual ~CUSBClientWrapper();

    /**
     * Turn USB services on or off
     *
     * @since S60 v3.2
     * @param aUSBStatus the requested USB service status
     * @param aStatus the Request status
     * @return None
     */
     void  ManageUSBServices(TBool aUSBStatus, TRequestStatus &aStatus);     
    
    /**
     * Cancel managment request and turn services off   
     * @since S60 v3.2
     *
     */
    void  CancelManageUSBServices();        

private:


    CUSBClientWrapper();

    void ConstructL();
    
private: // data

    /**
     * ObexServer Manager client 
     */
    RObexSMServer iServer; 

    };
#endif //   CUSBCLIENTWRAPPER_H
