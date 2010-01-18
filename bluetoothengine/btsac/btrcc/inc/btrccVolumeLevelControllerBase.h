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
* Description:   The base class for volume control of AVRCP Controller.	
*
*
*/

#ifndef BTRCCVOLUMELEVELCONTROLLERBASE_H
#define BTRCCVOLUMELEVELCONTROLLERBASE_H

//  INCLUDES
#include <e32base.h>
#include <remconcoreapicontrollerobserver.h> 
#include <bttypes.h> // BT protocol wide types
#include <e32property.h> 
#include "btrccActive.h"

// FORWARD DECLARATIONS
class CRemConInterfaceSelector; 
class CRemConCoreApiController; 

// CLASS DECLARATION

NONSHARABLE_CLASS(MBTRCCVolumeControllerObserver) 
    {
    public: 
        virtual void VolumeControlError(TInt aError) = 0; 
    }; 

/*!
 error code return by controller
 */
enum TBTRccControllerError
    {
    EVolumeAdjustmentFailed = 0,       /*!< Adjust remote volume failed */
    ERegisterNotificationsFailed   /*!< Register volume change notifications failed */
    };
/**
*  A listener for incoming handsfree attachments/detachments.
*/
NONSHARABLE_CLASS(CBTRCCVolumeLevelControllerBase) : public CBase, public MBTRCCActiveObserver
    {
    public:  // Constructors and destructor

        /**
        * Destructor.
        */
        virtual ~CBTRCCVolumeLevelControllerBase();

    protected:
            /**
            * C++ default constructor.
            */
            CBTRCCVolumeLevelControllerBase(MBTRCCVolumeControllerObserver &aObserver);

            /**
            * Symbian 2nd phase constructor.
            */
            void ConstructL();

    public:
        enum TVolControlState
            {
            ESubscribePhoneVolume,
            ESetPhoneVolume            
            };
            
    public: // From CBTRCCActiveObserver 
            virtual void RequestCompletedL(CBTRCCActive& aActive, TInt aErr);
            
            virtual void CancelRequest(TInt aServiceId);

    public: // New methods

        void Start(); // Starts monitoring for volume level changes. 

        void Stop(); // Stop monitoring for volume level changes. 

        void Reset(); // Reset the volume level.  

        TInt GetLocalMaxVolume();  
        
        TInt GetCurrentLocalVolume();

        void AccessoryChangedVolume(TInt aVolumeInPhoneScale);  

        virtual void DoStart(TInt aInitialVolume) = 0; // Starts monitoring for volume level changes. 

        virtual void DoStop() = 0; // Stop monitoring for volume level changes. 

        virtual void DoReset() = 0; // Reset the volume level.          

        virtual void VolumeControlError(TInt aError); 
        
        virtual void RegisterVolumeChangeNotification(); 
        
    private:
        void SetPhoneVolume();
        
    private: // New methods

        /**
        * To be implemented by the specialization class. 
        */
        virtual void AdjustRemoteVolume(TInt aVolume) = 0; 
    
    protected:    
        virtual TInt GetPhoneVolume(TInt &aVol);

    private:    // Data
        RProperty iVolLevelProperty; // owned
        RProperty iVolKeyEventProperty; // owned
        
        // This active object listens to the PS key changes.
        // Specialization classes may add more active objects if they need. 
        // Hence the ID in the CBTRCCActive. 
        CBTRCCActive *iBtrccActive;

        MBTRCCVolumeControllerObserver &iObserver; 
        
        TInt iLocalMaxVolume;
        TInt iPhoneVolume;
        TInt iRemoteVolume;
        
        TVolControlState iState;        
    };

#endif      // BTRCCVOLUMELEVELCONTROLLERBASE_H
            
// End of File
