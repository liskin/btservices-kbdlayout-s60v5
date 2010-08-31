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
* Description:  Define SRCS's transport ECom interface.
*
*/



#ifndef _SRCSTRANSPORT_
#define _SRCSTRANSPORT_

//  INCLUDES
#include <e32base.h>
#include <ecom.h>
#include <badesca.h>
#include <obex.h>
#include "SrcsClSv.h"

// CONSTANTS
// UID of this interface
const TUid KCSrcsTransportUid = {0x101F7C9A};

// default OBEX receiving buffer size, 
// used when service controller does not specify the receiving buffer size in its .rss
const TUint KSRCSDefaultReceiveMtu = 0x4000;

// default OBEX sending buffer size, 
// used when service controller does not specify the sending buffer size in its .rss
const TUint KSRCSDefaultTransmitMtu = 0x3000;

 
 

/**
 *	An abstract class being representative of the
 *	concrete class which the client wishes to use.
 *	It acts as a base, for a real class to provide all the
 *	functionality that a client requires.
 *	It supplies instantiation & destruction by using
 *	the ECom framework, and functional services
 *	by using the methods of the actual class.
 */
class CSrcsTransport : public CBase, public MObexAuthChallengeHandler
    {
public:

   /**
    * Two-phased constructor.
    */
    static CSrcsTransport* NewL(TUid aImplUid, CImplementationInformation* aParams );

   /**
    * Destructor.
    */
    virtual ~CSrcsTransport();

   /** 
	* Request a list of all available implementations which satisfies this given interface.
	*
    * @param aImplInfoArray Implementation instance array.
    * @return none.
	*/
    static void ListAllImplementationsL(RImplInfoPtrArray& aImplInfoArray);

   /** 
	* Request a list of matching implementations which satisfies this given interface.
	*
    * @param aMatchString	String to be matched by the implemention.
    * @param aImplInfoArray Implementation instance array.
    * @return none.
	*/
    static void ListImplementationsL(const TDesC8& aMatchString, RImplInfoPtrArray& aImplInfoArray);

   /** 
	* Check if OBEX service is still active.
	*
    * @return true: OBEX service is still active.
	*		  false: OBEX service is inactive.
	*/
	virtual TBool IsOBEXActive() = 0;
    
	/** 
    * Perform any outstanding initialization tasks.
    * 
    * The transport may decide to implement two-phase initialization to improve performance where needed.
    * For instance, USB Transport has to register all service controller's interfaces and complete the message from obex class controller
    * as soon as possible to satisfy timing requirements and defer service controller plugin creation to later stage.
    *
    * @return none.
    */
    virtual void PostInitializeL();
	

protected:
   /**
    * C++ default Constructor
    */
    inline CSrcsTransport();

   /**
    * MObexAuthChallengeHandler implementation
    */

   /**
    * Password for authenticated connection
    *
    * @param aUserID	user id.
    */
    virtual void GetUserPasswordL( const TDesC& aUserID ) = 0;


private:
    // Unique instance identifier key
    TUid iDtor_ID_Key;
    };

#include "SrcsTransport.inl"

#endif

