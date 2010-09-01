/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares Bluetooth notifiers base class.
*
*/


#ifndef BTNOTIFIERBASE_H
#define BTNOTIFIERBASE_H

// INCLUDES

#include "btnotif.h"
#include "btnotifuiutil.h"
#include <eikenv.h>
#include <btdevice.h>
#include <btengdevman.h>

// CONSTANTS

// Literals for resource and bitmap files ( drive, directory, filename(s) )

const TUid KBTInquiryChannel            = {0x00000601}; // Channel for inquiry notifier
const TUid KBTAuthorisationChannel      = {0x00000602}; // Channel for PIN query notifier
const TUid KBTAuthenticationChannel     = {0x00000603}; // Channel for authorisation notifier
const TUid KBTBBEventChannel            = {0x00000604}; // Channel for baseband event notifier
const TUid KBTObexPINChannel            = {0x00000605}; // Channel for OBEX PIN notifier
const TUid KBTPowerModeChannel          = {0x00000606}; // Channel for power mode notifier
const TUid KBTAudioAccessoryChannel     = {0x00000607}; // Channel for audio accessory notifier
const TUid KBTLowMemoryChannel          = {0x00000608}; // Channel for low memory notifier
const TUid KBTPairedDeviceSettingChannel  = {0x00000610};

// FORWARD DECLARATIONS
class CBTEngSettings;

enum TBTRegistryQueryState
{
	ENoQuery=0,
	EQueryPaired=1,
	EQueryUsed=2,
	ESetDeviceAuthorizeState,
	ESetDeviceBlocked,
	ESetDeviceUnblocked,
};

