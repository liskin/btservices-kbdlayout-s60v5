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

#ifndef AVCPANEL_H
#define AVCPANEL_H

/**
@file
@publishedPartner
@released
*/

namespace AVCPanel
	{
	enum TOperationId
		{
		ESelect				= 0x00,
		EUp					= 0x01,
		EDown				= 0x02,
		ELeft				= 0x03,
		ERight				= 0x04,
		ERightUp			= 0x05,
		ERightDown			= 0x06,
		ELeftUp				= 0x07,
		ELeftDown			= 0x08,
		ERootMenu			= 0x09,
		ESetupMenu			= 0x0a,
		EContentsMenu		= 0x0b,
		EFavoriteMenu		= 0x0c,
		EExit				= 0x0d,
		E0					= 0x20,
		E1					= 0x21,
		E2					= 0x22,
		E3					= 0x23,
		E4					= 0x24,
		E5					= 0x25,
		E6					= 0x26,
		E7					= 0x27,
		E8					= 0x28,
		E9					= 0x29,
		EDot				= 0x2a,
		EEnter				= 0x2b,
		EClear				= 0x2c,
		EChannelUp			= 0x30,
		EChannelDown		= 0x31,
		EPreviousChannel	= 0x32,
		ESoundSelect		= 0x33,
		EInputSelect		= 0x34,
		EDisplayInformation	= 0x35,
		EHelp				= 0x36,
		EPageUp				= 0x37,
		EPageDown			= 0x38,
		EPower				= 0x40,
		EVolumeUp			= 0x41,
		EVolumeDown			= 0x42,	
		EMute				= 0x43,	
		EPlay				= 0x44,
		EStop				= 0x45,
		EPause				= 0x46,
		ERecord				= 0x47,
		ERewind				= 0x48,
		EFastForward		= 0x49,
		EEject				= 0x4a,
		EForward			= 0x4b,
		EBackward			= 0x4c,
		EAngle				= 0x50,
		ESubpicture			= 0x51,
		EF1					= 0x71,
		EF2					= 0x72,
		EF3					= 0x73,
		EF4					= 0x74,
		EF5					= 0x75,
		EVendorUnique		= 0x7e
		};
				
	enum TButtonAction
		{
		EButtonPress = 0x0 << 7,
		EButtonRelease = 0x1 << 7,
		EButtonUnknown = 0x0 << 7  // Default to push
		};
	}

#endif // AVCPANEL_H
