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
* Description:   Maintain a Bluetooth devices data model for UI components.
*
*/

#ifndef BTDEVMODEL_H
#define BTDEVMODEL_H

#include <e32base.h>     
#include <btmanclient.h>
#include <btdevice.h>
#include <e32cmn.h>
  
// used to support different device list of UI
enum TBTDeviceGroup
    {
    EGroupPaired, // for paired device list
    EGroupBlocked // for blocked device list
    };
    
// for showing left side icon of a device in device list. Based on CoD major device
enum TBTDeviceType
    {
    EDeviceComputer = 0,// computer
    EDevicePhone,		// mobile phone
    EDeviceAudio, 		// headset or loudspeakers    
    EDeviceCarkit, 		// audio-caraudio
    EDevicePrinter, 	// imaging-printer
    EDeviceKeyboard,	// peripheral-keyboard
    EDeviceMice, 		// peripheral-pointer
    EDeviceGPS, 		// major service positioning
    EDeviceDefault,		// device of other type than define above.
    };

// for showing right side icon of a device on a device list    
enum TBTDeviceStatus
    {
    EStatusTrusted = 0x0001,	 		// show authorised icon on both paired and blocked lists
    EStatusBtuiConnected = 0x0002,  	// show connected icon on paired list
    EStatusBlocked = 0x0004, 			// show device on blocked list
    EStatusPaired = 0x0008, 			// show the device in paired devices list
    EStatusConnectable = 0x0010, 		// The device can be connected to.
    									// NOTE: connected devices are also connectable,
    									// despite they cannot be connected before disconnect.
	EStatusPhysicallyConnected = 0x0020  // Physical bluetooth connection. 
    };
    
enum TBTDeviceOp
    {
    EOpPair,	
    EOpUnpair,	
    EOpBlock,	
    EOpUnblock, 
    EOpConnect, 
    EOpDisconnect,
    EOpTrust,
    EOpUntrust,
    EOpChangeName,
    EOpNone,
    EOPInternalUntust // used by CBTDevModel internally
    };
    
/* TBTDevAddr that is not address of any device
*/    
#define KNullAddress TBTDevAddr(TInt64(0))
   
// contain all device info needed for shown on a device list
NONSHARABLE_CLASS( TBTDevice )
    {
public:
	/** Constructor, that initialized this as empty*/
    IMPORT_C TBTDevice();
    /** Copy constructor */
    IMPORT_C TBTDevice(const TBTDevice& aDevice);
    /** Copy  */
    IMPORT_C TBTDevice& operator=(const TBTDevice& aDevice);
    
    TBTDeviceType iType; 				// left side icon
    TBTDeviceClass iDeviceClass; 		// this is needed by connect
    TBTDeviceName iName; 				// a formated friendly/device name in corresponding CBTDevice
    TBTDeviceName iFriendlyName;        // still need to remember if friendly name has been set   
    TUint iStatus; 						// mask of TDeviceStatus
    TBTDeviceOp iOperation; 			// last operation on the device
    TBTDevAddr iAddr; 					// bt device address
    TInt iIndex; 						// new position of the device in the device list. 
                 						// Default value means iAddr should be used instead.
    TBTLinkKeyType iLinkKeyType;
    TNameEntry iNameEntry; // the device's EIR data
    };   
/** Static methods for TBTDeviceComparison. These are meant to be given to TBTDeviceSortOrder and
* to be used for defining sort orders.
*/
class TBTDeviceComparison
	{
public:
	/** 
	* Compare function for alphabetic sorting order.
	* @param aFirstDevice to be compared
	* @param aSecondDevice sencond device to be compared
	* @return 1 if aFirstDevice is before aSecondDevice -1 otherwise. May not return 0,
	* on devices that are not the same.
	*/
		
	IMPORT_C static TInt CompareAphabetically(
		const TBTDevice& aFirstDevice, const TBTDevice& aSecondDevice) ;
		
	/** 
	* Compare function for reverse alphabetic sorting order.
	* @param aFirstDevice to be compared
	* @param aSecondDevice sencond device to be compared
	* @return 1 if aFirstDevice is before aSecondDevice -1 otherwise. May not return 0,
	* on devices that are not the same.
	*/	
	IMPORT_C static TInt ReverseCompareAphabetically(
	 	const TBTDevice& aFirstDevice, const TBTDevice& aSecondDevice) ;
	};
	     
typedef RPointerArray<TBTDevice> RDeviceArray; 
typedef RPointerArray<TBTDeviceName> RBTDevNameArray; 
typedef TLinearOrder<TBTDevice> TBTDeviceSortOrder ;

class CBTDevModelBase;

/**
* Callback interface for informing about the changes in the devices shown by the listening view.
* Also a callback interface that informs the listener when the issued commands have been completed.
*/
class MBTDeviceObserver
    {
public: 
    /** 
    * Informs the observer that ChangeDevice command has been completed.
    *
    * The listener must not change the displaid devices, since RefreshDeviceList is used for that purpose.
    * Also this will not be called, if changes happen from outside such as pairing initiated by 3rd party
    * of blocking from notifier.
    *
    * @param aErr Symbian error code, KErrNone if no error.
    * @param aDevice the device which is changed. This has allways given to the listener by RefreshDeviceList 
    * prior this call, if that device is visible at the listening view. This not passed by refrence,
    * since this device could change, if registry is changed
    * @param aDevNameArray hold conflicting devices' short names if "AlreadyExist" error happened during "connect"
    * @return None.
    */
    virtual void NotifyChangeDeviceComplete(const TInt aErr, const TBTDevice& aDevice, 
        const RBTDevNameArray* aDevNameArray = NULL) = 0;
	
	/** 
	 * This is used to notify the listening graphic component that the deviceList is changed.
	 * After receiving this call the graphic component has to redraw its screen to match this list.
	 * @param aDevices The devices to be displaid. 
	 * @param aSelectedItemIndex The index of the item to be selected in the given devices.
	 */
	virtual void RefreshDeviceList(const RDeviceArray* aDevices,TInt aSelectedItemIndex) =0;
	
	/** 
	* This is used before calling RefreshDeviceList to save the currently selected index,
	* so that the same device could be selected after the refresh, if that device still exists.
	* @param None.
	* @return the selected device. KErrNotFound if the view has no devices.
	*/
	virtual TInt CurrentItemIndex() =0;
    };

