/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Handle BT power state related to PSM mode
*
*/


#ifndef BTPSMPLUGIN_H
#define BTPSMPLUGIN_H

#include <psmpluginbase.h>
#include <btengsettings.h>


/**
 *  Plug-in of PowerSaveMode (PSM)
 *  implements PSM plug-in and setting configuration API,
 *  handles BT power state when entering and leaving PSM mode.
 *
 *  @since S60 v5.1
 */
class CBTPsmPlugin : public CPsmPluginBase, 
					   public MBTEngSettingsObserver 
    {
	public:

	    /**
	     * Two-phased constructor.
	     *
	     * @param aInitParams Passed by the caller.
	     */
	    static CBTPsmPlugin* NewL(TPsmPluginCTorParams& aInitParams);
	    
	    /**
	     * Destructor.
	     */
	    virtual ~CBTPsmPlugin();

	    /**
	     * From CPsmPluginBase
	     *
	     * Notifies plugin about the power save mode change. There is only one 
	     * active plugin at a time in psmserver, meaning that when this plugin has done 
	     * its mode change, next plugin has process time to complete its mode change. 
	     * Because of this it is good not to make any heavy process during this mode change.
	     *
	     * @since S60 v5.1
	     * @param aMode Mode to change to.
		 */
		void NotifyModeChange( const TInt aMode );
	    
	    /**
	     * From MBTEngSettingsObserver
	     * When BT PowerState is changed in PSM mode, write it into PSM data storage.
	     * 
	     * @since S60 v5.1
	     * @param aState EBTPowerOff if the BT hardware has been turned off, 
     	 *               EBTPowerOn if it has been turned off.
		 */
	    void PowerStateChanged( TBTPowerStateValue aState );
	    
	    /**
	     * From MBTEngSettingsObserver
	     * Power Saving Mode change does not affect Bluetooth visibility.
	     *
	     * @since S60 v5.1
	     * @param aState EBTDiscModeHidden if the BT hardware is in hidden mode, 
     	 *               EBTDiscModeGeneral if it is in visible mode.
     	 */
	    void VisibilityModeChanged( TBTVisibilityMode /*aState*/ );
	    	     
	private: 
	    
	    /**
	     * Default C++ constructor
	     *
	     * @param aInitParams Passed by the caller.
	     */
	    CBTPsmPlugin(TPsmPluginCTorParams& aInitParams);
	    
	    /**
	     * Symbian 2nd-phase constructor
	     *
	     * @since S60 v5.1
	     */
	    void ConstructL();
	    
	    /**
	     * Leave function called by NotifyModeChange().
	     *
	     * @since S60 v5.1
	     */
	    void NotifyModeChangeL();
	     
	    /**
	     * Handle whether and how to update the current BT Power state
	     *
	     * @since S60 v5.1
	     */
	    void HandlePowerStateL();
	    
	    /**
	     * Get setting from PSM configuration file
	     *
	     * @since S60 v5.1
	     * @return TBTPowerStateValue The power state stored in PSM configuration storage.
	     */
	    TBTPowerStateValue GetSettingsFromPsmStorageL();
	     
	    /**
	     * Save configuration into PSM storage. Backup the setting before entering PSM 
	     * or update the setting when the user changes BT PowerState in PSM mode
	     *
	     * @param aState The current BT power state.
	     */
        void SavePsmConfigurationL(TBTPowerStateValue aState);
	    
	private: // data

	    /**
	     * BTEng Settings class
	     * Owned        
	     */
        CBTEngSettings*	iBtEngSettings;  
	    
	    /**
	     * TPsmsrvMode type defination in psmtypes.h 
	     */
        TInt iMode; 
	    
	    /**
	     * Flag if allow the change of power state to be written into PSM storage.
	     * EFalse is the default value, when EBTPowerOn before PSM is activated.
	     * ETrue when EBTPowerOff or active connections before PSM is activated, 
	     *    or when PowerON becomes OFF after PSM is activated.
	     */
        TBool iAllowToConfig;
    };

#endif // BTPSMPLUGIN_H