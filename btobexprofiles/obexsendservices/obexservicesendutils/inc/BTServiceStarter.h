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
* Description:  Service starter
*
*/



#ifndef BT_SERVICE_STARTER_H
#define BT_SERVICE_STARTER_H

//  INCLUDES
#include "BTServiceAPI.h"
#include "BTServiceUtils.h"
#include "BTServiceParameterList.h"
#include "BTServiceDelayedDestroyer.h"

#include <btengdiscovery.h>
#include <btengsettings.h>
#include <obexutilsdialog.h>
#include <msvapi.h>

// DATA TYPES

enum TBTServiceProfile
    {
    EBTSNone,
    EBTSBPP,
    EBTSOPP,
    EBTSBIP
    };

enum TBTActiveNotifier
    {
    ENoneQuery = 0,
    EOfflineQuery,
    ENameQuery
    };

// FORWARD DECLARATIONS
class CObexUtilsUiLayer;
class MBTServiceProgressGetter;
class CBTSController;


const TUint KBTSdpObjectPush            = 0x1105;
const TUint KBTSdpBasicImaging          = 0x111b;
const TUint KBTSdpBasicPrinting         = 0x1120;


// CLASS DECLARATION

/**
*  An observer interface for service controllers.
*/
class MBTServiceObserver
    {
    public: // New functions

        /**
        * Informs the observer that the handler has completed its tasks.
        * @param aStatus The the completion status of the handler.
        * @return None.
        */
        virtual void ControllerComplete( TInt aStatus ) = 0;

        /**
        * Informs the observer that a progress note should be shown.
        * @param aGetter A pointer to a progess status getter object.
        * @return None.
        */
        //todo reusing the second param aTotalsize to aFileCount as it wass no longer used check it being used in other places
        virtual void LaunchProgressNoteL( MBTServiceProgressGetter* aGetter, TInt aFileCount) = 0;
        
        
        /**
        * Informs the observer that a progress note should be shown.
        * @param aGetter A pointer to a progess status getter object.
        * @return None.
        */
        virtual void UpdateProgressNoteL(TInt aFileSize,TInt aFileIndex, const TDesC& aFileName ) = 0;
        
        /**
        *  Informs the observer that a confirmation query for sending
        *  should be shouwn for BIP sending
        *  @return
        */ 
        virtual void LaunchConfirmationQuery(const TDesC& aConfirmText)=0; 
        
        /**
        *  Informs the observer that a connect is timedout        
        *  @return
        */ 
        virtual void ConnectTimedOut()=0;
       
    };