// CLASS DECLARATION
/**
* This class is the base class for all notifiers.
*/
NONSHARABLE_CLASS(CBTNotifierBase): public CBase, public MEikSrvNotifierBase2, MBTEngDevManObserver
    {
    public: // Constructors and destructor

        /**
        * Destructor.
        */
        virtual ~CBTNotifierBase();

        /**
        * A utility function for checking whether autolock is on.
        * @param None.
        * @return A boolean according to autolock state.
        */
        TBool AutoLockOnL();

    protected: // Constructors and destructor

        /**
        * C++ default constructor.
        */
        CBTNotifierBase();

    protected: // New functions

        /**
        * Symbian 2nd phase constructor.
        */
        virtual void ConstructL();

        /**
        * Used in asynchronous notifier launch to store received parameters
        * into members variables and make needed initializations.
        * @param aBuffer A buffer containing received parameters
        * @param aReturnVal The return value to be passed back.
        * @param aMessage Should be completed when the notifier is deactivated.
        * @return None.
        */
        virtual void GetParamsL(const TDesC8& aBuffer,
                                TInt aReplySlot,
                                const RMessagePtr2& aMessage)=0;

        /**
        * A utility function for setting the power status of Bluetooth.
        * If the power is not on the user will be presented a query.
        * @return ETrue if BT is turned on successfully; EFalse otherwise
        */
        TBool CheckAndSetPowerOnL();

		/**
		* A function for setting BT name
		* @return ETrue if BT name is set successfully; EFalse otherwise
		*/
		TBool AskLocalBTNameQueryL();

        /**
		* Check if there is any existing connection to audio profiles from
		* the same device
		* @return ETrue if the device is connected.
		*/
		TBool IsExistingConnectionToAudioL(const TBTDevAddr& aDevAddr);

		/**
		* Complete Client/Server message and Nulled the reply slot.
		* @param aErr  Error code for client notification.
		* @return None
		*/
		void CompleteMessage(TInt aErr);

       /**
        * Complete Client/Server message and Nulled the reply slot.
        * @param aErr  Error code for client notification.
        * @return None
        */
        void CompleteMessage(TInt aValueToReplySlot, TInt aErr);

       /**
        * Complete Client/Server message and Nulled the reply slot.
        * @param aErr  Error code for client notification.
        * @return None
        */
        void CompleteMessage(const TDesC8& aDesToReplySlot, TInt aErr);

		/**
		* Checks from central repository whether the Bluetooth friendly name
        * has been modified .
		* @return ETure if the name has been modified.
		*/
		TBool IsLocalNameModifiedL();

		/**
		 * Get the device information from BT Registry by the given address.
		 */
		void GetDeviceFromRegL(const TBTDevAddr& aAddr);

		/**
		 * virtual function to be ovewritten by sub-classes.
		 * Will be called when Getting Device by BTEng DevMan is completed.
		 * @param aDev the device from registry if it is not null. indicates either a failure
		 *             or the device is not available in registry.
		 */
		virtual void HandleGetDeviceCompletedL(const CBTDevice* aDev);

        /**
         * Asks the user if s/he want's to block the device in question
         * Does not block the device. That is left to the caller.
         * NOTE: Cannot be called while iAuthQueryDlg is active.
         */
        void QueryBlockDeviceL();

        /**
         * Set BTDevice's security setting to blocked and modify device
         * @param None
         * @return None
         */
        void DoBlockDevice();

        /**
         * Set BTDevice's security setting to un/authorized and modify device
         * @param aTrust Set authorized if ETrue; otherwise, unauthorized.
         * @return None
         */
        void ChangeAuthorizeState( TBool aTrust);

        /**
         * Checks if the user is queried too often, and handle the result.
         * @return None
         */
        void CheckAndHandleQueryIntervalL();

    private:

        void DoHandleGetDevicesCompleteL(TInt aErr, CBTDeviceArray* aDeviceArray );

    protected: // Functions from base classes

        /**
        * From MEikSrvNotifierBase2 Called when a notifier is first loaded
        * to allow any initial construction that is required.
        * @param None.
        * @return A structure containing priority and channel info.
        */
        virtual TNotifierInfo RegisterL()=0;

        /**
        * From MEikSrvNotifierBase2 The notifier has been deactivated
        * so resources can be freed and outstanding messages completed.
        * @param None.
        * @return None.
        */
        virtual void Cancel();

#ifdef __SERIES60_HELP
		/** This is used to create TCallBack to help.
		 * @param TCoeHelpContext. Must be instance TCoeHelpContext, or NULL.
		 * If null this will open help about blocked devices.,
		 * @return KErrNone
		 */
		static TInt LaunchHelp(TAny *TCoeHelpContext=NULL);
#endif

    private: // Functions from base classes

        /**
        * From MEikSrvNotifierBase2 Called when all resources allocated
        * by notifiers should be freed.
        * @param None.
        * @return None.
        */
        virtual void Release();

        /**
        * From MEikSrvNotifierBase2 Return the priority a notifier takes
        * and the channels it acts on.
        * @param None.
        * @return A structure containing priority and channel info.
        */
        virtual TNotifierInfo Info() const;

        /**
        * From MEikSrvNotifierBase2 Synchronic notifier launch.
        * @param aBuffer Received parameter data.
        * @return A pointer to return value.
        */
        virtual TPtrC8 StartL(const TDesC8& aBuffer);

        /**
        * From MEikSrvNotifierBase2 Asynchronic notifier launch.
        * @param aBuffer A buffer containing received parameters
        * @param aReturnVal The return value to be passed back.
        * @param aMessage Should be completed when the notifier is deactivated.
        * @return A pointer to return value.
        */
        virtual void StartL(const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage);

        /**
        * From MEikSrvNotifierBase2 Updates a currently active notifier.
        * @param aBuffer The updated data.
        * @return A pointer to return value.
        */
        virtual TPtrC8 UpdateL(const TDesC8& aBuffer);

        /**
         * From MBTEngDevManObserver
         * Indicates to the caller that adding, deleting or modifying a device
         * has completed.
         */
        virtual void HandleDevManComplete(TInt aErr);

        /**
         * From MBTEngDevManObserver
         * Call back function when GetDevices() request is completed.
         * @param aErr Status information, if there is an error.
         * @param aDeviceArray Array of devices that match the given criteria
         *                     (the array provided by the calller).
         */
        void HandleGetDevicesComplete( TInt aErr, CBTDeviceArray* aDeviceArray );

        /**
         * Checks the timestamp since last query.
         * @return true if too often
         */
        TBool CheckQueryInterval();

    protected: // Data

        RMessagePtr2                iMessage;               // Received message
        TInt                        iReplySlot;             // Reply slot
        TNotifierInfo               iInfo;                  // Notifier parameters structure
        TBTRegistryQueryState       iBTRegistryQueryState;
        CBTDeviceArray*             iDeviceArray;  // for getting device from registry
        CBTDevice*                  iDevice;                // Current Bluetooth device
        TBTDevAddr                  iBTAddr; // Gotten from PckBuffer, constant no matter how iDevice changes.
        CBTNotifUIUtil*             iNotifUiUtil;           // Utility to show UI notes & queries
        CBTEngSettings*             iBTEngSettings;
        TBool                       iIsCoverUI;

    private:
        CBTEngDevMan*               iDevMan;  // for BT registry manipulation



    };

#endif

// End of File
