/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  The set of messages provided to Secondary Display software by
*              : BTUI application.
*
*/

#ifndef BTUISECONDARYDISPLAYAPI_H
#define BTUISECONDARYDISPLAYAPI_H

#include <e32base.h>
#include <btdevice.h>

const TUid KUidCoverUiCategoryBtui = { 0x10005951 };   // UID of the bluetooth application

/*
* ==============================================================================
* Dialogs shown by BTUI. These messages are handled using the
* Secondary Display support in Avkon.
* ==============================================================================
*/
enum TSecondaryDisplayBtuiDialogs
    {

    /**
    * A command for showing "Active Bluetooth connection. Switch off anyway?"
    */
    ECmdShowBtSwitchOffAnywayQuery = 1,   
//#define qtn_bt_switch_off_anyway "Active Bluetooth connection. Switch off anyway?"

    /**
    * A command for showing "Do you want to activate bluetooth in off-line mode?"
    */
    ECmdShowBtActivateInOfflineQuery,   
//#define qtn_bt_activate_in_offline "Do you want to activate bluetooth in off-line mode?"

    /**
    * A command for showing "Bluetooth can't activated in off-line mode"
    */
    ECmdShowBtOfflineDisabledNote,   
//#define qtn_bt_offline_disabled "Bluetooth can't activated in off-line mode"

    /**
    * A command for showing "Phone is detectable by all other devices."
    */
    ECmdShowBtPhoneVisibToAllNote,   
//#define qtn_bt_phone_visib_to_oth "Phone is detectable by all other devices."

    /**
    * A command for showing "Unable to perform Bluetooth operation"
    */
    ECmdShowBtGeneralErrorNote,   
//#define qtn_bt_general_error "Unable to perform Bluetooth operation"

    /**
    * A command for showing "Pairing with %U complete" 
    */
    ECmdShowBtSuccesfullyPairedNote,   
//#define qtn_bt_succesfully_paired "Pairing with %U complete" 

    /**
    * A command for showing "Authorise this device to make connections automatically?"
    */
    ECmdShowBtAuthorizationQuery,   
//#define qtn_bt_authorisation_query "Authorise this device to make connections automatically?"

    /**
    * A command for showing "Connect to %U?"
    */
    ECmdShowBtConnectQuery,   
//#define qtn_bt_query_connect "Connect to %U?"

    /**
    * A command for showing "Failed to pair with %U"
    */
    ECmdShowBtPairingFailedNote,   
//#define qtn_bt_pairing_failed "Failed to pair with %U"

    /**
    * A command for showing "Delete pairing with: %U"
    */
    ECmdShowBtWarnEraseNote,   
//#define qtn_bt_warn_erase "Delete pairing with: %U"

    /**
    * A command for showing "Delete all pairings? Some devices may be disconnected." 
    */
    ECmdShowBtWarnEraseAllConnectedQuery,   
//#define qtn_bt_warn_erase_all_connected "Delete all pairings? Some devices may be disconnected." 

    /**
    * A command for showing "Delete all pairings?" 
    */
    ECmdShowBtWarnEraseAllQuery,   
//#define qtn_bt_warn_erase_all "Delete all pairings?"

    /**
    * A command for showing "Connections will take place automatically without confirmation. Continue?"
    */
    ECmdShowBtAuthConfirmQuery,   
//#define qtn_bt_conf_aut "Connections will take place automatically without confirmation. Continue?"

    /**
    * A command for showing "Device set as unauthorised. Connection requests will require confirmation." 
    */
    ECmdShowBtDevSetAsUntrustNote,   
//#define qtn_bt_dev_set_as_untrust "Device set as unauthorised. Connection requests will require confirmation." 

    /**
    * A command for showing "Disconnect wired accessory" 
    */
    ECmdShowBtDisconnectWiredtNote,   
//#define qtn_bt_disconnect_wired "Disconnect wired accessory"

    /**
    * A command for showing "Not possible during a call" 
    */
    ECmdShowBtImpossibleIncallNote,   
//#define qtn_bt_not_incall "Not possible during a call"

    /**
    * A command for showing "Disconnect %U first" 
    */
    ECmdShowBtDisconnectFirstNote,   
//#define qtn_bt_disconnect_first "Disconnect %U first"


    /**
    * A command for showing "Disconnect from: %U" 
    */
    ECmdShowDisconnectQuery,   
//#define qtn_bt_disconn_from "Disconnect from: %U"

    /**
    * A command for showing "Enabling SIM Access Profile connection Bluetooth must be on"
    */
    ECmdShowBtSapEnableNote,   
//#define qtn_bt_sap_enable_info "Enabling SIM Access Profile connection Bluetooth must be on"


    /**
    * A command for showing "Waiting for response from %U"
    */
    ECmdShowBtWaitingNote,   
//#define qtn_bt_waiting "Waiting for response from %U"

    /**
    * A command for showing "Connecting to %U"
    */
    ECmdShowBtWaitConnectingToNote,   
//#define qtn_bt_wait_connecting_to "Connecting to %U"

    /**
    * A command for showing "Phone is not detectable in searches made by other devices" 
    */
    ECmdBtuiShowBtHiddenFromOthersNote,   
//#define qtn_bt_hidden_from_others "Phone is not detectable in searches made by other devices" 

    /**
    * A command for showing "Maximum amount of Bluetooth connections in use. Can't establish a new one."
    */
    ECmdBtuiShowBtBusyNote,
//#define qtn_bt_busy "Maximum amount of Bluetooth connections in use. Can't establish a new one."    

    /**
    * A command for showing "Bluetooth can't activated in off-line mode".
    */
    ECmdBtuiShowBtOfflineDisableNote,
//#define qtn_bt_offline_disabled "Bluetooth can't activated in off-line mode"

    /**
    * A command for showing "Bluetooth is currently switched off. Switch on?".
    */
    ECmdBtuiShowBtIsOffDlg,    
//#define qtn_bt_is_off "Bluetooth is currently switched off. Switch on?"

    /**
    * A command for showing "Do you want to activate bluetooth in off-line mode?"
    */
    ECmdBtuiShowBtActivateInOfflineDlg,    
//#define qtn_bt_activate_in_offline "Do you want to activate bluetooth in off-line mode?"
	
	/**
	* A command for showing "Cannot establish Bluetooth connection"
	*/
	ECmdBtuiShowBtDeviceNotAvail,
//#define qtn_bt_device_not_avail "Cannot establish Bluetooth connection"

    /*
	* A command for showing "Unsupported device: %U"
	*/
	ECmdBtuiShowBtDeviceNotSupported,
//#define qtn_bt_device_not_supported "Unsupported device: %U"

	/**
	* A command for showing "Audio routed to BT handsfree"
	*/
	ECmdBtuiShowBtAudioAccessory,
//#define qtn_bt_audio_accessory "Audio routed to BT handsfree"

	/**
	* A command for showing "Connected to %U"
	*/
	ECmdBtuiShowBtConfConnected
//#define qtn_bt_conf_connected "Connected to %U"
    };

 
#endif      // BTUI_SECONDARY_DISPLAY_API_H
