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
#ifndef BTUIDEVICECONTAINER_H
#define BTUIDEVICECONTAINER_H

#include <aknview.h>                // AVKON components
#include <aknlists.h>
#include <aknPopup.h>               // Commands popup window
#include <eikclbd.h>                // For column listbox access
#include <AknIconArray.h>

#include "btdevmodel.h"
#include "btui.hrh"

class CBTEngConnMan;
class CAknView;
class CBTDevModel;
class CBTUIListedDevicesView;

/**
* This class displays lists of bluetooth devices.
*
* This class is used by blocked devices view and paired devices view to show
* lists of blocked or paired devices.
*
*@lib BTUIPlugin.dll
*@since S60 v3.2
*/
NONSHARABLE_CLASS(CBTUiDeviceContainer) : public CCoeControl, 
			public MCoeControlObserver,
			public MEikListBoxObserver  
    {
    public: // Constructors and destructor

        /** Create new instance
	     * @param aView PairedDevicesView or BlockedDevicesView.
	     * @param aStorage The handler of 
	     * @param aContainerMode are we serving as blocked or paired device list		
	     * @param aParent used by SetMopParent
		 */	
		static CBTUiDeviceContainer* NewL(const TRect& aRect, 
			CBTUIListedDevicesView* aView, TBTDeviceGroup aGroup, MObjectProvider* aParent);

		/**
        * Destructor.
        */
        virtual ~CBTUiDeviceContainer();

    public: // Methods ralated devicelist upkeep.

	     /** Refreshes this container to show these devices.
	     * Note: This container will not refresh screen if the listed
	     * devices have not changed.
	     *@param aDeviceArray The items to be displayed
	     *@param aSelectedItem which one of the items is selected after the refresh. 
	     *Must be between 0 and aDeviceArray.Count()-1 or -1 if the list is empty.
	     */
	    void RefreshDeviceListL(const RDeviceArray* aDeviceArray,TInt aSelectedItem);

	    /** Counts how many devices this container displays.     
	     * @return the amount of items in this container
	     */
	    TInt CountItems();

		/** 
		 * Return the currently selected item index. KErrNotFound found if
		 * this container has no items and therefore no selected item.
		 *
		 * @return index of currently selected item, or KErrNotFund if none.
		 */
		TInt CurrentItemIndex() ;   

   	private: 
		/** 2nd phase constructor 
	     * @param aView PairedDevicesView or BlockedDevicesView.
	     * @param aStorage The handler of 
	     * @param aContainerMode are we serving as blocked or paired device list		
	     * @param aParent used by SetMopParent		
		*/
		void ConstructL(const TRect& aRect, CBTUIListedDevicesView* aView, TBTDeviceGroup aGroup);

        /**
        * From CoeControl Responds to size changes. 
        * Sets the size and position of the contents of this control.
        * @param None.
        * @return None.
        */
        void SizeChanged();

        /**
        * From CoeControl Gets the number of controls contained 
        * in a compound control.        
        * @param None.
        * @return The number of component controls contained by this control.
        */
        TInt CountComponentControls() const;

        /**
        * From CoeControl Gets the specified component of a compound control.
        * @param aIndex The index of the control to get.
        * @return Reference to the component control.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;

        /**
        * From CoeControl Draws the control.
        * @param aRect The region of the control to be redrawn.
        * @return None.
        */
        void Draw(const TRect& aRect) const;

        /**
        * From CoeControl Handles key events.
        * @param aKeyEvent The key event that occurred.
        * @param aType The event type.
        * @return Response to the key event.
        */
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);        
        
        /**
        * From MCoeControlObserver Handles an event from an observed control.
        * @param aControl The control that sent the event.
        * @param aEventType The event type.
        * @return None.
        */
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);

		/** returns help context or empty context, if help is not supported 
		 */
		void GetHelpContext(TCoeHelpContext& aContext) const;
        
        /**
		* From CCoeControl
		*
		* Handles a change to the control's resources.
		*/
		void HandleResourceChange( TInt aType );

		/**
        * From CCoeControl 
        *
        * Responds to a change in focus.
        */
		void FocusChanged(TDrawNow aDrawNow);
		
		/**
        * From MEikListBoxObserver Handles list box events.
        * 
        * This is used to handle touch screen double click.
        *
        * @param aListBox The originating list box.
        * @param aEventType The event type.
        * @return None.
        */
        void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
		

		/** Create descriptor representation of given device
		 * @param aDevice The device to be represented
		 * @param aListItem where the representation will be written.
		 */   
		void CreateListItemL(const TBTDevice& aDevice, TBTDeviceName& alistItem);

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
	     * Internal leave function called by HandleResourceChange()
	     * @param aType Type of resource change
	     */
	    void HandleResourceChangeL( TInt aType );
	    
    private: 
        CAknSingleGraphicStyleListBox*  	iDeviceList;      // Other paired BT-devices list
		CBTUIListedDevicesView*				iView;			// The reference to PairedDevicesView or BlockedDevicesView
		TBTDeviceGroup						iGroup; 		// are we operating blocked or paired devices container

	};

#endif


