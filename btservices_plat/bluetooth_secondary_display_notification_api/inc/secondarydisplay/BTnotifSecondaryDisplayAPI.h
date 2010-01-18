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
*              : BT notifier.
*
*/

#ifndef BTNOTIF_SECONDARY_DISPLAY_API_H
#define BTNOTIF_SECONDARY_DISPLAY_API_H

// INCLUDES
#include <e32base.h>

/*
* ==============================================================================
* Input parameter TBTDeviceName is defined in btdevice.h
* "typedef TBuf<KMaxBCBluetoothNameLen> TBTDeviceName"
* ==============================================================================
*/
#include <btdevice.h>


// The category UID for the messages in this header file.
//
const TUid KUidCoverUiCategoryBTnotif = { 0x10005953 };   // UID of the BTnotif Dll


/*
* ==============================================================================
* Dialogs shown by BT notifier. These messages are handled using the
* Secondary Display support in Avkon.
* ==============================================================================
*/
enum TSecondaryDisplayBTnotifDialogs
    {
    /**
     * Command for a specified dialog is unavailable  
     */
    ECmdBTnotifUnavailable = 0,

    /**
    * A command for showing "e.g. Open cover and enter pin code" 
    */
    // do we need this one at all ??? 
    ECmdShowBtOpenCoverNote = 1,   
        
    /**
    * A command for showing the "Receive message via Bluetooth from paired device %U?"
    */
    ECmdShowReceiveMessageFromPairedDeviceDlg,
//#define qtn_bt_receive_message_paired "Receive message via Bluetooth from paired device %U?"

    /**
    * A command for showing the "Receive Bluetooth message from: %U"
    */
    ECmdShowReceiveMessageFromDeviceDlg,
//#define qtn_bt_receive_message "Receive Bluetooth message from: %U"

    /**
    * A command for showing the "Accept connection request from:\n%U"
    */
    ECmdShowAcceptConnRequestDlg,
//#define qtn_bt_accept_conn_request "Accept connection request from:\n%U"

    /**
    * A command for showing "Maximum amount of Bluetooth connections in use. Can't establish a new one."
    */
    ECmdShowBtBusyNote,
//#define qtn_bt_busy "Maximum amount of Bluetooth connections in use. Can't establish a new one."    
    
    /**
    * A command for showing "Bluetooth can't activated in off-line mode".
    */
    ECmdShowBtOfflineDisableNote,
//#define qtn_bt_offline_disabled "Bluetooth can't activated in off-line mode"
  
    /**
    * A command for showing "Bluetooth is currently switched off. Switch on?".
    */
    ECmdShowBtIsOffDlg,    
//#define qtn_bt_is_off "Bluetooth is currently switched off. Switch on?"

    /**
    * A command for showing "Do you want to activate bluetooth in off-line mode?"
    */
    ECmdShowBtActivateInOfflineDlg,    
//#define qtn_bt_activate_in_offline "Do you want to activate bluetooth in off-line mode?"

    /**
    * A command for showing "Connected to %U"
    */
    ECmdShowBtConnectedNote,   
//#define qtn_bt_conf_connected "Connected to %U"

    /**
    * A command for showing "Disconnected from %U"
    */
    ECmdShowBtDisconnectedNote,    
//#define qtn_bt_conf_disconnected "Disconnected from %U"

    /**
    * A command for showing "Bluetooth connection to %U closed"
    */
    ECmdShowBtConnectionClosedNote,    
//#define qtn_bt_conn_closed "Bluetooth connection to %U closed"

    /**
    * A command for showing "Audio routed to BT handsfree"
    */
    ECmdShowBtAudioAccessoryNote,    
//#define qtn_bt_audio_accessory "Audio routed to BT handsfree"

    /**
    * A command for showing "Audio routed to handset"
    */
    ECmdShowBtAudioHandsetNote,    
//#define qtn_bt_audio_handset "Audio routed to handset"

    /**
    * A command for showing "Cannot establish Bluetooth connection"
    */
    ECmdShowBtDeviceNotAvailableNote,    
//#define qtn_bt_device_not_avail "Cannot establish Bluetooth connection"

    /**
    * A command for showing "Phone entering SIM access profile mode"
    */
    ECmdShowBtSapEnteringNote,   
//#define qtn_bt_wait_sap_entering "Phone entering SIM access profile mode"

    /**
    * A command for showing "Phone in SIM access profile mode"
    */
    ECmdShowBtSapEnteringSucceededNote,   
//#define qtn_bt_sap_entering_succeeded "Phone in SIM access profile mode"

    /**
    * A command for showing "Unable to enter SIM access profile mode"
    */
    ECmdShowBtSapEnteringFailedNote,   
//#define qtn_bt_sap_entering_failed "Unable to enter SIM access profile mode"

    /**
    * A command for showing "No sim card in phone" 
    */
    ECmdShowBtSapNoSimNote,   
//#define qtn_bt_sap_no sim ""No sim card in phone"


    /**
    * A command for showing "Phone is not detectable in searches made by other devices" 
    */
    ECmdShowBtHiddenFromOthersNote,   
//#define qtn_bt_hidden_from_others "Phone is not detectable in searches made by other devices" 

    /**
	* From .loc file: #define qtn_bt_is_off_java "The application you are using will not be fully functional while Bluetooth is off. \n\nDo you want to turn Bluetooth on?"
	*/
	ECmdShowBtBtIsOffJavaDlg,

    /**
	* From .loc file: #define qtn_bt_is_hidden_java "Do you want to change your Bluetooth visibility setting value to 'Shown to all'? \n\nThe application you are using will not be fully functional while your Bluetooth visibility is set as 'Hidden'."
	*/
	ECmdShowBtIsNotVisibleDlg, 

    /**
	* From .loc file: #define qtn_ir_not_supported  "Infrared not supported"
	*/
	ECmdShowIrNotSupportedNote,
	
	/**
	* From .loc file: #define qtn_bt_accessory_low  "%U battery level low"
	*/
	ECmdShowBtBatteryLowNote,
		
	/**
	* From .loc file: #define qtn_bt_accessory_critical  "%U battery level critical"
	*/
	ECmdShowBtBatteryCriticalNote
    };

 

#endif      // BTNOTIF_SECONDARY_DISPLAY_API_H            
// End of File
