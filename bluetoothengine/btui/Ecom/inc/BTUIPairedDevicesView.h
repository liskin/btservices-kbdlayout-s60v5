/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The view that handles paired devices.
*
*/

#ifndef BTUIPAIREDDEVICESVIEW_H
#define BTUIPAIREDDEVICESVIEW_H

#include <aknview.h>
#include <eikmenup.h>       // Menu pane definitions
#include <AknWaitDialog.h>
#include <AknQueryDialog.h>
#include <btengconstants.h>
#include <btengdiscovery.h>
#include "BtuiPluginInterface.h"
#include "btdevmodel.h"
#include "BTUIListedDevicesView.h"
#include "btuipluginman.h"
#include "BTUIActive.h"
#include <SecondaryDisplay/BtuiSecondaryDisplayAPI.h>
#include <gstabbedview.h>

class CAknNavigationControlContainer;
class CBTUiDeviceContainer;        
class CGSTabHelper;
class CBTEngSettings;
class CBTUIMainView;

// if there are more connection that this, it is disallowed to
// connect to any device
const TInt KBtUiMaxConnections = 6; 

/**
* Paired devicew view of the pluetooth application.
*
* This view handles lists of paired device and pairing new devices, unpairing devices
* and setting previously paired devices as blocked. Also connecting and disconnecting 
* are done by using this view.
*
*@lib BTUIPlugin.dll
*@since S60 v3.0
*/
class CBTUIPairedDevicesView : public CBTUIListedDevicesView,
							  public MBTEngSettingsObserver,
		                      public MProgressDialogCallback,
		                      public MBTEngSdpResultReceiver, // for bt discovery. SDP is not used
		                      public MGSTabbedView,
		                      public MBTUIActiveObserver
{	
    public:

		/**
        * ECOM implementation instantiation function of 
        * interface "CBtuiPluginInterface", to be used by BTUI Application.
        */        
        static CBTUIPairedDevicesView* NewL(MBtuiPluginViewActivationObserver* aObserver = NULL);

		static CBTUIPairedDevicesView* NewLC(CBTUIMainView* aBaseView, CArrayPtrFlat<MGSTabbedView>* aTabViewArray);
		
		/**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();

		void ConstructL( CBTUIMainView* aBaseView, CArrayPtrFlat<MGSTabbedView>* aTabViewArray );

		/**
        * Destructor.
        */
        virtual ~CBTUIPairedDevicesView();

        /**
        * From MGSTabbedView:
        * Creates new icon for tab. Ownership is transferred to client.
        * @since 3.1
        */       
        CGulIcon* CreateTabIconL();        

    public: // Functions from base classes
        
        /**
        * From CAknView Returns view id.
        * @param None.
        * @return View id.
        */
        TUid Id() const;

        /**
        * From CAknView Handles user commands.
        * @param aCommand A command id.        
        * @return None.
        */
        void HandleCommandL(TInt aCommand);

	protected:		
        /**
        * C++ default constructor.
        */
        CBTUIPairedDevicesView (MBtuiPluginViewActivationObserver* aObserver= NULL);

    private: // Functions from base classes

        /**
        * From CAknView Activates view.
        * @param aPrevViewId Id of previous view.
        * @param aCustomMessageId Custom message id.
        * @param aCustomMessage Custom message.
        * @return None.
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,
                                TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );

        /**
        * From CAknView Deactivates view.        
        * @param None.
        * @return None.
        */
        void DoDeactivate();       

        /**
        * From CAknView Dynamically initialises options menu.
        * @param aResourceId Id identifying the menu pane to initialise.
        * @param aMenuPane The in-memory representation of the menu pane.
        * @return None.
        */
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

		// implemented from MProgressDialogCallback
        /** This is used to cancel connecting and pairing, if user presses the Cancel-button.
        * From MProgressDialogCallback Get's called when a dialog is dismissed.        
        * @param aButtonId Id of the pressed button.
        * @return None.
        */
        void DialogDismissedL( TInt aButtonId );

	public: // from MBTDeviceObserver
    	/**
        * Informs the observer that an error happened in changing device status or a new device added.
        * @param aErr Symbian error code
        * @param aDevice the device which is changed.
        * @param aDevNameArray hold conflicting devices' short names if "AlreadyExist" error happened during "connect"
        * @return None.
        */
		void NotifyChangeDeviceComplete(const TInt aErr, const TBTDevice& aDevice,
			const RBTDevNameArray* /*aDevNameArray*/ )	;
    	/**
        * Leaving version of NotifyChangeDeviceComplete
        * @param aErr Symbian error code
        * @param aDevice the device which is changed.
        * @param aDevNameArray hold conflicting devices' short names if "AlreadyExist" error happened during "connect"
        * @return None.
        */
		void NotifyChangeDeviceCompleteL(const TInt aErr, const TBTDevice& aDevice,
			const RBTDevNameArray* /*aDevNameArray*/ )	;
		
		/** Updates the shown device list. This module will send this list forward
		 * to container that takes care of the actual display of those devices
		 *
		 *@param aDevices all the devices to be shown
		 *@param aSelectedItemIndex currently selected item after refresh.
		 */
		void RefreshDeviceList(const RDeviceArray* aDevices,TInt aSelectedItemIndex);	
		
	private: // From MBTUIActiveObserver
	    
	    /**
	     * Callback to notify that an outstanding request has completed.
	     * @param aActive Pointer to the active object that completed.
	     * @param aId The ID that identifies the outstanding request.
	     * @param aStatus The status of the completed request.
	     */
	    void RequestCompletedL( CBTUIActive* aActive, TInt aId, 
	                            TInt aStatus );

	    /**
	     * Callback to notify that an error has occurred in RunL.
	     *
	     * @param aActive Pointer to the active object that completed.
	     * @param aId The ID that identifies the outstanding request.
	     * @param aStatus The status of the completed request.
	     */
	    void HandleError( CBTUIActive* aActive, TInt aId, 
	                      TInt aError );
		
	public: // Operations to devices not containing confirmation queries
        /**
        * Sets the current security setting of the device.
        * Does not ask user about it.
        *
        * @param aTrusted  ETrue when trusted, EFalse if untrusted.
        * @return None.
        */
        void ChangeDeviceSecurityL( TBool aTrusted );
        		
        /**
        * Issues prompts to connects to specified accessory.
        * Connects to it if user andswers yes.
        *
        * @param TBTDevice  connect to this device
        * @return None.
        */
		void ConnectL( const TBTDevice& aDevice, TBool aNewWaitNote );        
   
        /**
        * Disconnects from the selected BT Accessory.
        *
        * @param None.
        * @return None.
        */
        void DisconnectL();
		
		/**
        * Removes iDisconnectQueryDlg created in DisconnectL
        *
        * @param None.
        * @return None.
        */        
        void CancelDisconnectQueryDlg();
        
        /**
        * Disconnects from the selected BT Accessory without query.
        *
        * @param None.
        * @return None.
        */        
        void DisconnectWithoutQuery();

    	/** Checks if there is an active phonecall.
    	*@return ETrue if there is, EFalse otherwise
    	*/
    	TBool CallOnGoing();
              		

		/** This is used to create TCallBack to help. This is static, since TCallBack does not
		 * allow instance methods. This is used by BlockDeviceDlgL to add help callback to the query.
		 *
		 * @param aTCoeHelpContext. Must be instance TCoeHelpContext, or NULL.
		 * If this is not specified this will open help about blocked devices.
		 * This is not an instance of TCoeHelpContext, since TCallback does not allow
		 * parameters of other type that TAny*.
		 * @return allways KErrNone. TCallBack does not allow void functions
		 */
		static TInt LaunchHelp(TAny *aTCoeHelpContext=NULL);

	private: //These handle command given by user	
        /**
        * Asks new nick name for the BT Device from user and
        * attempt to rename the device. Failures are handled by NotifyChangeDeviceComplete.
        *
        * @param None.
        * @return None.
        */
        void RenameDeviceDlgL();

        /**
        * Asks confirmation for deleting a BT Device from user.
        * Delete the current device if yes replied yes.
        *
        * @param None.
        * @return None.
        */
        void DeleteDeviceDlgL();

        /**
        * Asks confirmation for deleting all BT devices from user.
        * Delete the devices if yes replied yes.
        *
        * @param None.
        * @return None.
        */
        void DeleteAllDevicesDlgL();

        /**
        * Initiates the pairing procedure to get a new paired device.
        * DeviceSearchComplete is called after the dialog is done.
        *
        * @param None.
        * @return None.
        */
        void NewPairedDeviceDlgL();
        
  		/**
        * Asks confirmation for Blocking the selected BT Device.
        * Blocks the current device if yes replied yes.
        *
        * @param None.
        * @return None.
        */
		void BlockDeviceDlgL();
		
		/**Opens a wait note.
		 *
		 *@param dialog. The dialog object to be used for displaying this note.
		 *@param aNoteResource The resource of the note, not including the text.
		 *@param aNoteTextResource The text resource. The %U if this note is filled up with aDevName.
		 *@param aSec SecondaryDisplaycommand
		 *@param aDevName The dev name filled in aNoteTextResource
		 */		
		void OpenWaitNoteL(CAknWaitDialog *&dialog,TUint aNoteResource,TUint aNoteTextResource,
			TSecondaryDisplayBtuiDialogs aSec,const TDesC& aDevName);
		
		/** Open Setting view for peripheral devices
		 * 
		 * @param None.
		 * @return None.
		 */
		void LaunchSettingViewL();

		/** Setup device specific menu options
		 * 
		 * @param None.
		 * @return None.
		 */
		void SetupMenuCmd(TInt aIndex, CEikMenuPane* aMenu);
		
		
	public:	// Inherited from MBTEngSdpResultReceiver
		
		
		/** Inherited from MBTEngSdpResultReceiver and implemented as empty, 
		 * since this class will not do service searches, only device searches.
		 */
		void ServiceSearchComplete( const RSdpRecHandleArray& aResult, 
			TUint aTotalRecordsCount, TInt aErr ) ;
		/** Inherited from MBTEngSdpResultReceiver and implemented as empty, 
		 * since this class will not do attributes seaches, only device searches.
		 */
    	void AttributeSearchComplete( TSdpServRecordHandle aHandle, 
    		const RSdpResultArray& aAttr,TInt aErr ) ;		
		/** Inherited from MBTEngSdpResultReceiver and implemented as empty, 
		 * since this class will not do service attribute seaches, only device searches.
		 */
	    void ServiceAttributeSearchComplete( TSdpServRecordHandle aHandle,
	    	const RSdpResultArray& aAttr,TInt aErr );

	  /**
	    * Provides notification of the result of the discovery of nearby 
	    * Bluetooth devices. If a device is selected this class will try to pair it.
		*
	    * @param aDevice The data structure encapsulates all information 
	    *                about the selected device. Ownership of the data 
	    *                structure has not been transfered and is still with
	    *                the API client.
	    *				 This is not TBTDevice that is normally used inside the UI.	    
	    * @param aErr Error code of the device search operation; KErrNone if 
	    *             sucessful, KErrCancel if the user cancelled the 
	    *             dialog, KErrAbort if CBTEngDiscovery::CancelSearchRemoteDevice
	    *             was called; otherwise one of the system-wide error codes.
	    */
	    void DeviceSearchComplete( CBTDevice* aDevice, TInt aErr );
	    
	    /**
	     * Provides notification of the result of the discovery of nearby 
	     * Bluetooth devices and EIR data. If a device is selected this class will try to pair it.
	     *
	     * @since S60 v5.1
	     * @param aDevice The data structure encapsulates information 
	     *                about the selected device. 
	     *                Ownership of the data structure has not been transfered and
	     *                is still with the API client.
	     * @param aEirWrapper Contains the EIR data of the remote device.
	     *                Ownership of the data structure has not been transfered and
	     *                is still with the API client. 
	     * @param aErr Error code of the device search operation; KErrNone if 
	     *             sucessful, KErrCancel if the user cancelled the 
	     *             dialog, KErrAbort if CBTEngDiscovery::CancelSearchRemoteDevice
	     *             was called; otherwise one of the system-wide error codes.
	     */	    
	    void DeviceSearchComplete( CBTDevice* aDevice, 
	            TNameEntry* aNameEntry, 
	            TInt aErr );
	    
private:	    
	  /** Leaving version of DeviceSearchComplete.
		*
	    * @param aDevice The data structure encapsulates all information 
	    *                about the selected device. Ownership of the data 
	    *                structure has not been transfered and is still with
	    *                the API client.
	    *				 This is not TBTDevice that is normally used inside the UI.
	    * @param aEirWrapper Contains the EIR data of the remote device.
        *                Ownership of the data structure has not been transfered and
        *                is still with the API client.	    
	    * @param aErr Error code of the device search operation; KErrNone if 
	    *             sucessful, KErrCancel if the user cancelled the 
	    *             dialog, KErrAbort if CBTEngDiscovery::CancelSearchRemoteDevice
	    *             was called; otherwise one of the system-wide error codes.
	    */
	    
	    void DeviceSearchCompleteL( CBTDevice* aDevice, 
	            TNameEntry* aNameEntry, TInt aErr );
	    
	  /** Shows 'Disonnect first' note
		*
	    * @param aDevice 
	    */	    
        void ShowDisconnecNoteL(TBTDevice *aDevice);	
        
        /**
         * Auto connect to headset after pairing succeeded.
         * @param aCount the counter of connecting request issuing staring from 0.
         */
        void ConnectIfNeededAfterPairL( TInt aCount );
        
	public: // internal methods for informing operation completes	    
        /**
        * Informs that pairing attempt of a device has been completed.
        * Dismisses wait note and displays questions about authorizing and
        * connecting, if the device is connectable.
        *
        * This method is called by ModifyDeviceChangeL. This method will not inform
        * iContainer about the pairing since that is done by ModifyDeviceChangeL.
        *
        * @param aError Status code of the operation.
        * @param aDeviceIndex the new place of the device in UI
        * @return None.
        */  
	    void BondingCompleteL( TInt aError,const TBTDevice& aDevice);
	    
	    /** Informs that attempt to connect to a device is completed. Closes wait
	     * note and if there was an error shows the error notice.
	     *
         * This method is called by ModifyDeviceChangeL. This method will not inform
         * iContainer about the pairing since that is done by ModifyDeviceChangeL.	    
         *
	     * @param aError KErrNone or system wide error code
	     * @param aDeviceIndex The index of the new device 
	     * @param aConflictingDeviceName this is filled only when
	     */    	
    	void ConnectCompleteL(TInt aError,const TBTDevice& aDevice,
    	  const RBTDevNameArray* aConflictingDeviceNames=NULL);
    	  
    	  
       /**
        * From MBTEngSettingsObserver - Called when powerstate is changed
        * @param TBTPowerStateValue power state
        * @return None
        */       
		void PowerStateChanged( TBTPowerStateValue aState );
		
        /**
        * From MBTEngSettingsObserver - Called when visibility is changed
        * @param TBTPowerStateValue state of visibility mode
        * @return None
        */    		
		void VisibilityModeChanged( TBTVisibilityMode aState );
		 
    	
    private: // Data
    
        CAknWaitDialog*                 iWaitForPairingDlg; 	// Wait note for pairing
        CAknWaitDialog*                 iWaitForConnectDlg; 	// Wait note for connecting to BT Accessory        
		CAknQueryDialog* 				iDisconnectQueryDlg;	// Question dialog for disconnecting BT Accessory
        TBool                           iHelpEnabled; 				// Flag for the Help feature
		
		// these are needed so these are owened by someone
		// and can be cleaned up, in case the operations are interrupted by the user.
		CBTDevice*						iPairingDevice;
		TNameEntry                      iPairingDevNameEntry;
		TBTDevice* 						iConnectDevice; 	//device we are trying to connect to
		TBTDevice 						iDisconnectQueryDevice; 	//device we are trying to disconnect
				
		CBTEngDiscovery*				iDeviceSearcher;	// used only for searching devices, not for device discovery
		CBTEngSettings*					iBtEngSettings;		// used only for turning on bluetooth, 
		CBTUIPluginMan* 				iBTPluginMan;		
        CGSTabHelper* 					iTabHelper;
	    CBTUIMainView*					iBaseView; 
		TBool							iConstructAsGsPlugin;	// Flag that indicate this obj is served as GS plugin not BTUI app's plugin.
        TInt                            iExpectedCallBack;      // In case of DeleteAll, counter for expected callback NotifyChangeDeviceCompleteL()
        TBool                           iSearchOngoing;        
        TBool							iIsConnectDevice;
		MBtuiPluginViewActivationObserver* iActivationObserver; // view activation observer        
        TBool 							iIsAlreadyConnected; // used only for showing "Connected to" note once for Stereo device 
        TBTDevice                       iMenuDevice;
        CEikMenuPane*                   iActiveMenuPane;
        CBTUIActive*                    iActive;   // used for subscribing notifier lock status
        TBTDevAddr                      iPairConnectAddr; // the address of the just paired device
                                                          // with which a connection establishment is pending
        TBool                           iIsPairConnect; // if we are doing auto-connecting after pairing or not

};

#endif


