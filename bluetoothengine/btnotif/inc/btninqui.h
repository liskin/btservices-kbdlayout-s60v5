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
* Description:  Declares Bluetooth device inquiry user interface class.
*
*/


#ifndef BTNINQUI_H
#define BTNINQUI_H

// INCLUDES

#include <btextnotifiers.h> // Bluetooth notifiers API
#include <badesca.h>        // For array handling
#include <AknIconArray.h>
#include "btnotifactive.h"      // Active Object Helper class for inquiry
#include <bt_sock.h>
#include <btengdevman.h>
#include "btnotifier.h"

const TInt KPeriodicTimerInterval4Sec(4000000);


// FORWARD DECLARATIONS

class CAknStaticNoteDialog;
class CAknSingleGraphicPopupMenuStyleListBox;
class CAknPopupList;
class CAknQueryDialog;
class CBTInqNotifier;

// CONSTANTS
enum TSearchFlowState
    {
    ESearchCompleted,  // search completed for any reason, end of UI flow.
    ESearchAgain,      // no device found, user wants to search again.
    ESearchFirstDeviceFound, // the first device is found, show found device list.
    };   

// CLASS DECLARATION
/**
 *  Class MBTNDeviceSearchObserver
 *
 *  Callback class to notify device searching completed. 
 */
class MBTNDeviceSearchObserver
    {
public:

    /**
     * Callback to notify that the device search request has completed.
     *
     * @since S60 5.1
     * @param aErr Error code
     * @param aDevice Pckgbuf to retrieve the response from the device selection.
     */
    virtual void NotifyDeviceSearchCompleted(TInt aErr, 
            const TBTDeviceResponseParams& aDevice = TBTDeviceResponseParams()) = 0;
    };

