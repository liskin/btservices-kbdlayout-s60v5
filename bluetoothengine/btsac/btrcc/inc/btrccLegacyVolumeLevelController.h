/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   This class implements legacy volume control of AVRCP Controller.	
*
*
*/

#ifndef BTRCCLEGACYVOLUMELEVELCONTROLLER_H
#define BTRCCLEGACYVOLUMELEVELCONTROLLER_H

//  INCLUDES
#include <e32base.h>
#include <remconcoreapicontrollerobserver.h> 
#include <bttypes.h> // BT protocol wide types 
#include "btrccVolumeLevelControllerBase.h"

// FORWARD DECLARATIONS
class CRemConInterfaceSelector; 
class CRemConCoreApiController; 
class CBTRCCActive;

// CLASS DECLARATION


NONSHARABLE_CLASS(CBTRCCLegacyVolumeLevelController) :  
    public CBTRCCVolumeLevelControllerBase, 
    public MRemConCoreApiControllerObserver 
	{
	public:  // Constructors and destructor

		/**
		* Two-phased constructor.
		*      
		*/
		static CBTRCCLegacyVolumeLevelController* NewL(CRemConInterfaceSelector& aInterfaceSelector, MBTRCCVolumeControllerObserver& aObserver);

		/**
		* Destructor.
		*/
		virtual ~CBTRCCLegacyVolumeLevelController();
		
	private:
		enum TState
			{
			EStateWaitingForVolumeLevelChanges,
			EStateDelay,
			EStateVolumeUpTiming,
			EStateVolumeUp,	
			EStateVolumeDownTiming,
			EStateVolumeDown,	
			}; 

        /**
        * C++ default constructor.
        */
        CBTRCCLegacyVolumeLevelController(MBTRCCVolumeControllerObserver& aObserver);

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL(CRemConInterfaceSelector& aInterfaceSelector);

        /**
         * Decides the next operation for volume controlling
         */
        void DoRemoteControl();
        
        /**
         * Starts a timer. The timeout value is specified by aInterval
         */
        void StartTimer(TInt aInterval);
        
        /**
         * Send a volume up or down command decided by aDirection.
         */
        void SendVolumeCommand(TInt aDirection);
            
	public: // From CBTRCCVolumeLevelControllerBase
	    void DoStart(TInt aInitialVolume); // Starts monitoring for volume level changes. 
	    void DoStop(); // Stop monitoring for volume level changes. 
	    void DoReset(); // Reset the volume level.  
    
	public: // From MBTRCCActiveObserver
        void RequestCompletedL(CBTRCCActive& aActive, TInt aErr);            
        void CancelRequest(TInt aServiceId);
    
	private: // From CBTRCCVolumeLevelControllerBase
        void AdjustRemoteVolume(TInt aVolume); 
        TInt GetPhoneVolume(TInt &aVol);

    private:  // From MRemConCoreApiControllerObserver
		TInt MrccacoResponse(); 
    
	private: // Data
		CRemConCoreApiController* iRemConVolController; // changes the accessory's volume level. 
       
        TUint iNumRemotes; 
        TState iState;
        
        TInt iPhoneVolume; // target volume level
        TInt iCurrentPhoneVolume; // current volume level of phone
		TInt iCurrentRemoteVolume;  // current volume level of remote
	    TInt iLocalMaxVolume; // maximum volume level of phone
		
		// Active object for various timers and volume control operation.
		CBTRCCActive *iBtrccLegacyVolumeActive;
		
		// Not owned
		CRemConInterfaceSelector *iInterfaceSelector;

		RTimer iCtrlRTimer;
    };

#endif      // BTRCCLEGACYVOLUMELEVELCONTROLLER_H
            
// End of File
