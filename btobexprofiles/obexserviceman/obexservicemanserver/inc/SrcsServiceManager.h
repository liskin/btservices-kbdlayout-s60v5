/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class handles service management requests.
*
*/


#ifndef _SRCSSERVICEMANAGER_H
#define _SRCSSERVICEMANAGER_H

//  INCLUDES
#include <e32base.h>
#include "obexserviceman.h"
#include "SrcsTransport.h"
#include "SrcsInterface.h"



// CONSTANTS


// FORWARD DECLARATIONS

/**
 *  Callback interface for completing request
 * 
 *  @since S60 v3.2
 */ 
class MObexSMRequestObserver
    {   
     public:        
        /**
        * Comleting request
        *
        * @since    S60 v3.2
        * @param    aMessage Reference to message to be completed
        * @param    aError   Possible error code             
        */
        virtual void RequestCompleted(const RMessage2 &aMessage, TInt aError)=0;   
    };
/**
*  SRCS service manager class.
*  It handles all service requests.
*/ 

class CSrcsServiceManager: public CActive
    {
public:

    /**
    * Two-phased constructor.
    */
    static CSrcsServiceManager* NewL();

    /**
    * Destructor.
    */
    virtual ~CSrcsServiceManager();
        
public:

   /**
    * Manage services on or off
    *
    * @since    S60 v3.2
    * @param    aTransport  Transporter name for request
    * @param    aState      Request On or Off
    * @param    aObserver   Callback interface for completing request
    * @param    aMessage    Reference to message
    * @return   TInt    Indicates if the service request has succeed.
    */
    TInt ManageServices( TSrcsTransport aTransport, TBool aState, 
                                            MObexSMRequestObserver* aObserver, 
                                            const RMessage2& aMessage );
   /**
    * Manage services on or off
    *
    * @since    S60 v3.2
    * @param    aTransport  Transporter name for request
    * @param    aState      Request On or Off
    * @param    aObserver   Callback interface for completing request
    * @param    aMessage    Reference to message
    * @return   None
    */    
    void DoManageServices( TSrcsTransport aTransport, TBool aState, MObexSMRequestObserver* aObserver, 
                                            const RMessage2& aMessage);
   /**
    * Manage services on or off
    *
    * @since    S60 v3.2
    * @param    aTransport  Transporter name for request
    * @param    aState      Request On or Off    
    * @return   None
    */                                            
    void RealDoManageServiceL(TSrcsTransport aTransport, TBool aState);	
    
private:
   /**
    * Second phase constructor.
    */
    void ConstructL();
    
   /**
    * From CActive
    *
    * @since    S60 v3.2
    */
    void RunL();
    
    
   /**
    * From CActive
    *
    * @since    S60 v3.2
    */
    void RunError();
    
    
   /**
    * From CActive
    *
    * @since    S60 v3.2
    */
    void DoCancel();    

    /**
    * Handling of Service array 
    * 
    */	
    void ServiceArrayL(CArrayPtr<CSrcsTransport> &aTransport, TBool aState);
    
private:
    
   /**
    * C++ default constructor.
    */
    CSrcsServiceManager();
    
    /*
     * Perform service controllers post-initialization
     */
    void PostInitialize(CArrayPtr<CSrcsTransport> &aTransport);
    
    CArrayPtr<CSrcsTransport>*		iBTConnectionArray;		// array of BT Connections
    CArrayPtr<CSrcsTransport>*		iUSBConnectionArray;	// array of USB Connections
    CArrayPtr<CSrcsTransport>*		iIrDAConnectionArray;	// array of IrDA Connections
    MObexSMRequestObserver*         iRequestObserver;	
    MObexSMRequestObserver*         iObserver; 
    RMessage2                       iMessage;
    TInt                            iErrorState;
    TPtrC8                          iTransportName;     // Service name
    TSrcsTransport                  iTransportType;     // Service type
    TBool                           iTransportState;    // Service On/Off
    };
#endif      // SRCSSERVICEMANAGER_H

// End of File
