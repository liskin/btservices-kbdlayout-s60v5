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
* Description:  This class handles service connection settings.
*
*/



#ifndef OBEXSMUSBCONNECTION_H
#define OBEXSMUSBCONNECTION_H

//  INCLUDES
#include <e32base.h>


#include <obex.h>
#include "SrcsTransport.h"
#include "SrcsInterface.h"


// CLASS DECLARATION

    
/**
 * Class to handle USB connection
 */
class CObexSMUsbConnection: public CSrcsTransport
    {
public:   
   /**
    * Two-phased constructor.
	* @param	aInitParams initial parameters
    */
    static CObexSMUsbConnection* NewL(TAny* aInitParams);

   /**
    * Destructor.
    */
    virtual ~CObexSMUsbConnection();

   /** 
	* Check if OBEX service is still active.
    * @return ETrue: OBEX service is still active.
	*		  EFalse: OBEX service is inactive.
	*/
	TBool IsOBEXActive();
	    
   /** 
    * Perform any outstanding initialization tasks
    *
    * @return none.
    */
   void PostInitializeL();

private:

   /**
    * C++ default constructor.
    */
    CObexSMUsbConnection();

   /**
    * Constructor.
    *
    * @param	aInitParams initial parameters
	*/
    CObexSMUsbConnection(TAny* aInitParams);

   /**
    * Constructor
    * registers to sdp and btmanager
    * sets up the protocol stack
    * creates new messagehandler
    */
    void ConstructL();

   /**
    * MObexAuthChallengeHandler implementation
    */

   /**
    * Password for authenticated connection
    *
	* @param	aUserID		User ID.
    */
    void GetUserPasswordL( const TDesC& aUserID );
    
    

private:
    CObexServer* iServer;                               // Obex Server
    CSrcsInterface* iController;                        // Service Controller callback
    CImplementationInformation* iImplementationInfo;    // Implementation Info    
    TUid iImplUid;
    };

#endif      // OBEXSMUSBCONNECTION_H

// End of File
