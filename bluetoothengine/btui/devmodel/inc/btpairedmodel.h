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
* Description:  The model that is used to list and handle all paired devices.
*
*/


#ifndef BTPAIREDMODEL_H
#define BTPAIREDMODEL_H

#include "btdevmodelbase.h"
#include <btengconnman.h>
#include <bt_sock.h> 				// for TBTDevAddr
#include <btserversdkcrkeys.h> 		//for TBTPowerStateValue
#include <btengconstants.h>
#include <btengsettings.h>

enum TPairedModelState
	{
	EIdleState=0,
	EDeleteDeviceState,
	EAddDeviceState,
	EPairDeviceState,
	EDisconnectBeforeOperationState,	
	};
class CBTEngSettings;

/**
* This is a adds pairing and connecting functionality to BTDevModelBase.
*
* It act as a mediator between UI applications and Bluetooth Engine Device Management API.
*
*@lib btdevmodel.dll
*@since S60 V3.2
*/	
NONSHARABLE_CLASS(CBTPairedModel) :
		public CBTDevModelBase, public MBTEngConnObserver, 
		public MBluetoothPhysicalLinksNotifier,
		public MBTEngSettingsObserver
    {
    
public: // Constructors and destructor

	/**
    * C++ default constructor.
    */
	CBTPairedModel(MBTDeviceObserver* aObserver, 
                  TBTDeviceSortOrder* aOrder  );
                  
    /** Symbian 2nd phase constuctor that may leave.
    *@param None.
    *@return None.
    */
    void ConstructL();

	/** 
	* NewL function 
	* @param aObserver The observer will receive information about command completes and
	* changes in the devices.
	* @param aOrder The order the devices are sorted. Default order is added by CBTDevMan, so
	* this class can assume this parameter to be used. This class is responsible for deleting this parameter on
	* its destructor.
	*/
    static CBTPairedModel* NewL(MBTDeviceObserver* aObserver,
                                TBTDeviceSortOrder* aOrder  );
    
    /** 
    * Destructor
    * @param None.
    */
    virtual ~CBTPairedModel();       

	/** 
	* Checks if any of the devices shown in paired devices view have active bluetooth connection.
    *
	* NOTE: TBTUIViewsCommonUtils::IsAnyDeviceConnectedL will also check
	* those devices that are not paired.
	*
	*@return ETrue, if one or more devices are connected. EFalse if no devices are connected.
	*/
	TBool virtual IsAnyDeviceConnected();
	
	/** 
	* Start new device pairing. This is handled by Queue, like
	* all other device changes. 
	*
	* This is used by CBTDevice::ChangeDevice(CBTDevice* ,EOpPair)
	*
	* @param aPairingDevice the device to be paired.
	* @return KErrInUse, if another operation has be queued and not finnished. KErrNone otherwise.
	*/
	TInt PairDeviceL(const CBTDevice* aPairingDevice,
	        TNameEntry* aNameEntry);
	
protected:

    /**
	* Change device status, add device, e.g. block, unpair,unblock etc.
    *
    * This is the operation that is used to to execute the operation queued by ChangeDevice.
    *
    * This class will add connection releted operations to CBTDevModelBase::DoChangeDevice.
    *
    * @param aDevice. The device and the type of the change. If the address is empty, then the 
    * device index means what device is to be used.
    * @return KerrNone or system wide error code.
    */
    TInt DoChangeDeviceL(const TBTDevice& aDevice);

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
    void DoCancelChangeL(const TBTDevice& aDevice);

    /** 
    * These two methods set the EStatusPhysicallyConnected attribute to 
    * the devices that have active connection.
    */
    void FlagPhysicallyConnectedDevices(RDeviceArray &aDevices);
    void FlagPhysicallyConnectedDevices(TBTDevice& aDevice, RBTDevAddrArray& aBTDeviceArray);

	
private:
			
	/** 
	* Start the actual pairing of iPairingDevice
	* This is called by DoChangeDevice, if the operation is pairing.
	*@param none.
	*@return none.
	*/
	void DoPairDeviceL();

	/** 
	* BTEngConnMan will call this when the connecting is finnished.
	* This is called as callback not only for connections initiated by this class,
	* but also for all BT Engines supported porofile connections, including those
	* initiated by remote devices.
	* @param aAddr The address of the device whose connection attempt has been finnished.
	* @param aErr KErrNone if connection is succesfull, Otherwise system wide error code.
	* @param aConflicts The conflicting devices in case of unsuccesfull connection attempt if any.
	* @return none.
	*/    
    void ConnectComplete(TBTDevAddr& aAddr, TInt aErr, RBTDevAddrArray* aConflicts = NULL);

	/** 
	* BTEngConnMan will call this when the disconnecting is finnished.
	* This is called as callback not only for disconnections initiated by this class,
	* but also for disconnection of BT Engines supported porofile connections, including those
	* initiated by remote devices.
	* @param aAddr The address of the device whose connection attempt has been finnished.
	* @param aErr KErrNone if disconnection is succesfull, Otherwise system wide error code.
	* @return none.
	*/    
	void DisconnectComplete(TBTDevAddr& aAddr, TInt aErr);
    
	/**
	*  from BTEng's MBTEngConnObserver
	*/
    void PairingComplete(TBTDevAddr& aAddr, TInt aErr);
    
    /** 
    * The leaving version of PairingComplete. Internally called by PairingComplete
    */
    void PairingCompleteL(TBTDevAddr& aAddr, TInt aErr);

    /** 
    * Creates and allocates TBTDevice representation of this device
    *
    * This class will add the connection status information to 
    * to CBTDevModelBase::CreateDevice.
    *
    * @param aRegDevice the device to be converted
    * @return the TBTDevice representation of the same device. (Note the caller is responsible for deleting
    * the returned item.)
    */    
    TBTDevice* CreateDeviceL(const CBTDevice* aRegDevice, TNameEntry* aNameEntry);
    
    /** 
    * Adds the given device to internal structures of this class
    * using AddDeviceL, if it is paired and not blocked.
    *
    * @param aRegDevice
    */    
    void HandleNewDeviceL(const CBTDevice* aRegDevice,
            TNameEntry* aNameEntry);

    /** 
    * This callback is used to to notify this call from completed changed from iDevMan.
    * Inherited from MBTEngDevManObserver.    
    *
    * This funtion will trap any leave if they may occur and report them as errors in the
    * corresponfing device change operarion.
    *
    * @param aErr KErrNone or system wide error code.
    */
    void HandleDevManComplete(TInt aErr);
    
public: 
    
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
    
    
public:    //from MBluetoothPhysicalLinksNotifier

	/** 
	* Inherited from MBluetoothPhysicalLinksNotifier and implemented empty
	*/
	void HandleCreateConnectionCompleteL(TInt ) {};
	
	/** 
	* Disconnect from non BTEng device by iLinks is complete.
	*@param aErr KErrNone or system wide errorcode.
	*/	
	void HandleDisconnectCompleteL(TInt aErr) ;
	
	/** Inherited from MBluetoothPhysicalLinksNotifier and not used.
	*/
	void HandleDisconnectAllCompleteL(TInt aErr) {HandleDisconnectCompleteL(aErr); }

    /**
    * Get device based on addr or index.
    * @param aDevice the TBTDevice holder. The device address is passed in
    * @return KErrNone if ok.
    * KErrArgument, if neither index not address is filled.
    * KErrOverFlow if index is out of range. 
    * KErrNotFound if the given address does not exists, in the internal array.
    */
    TInt virtual GetDevice(TBTDevice& aDevice);

	/** 
	* This is used to make callback to iObserver, when a leave has been detected.
	*
	*@param aErr the leave error code
	*@param aDevice The device and operation that cause the leave.
	*/
	virtual void HandleLeave(TInt aErr,const TBTDevice* aDevice );               
	
private:

    CBTEngConnMan* iConnMan;
    CBTDevice* iPairingDevice;	

    TBTDevice* iDisconnectDevice;
    TPairedModelState iState;	// The device is deleted and written back to bt registry before pairing
    							// this is used to control those operations
	//used for disconnecting connections not managed by BTEngine    							
	CBluetoothPhysicalLinks *iLinks;   							
	RSocketServ iSocketServ;
	CBTEngSettings *iBtEngSettings; // used for turning on BT, if not enabled
    };
    
#endif
