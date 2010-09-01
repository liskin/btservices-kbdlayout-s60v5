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
* Description:   Maintains a Bluetooth devices data model for blocked devices view.
*
*/

#ifndef BTBLOCKEDMODEL_H
#define BTBLOCKEDMODEL_H

#include "btdevmodelbase.h"
/**
* This is a adds funtionality to show only blocked devices to BTDevModelBase.
*
* It act as a mediator between UI applications and Bluetooth Engine Device Management API.
*
*@lib btdevmodel.dll
*@since S60 V3.2
*/
NONSHARABLE_CLASS(CBTBlockedModel) : public CBTDevModelBase
    {
public: // Constructors and destructor
	/** NewL function 
	 * @param aObserver The observer will receive information about command completes and
	 * changes in the devices.
	 * @param aOrder The order the devices are sorted. Default order is added by CBTDevMan, so
	 * this class can assume this parameter to be used. This class is responsible for deleting this parameter on
	 * its destructor.
	 */
    static CBTBlockedModel* NewL(MBTDeviceObserver* aObserver , 
                                 TBTDeviceSortOrder* aOrder  );
    /** Destructor
     */
    virtual ~CBTBlockedModel();       

    /** Add new device to the list, but only if the device is blocked.
     * This function will filter out those devices that are not blocked.
     *@param aRegdevice the device to be added to the list
     */
    void HandleNewDeviceL(const CBTDevice* aRegDevice,
            TNameEntry* aNameEntry);

private:
	/** Constructor
	 * @param aObserver The observer will receive information about command completes and
	 * changes in the devices.
	 * @param aOrder The order the devices are sorted. Default order is added by CBTDevMan, so
	 * this class can assume this parameter to be used. This class is responsible for deleting this parameter on
	 * its destructor.	
	 */
    CBTBlockedModel(MBTDeviceObserver* aObserver , 
                     TBTDeviceSortOrder* aOrder  );
    /** Symbian 2nd phase constuctor that may leave.
     *@param None.
     * @return None.
     */
    void ConstructL();
    
    
    };
    
#endif 

