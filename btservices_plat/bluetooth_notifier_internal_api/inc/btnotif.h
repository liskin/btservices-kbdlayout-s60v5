/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declares Bluetooth notifiers API (partly).
*
*/



#ifndef BTNOTIF_H
#define BTNOTIF_H

#include <eiknotapi.h>
#include <btnotifierapi.h>		//Definition of "KPowerModeSettingNotifierUid"
#include <btextnotifiers.h>
#include <bttypes.h>			//BT protocol wide types
#include <btdevice.h>
#include <bt_subscribe.h>

const TUid KBTObexPasskeyQueryNotifierUid = {0x100059A0};
const TUid KBTGenericInfoNotifierUid = {0x20002778};
const TUid KBTGenericQueryNotifierUid = {0x20002779}; 
const TUid KBTPairedDeviceSettingNotifierUid = {0x100059E3}; 
const TUid KBTEnterpriseItSecurityInfoNotifierUid = {0x20028779};

//Type definition for ObexPin notifier
const TInt KBTObexPasskeyMaxLength = 16;

//Copied from BTUI.h, btui and btnotif under same aContext.iMajor.
const TUid KUidBTUI = { 0x10005951 }; 

typedef TBuf8<KBTObexPasskeyMaxLength> TBTObexPasskey;
typedef TPckgBuf<TBTObexPasskey>	TObexPasskeyBuf;

enum TBTGenericQueryNoteType
	{
	EBTReceiveMessageQuery=300,
	EBTReceiveMessagePairedQuery,
	EBTIsOffQuery,
	EBTActivateOffLineQuery,
	EBTNoDevicesFoundQuery,
	EBTAcceptRequestQuery,
	EBTBlockConnectionQuery,
	EBTBlockPairedConnectionQuery,
	
	EBTIsNotShownQuery,
	EBTIsOffJavaQuery,
	EBTNameQuery, 
	EBTSwitchOffAnyway
	};

enum TBTGenericInfoNoteType
	{
	EBTConnected=0 /*R_BT_CONF_CONNECTED_PROMPT*/,
	EBTDisconnected /*R_BT_CONF_DISCONNECTED_PROMPT*/,	
	EBTAudioAccessory /*R_BT_AUDIO_ACCESSORY_PROMPT*/,
	EBTAudioHandset /*R_BT_AUDIO_HANDSET_PROMPT*/,		
	EBTClosed /*R_BT_CONN_CLOSED_PROMPT*/,
	EBTDeviceNotAvailable /*R_BT_DEVICE_NOT_AVAIL*/,
	EBTOfflineDisabled /*R_BT_OFFLINE_DISABLED*/,
	
	EBTVisibilityTimeout /*R_BT_HIDDEN_FROM_OTHERS_NOTE*/,
	
	EBTEnterSap /*R_BT_ENTER_SAP_NOTE*/,
	EBTSapOk /*R_BT_ENTER_SAP_SUCCEED_NOTE*/,
	EBTSapFailed /*R_BT_ENTER_SAP_FAILED_NOTE*/,
    EBTSapNoSim  /*R_BT_SAP_NO_SIM_NOTE*/,	
	
	EBTDeviceBusy /*R_BT_BUSY_NOTE */,
	
	EIRNotSupported /*R_IR_NOT_SUPPORTED */,
	
	ECmdShowBtBatteryLow /*R_BT_ACCESSORY_LOW */,
	ECmdShowBtBatteryCritical /*R_BT_ACCESSORY_CRITICAL */,
	EBTStayPowerOn /*R_BT_PERMANENT_ON*/, 
	EBTSwitchedOn /*R_BT_SWITCHED_ON*/,
	EBTSwitchedOff /*R_BT_SWITCHED_OFF*/
	};
	
struct TBTGenericInfoNotifierParams
	{
    TBuf8<KBTDevAddrSize>	iRemoteAddr;
	TBTGenericInfoNoteType iMessageType;
	};
	
struct TBTGenericQueryNotifierParams
	{
	TBTGenericQueryNoteType iMessageType;
	TBool iNameExists; // False if the name is not filled.
	TBTDeviceName	iName; 
	};

struct TBTPairedDeviceSettingParams
	{
	TBTDevAddr	iRemoteAddr;
	TInt iPairingStatus;
	};

typedef TPckgBuf<TBTGenericInfoNotifierParams> TBTGenericInfoNotiferParamsPckg;    	
typedef TPckgBuf<TBTGenericQueryNotifierParams> TBTGenericQueryNotiferParamsPckg;    	
typedef TPckgBuf<TBTPairedDeviceSettingParams> TBTPairedDeviceSettingParamsPckg;   

#endif // BTNOTIF_H

// End of File