/**
*  A starter class for Bluetooth services.
*/
NONSHARABLE_CLASS (CBTServiceStarter) : public CActive,
                          public MBTServiceObserver,
                          public MObexUtilsDialogObserver,
                          public MObexUtilsProgressObserver,
                          public MBTEngSdpResultReceiver,
                          public MBTEngSettingsObserver
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBTServiceStarter* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CBTServiceStarter();

    public: // New functions
        
        /**
        * Starts the given service.
        * @param aService The service to be started.
        * @param aList Parameters for the service.
        * @param aWaiter A pointer to a waiter object.
        * @return None.
        */
        void StartServiceL( TBTServiceType aService, 
                            CBTServiceParameterList* aList,
                            CActiveSchedulerWait* aWaiter = NULL );

    private: // Functions from base classes



        /**
        * From MBTServiceObserver A controller has completed.
        * @param aStatus The completion status of the controller.
        * @return None.
        */
        void ControllerComplete( TInt aStatus );

        /**
        * From MBTServiceObserver A progress note should be shown.
        * @param aGetter A pointer to a progess status getter object.
        * @param aTotalSize Max size of the progress bar.
        * @param aFileCount Total Number of files to be sent
        * @return None.
        */
        //todo reuse the second param as it is no longer used check it being used in other places
        void LaunchProgressNoteL( MBTServiceProgressGetter* aGetter, TInt aFileCount);
        
        
        /**
        * From MBTServiceObserver Updating progress note when multiple files are sent.
        * @param aFileSize Size of the file to be send.
        * @param aFileIndex index of the file to be send.
        * @param aFileName name of the file to be send.
        * @return None.
        */
        void UpdateProgressNoteL(TInt aFileSize,TInt aFileIndex, const TDesC& aFileName );
		

        /**
        * From MObexUtilsWaitDialogObserver The wait note has been dismissed.
        * @param aButtonId The button id that was used to dismiss the dialog.
        * @return None.
        */
        void DialogDismissed(TInt aButtonId);
        
        /**
        * From MObexUtilsProgressObserver request for number of bytes sent
        * @Param None.
        * @return Number of bytes sent.
        */
        TInt GetProgressStatus();
        
        /**
        *  From MBTServiceObserver A confirmation query for BIP
        *  profile  should be shown.
        *  @return key
        */ 
        TInt LaunchConfirmationQuery(TInt aResourceId); 
        
        
        /**
        *  From MBTServiceObserver A confirmation query for BIP
        *  profile  should be shown.
        */ 
        void LaunchConfirmationQuery(const TDesC& aConfirmText);
        
        /**
        *  From MBTServiceObserver a connect request is timedout        
        *  @return
        */ 
        void ConnectTimedOut();
        
        
        /**
        * Provides notification of the result of a service search that matches 
        * the requested UUID (through CBTEngDiscovery::RemoteSdpQuery).
        * This method indicates that the search has completed, and returns 
        * all the results to the caller at once.
        *
        * @since S60 v3.2
        * @param aResult Array of record handles that match the requested UUID.
        *                Note: the array will not be available anymore after 
        *                this method returns.
        * @param aTotalRecordsCount The total number of records returned.
        * @param aErr Error code of the service search operation; KErrNone if 
        *             sucessful, KErrEof if no record matched the requested UUID, 
        *             KErrCouldNotConnect and KErrCouldDisconnected in case of 
        *             Bluetooth connection errors; otherwise one of the 
        *             system-wide error codes.
        */
        void ServiceSearchComplete( const RSdpRecHandleArray& aResult, 
                                    TUint aTotalRecordsCount, TInt aErr );

        /**
        * Provides notification of the result of an attribute search that matches 
        * the requested attribute (through CBTEngDiscovery::RemoteSdpQuery).
        * This method indicates that the search has completed, and returns 
        * all the results to the caller at once.
        *
        * @since S60 v3.2
        * @param aHandle Record handle of the service record containing the result.
        * @param aAttr Array containing the attribute that matches the 
        *              requested attribute.
        *              Note: the array will not be available anymore after 
        *              this method returns.
        * @param aErr Error code of the service search operation; KErrNone if 
        *             sucessful, KErrEof if the requested attribute was not 
        *             contained in the specified service record, 
        *             KErrCouldNotConnect and KErrCouldDisconnected in case of 
        *             Bluetooth connection errors; otherwise one of the 
        *             system-wide error codes.
        */
        void AttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                           const RSdpResultArray& aAttr, 
                                           TInt aErr );

        /**
        * Provides notification of the result of an combination of a service 
        * and attribute search (through CBTEngDiscovery::RemoteSdpQuery).
        * This method is called for each service and attribute combination for 
        * which a match was found. The last result (which could be empty if no 
        * match was found) contain error code KErrEof to indicate that the 
        * search has completed.
        *
        * @since S60 v3.2
        * @param aHandle Record handle of the service record containing the result.
        * @param aAttr Array containing the attribute that matches the 
        *              requested attribute.
        *              Note: the array will not be available anymore after 
        *              this method returns.
        * @param aErr Error code of the service search operation; KErrNone if 
        *             sucessful and more results follow, KErrEof indicates that 
        *             this is the last result (which could be empty if no match 
        *             was found), KErrCouldNotConnect and KErrCouldDisconnected 
        *             in case of Bluetooth connection errors; otherwise one of 
        *             the system-wide error codes.
        */
        void ServiceAttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                                      const RSdpResultArray& aAttr, 
                                                      TInt aErr );

        /**
        * Provides notification of the result of the discovery of nearby 
        * Bluetooth devices.
        *
        * @since S60 v3.2
        * @param aDevice The data structure encapsulates all information 
        *                about the selected device. Ownership of the data 
        *                structure has not been transfered and is still with
        *                the API client.
        * @param aErr Error code of the device search operation; KErrNone if 
        *             sucessful, KErrCancel if the user cancelled the 
        *             dialog, KErrCancel if CBTEngDiscovery::CancelSearchRemoteDevice
        *             was called; otherwise one of the system-wide error codes.
        */
        void DeviceSearchComplete( CBTDevice* aDevice, TInt aErr );
    
        /**
         * From MBTEngSettingsObserver.
         * Provides notification of changes in the power state 
         * of the Bluetooth hardware.
         *
         * @since S60 v3.2
         * @param aState EBTPowerOff if the BT hardware has been turned off, 
         *               EBTPowerOn if it has been turned off.
         */
        void PowerStateChanged( TBTPowerStateValue aState );

        /**
         * From MBTEngSettingsObserver.
         * Provides notification of changes in the discoverability 
         * mode of the Bluetooth hardware.
         *
         * @since S60 v3.2
         * @param aState EBTDiscModeHidden if the BT hardware is in hidden mode, 
         *               EBTDiscModeGeneral if it is in visible mode.
         */
        void VisibilityModeChanged( TBTVisibilityMode aState );

        /**
         * From CActive.
         * Called by the active scheduler when the request has been cancelled.
         * 
         * @since S60 v3.2
         */
        void DoCancel();

        /**
         * From CActive.
         * Called by the active scheduler when the request has been completed.
         * 
         * @since S60 v3.2
         */
        void RunL();

        /**
         * From CActive.
         * Called by the active scheduler when an error in RunL has occurred.
         * 
         * @since S60 v3.2
         */
        TInt RunError( TInt aError );

    private:

        /**
        * C++ default constructor.
        */
        CBTServiceStarter();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Check that the parameters are valid for a given service.
        * @param aService The requested service.
        * @param aList Parameters for the service.        
        * @return None.
        */
        TBool ValidParameters( TBTServiceType aService, 
                               const CBTServiceParameterList* aList) const;

        /**
        * Starts finding a Bluetooth profile suitable for requested service.
        * @param aService The requested service.
        * @return None.
        */
        void StartProfileSelectL( TBTServiceType aService );

        /**
        * Start a controller for the given Bluetooth profile.
        * @param aProfile The profile to be started.
        * @return None.
        */
        void StartProfileL( TBTServiceProfile aProfile );

        /**
        * Launches a connecting wait note
        * @param None.
        * @return None.
        */
        void LaunchWaitNoteL();
        
        /**
        * Cancel connecting wait note
        * @param None.
        * @return None.
        */
        void CancelWaitNote();

        /**
        * Show a note with given error value
        * @param aReason Error value
        * @return None.
        */
        void ShowErrorNote( TInt aReason ) const;

        /**
        * Cancel progress note
        * @param None.
        * @return None.
        */
        void CancelProgressNote();
        
        /**
        * StopTransfer and show information note
        * @param aError Error code if error exist.
        * @return None.
        */
        void StopTransfer( TInt aError );

        /**
         * Check if phone is in offline mode, and ask the user if it is..
         * @return ETrue if the phone is in offline mode, otherwise EFalse.
         */
        TBool CheckOfflineModeL();

        /**
        * Start BT device discovery.
        * @return None.
        */
        void StartDiscoveryL();

        /**
         * Turn BT power on.
         * @param aState The current BT power state.
         * @return None.
         */
        void TurnBTPowerOnL( const TBTPowerStateValue aState );
        
    private:    // Data definitions

        enum TBTServiceStarterState
            {
            EBTSStarterIdle,
            EBTSStarterFindingBPP,
            EBTSStarterFindingOPP,
            EBTSStarterFindingBIP,
            EBTSStarterStoppingService
            };

    private:    // Data

        CBTDevice*                  iDevice;
        CBTEngDiscovery*            iBTEngDiscovery;
        CBTServiceParameterList*    iList;
        CObexUtilsDialog*           iDialog;
        CBTSController*       iController;

        TBTServiceType              iService;
        TInt                        iClientChannel;
        TBTServiceStarterState      iState;
        //todo verify the usage of below variable when file sending to mulitple devices or
        //when files from different applications are sent simultaneously
        TBool                       iServiceStarted;
        TMsvId                      iMessageServerIndex;
        
        MBTServiceProgressGetter*   iProgressGetter;
        CActiveSchedulerWait*       iWaiter;
        TBool                       iAllSend;                     
        TInt						iBytesSendWithBIP;        
        //todo verify the usage of below variable as this part of legacy code
        //and false condition will never be hit.
        TBool						iProgressDialogActive;        
        TBool                       iUserCancel;
        CBTEngSettings*             iBTEngSettings;
        TBool                       iWaitingForBTPower;
        RNotifier                   iNotifier;  // Handle to the generic notifier.
        TPckgBuf<TBool>             iOffline;
        TPckgBuf<TBool>             iName;
        TBool                       iSendToBIPOnlyDevice;
        TBTActiveNotifier           iActiveNotifier;
        TBool                       iFeatureManagerInitialized;
        TBool                       iTriedBIP;
        TBool                       iTriedOPP;
        CBTServiceDelayedDestroyer* iDelayedDestroyer;
        TBool                       iLocalisationInit;
    };

#endif      // BT_SERVICE_CONTROLLER_H
            
// End of File

