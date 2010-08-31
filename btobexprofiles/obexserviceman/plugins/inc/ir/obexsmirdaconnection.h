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
* Description:  This class handles irda obex settings.
*
*/



#ifndef _SRCSIRDACONNECTION_H
#define _SRCSIRDACONNECTION_H

//  INCLUDES
#include <e32base.h>

#include <obex.h>
#include "SrcsTransport.h"
#include "SrcsInterface.h"

// CLASS DECLARATION

/**
 * Class to handle IRDA connection
 */
class CObexSMIrDAConnection: public CSrcsTransport
    {
public:

   /**
    * Two-phased constructor.
	* @param	aInitParams initial parameters
    */
    static CObexSMIrDAConnection* NewL(TAny* aInitParams);

   /**
    * Destructor.
    */
    virtual ~CObexSMIrDAConnection();

   /** 
	* Check if OBEX service is still active.
    * @return ETrue: OBEX service is still active.
	*		  EFalse: OBEX service is inactive.
	*/
	TBool IsOBEXActive();

private:

   /**
    * C++ default constructor.
    */
    CObexSMIrDAConnection();

   /**
    * Constructor.
    *
    * @param	aInitParams initial parameters
	*/
	CObexSMIrDAConnection(TAny* aInitParams);

   /**
    * Constructor
    * Open IRDA connection and creates OBEX server.
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
    };



#endif      // _SRCSIRDACONNECTION_H

// End of File
