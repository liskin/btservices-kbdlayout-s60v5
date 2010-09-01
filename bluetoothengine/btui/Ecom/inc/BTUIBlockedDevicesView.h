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

#ifndef BTUIBLOCKEDDEVICESVIEW_H
#define BTUIBLOCKEDDEVICESVIEW_H

#include <aknview.h>
#include <eikmenup.h>       // Menu pane definitions
#include <btengdevman.h>
#include "BtuiPluginInterface.h"
#include "BTUIListedDevicesView.h"
#include <gstabbedview.h>


class CGSTabHelper;
class CBTUIMainView;  //base view
class CBTUiDeviceContainer;        
class CBTDevModel;

/**
* Blocked devicew view of the pluetooth application.
*
* This view handles lists of blocked devices and the abolity to unblock them.
*
*@lib BTUIPlugin.dll
*@since S60 v3.2
*/
class CBTUIBlockedDevicesView : public CBTUIListedDevicesView,
		                      public MGSTabbedView 

    {
    public: // New functions
		
		/**
        * Symbian 2nd phase constructor.
        */       
        void ConstructL( );
        
		/**
        * Symbian 2nd phase constructor when serve as GS plugin.
        * @param aBaseView, GS base view pointer.
        * @param aTabViewArray, the array which contains all the tabed views' pointer.
        * @return None. 
        */         
        void ConstructL( CBTUIMainView* aBaseView, CArrayPtrFlat<MGSTabbedView>* aTabViewArray );
        
        /*
        * ECOM implementation instantiation factory function of 
        * interface "CBtuiPluginInterface", to be used by BTUI Application.
        */        
        static CBTUIBlockedDevicesView* NewL(MBtuiPluginViewActivationObserver* aObserver = NULL);
 
 		/**
        * Symbian 2 phase constructor used by base view when serve as GS plugin.
       	* @param aBaseView, GS base view pointer.
        * @param aTabViewArray, the array which contains all the tabed views' pointer.        
        * @return None.
        */ 		
        static CBTUIBlockedDevicesView* NewLC(CBTUIMainView* aBaseView, 
        					CArrayPtrFlat<MGSTabbedView>* aTabViewArray);  
        					         
		/**
        * Destructor.
        */
        virtual ~CBTUIBlockedDevicesView();       
        
            
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
        
		/** Dim/unDim middleSoftkey.
		 * This function is called by CBTUiDeviceContainer
		 * durings screen refreshes based on if there are devices or not.
         *
		 * @param aDimmed. If this is true the middleSoftKey is invisible. 
		 */
		void DimMskL(TBool aDimmed);
		
        /**
        * From CAknView Dynamically initialises options menu.
        * @param aResourceId Id identifying the menu pane to initialise.
        * @param aMenuPane The in-memory representation of the menu pane.
        * @return None.
        */
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);    
            
    public:
    	/**
        * Informs the observer that adevice has been changes or a new device added.
        * @param aErr Symbian error code
        * @param aDevice the device which is changed.  If iAddr == 0, it's for notifiying "GetAllDevices()", 
        * @param aEnable True: pair/authorize/block/connect; False: unpair/unauthorize/unblock/disconnect
        * @param aDevNameArray hold conflicting devices' short names if "AlreadyExist" error happened during "connect"
        * @return None.
        */    
  		void NotifyChangeDeviceComplete(const TInt aErr, const TBTDevice& aDevice, 
          											const RBTDevNameArray* aDevNameArray = NULL);        
          
		/** Updates the shown device list. This module will send this list forward
		 * to container that takes care of the actual display of those devices
		 *
		 *@param aDevices all the devices to be shown
		 *@param aSelectedItemIndex currently selected item after refresh.
		 */
		void RefreshDeviceList(const RDeviceArray* aDevices,TInt aSelectedItemIndex);

	protected:		
        /**
        * C++ default constructor.
        */
        CBTUIBlockedDevicesView (MBtuiPluginViewActivationObserver* aObserver= NULL);	

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
       
    private: // Data
        
        CGSTabHelper* 					iTabHelper;
	    CBTUIMainView*					iBaseView; 
		TBool							iConstructAsGsPlugin;	// Flag that indicate this obj is served as GS plugin not BTUI app's plugin.    
		MBtuiPluginViewActivationObserver* iActivationObserver; // view activation observer
    };

#endif

