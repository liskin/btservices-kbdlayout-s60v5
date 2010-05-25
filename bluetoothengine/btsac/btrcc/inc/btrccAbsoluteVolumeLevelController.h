/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   This class implements volume control of AVRCP Controller.	
*
*
*/


#ifndef BTRCCABSOLUTEVOLUMELEVELCONTROLLER_H
#define BTRCCABSOLUTEVOLUMELEVELCONTROLLER_H

//  INCLUDES
#include <e32base.h>
#include <bttypes.h> // BT protocol wide types
#include "btrccVolumeLevelControllerBase.h"
#include "absolutevolumeapicontrollerobserver.h"  
#include "absolutevolumeapicontroller.h"
#include "absolutevolumeapitargetobserver.h"
//#include <remconinterfaceselector.h> 

// FORWARD DECLARATIONS
class CRemConInterfaceSelector; 
class CRemConCoreApiController; 
class CBTRCCActive; 

// CLASS DECLARATION

/**
*  A listener for incoming handsfree attachments/detachments.
*/
NONSHARABLE_CLASS(CBTRCCAbsoluteVolumeLevelController) : 
    public CBTRCCVolumeLevelControllerBase, 
    public MRemConAbsoluteVolumeControllerObserver 
	{
	public:  // Constructors and destructor

		/**
		* Two-phased constructor.
		*      
		*/
		static CBTRCCAbsoluteVolumeLevelController* NewL(CRemConInterfaceSelector& aInterfaceSelector, MBTRCCVolumeControllerObserver& aObserver); 

		/**
		* Destructor.
		*/
		virtual ~CBTRCCAbsoluteVolumeLevelController();


	private:

    		/**
    		* C++ default constructor.
    		*/
            CBTRCCAbsoluteVolumeLevelController(MBTRCCVolumeControllerObserver& aObserver);

    		/**
    		* Symbian 2nd phase constructor.
    		*/
    		void ConstructL(CRemConInterfaceSelector& aInterfaceSelector);

    public: // From MBTRCCActiveObserver

        void RequestCompletedL(CBTRCCActive& aActive, TInt aErr);

        void CancelRequest(TInt aServiceId);

    public: // From CBTRCCVolumeLevelControllerBase

        void DoStart(TInt aInitialVolume); // Set the initial volume. 

        void DoStop(); // Stop monitoring for volume level changes. 

        void DoReset(); // Reset the volume level.          

    private: // From CBTRCCVolumeLevelControllerBase

        void AdjustRemoteVolume(TInt aVolume); 
        
        TInt RoundRemoteVolume(TInt aPrevPhVol);        

	public: // New methods


    private:  // From MRemConAbsoluteVolumeControllerObserver

        void MrcavcoCurrentVolume(TUint32 aVolume, TUint32 aMaxVolume, TInt aError); 

        void MrcavcoSetAbsoluteVolumeResponse(TUint32 aVolume, TUint32 aMaxVolume, TInt aError); 

        void MrcavcoAbsoluteVolumeNotificationError(); 
    
    private:
        void AccessoryChangedVolume(TUint32 aVolume, TUint32 aMaxVolume, TInt aError);
        TBool ValidVolumeParams(TUint32 aVolume, TUint32 aMaxVolume);
        
        void RegisterVolumeChangeNotification();
    
	private:    // Data

	    CRemConAbsoluteVolumeController* iRemConAbsoluteVolumeController; // changes the accessory's volume level. 

        // This active object listens to the absolute volume API. 
        CBTRCCActive *iBtrccAbsoluteVolumeActive; 
        
        // Not owned
        CRemConInterfaceSelector *iInterfaceSelector;

		TInt iLocalMaxVolume; // maximum volume level of phone

		TUint iNumRemotes; // not used, but needed for the remote controlling API.
		
		TInt iRetryCounter;
		
        TInt iStep;
    };

#endif      // BTRCCABSOLUTEVOLUMELEVELCONTROLLER_H
            
// End of File
