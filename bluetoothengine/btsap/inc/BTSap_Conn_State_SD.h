/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     An enumeration type defined for the values of the SharedData key: KSapState
*
*/


#ifndef __BTSAP_CONN_STATE_SD_H__
#define __BTSAP_CONN_STATE_SD_H__

//  INCLUDES

// FORWARD DECLARATIONS

// CLASS DECLARATION

typedef enum TBTSap_Conn_State_SD
    {
    ESapNotConnected,
    ESapConnecting,
    ESapConnected
	// TODO: Add further values here
    } TBTSapConnectionState;

#endif // __BTSAP_CONN_STATE_SD_H__
