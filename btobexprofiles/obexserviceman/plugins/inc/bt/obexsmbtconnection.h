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
* Description:  This class handles bt obex settings.
*
*/


#ifndef _OBEXSMBTCONNECTION_H
#define _OBEXSMBTCONNECTION_H

//  INCLUDES
#include <e32base.h>

#include <obex.h>
#include "SrcsTransport.h"
#include "obexsmpasskey.h"
#include "SrcsInterface.h"
#include "btengdiscovery.h"
#include "obexutilsopaquedata.h"

// CLASS DECLARATION
/**
* Class to handle Bluetooth connection
*/
class CObexSMBtConnection: public CSrcsTransport
    {
public:

   /**
    * Two-phased constructor.
    *
	* @param	aInitParams initial parameters
    */
    static CObexSMBtConnection* NewL(TAny* aInitParams);

   /**
    * Destructor.
    *
    */
    virtual ~CObexSMBtConnection();

   /** 
	* Check if OBEX service is still active.
	*
    * @return ETrue: OBEX service is still active.
	*		  EFalse: OBEX service is inactive.
	*/
	TBool IsOBEXActive();

private:

   /**
    * C++ default constructor.
    */
    CObexSMBtConnection();

   /**
    * Constructor.
    *
    * @param	aInitParams initial parameters
	*/
    CObexSMBtConnection(TAny* aInitParams);

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
    CObexServer*                iServer;             // Obex Server    
    TSdpServRecordHandle        iSDPHandle;          // SDP handle for registrations
    CObexSMPasskey*             iPasskey;            // Class to handle Obex passkey

    CSrcsInterface*             iController;         // Service Controller callback
    CImplementationInformation* iImplementationInfo; // Implementation Info    

    TUint                       iServiceUid;         // short form service id, so we can unregister later
    TUUID                       iServiceUUID;	     // 128-bit UUID for Nokia proprietory service
    CBTEngDiscovery*            iBtDiscovery;        // Bteng discovery
    };



#endif      // SRCSBTCONNECTION_H

// End of File
