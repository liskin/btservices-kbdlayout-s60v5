/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  
*
*/


#ifndef BTMONOCDMACALLMONITOR_H
#define BTMONOCDMACALLMONITOR_H

//  INCLUDES
#include <e32base.h>
#include <etelmm.h>
#include <etelcdma.h>
#include "BTMonoActive.h"

NONSHARABLE_CLASS(CBTMonoCdmaIncomingFlash) : public CBTMonoActive
	{
	public:	//Constructors and descructor
		
		/**
        * Two-phased constructor.
        */
		static CBTMonoCdmaIncomingFlash* NewL(
            MBTMonoActiveObserver& aObserver, 
            CActive::TPriority aPriority, 
            TInt aServiceId,		
		    RMobileLine& aLine,
			const TName& aName );

		/**
        * Two-phased constructor.
        */
		static CBTMonoCdmaIncomingFlash* NewLC(
            MBTMonoActiveObserver& aObserver, 
            CActive::TPriority aPriority, 
            TInt aServiceId,		
		    RMobileLine& aLine,
			const TName& aName );

		/**
        * Destructor.
        */
		virtual ~CBTMonoCdmaIncomingFlash();

        void GoActive();

	protected: // From CActive
		/**
        * Cancels asyncronous request(s).
        * 
		* @return None.
        */
        void DoCancel();

		/**
        * Informs object that asyncronous request is ready.
        * 
		* @return None.
        */
        void RunL();
        
        TInt RunError(TInt aErr);
        

    private:

        /**
		* C++ default constructor can NOT contain any code, that might leave
        */
		CBTMonoCdmaIncomingFlash(
            MBTMonoActiveObserver& aObserver, 
            CActive::TPriority aPriority, 
            TInt aServiceId,		
		    RMobileLine& aLine,
			const TName& aName );

        /**
        * By default Symbian OS constructor is private.
        */
        void ConstructL();
	
    private:

		// needed for opening RMobileCall
		RMobileLine& iCdmaLine; // unowned
    
		// handle to CDMA Phone
		RCdmaMobileCall iCdmaCall; // owned
		
		// stores the call name 
		TName iCallName; 
		
		// call waiting information package
		RCdmaMobileCall::TMobileCallIncomingFlashMessageV1Pckg iFlashMessagePckg;		
	}; 

#endif  // BTMONOCDMACALLMONITOR_H

// End of File
