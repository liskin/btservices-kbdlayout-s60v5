// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//



/**
 @file
 @internalComponent
 @released
*/

#include <bluetoothav.h>
#include "avrcpcommandframer.h"
#include "avcframe.h"
#include "avrcplog.h"
#include <remcon/avrcpspec.h>
#include <absolutevolumeapi.h>

// The Bluetooth SIG registered VendorId, see table 4.7
const AVC::TAVCVendorId KBluetoothSIGVendorId = 0x001958;

/** Returns a new CAVCFrame representing an AV/C passthrough command.
@param aOperationId The AV/C operation id of this command.
@param aButtonAction The AV/C button action.
@return An AV/C frame representing a passthrough command.
@leave error
*/
CAVCFrame* AvrcpCommandFramer::PassthroughL(AVCPanel::TOperationId aOperationId, AVCPanel::TButtonAction aButtonAction)
	{
	LOG_STATIC_FUNC
	CAVCFrame* frame = CAVCFrame::NewL(AVC::ECommand, AVC::EControl, AVC::EPanel, AVC::EID0);
	frame->Append(TChar(AVC::EPassThrough));
	frame->Append(TChar(aOperationId | aButtonAction));
	frame->Append(TChar(0));
	return frame;
	}

/** Returns a new CAVCFrame representing an AV/C Unit Info response.
@return An AV/C frame representing a Unit Info response.
@leave error
*/	
CAVCFrame* AvrcpCommandFramer::UnitInfoResponseL()
	{
	LOG_STATIC_FUNC
	CAVCFrame* frame = CAVCFrame::NewL(AVC::EResponse, AVC::EStable, AVC::EUnit, AVC::EIgnore);
	frame->Append(TChar(AVC::EUnitInfo));
	frame->Append(TChar(0x7));
	frame->Append(TChar(0x48));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	return frame;
	}

/** Returns a new CAVCFrame representing an AV/C Subunit Info response.
@return An AV/C frame representing a Subunit Info response.
@leave error
*/	
CAVCFrame* AvrcpCommandFramer::SubunitInfoResponseL()
	{
	LOG_STATIC_FUNC
	CAVCFrame* frame = CAVCFrame::NewL(AVC::EResponse, AVC::EStable, AVC::EUnit, AVC::EIgnore);
	frame->Append(TChar(AVC::ESubunitInfo));
	frame->Append(TChar(0x7));
	frame->Append(TChar(0x48));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	frame->Append(TChar(0xff));
	return frame;
	}

/** Returns a new CAVCFrame representing an AV/C 'set absolute volume' command
@return An AV/C frame representing a 'set absolute volume' command.
@leave error
*/	
CAVCFrame* AvrcpCommandFramer::SetAbsoluteVolumeCommandL(TUint8 aAbsVol)
	{
	LOG_STATIC_FUNC
	CAVCFrame* frame = CAVCFrame::NewL(AVC::ECommand, AVC::EControl, AVC::EPanel, AVC::EID0);
	frame->Append(TChar(AVC::EVendorDependent));
	frame->Append(KBluetoothSIGVendorId>>16 & 0xff);	
	frame->Append(KBluetoothSIGVendorId>>8 & 0xff);	
	frame->Append(KBluetoothSIGVendorId & 0xff);
	
	frame->Append(ESetAbsoluteVolume);
	frame->Append(TChar(AVC::EASingle));
	
	TUint16 len = KLengthSetAbsoluteVolumeRequestParameter<<8 & 0xffff;
	TPckgBuf<TUint16> parameterLength(len);
	frame->Append(parameterLength);
	frame->Append(aAbsVol);
	return frame;
	}

/** Returns a new CAVCFrame representing an AV/C 'Register notification absolute volume changed' command.
@return An AV/C frame representing a 'Register notification absolute volume changed' command.
@leave error
*/	
CAVCFrame* AvrcpCommandFramer::NotifyVolumeChangeCommandL()
	{
	LOG_STATIC_FUNC
	CAVCFrame* frame = CAVCFrame::NewL(AVC::ECommand, AVC::ENotify, AVC::EPanel, AVC::EID0);
	frame->Append(TChar(AVC::EVendorDependent));
	frame->Append(KBluetoothSIGVendorId>>16 & 0xff);	
	frame->Append(KBluetoothSIGVendorId>>8 & 0xff);	
	frame->Append(KBluetoothSIGVendorId & 0xff);
	
	frame->Append(ERegisterNotification);
	frame->Append(TChar(AVC::EASingle));
	
	TUint16 len = KLengthNotifyVolumeChangeRequestParameter<<8 & 0xffff;
	TPckgBuf<TUint16> parameterLength(len);
	frame->Append(parameterLength);
	frame->Append(ERegisterNotificationVolumeChanged);
	
	TBuf8<4> reservedBuffer(KLengthNotifyVolumeChangeRequestParameter - 1);
	reservedBuffer.FillZ();
	frame->Append(reservedBuffer);
	return frame;
	}
