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
* Description:  Abstract base class for concete models.
*
*/


#ifndef BTDEVMODELBASE_H
#define BTDEVMODELBASE_H

#include <btengdevman.h>       // for MBTEngDevManObserver

#include "btdevmodel.h"
#include "btregistryobserver.h" // for MBTRegistryObserver

/**
* This is a abstract class providing basic common fucntions for derived classes.
* It act as a mediator between UI applications and Bluetooth Engine Device Management API.
*
*@lib btdevmodel.dll
*@since S60 V3.2
*/
NONSHARABLE_CLASS( CBTDevModelBase ): public CBase, public MBTEngDevManObserver, public MBTRegistryObserver
    {
public: // Constructors and destructor
    /** destructor 
    *@param none.
    *@return none.
    */
    virtual ~CBTDevModelBase();       
public: 

	/** Checks if any of the devices shown in view have active bluetooth connection.
	 * This baseclass does not support connections, so the answer is allways EFalse.
	 *
	 * NOTE: TBTUIViewsCommonUtils::IsAnyDeviceConnectedL will report also those devices
	 * that are not part of particular view, such as paired devices.
	 * CBTPairedModel::IsAnyDeviceConnected reports only connection from paired devices.
	 *
	 *@return ETrue, if one or more devices are connected. EFalse if no devices are connected.
	 */
	TBool virtual IsAnyDeviceConnected();
    /**
    * Get device based on addr or index.
    * @param aDevice the TBTDevice holder. You have to set either the deviceAddress or the
    * or the device index. If you specify both the device address and the index then the address is used,
    * this includes those cases when there is no device corresponding the address.
    * @return KErrNone if ok. 
    * KErrArgument, if neither index not address is filled.
    * KErrOverFlow if index is out of range. which means it is negative or the internal array does not have that
    * many items.
    * KErrNotFound if the given address does not exists, in the internal array.
    */
    TInt virtual GetDevice(TBTDevice& aDevice);
       
    /**
    * Change status of all devices on the list. This is used by Delete All operations in paired devices view
    * (EOpPair) and Blocked devices view(EOpUnblock)
    *
    * This creates multiple operations in the intergal queue that are all executed on by one.
    *
    * @param aOperation unpair/unblock
    * @return None.
    */
    void ChangeAllDevices(const TBTDeviceOp aOperation);

    /**
    * Change device status, add device, e.g. block, unpair,unblock etc.
    *
    * The operation is put to queue and executed when the prior operations
    * of the queue are finnished. Immeadiately if the queue is empty.
    *
    * @param aDevice. The device and the type of the change. If the address is empty, then the 
    * device index means what device is to be used.
    * @return none
    */
    virtual void ChangeDeviceL(const TBTDevice& aDevice);
    
    /**
    * Change device status, add device, e.g. block, unpair,unblock etc.
    * This will check for leaves and make a callback, if leave occurs.
    *
    * The operation is put to queue and executed when the prior operations
    * of the queue are finnished. Immeadiately if the queue is empty.
    *
    * @param aDevice. The device and the type of the change. If the address is empty, then the 
    * device index means what device is to be used.
    * @return none
    */
    
    virtual void ChangeDevice(const TBTDevice& aDevice);
    
    /**
    * Cancel connecting, pairing, modifying device in BTRegistry
    *
    * Cancels ongoing operation or operation from queue, if it has not been started yet.
    * Cancels it, if it is being executed. Used by Canceling Connecting and pairing operations.
    *
    * @param aDevice. The device and the type of the change that will be canceled. 
    * If the address is empty, then the 
    * device index means what device is to be used.
    * @return None.
    */
    virtual void CancelChange(const TBTDevice& aDevice);
    
    /** Creates and allocates TBTDevice representation of this device
     *@param aRegDevice the device to be converted,
     *@param aNameEntry contains the EIR data of the device if its data is valid
     *@return the TBTDevice representation of the same device. (Note the caller is responsible for deleting
     * the returned item.)
     */
    virtual TBTDevice* CreateDeviceL(const CBTDevice* aRegDevice, 
            TNameEntry* aNameEntry);    
    
	/** Checks if there are any ChangeDeviceCommands that are not finnished.
	 *@return ETrue if the are one or more commands being executed or are in the execution queue.
	 */
	virtual TBool DeviceChangeInProgress();
	
	/** This is used to make callback to iObserver, when a leave has been detected.
	*It will also clean up the partial operation.
	*
	*@param aErr the leave error code
	*@param aDevice The device and operation that cause the leave.
	* If this is inserted in the queue, it is deleted from there.
	* If the leave occurred before inserting it to execution queue 
	* (not common), then it will not deleted from there.
	*@return none.
	*/
	virtual void HandleLeave(TInt aErr,const TBTDevice* aDevice );

protected:
    /**
    * Change device status, add device, e.g. block, unpair,unblock etc.
    *
    * This is the operation that is used to to execute the operation queued by ChangeDevice.
    *
    * @param aDevice. The device and the type of the change. If the address is empty, then the 
    * device index means what device is to be used.
    * @return KerrNone or system wide error code.
    */
    virtual TInt DoChangeDeviceL(const TBTDevice& aDevice);    
    
    /**
    * Cancel ongoing connecting, pairing, modifying device in BTRegistry
    *
    * This is the operation that is used to cancel ongoing operation.
    *
    * @param aDevice. The device and the type of the change that will be canceled. 
    * If the address is empty, then the 
    * device index means what device is to be used.
    * @return None.
    */   
    virtual void DoCancelChangeL(const TBTDevice& aDevice);

	/** This function recounts the indexes of iDeviceArray.
	 *@return none.
	 */
	void RenumberDeviceArray();

	/** Constructor 
	 * @param aObserver The observer will receive information about command completes and
	 * changes in the devices.
	 * @param aOrder The order the devices are sorted. Default order is added by CBTDevMan, so
	 * this class can assume this parameter to be used. This class is responsible for deleting this parameter on
	 * its destructor.
	 */
    CBTDevModelBase(MBTDeviceObserver* aObserver,
                    TBTDeviceSortOrder* aOrder );

	/** Get the index for a device specified by bluetooth device address.
	 *
	 *@param aAddr the address of the searched device or KNullAddres.
	 *@param aIndexIfNullAddress, if this parameter is defined this function 
	 *returns this index instead of KErrNotFound, it the device has nullAddress.
	 *@return index of the address. If the given address is KNullAddress then returns aIndexIfNullAddress or KErrNotFound if one is not specified by called.
	 *This will allways return KErrNone if the given address is not a KNullAddress  and is not found.	 
	 */
    TInt GetIndexByAddress(const TBTDevAddr aAddr,TInt aIndexIfNullAddress=KErrNotFound);
	    
    /** Adds the TBTDevice representation of this device to iDeviceArray, if
     * this device is of that type (paired/blocked) that this model supports.
     * If the device is not that type, it will not be added.
     *
     * NOTE: This function is not guaranteed againt duplicates, so multiple
     * additions of the same device will lead to multiple copies of it.
     *
     * Subclasses can also implement their version of this function to add information
     * to the device objects, such as connection information.
     *
     * @param aRegDevice the device to be added (if not filtered).
     * @param aNameEntry contains the EIR data of the device if its data is valid
     * @aOperation the operation that this device will contain when added.
     */
    void AddDeviceL(const CBTDevice* aRegDevice, 
            TNameEntry* aNameEntry,
            const TBTDeviceOp aOperation=EOpNone);
        
    /** Replaces iDeviceArray contents with the given devices 
     *@param aDeviceArray the devices that will be added to internal structures.
     */
    virtual void CreateDevicesL(const CBTDeviceArray* aDeviceArray);
    
    /** Adds the given device to internal structures of this class
     * using AddDeviceL, if the device is such that it should not be listed by this
     * class, then it is not added.
     *
     * Implementing is used to filter out devices that are not handled by particular view.
     * 
     * @param aNameEntry contains the EIR data of the device if its data is valid
     */
    virtual void HandleNewDeviceL(const CBTDevice* aRegDevice,
            TNameEntry* aNameEntry) = 0;
	
	/** Sends refresh of list of shown devices to listener.
	 *
	 * @param aErr If this is not KErrNone, then this function
	 * will send refresh to the listener. Allways send, if not specified. 
	 * If this funtion is not KEerrNone, then the funtion does nothing.	
	 * @param aSelectedItem. If this is specified, then select this item.
	 * if not specified uses the same item that the listener used to have active.
	 */
	virtual void SendRefreshIfNoError(TInt aErr=KErrNone,TInt aSelectedItem = KErrNotSupported );
	 
protected:

    /** This callback is used to to notify this call from completed changed from iDevMan.
    * Inherited from MBTEngDevManObserver.
    *
    * This funtion will trap any leave if they may occur and report them as errors in the
    * corresponfing device change operarion.
    */
    void HandleDevManComplete(TInt aErr); 
        
    /** Inherited from BTEng's MBTEngDevManObserver, and implemented as empty,    
    * because we do ask devicelists directly from iDevman, but trough iRegistryObserver->Refresh()
    */
    void HandleGetDevicesComplete(TInt aErr, CBTDeviceArray* aDeviceArray); 

    /** This callback is used by iRegistryObserver to notify device changes, that
     * originate both from changes of the BTRegistry as well as refreshes requested
     * by this class. 
     * @aDeviceArray the devices that the registry observer gets.
     */
    void RegistryChangedL(const CBTDeviceArray* aDeviceArray);

	/** Check if there is allready one or more devices with this name.
	 *@return ETrue if there is.
	 */
    TBool IsNameExisting(const TDesC& aName);
    
    /** Executes the next one in the queue, if any.
    * Leaves are trapped by this class and send as failed commands to iObserver
    * using HandleLeave.
	*@param none.
	*@return none.
    */
	void HandleQueue();    
		
protected: // Data
    MBTDeviceObserver* iObserver;	// the listener of the completed commands and registry changes.
    CBTEngDevMan* iDevMan;			// This is used to change the devices.
    TBTDeviceSortOrder* iSortOrder;	// The order the devices are sorted to.    				
    RDeviceArray iDeviceArray;		// The list of current devices.
    CBTRegistryObserver* iRegistryObserver;	// This takes care of sending the devicelist to us each time 
    										// something may have been changes.
    TBTRegistrySearch iSearchPattern; // the pattern iRegistrySearch uses in retrieving devicelist.
    								  // NOTE: HandleDeviceL is used to filter the results.
    TBool iIsChangeAll;    
    TBTDevice* iDevice;				 // the currently changed device, if any
    RDeviceArray iQueue;			// The queue of commands to be executed
    };
  
#endif // BT_DEV_MODEL_BASE_H

