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
* Description:  Declares container control for application.
*
*/

#ifndef BTUIMAINCONTAINER_H
#define BTUIMAINCONTAINER_H

#include <aknview.h>    // AVKON components
#include <aknlists.h>
#include "btui.hrh"
#include "BTUIMainView.h"
#include "btengsettings.h"

/**
* This class handles main view dependent requests from user and 
* contains listbox for main view items.
*
*@lib BTUIPlugin.dll
*@since S60 v3.0
*/
class CBTUIMainContainer : public CCoeControl,
                           public MCoeControlObserver,
                           public MEikListBoxObserver
    {
    public: // Constructors and destructor

		/**
        * Symbian 2nd phase constructor.
        */
        void ConstructL( const TRect& aRect, CBTUIMainView* aMainView );

		/**
        * Destructor.
        */
        virtual ~CBTUIMainContainer();


    public: // New functions
        
        /**
        * Returns the current item index (highlight position) 
        * for main setting list.
        * @param None.
        * @return Current item index.
        */
        TInt CurrentItemIndex();

		/**
        * From MBTUISettingsObserver Handles setting changes.
        * @param aListItem The changed setting item.
        * @return None.
        */
        void SettingChanged( TBTMainListItemIndexes aListItem );
        		        
    private: // Functions from base classes
        
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
        * From MEikListBoxObserver Handles list box events.
        * @param aListBox The originating list box.
        * @param aEventType The event type.
        * @return None.
        */
        void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);

        /**
        * From MCoeControlObserver Handles an event from an observed control.
        * @param aControl The control that sent the event.
        * @param aEventType The event type.
        * @return None.
        */
        void HandleControlEventL(CCoeControl* aControl, TCoeEvent aEventType);        

       
        /**
        * From CoeControl Gets the control's help context.
        * @param aContext The control's help context
        * @return None.
        */
        void GetHelpContext(TCoeHelpContext& aContext) const;
        
        /**
		* From CCoeControl
		*/
		void HandleResourceChange( TInt aType );

		/**
        * From CCoeControl 
        */
		void FocusChanged(TDrawNow /*aDrawNow*/);


    private: // Data

        /**
        *  Friend class for generating format string for main list.
        */
        class TMainListItemArray : public MDesCArray
	        {
            public: // Constructors and destructor

		        /**
		        * C++ default constructor.
		        */                
                TMainListItemArray( CDesCArray* aBtAllItemsArray, CBTUIMainView* aMainView,
                  CBTEngSettings* aBtEngSettings );

            public: // New functions

                /**
                * Returns number of items in array / main list.
                * @param None.
                * @return Number of items.
                */
                TInt MdcaCount() const;      

                /**
                * Returns the current format string for the given item.
                * @param aIndex Index of the item.
                * @return Pointer to the format string.
                */
                TPtrC MdcaPoint(TInt aIndex) const;
            private:
                static void GetSapStatusL( TInt& aStatus );
        
            private: // Data
    
                // Local BT name format string
                __MUTABLE TBuf<KBTUIMaxFormattedNameLength> iItemText;
                
                // No ownership (reference)
                CDesCArray& 				iArray;
                CBTUIMainView*				iMainView;     
        		CBTEngSettings*				iBtEngSettingsRef;
         
            }; 
        
        CAknSettingStyleListBox*    iBtMainListBox;		// The setting items list (scrollable)
        CDesCArray*                 iBtAllItemsArray;   // All format string combinations (ownership)
        TMainListItemArray*         iBtListItemArray;   // Format strings for list items
        TInt                        iCurrentItemIndex;  // Current highlighted main list item		
		CBTUIMainView*				iMainView;			
		CBTEngSettings*				iBtEngSettingsRef;
		
		
		
    private: // Friend classes
        
        friend class TMainListItemArray;

		 

};

#endif
