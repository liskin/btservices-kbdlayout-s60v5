/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares disk status watcher class for ObexUtils.
*
*/



#ifndef _OBEXUTILSPROPERTYNOTIFIER_H
#define _OBEXUTILSPROPERTYNOTIFIER_H

// INCLUDES
#include <e32base.h>
#include <e32property.h>
#include <UikonInternalPSKeys.h>

// CLASS DECLARATION

	/**
	Type of memory property check, 
	@param  ECheckPhoneMemory, phone memory is checked, default value if not mentioned explicitly.	
    @param  ECheckMMCMemory, MMC memory is checked.	
	*/

enum TMemoryPropertyCheckType
	{
	ECheckPhoneMemory,
	ECheckMMCMemory
	};

	/**
	A callback interface for informing content change of disk status. Client must derive 
    from this class and implement HandleNotifyL() method. When disk space will cross warning
    level or critical level this method will be call back.
    */

NONSHARABLE_CLASS(  MObexUtilsPropertyNotifyHandler )
	{
	public:
        /**
        * Callback method for disk status change
        * @param aUid  UID identifying a shared data file where notify came from.
        * @param aKey  keyword of which value was changed
        */
        virtual void HandleNotifyL( TMemoryPropertyCheckType aCheckType ) =0;
	};

NONSHARABLE_CLASS( CObexUtilsPropertyNotifier ) : public CActive
	{
public:  // NewL, Constructors and destructor

    /*
    * Two-phased constructor.
    * @param aHandler -Pointer to the MObexUtilsPropertyNotifyHandler derived class
    * @param aCheckType -Constant defining type of memory checking to be done (phone/MMC)
    * @return CObexUtilsPropertyNotifier* -Initialized object.
    */

	IMPORT_C static CObexUtilsPropertyNotifier* NewL(
	    MObexUtilsPropertyNotifyHandler* aHandler,
	    TMemoryPropertyCheckType aCheckType=ECheckPhoneMemory );

    /**
    * Destructor.
    */

	virtual ~CObexUtilsPropertyNotifier();
private:  // Functions from base classes

    /**
    * Constructor.
    * @param aHandler -Pointer to the MObexUtilsPropertyNotifyHandler derived class
    * @param aCheckType -Constant defining type of memory checking to be done (phone/MMC)
    */

	CObexUtilsPropertyNotifier(
	    MObexUtilsPropertyNotifyHandler* aHandler,
	    TMemoryPropertyCheckType aCheckType );

	void ConstructL();

	/**
	Subscribes to a property and sets active
	*/
	void Subscribe();

    /**
    * From CActive Gets called when CActive::Cancel is called, 
    *              cancels disk status watching.
    *
    * @param None.
    * @return None.
    */

	void DoCancel();

    /**
    * From CActive Gets called when content of disk status is changed, 
    *              calls MObexUtilsPropertyNotifyHandler::HandleNotify.
    *
    * @param None.
    * @return None.
    */

	void RunL();

private:
	// Reference to observer
	MObexUtilsPropertyNotifyHandler* iHandler;
	// Type of memory check (phone/MMC)
	TMemoryPropertyCheckType iCheckType;
	// Database handle	
	RProperty iProperty;
	};

#endif  // _OBEXUTILSPROPERTYNOTIFIER_H
