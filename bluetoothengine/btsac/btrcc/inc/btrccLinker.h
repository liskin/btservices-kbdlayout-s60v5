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
* Description:  State Machine of BTRCC
*
*/


#ifndef BTRCCLINKER_H
#define BTRCCLINKER_H


//  INCLUDES
#include <e32base.h>
#include <bttypes.h> // BT protocol wide types
#include <e32property.h> // pub-sub definitions
#include <remconbatterytarget.h>
#include <remconbatterytargetobserver.h>
#include <absolutevolumeapitargetobserver.h>
#include <remconinterfaceselector.h>

#include "btrccVolumeLevelControllerBase.h"

// DATA TYPES
enum TBTRCCStateIndex
    {
    EStateIndexIdle = 0,
    EStateIndexConnecting,
    EStateIndexConnected,
    EStateIndexDisconnect
    };
    
// FORWARD DECLARATIONS
class MBTAccObserver;
class CRemConInterfaceSelector;
class CRemConCoreApiController;
class CBTRCCVolumeLevelController;
class CRemConCoreApiTarget;
class CPlayerStarter;
class CBTRCCBrowsingAdapter;

// CLASS DECLARATION

/**
*  The main controller for BT Remote Control Profile. Notifies BT Accessory Server
* about changes in remote contorl profile connections. Perform requests from BT Accessory
* Server to disconnect the accessory. 
*/
NONSHARABLE_CLASS(CBTRCCLinker) : public CActive, public MRemConBatteryTargetObserver, public MBTRCCVolumeControllerObserver
	{
    private:
        /**
        * The base class of state
        */
        class TState
            {
            public:
                
                /**
                * Entry of this state
                * @param aStatus the client status, will be completed when this state should exit.
                */
                virtual void Enter() = 0;
                
                /**
                * Called by parent's DoCancel
                * Cancels the outstanding operations in this state.
                */
                virtual void DoCancel();

                virtual void Connect(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

                virtual void CancelConnect(const TBTDevAddr& aAddr);                
                
                /**
                * Makes a request to disconnect the connection.
                * @param aBdAddress	BT device address of remote side
                */
                virtual void Disconnect(TRequestStatus& aStatus, const TBTDevAddr& aAddr);
                
                /**
                * This will be called whenever an async operation to RemCon FW completes.
                */
                virtual void RemConRequestCompleted(TInt aErr);
                
                virtual void UpdateRemoteVolumeControling(TBool aActivated);
                
            protected:
            
                /**
                * Default constructor
                */
                TState(CBTRCCLinker& aParent);
                                
            protected:
                CBTRCCLinker& iParent;
            };
        friend class TState;

        class TStateIdle : public TState
            {
            public:

                TStateIdle(CBTRCCLinker& aParent);
 
                void Enter();

                void DoCancel();

                void Connect(const TBTDevAddr& aAddr, TRequestStatus& aStatus);
                
                void RemConRequestCompleted(TInt aErr);
            };
        friend class TStateIdle;

        class TStateConnecting : public TState
            {
            public:

                TStateConnecting(CBTRCCLinker& aParent);
 
                void Enter();

                void DoCancel();

                void CancelConnect(const TBTDevAddr& aAddr);
                
                void RemConRequestCompleted(TInt aErr);
                
            private:
                TBool iConnectCanceled;
            };
        friend class TStateConnecting;
        
        class TStateConnected : public TState
            {
            public:
                TStateConnected(CBTRCCLinker& aParent);
 
                void Enter();
             
                void DoCancel();
                
                void Connect(const TBTDevAddr& aAddr, TRequestStatus& aStatus);
                
                void Disconnect(TRequestStatus& aStatus, const TBTDevAddr& aAddr);
                
                void RemConRequestCompleted(TInt aErr);
                
                void UpdateRemoteVolumeControling(TBool aActivated);
            };
        friend class TStateConnected;  

        class TStateDisconnect : public TState
            {
            public:
                TStateDisconnect(CBTRCCLinker& aParent);
 
                void Enter();

                void DoCancel();
                
                void RemConRequestCompleted(TInt aErr);
            };
        friend class TStateDisconnect;
                      
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * After construction the instance is ready to serve requests.
        * Also starts listening to incoming connections from BT audio 
        * accessories.
        */
        static CBTRCCLinker* NewL(MBTAccObserver& aAccObserver);
        
        /**
        * Destructor.
        * Removes service and security registrations and stops listening to
        * incoming connections.
        */
        virtual ~CBTRCCLinker();

		void MrcbstoBatteryStatus(TControllerBatteryStatus& aBatteryStatus);

    public: // New functions
        
        void Connect(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

        void CancelConnect(const TBTDevAddr& aAddr);        

        /**
        * DiSconnects from the currently connected BT audio accessory.
        * @return                   None.
        */
        void Disconnect(TRequestStatus& aStatus, const TBTDevAddr& aAddr);
        
        MBTAccObserver& AccObserver();
        
        void ActivateRemoteVolumeControl();

        void DeActivateRemoteVolumeControl();

    public:  // From MBTRCCVolumeControllerObserver 
        void VolumeControlError(TInt aError);                 

    private:  // From CActive

        /**
        * Gets called when the asynchronous control connection open operation 
        * completes. Informs the control connection observer about the 
        * connection.
        * @param                            None.
        * @return                           None.
        */
        void RunL();

		/**
        * Gets called if RunL leaves 
        * @param                            None.
        * @return                           None.
        */
	    TInt RunError(TInt aError);

        /**
        * Cancels the ongoing control connection open operation and informs
        * the control connection observer about that.
        * @param                            None.
        * @return                           None.
        */
        void DoCancel();


    private:

        /**
        * C++ default constructor.
        */
        CBTRCCLinker(MBTAccObserver& aAccObserver);

        /**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();
        
        /**
        * Connect the accessory.
        */
        void DoConnect();

        void DoSubscribeConnetionStatus();

        void DoCancelSubscribe();
        
        /**
        * DiSconnects from the currently connected BT audio accessory.
        * @return                   None.
        */
        void DoDisconnect();
        
        void ChangeState(TBTRCCStateIndex aNextState);    

        /**
        * Re-orient AVRCP bearer plugin to the remote for the current connect /disconnect request
        */
        TInt DoRemConOrientation();
        
        /**
        * aConnectedAddr will hold the remote BD address if there is a connection.
        * otherwise it is TBTDevAddr()
        */
        void DoGetRemConConnectionStatus(RArray<TBTDevAddr>& aConnects);

        void StartRemoteVolumeControl();
        
        void StopRemoteVolumeControl();

        void ResetVolmeControlSetting();

   private:    // Data
    
		CRemConInterfaceSelector *iInterfaceSelector; 
		
		// For receiving the accessory battery status
        CRemConBatteryApiTarget *iRemConBatteryTgt;
        
	    CRemConCoreApiTarget* iCoreTarget; // not own
	    
	    CPlayerStarter* iPlayerStarter; // owned
	    
        // For handling the browsing commands. 
	    CBTRCCBrowsingAdapter *iBrowsingAdapter;

	    // This is only needed for disconnecting a AVRCP connection
        // when AVRCP Remote Volume Control is not supported.
        CRemConInterfaceSelector *iInterfaceSelectorForDisconnectingTargetSession; 
        
        TBTDevAddr iRemoteAddr;
        
		MBTAccObserver& iAccObserver;

        RPointerArray<TState> iStateArray;  // Array of all states in BTRCC 
        TBTRCCStateIndex iCurrentStateIndex;  // Index of current state

        TRequestStatus* iClientRequest;
        TBool iRemConOriented;
        
        TBool iRvcActivated;
        
        CBTRCCVolumeLevelControllerBase* iVolController;
        CBTRCCVolumeLevelControllerBase* iAbsoluteVolController;
        
        TInt iRegisterVolumeChangeNotificationCounter;//Counter for re-register for remote volume control notification
};


#endif      // BTRCCLINKER_H
            
// End of File
