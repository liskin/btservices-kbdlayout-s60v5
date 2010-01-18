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

#ifndef BTUILISTEDDEVICESVIEW_H
#define BTUILISTEDDEVICESVIEW_H

#include "BtuiPluginInterface.h"
#include "BTUIDeviceContainer.h"
#include "btdevmodel.h"
#include "BTUIViewsCommonUtils.h"

/**
* SuperClass of PairedDevicesView and BlockedDevicesview.
*
* This defines functionality common to PairedDevicesView and BlockedDevicesview.
*
*@lib BTUIPlugin.dll
*@since S60 v3.2
*/
class CBTUIListedDevicesView : public CBtuiPluginInterface,
		                     public MBTDeviceObserver,
		                     public TBTUIViewsCommonUtils		                      
		                      
    {
    public:

	    /** Checks if there a device change operation
	     * ongoing.
	     * @return ETrue if there is. EFalse otherwise
	     */    	
    	TBool DeviceChangeInProgress()
    		{
   			return iModel && iModel->DeviceChangeInProgress();
    		}
    		
    	/** The index of currently selected item.
    	 * This function delegates the question to BTUIDevicesContainer.
    	 * @return the index, or KErrNotFound in the list is empty 
    	 * or if iContainer is not yet initialized;
    	 */	
    	TInt CurrentItemIndex()
    	{
    		if(iContainer)
    			return iContainer->CurrentItemIndex();
    		else
    			return KErrNotFound;	
    	}
    protected:    
        CBTUiDeviceContainer* 	iContainer;
        CBTDevModel* 		  	iModel;
        TBool					iCoverDisplayEnabled;
        MBtuiPluginViewActivationObserver*  iActivationObserver;
    };

#endif


