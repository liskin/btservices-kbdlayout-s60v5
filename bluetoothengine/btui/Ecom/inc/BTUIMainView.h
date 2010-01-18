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
* Description:  Declares the settings view for BTUI application.
*
*/


#ifndef BTUIMAINVIEW_H
#define BTUIMAINVIEW_H

#include "BtuiPluginInterface.h"
#include "BTUIViewsCommonUtils.h"
#include "btui.hrh"

#include <aknview.h>        	// AVKON components
#include <btengconnman.h>       // for disconnect type and connection observer
#include <btengdomaincrkeys.h>
#include <btengprivatecrkeys.h>
#include <btengconstants.h>
#include <btengdevman.h>
#include <btengsettings.h>
#include <bt_sock.h>
#include <gsplugininterface.h>
#include <gstabbedview.h>
#include <eikclb.h>
#include <ConeResLoader.h>

class CBTUIMainContainer;
class CBTDevModel;
class TBTUIViewsCommonUtils;
class CGSTabHelper;
class CBTUIKeyWatcher;
class CAknViewAppUi;
class MGSTabbedView;

/**
* Main view of the application.
*
* This view handles option activation, 
* deactivation, commands and dynamic options menus.
*
*@lib BTUIPlugin.dll
*@since S60 v3.0
*/
class CBTUIMainView : public CBtuiPluginInterface,
                      public MBTEngSettingsObserver,
                      public TBTUIViewsCommonUtils,
                      public MGSTabbedView 
    {
    public: // Constructors and destructor


		/**
        * ECOM implementation instantiation function of 
        * interface "CBtuiPluginInterface", to be used by BTUI Application.
        */	            
        static CBTUIMainView* NewL(MBtuiPluginViewActivationObserver* aObserver= NULL);
		/**
        * ECOM implementation instantiation function of 
        * interface "CGSPluginInterface", to be used by GS application.
        */
        static CBTUIMainView* NewGsPluginL();

		/**
        * Destructor.
        */
        virtual ~CBTUIMainView();

        /**
        * From MGSTabbedView:
        * Creates new icon for tab. Ownership is transferred to client.
        * @since 3.1
        */       
        CGulIcon* CreateTabIconL();

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
                
    protected:
    
        /**
        * C++ default constructor.
        */
        CBTUIMainView (MBtuiPluginViewActivationObserver* aObserver= NULL);

		/**
        * Symbian 2nd phase constructor.
        */
        void ConstructL();
    

    public: // Functions from base classes

        /**
        * From CAknView Returns view id.
        * @param None.
        * @return View id.
        */
        TUid Id() const;
		
		/**
        * Calls possibly leaving HandleCommandL function
        * @param aCommand A command id.        
        * @return None.
        */
        void HandleCommand(TInt aCommand); 		
        
		/**
        * From CAknView Handles user commands.
        * @param aCommand A command id.        
        * @return None.
        */
        void HandleCommandL(TInt aCommand);
        
        /**
		* Used by BTUIAppUI to Redraw BT name
		* field.
		* @param aListItem The changed setting item.
		* @return None.
		*/
		void SettingChangedL( TBTMainListItemIndexes aListItem );

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
        
        /**
        * From MProgressDialogCallback Get's called when a dialog is dismissed.        
        * @param aButtonId Id of the pressed button.
        * @return None.
        */
        void DialogDismissedL( TInt aButtonId );

    public: // New functions
		
        /**
        * Set the power status of Bluetooth.
        *
        * @param aValue The new power status.
        * @return Symbian OS error code.
        */
        TInt SetPowerStatusL( TBool aValue );

        /**
        * Set the visibility mode of Bluetooth.
        *
        * @param aMode The new visibility mode.
        * @return None.
        */
        void SetVisibilityStatusL( TBTVisibilityMode aMode );
        

        /**
        * Returns the local name of BT Device.
        *
        * @param None.
        * @return The local name.
        */
        TDesC* GetBTLocalName();
          
        /**
        * Returns a reference to class CBTEngSettings.
        *
        * @param None.
        * @return A reference to class CBTEngSettings.
        */        
        CBTEngSettings* GetBtSettingsReference();     		   
        
        /**
        * Set the SIM Access Profile status .
        *
        * @param aValue The new SAP status.
        */
        void SetSapStatusL( TBTSapMode aValue );
        
        /**
        * Update its parent's view
        * @param None.
        * @return None.
        */        
        void UpdateParentView();
        
    private:
        /** takes care of middle softkey press.
         */
		void HandleMiddleSoftkey();

         /**
        * Asks new power mode from user via selection buttons.
        *
        * @param None.
        * @return None.
        */
        void AskPowerModeDlgL();

        /**
        * Asks new visibility mode from user via selection buttons.
        *
        * @param None.
        * @return None.
        */
        void AskVisibilityDlgL();

        /**
        * Asks new local BT Device name from user.
        *
        * @param None.
        * @return None.
        */
        void AskNewBtNameDlgL();
       
        /**
        * A callback for launching local BT Device name query.
        *
        * @param aAppUi A reference to self.
        * @return Symbian OS error code.
        */
        static TInt LaunchBTLocalNameQueryL(TAny* aAppUi);

        /**
        * Asks local BT Device name from user at startup.
        *
        * @param None.
        * @return Symbian OS error code.
        */
        TInt AskBTLocalNameQueryL();

        /**
        * Initiates BT local name query after Symbian OS has entered idle state.
        *
        * @param None.
        * @return None.
        */
        void InitiateBTLocalNameQueryL();
		
        /**
        * blaa blaa
        *
        * @param aNum  count of connectionsions.
        * @return None.
        */			
		TInt GetConnectionNumber(TInt& aNum);
        
		/**
        * Asks new SAP mode from user via selection buttons.
        *
        * @param None.
        * @return None.
        */
        void AskSapModeDlgL();
             
        /**
        * Connected sap name
        *
        * @param aName
        * @return TInt.
        */  	
		TInt GetConnectedSapNameL( TDes& aName );
		
		/**
        * Get settings' values including power state, discovery mode and connection state
        * Then set member varient iBtState.
        */  	
		void RefreshSettingsStatusL();
		
		/**
		* Internal leave function used by GetValue().
		*/
		void GetValueL( const TGSPluginValueKeys aKey, TDes& aValue );

		/**
		* Checks from central repository whether the Bluetooth friendly name 
        * has been modified .
		* @param aStatus
		* @return error code
		*/
		
		TInt IsLocalNameModifiedL( TBool& aStatus );
		
	public: // Enumerations
	
	    enum TBTStateSummary
			{
			EBTOff = 0,
			EBTOnShown,
			EBTOnHidden,
			EBTConnectedShown,
			EBTConnectedHidden
			};
											
	private: // from CGSBaseView
	
        /**
        * Handles OK key press.
        * @since 3.1
        */
        void HandleListBoxSelectionL();
        
        /**
        * Creates new container.
        * @since 3.1
        */
        void NewContainerL();
 
 
	public: //Functions from CGSPluginInterface
	
        /**
        * Method for checking plugin's Uid. Uid identifies this GS plugin. Use
        * same Uid as the ECOM plugin implementation Uid.
        *
        * @return PluginUid
        */
        TUid PluginUid() const;

        /**
        * Method for getting caption of this plugin. This should be the 
        * localized name of the settings view to be shown in parent view.
        *
        * @param aCaption pointer to Caption variable
        */
        void GetCaptionL( TDes& aCaption ) const;


        /**
        * Creates a new icon of desired type. 
        * @param aIconType UID Icon type UID of the icon to be created.
        * @return Pointer of the icon. NOTE: Ownership of this icon is
        *         transferred to the caller.
        */
        CGulIcon* CreateIconL( const TUid aIconType );
        
        /**
        * Function for getting plugin's value for a certain key.
        * Override to provide own functionality.
        *
        * @param aKey Key for the value to be retrieved.
        * @parem aValue Value for the given gey in TDes format.
        */
        void GetValue( const TGSPluginValueKeys aKey, TDes& aValue );
             
        /**
        * Returns sap status
        *
        * @param None
        * @return TInt.
        */        
		TInt GetSapStatusL();          
 
 		TVwsViewId GetParentView();
 
         /**
        * From MGSTabbedView
        * @return Reference to owned tabbed views which should be included in 
        *         the sub-view's tab group.
        * @since 3.1
        */
        CArrayPtrFlat<MGSTabbedView>* TabbedViews();
    		       	
    private: // Data
        CBTUIMainContainer*             iContainer;      	    // Main container           
        CBTEngSettings*					iBtEngSettings;         // Settings class        
        CIdle*                          iIdle;              	// Idle time class (for doing operation delayed)
        TBTDeviceName                   iTempNameBuffer;    	// Temporary storage for BT name        
        CEikonEnv*                  	iEikEnv;                // Local eikonenv		
        TInt                        	iResourceFileFlag;      // Flag for eikon env.
		TBool							iConstructAsGsPlugin;	// Flag that indicate who constrct this obj
		TBool							iCoverDisplayEnabled;	// Flag that indicate cover UI feature							                			

        CGSTabHelper* 					iTabHelper;
	    TBool							iParentViewNotSetYet;
        CArrayPtrFlat<MGSTabbedView>* 	iViewArray; 
        TVwsViewId 						iParentView; 
        TBTStateSummary					iBtState;
        CBTUIKeyWatcher*				iKeyWatcher;
        MBtuiPluginViewActivationObserver* iActivationObserver; // view activation observer
    };
    
#endif