/**
* A helper class used in Bluetooth device search.
* This class is responsible of the UI activity.
*/
NONSHARABLE_CLASS(CBTInqUI) : public CBase, public MBTEngDevManObserver, public MBTNotifActiveObserver
    {
    public: // Constructor and destructor
        /**
        * C++ default constructor.
        */
        CBTInqUI( MBTNDeviceSearchObserver* aObserver,
                CBTNotifUIUtil* aUtil, const TBTDeviceClass& aDesiredDevClass);

        /**
        * The 2nd phase constructor is required to do Asyncronous operations,
        * So the readiness of the CBTInqUI is notified Asynchronously to the caller
        */
        void ConstructL();     
        
        /**
        * Destructor.
        */
        virtual ~CBTInqUI();

    public: // New functions

        /**
        * Cancels all inquiry activity.
        */
        void Cancel();
        
        /**
        * Issues a device inquiry request.
        * @param None.
        * @return None.
        */
        void IssueRequestL();

        /**
         * Check if Eir device name available.
         * @param TNameEntry    Decode it to get device name
         * @param TBTDeviceName Device name returned
         * @param TBool         Is the name returned complete
         * @return ETrue if Eir data contains name.
         */
        static TBool CheckEirDeviceName( TNameEntry& aEntry, TBTDeviceName& aName, TBool& aIsComplete );

    private: // Functions from base classes
        
        /**
         * From MBTNotifActiveObserver
         */
        void RequestCompletedL( CBTNotifActive* aActive, TInt aId, TInt aStatus );
        
        /**
         * From MBTNotifActiveObserver
         */
        void HandleError( CBTNotifActive* aActive, TInt aId, TInt aError );
        
        /**
         * From MBTNotifActiveObserver
         */
        void DoCancelRequest( CBTNotifActive* aActive, TInt aId );
        
        /**
         * From MBTEngDevManObserver
         */
        void HandleGetDevicesComplete(TInt aErr,  CBTDeviceArray* aDeviceArray);
        
        /**
         * From MBTEngDevManObserver
         */
        void HandleDevManComplete(TInt aErr);        

   
    private:
        
        /**
         * Create the device search active object and prepare for search.
         */
        void CreateScannerL();
        
        /**
        * Removes the device search active object if it still exists.
        * @param None.
        * @return None.
        */
        void RemoveScanner();

        /**
        * Retrieves paired devices from BT Registry
        * Calls HandleGetDevicesComplete, when done
        */
        void RetrievePairedDevices();

        /**
        * Retrieves used devices from BT Registry
        * Calls HandleGetDevicesComplete, when done
        * @return None.        
        */
        void RetrieveUsedDevices();

        /**
        * Searches given BT Device from device array.
        * @param aDeviceArray The device array to search in 
        * @param aDevice The device to be searched.
        * @return Index of the device if found.
        */
        TInt LookupFromDevicesArray( const CBTDeviceArray* aDeviceArray, const CBTDevice* aDevice ) const;
        
        /**
        * Constructs a device popup list from resources.
        * @param aSoftkeysResourceId Resource id for softkeys of the list.
        * @param aTitleResourceId Resource id for the title of the list.
        * @return None.
        */
        void CreatePopupListL(TInt aSoftkeysResourceId, TInt aTitleResourceId );

        /**
        * Generates a format string and adds it to iDeviceItems array.
        * Updates friendly name of given device if one is found from registry.
        * @param aDevice The device to be added to list.
        * @param aSignalStrength The signal bar for RSSI
        * @return None.
        */
        void AddToDeviceListBoxL(CBTDevice* aDevice, const TInt aSignalStrength = 0 );             
		
		/**
        * Creates and adds local bitmap to icon array.
        * @param aID Item ID of the masked bitmap to be created.
        * @param aFilename Filename to be used to construct the item.
        * @param aBitmapId The ID if bitmap 
        * @param aMaskId The ID of bitmap's mask
		* @param aIconList The icon list for BT devices listbox.
        */
        void CreateAndAppendIconL( const TAknsItemID& aID,
			     				   const TDesC& aFileName,
								   const TInt aBitmapId,
								   const TInt aMaskId,
								   CAknIconArray* aIconList);
								  
        /**
        * Adjust array, leave 5 lastet used devices in the array and remove the rest,
        * and then insert the paired device array to this array by used time order.
        * In the adjusted array, the first item will be the most lately used device.
        * @param aDevArray The bluetooth device arrary to be adjusted.
        * @return None.
        */
		void AdjustDeviceArrayL( CBTDeviceArray* aDevArray);
		
		/**
		* Filter for searching device
		* @param The desired device class user selects from the filter
		* @return User's input select/cancel
		*/
		void SelectSearchCategoryL();
		
		/**
		* Add filtering items to search device filter
		* @param aResourceId
		* @return None
		*/
		void AddItemToSearchFilterL ( TInt aResourceId );

		/**
		* Show query to ask if unblock device
		* @param aDevice Selected device
		* @return Keypress value
		*/
		TInt QueryUnblockDeviceL(CBTDevice* aDevice);
		
		 /**
         * The callback function for CPeriodic 
         * to reset the inactivity time
         */
		 static TInt PeriodicTimerCallBack(TAny* /*aAny*/);
		 
	     void GetColorIconL( TFileName& aFilename, TAknsItemID aItemID, TInt aPic, TInt aPicmask, CAknIconArray* aIconList );
     
        /**
        * Check if VoIP call is ongoing via WLAN connection.
        * @return true if it is; false otherwise.
        */
        TBool IsActiveVoIPOverWLAN();
         
        void DoDeviceFrontListSelection();
    
        /**
        * Displays previous inquiry results.
        */
        void DisplayDevicesFrontListL();
        
        /**
         * Unblock device after unblock query.
         * @param aIndex Parameter containing information about selected device.
         * @return Symbian OS error code.
         */
        TInt UnblockDevice(TInt aIndex);

        void DeviceSearchUiL();
        
        /**
         * the initiator of inquiry until the first device found, or no device found, or
         * other reasons causing inquiry stopped.
         * @param aReason the reason of the search completion
         */
        TSearchFlowState InitInquiryL(TInt& aReason);
        
        void HandleDeviceSelectionL(CBTDevice* aDev);
        
        /**
         * Adds found Bluetooth device into popup list.
         * @param aInquiryResultRecord TNameEntry and flag, which indicates if Eir device.
         */
        void DeviceAvailableL( const TNameRecord& aNameRecord, const TDesC& aDevName, TBool aIsNameComplete );
        
        /**
        * The inquiry has been completed.
        * @param aError Status code of the operation.
        * @return None.
        */
        void InquiryComplete( TInt aError );
        
        /**
         * Inform the caller of the properties of found BTdevice, 
         * which passes the search filter. Its name will be retrived later
         */
        void HandleInquiryDeviceL();

        /**
         * Name lookup complete for the current device in the unnamed devices queue.
         */
        void HandleFoundNameL();  

        /**
         * Name lookup complete for device aLastSeenIndex in the last seen array
         * (this is done for devices with partial names returned by EIR).
         */
        void HandleUpdatedNameL(TInt aLastSeenIndex);

        /**
         * Allow/Disallow dialer and app key presses.
         * @param aAllow ETrue to allow  key press; EFalse to disallow.
         */
        void AllowDialerAndAppKeyPress( TBool aAllow );

        /**
         * Update the inquiry list box with a new (complete) device name
         * (this is done for devices with partial names returned by EIR).
         */
        void DeviceNameUpdatedL(const TNameRecord& aNameRecord, TInt aLastSeenIndex);
        void DeviceUpdatedL(TInt aSignalStrength, TInt aLastSeenIndex);

        /**
         * React to page timeout on a device that's already in the inquiry list box
         * (which can happen for devices with partial names returned by EIR).
         */
        void PageTimeoutOnDeviceWithPartialNameL(TInt aLastSeenIndex);

        void FormatListBoxEntryL(CBTDevice& aDevice, const TInt aSignalStrength, TPtr aFormatString);

        TBool HaveDevsWithPartialName(TInt& aFirstFoundIndex);

    private:  // Data
        CBTNotifUIUtil*         iUiUtil;
        CBTDeviceArray*         iLastSeenDevicesArray;           // Last seen devices array from BTInqNotifier
        RArray<TBool>           iLastSeenDevicesNameComplete;    // Is the name in corresponding cell of iLastSeenDevicesArray complete ?
        CBTDeviceArray*         iLastUsedDevicesArray;           // Used devices from BT registry
        CBTDeviceArray*         iPairedDevicesArray;             // Paired devices from Bluetooth registry
        CBTDeviceArray*         iAdjustedUsedDeviceArray;         // 5 Lately used devices + All the Paired devices 
        CBTEngDevMan*           iDevMan;                        //Async deviceHandler
         
        CDesCArray*             iDefaultDeviceNamesArray;        // Default device names array
        CEikonEnv&              iEikonEnvRef;                    // Reference to eikonenv        
        
        CAknPopupList*          iDevicePopupList;                // Popup list for devices        
        CDesCArrayFlat*         iDeviceListRows;                 // Array for device list format string rows
        CAknSingleGraphicPopupMenuStyleListBox* iDeviceListBox; // ListBox for bluetooth devices
        CAknPopupList*          iSearchFilterPopupList;
        
        CBTNotifActive*         iScanner;
		
        TBool                   iDevicesFound;                   // Flag if any devices has been found
        TBool                   iInquiryComplete;
        TBool                   iBroughtForwards;                // Flag for BringForwards
        TInt                    iInquiryStatus;                  // Error code from inquiry result

        TBTRegistryQueryState     iBTRegistryQueryState;            // which registry query (if any) is in progress.
        TBTDeviceClass            iDesiredDeviceClass;         // Device Class user selects from search filter 
        CPeriodic* iPeriodicTimer; // the timer for reseting the user inactivity time
        TBool                   iSystemCancel;               // Used for a trick to avoid final device list is still on show 
                                                                // while taking out mmc during BT device discovery 
        TBTDeviceResponseParams iDevParams;
        TInt iHighLightedItemIndex;
        
        RSocketServ         iSocketServer;      // Socket server object
        RHostResolver       iHostResolver;      // Host resolver object
        TInquirySockAddr    iInquirySockAddr;   // Socket address record
        TNameEntry          iEntry;             // Inquiry results record
        TBool               iPageForName;    // Flag for inquiry name
        RArray<TNameRecord> iDevsWithoutName;    // Devices without dev name 
        TInt                iCurrentlyResolvingUnnamedDeviceIndex;             // Current btdevice to inquiry its name 
        MBTNDeviceSearchObserver* iDevSearchObserver;

		TBool               iExcludePairedDevices;
		

    };

#endif

// End of File