/**
* This is a abstract class providing basic common fucntions for derived classes.
* It act as a facade between UI applications and Bluetooth Engine Device Management API.
*/
NONSHARABLE_CLASS( CBTDevModel ) : public CBase
    {
public: // Constructors and destructor
    /**
    * Constructs devmodel.
    * @param aGroup. do you wish to list other or blocked devices.
    * @param MBTDeviceObserver The object that will receive the notifications for command completes,
    * @param the order the devices are sorted in the list. If you do not give this the devices
    * will be sorted in ascending alphabetic order. Devices with same name are sorted according to
    * bluetooth addreses. DevModel will take the ownership of this object.
    */    
    IMPORT_C static CBTDevModel* NewL(const TBTDeviceGroup aGroup,
                                      MBTDeviceObserver* aObserver=NULL, 
                                      TBTDeviceSortOrder* aOrder=NULL);
    /** Destructor */
    virtual ~CBTDevModel();       

public: // New functions

   /**
    * Get device based on addr or index.
    * @param aDevice the TBTDevice holder. The device address is passed in
    * @return KErrNone if ok.
    * KErrArgument, if neither index not address is filled.
    * KErrOverFlow if index is out of range. 
    * KErrNotFound if the given address does not exists, in the internal array.
    */
    IMPORT_C TInt GetDevice(TBTDevice& aDevice);
    
	/** 
	* Checks if any of the devices shown in view have active bluetooth connection.
	* @return ETrue, if one or more devices are connected. EFalse if no devices are connected.
	* This also return EFalse, in case of blocked devices.
	*/
	IMPORT_C TBool IsAnyDeviceConnected();
	
    /** 
    * Creates and allocates TBTDevice representation of this device.
    * The caller is responsibe for the garbage connection of the created device.
    * @param aRegDevice the device to be converted
    * @return the TBTDevice representation of the same device
    */
    IMPORT_C TBTDevice* CreateDeviceL(const CBTDevice* aRegDevice,
            TNameEntry* aNameEntry); 
           
    /**
    * Change status of all devices on the list, e.g. used by unpair all and unblock all commands.
    * @param aOperation unpair/unblock
    * @return None.
    */
    IMPORT_C void ChangeAllDevices(const TBTDeviceOp aOperation);
    
    /**
    * Change device status, add device, e.g. pair/block, or delete device,e.g. unpair/unblcok
    * You must not pair device without properly filling deviceName.
    * @param aDevice the TBTDevice holder. The device address is passed in
    * @return None
    */
    IMPORT_C void ChangeDevice(const TBTDevice& aDevice);
    
    /**
    * Change device status, add device, e.g. pair/block, or delete device,e.g. unpair/unblcok
    * @param aDevice the CBTDevice holder.
    * @return None.
    */
  	IMPORT_C void ChangeDevice(const CBTDevice* aDevice,
  	      TNameEntry* aNameEntry,
  	        TBTDeviceOp aOp) ;
  	
    /**
    * Cancel ongoing operation, such as connecting or pairing.
    * At the moment btui does not support other Other operations than pairing and connecting cannot be canceled.
    *
    * @param aDevice the device, whose command is to be canceled.   
    * @return None.
    */
    IMPORT_C void CancelChange(const TBTDevice& aDevice);

	/** 
	*Checks if there are ChangeDeviceCommands that are not finnished.
	*/
	IMPORT_C TBool DeviceChangeInProgress();
	
	
private:
	/** constructor */
    CBTDevModel();
    
    /** Symbian 2nd phase constuctor that may leave.
     * @param aGroup EGroupPaired and EGroupBlocked are supported.
     * @param aObserver the observer of the device list. This is not mandatory
     * @param aOrder The order the devices are sorted
     */
    void ConstructL(const TBTDeviceGroup aGroup,
                    MBTDeviceObserver* aObserver,
					TBTDeviceSortOrder* aOrder );
private:
	
    CBTDevModelBase* iDevModel; // CBTDevModelPaired or CBTDevModel Blocked 
    TBTDeviceGroup iGroup;
    };    
	
     
/** Adds flags to iStatus of TBTDevice.
 *@param aStatus the status of the device
 *@param aFlags the flags to be set
 */
inline TUint SetStatusFlags(TUint& aStatus,TBTDeviceStatus aFlags)
	{
		aStatus |=aFlags;
		return aStatus;
	}
/** Clear status flags to iStatus of TBTDevice.
 *@param aStatus the status of the device
 *@param aFlags the flags to be set
 */	
inline TUint UnsetStatusFlags(TUint& aStatus,TBTDeviceStatus aFlags)
	{
		aStatus &= ~aFlags;
		return aStatus;
	}

#endif     

