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
* Description:  Define obexserviceman's service controller ECom interface.
*
*/



#ifndef _SRCSINTERFACE_
#define _SRCSINTERFACE_


//INCLUDES
#include <e32base.h>
#include <ecom/ecom.h>
#include <badesca.h>

// CONSTANTS
const TUid KCSrcsInterfaceUid = {0x101F7C8C}; // UID of this interface


enum TSrcsMediaType //transport media types
    {
        ESrcsMediaBT = 1,
        ESrcsMediaIrDA,
        ESrcsMediaUSB
    };

/**
*   An abstract class being representative of the
*   concrete class which the client wishes to use.
*
*   It acts as a base, for a real class to provide all the
*   functionality that a client requires.
*   It supplies instantiation & destruction by using
*   the ECom framework, and functional services
*   by using the methods of the actual class.
*/
class CSrcsInterface : public CBase
    {
public:

    /**
    * Two-phased constructor.
    * @param aImplUid Implementation UId.
    * @return newly created CSrcsInterface.
    */
    static CSrcsInterface* NewL(TUid aImplUid);

    /**
    * Destructor
    */
    virtual ~CSrcsInterface();

    /** 
	* Request a list of all available implementations which satisfies this given interface.
    * @param aImplInfoArray Implementation instance array.
    * @return none.
	*/
    static void ListAllImplementationsL(RImplInfoPtrArray& aImplInfoArray);

    /** 
	* Request a list of matching implementations which satisfies this given interface.
    * @param aMatchString	String to be matched by the implemention.
    * @param aImplInfoArray Implementation instance array.
    * @return none.
	*/
    static void ListImplementationsL(const TDesC8& aMatchString, RImplInfoPtrArray& aImplInfoArray);
    

	/** 
	* Check if OBEX service is still active.
    * @return ETrue: OBEX service is still active.
	*		  EFalse: OBEX service is inactive.
	*/
	virtual TBool IsOBEXActive() = 0;

	/** 
	* Provide transport media information to service controller.
	* @param aMediaType	enum value specifying transport media type.
	* @return none.
	*/
	virtual void SetMediaType( TSrcsMediaType aMediaType ) ;

	/** 
	* SetObexServer
	* @param aObexServer pass obex server pointer to sevice controller
	* @return none.
	*/
	virtual TInt SetObexServer( CObexServer* aObexServer );

protected:
    /**
    * C++ default Constructor
    */
    inline CSrcsInterface();

private:
    // Unique instance identifier key
    TUid iDtor_ID_Key;
    };

#include "SrcsInterface.inl"

#endif

