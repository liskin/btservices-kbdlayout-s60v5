/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares User confirmation notifier for Secure Simle Pairing 
*
*/


#ifndef BTUSERCONFNOTIFIER_H
#define BTUSERCONFNOTIFIER_H

// INCLUDES

#include "btnpairnotifier.h" // Base class

// These declarations are here temporarily so the submission of the notifiers and the stack do not have to be synchronised.
// Should be removed once all submissions are in.

const TUid KBTUserConfirmationNotifierUidCopy={0x2002E224};

NONSHARABLE_CLASS(TBTNotifierParams2Copy)	
	{
public:
	inline TBTDevAddr DeviceAddress() const { return iBDAddr; };
	inline TPtrC DeviceName() const { return iName; };
private:
	TBTDevAddr		iBDAddr;
	TBTDeviceName	iName;	
	
	// This data padding has been added to help prevent future binary compatibility breaks	
	// Neither iPadding1 nor iPadding2 have been zero'd because they are currently not used
	TUint32	iPadding1; 
	TUint32	iPadding2; 
	};

NONSHARABLE_CLASS(TBTUserConfirmationParamsCopy)
	: public TBTNotifierParams2Copy
	{	
public:
	inline TBool LocallyInitiated() const { return iLocallyInitiated; };
private:
	TBool				iLocallyInitiated;
	
	// This data padding has been added to help prevent future binary compatibility breaks	
	// Neither iPadding1 nor iPadding2 have been zero'd because they are currently not used
	TUint32	iPadding1;
	TUint32 iPadding2;
	};

typedef TPckgBuf<TBTUserConfirmationParamsCopy> TBTUserConfirmationParamsPckgCopy;

// CLASS DECLARATION
/**
* This class is used to ask user to compare passcode in two devices.
*/


NONSHARABLE_CLASS(CBTUserConfNotifier): public CBTNPairNotifierBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBTUserConfNotifier* NewL();  // Constructor (public)

        /**
        * Destructor.
        */
        virtual ~CBTUserConfNotifier();      // Destructor

    private:
        /**
        * From CBTNotifierBase Called when a notifier is first loaded 
        * to allow any initial construction that is required.
        * @param None.
        * @return A structure containing priority and channel info.
        */
        TNotifierInfo RegisterL();

        
        /**
        * From CBTNotifierBase
        * Used in asynchronous notifier launch to start the actual processing
        * of parameters received in StartL. StartL just stores a copy of the
        * parameter buffer, schedules a callback to call this method and returns
        * ASAP, so the actual work begins here, safely outside of the StartL
        * context so that waiting dialogs can be freely used if need be.
        * @return None.
        */
        void ProcessStartParamsL();
                                         
        /**
        * From CBTNotifierBase
        * Show notes to ask user response and return to Notifier caller by completing message.
        */      
        void HandleGetDeviceCompletedL(const CBTDevice* aDev);        
        
    private:

        /**
        * C++ default constructor.
        */
        CBTUserConfNotifier();               // Default constructor
    };

#endif

// End of File
