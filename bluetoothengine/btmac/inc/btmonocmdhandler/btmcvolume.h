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
* Description:  volume handling
*
*/


#ifndef BTMCVOLUME_H
#define BTMCVOLUME_H

// INCLUDES
#include <e32base.h>
#include <e32property.h> 
#include "btmcactive.h"

class CBtmcPhoneStatus;

/**
*  CBtmcVolume listens network status events (from RMobilePhone).
*/
NONSHARABLE_CLASS(CBtmcVolume) : public CBase, public MBtmcActiveObserver
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CBtmcVolume* NewL(CBtmcPhoneStatus& aParent);
        
        /**
        * Destructor.
        */
        ~CBtmcVolume();

        void SetSpeakerVolumeL(TInt aHfVol);
        
        void SetMicrophoneVolumeL(TInt aHfVol);
        
        TInt GetVolume();
        
        void ActivateRemoteVolumeControl();

        void DeActivateRemoteVolumeControl();        
        
    private:  // From MBtmcActiveObserver
    
        void RequestCompletedL(CBtmcActive& aActive, TInt aErr);
        
        void CancelRequest(TInt aServiceId);
        
    private:
    
        /**
        * C++ default constructor.
        */
        CBtmcVolume(CBtmcPhoneStatus& aParent);
        
        /**
        * C++ default constructor.
        */
        void ConstructL();
        
        void DoSetSpeakerVolL( TInt aPrevPhVol );
        
        /**
        * Converts volume level (0..15 to 0..10).
        * @param aVolume Volume level
        * @return TInt
        */
        TInt HfToPhoneVolScale(TInt aHfVol);

        /**
        * Converts volume level (0..10 to 0..15).
        * @param aVolume Volume level
        * @return TInt
        */
        TInt PhoneToHfVolScale(TInt aPhoneVol);

        TInt GetNewPhoneVol();

    private:
        enum TVolSyncAction
            {
            ESpeakerVolSubscribe,
            ESpeakerVolSet,
            };

    private:
        CBtmcPhoneStatus& iParent;
    
        CBtmcActive* iActive; // owned
        
        RProperty iVolLevelProperty; // owned
        RProperty iVolKeyEventProperty; // owned
        
        // current volume level of speaker
        TInt iPhnSpkrVol; // phone speaker volume
        TInt iAccSpkrVol; // Acc speaker volume in phone side scale
		TInt iMaxSpkrVol; // Maximum volume level of the phone
		TInt iStep;
        TVolSyncAction iAction;
    	
    	TBool iVolCtrlActivated;
    };


#endif  // BTMCVOLUME_H

// End of File

