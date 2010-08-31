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
* Description:  ObexServicemanager opcodes, panic enumerations and other 
*                definitions.                 
*
*/


#ifndef _SRCSCLSV_H
#define _SRCSCLSV_H

//  INCLUDES

#include <e32base.h>

// CONSTANTS

// Panic definitions
_LIT(KSrcsPanic,"Srcs panic");

// reasons for server panic
enum TSrcsPanic
{
    ESrcsBadRequest,
    ESrcsBadDescriptor,
    ESrcsBadHelper,
    ESrcsBadMessage,
    ESrcsNoLimitedMessage,
    ESrcsClassMemberVariableIsNull,
    ESrcsClassMemberVariableIsNotNull,
    ESrcsMainSchedulerError,
    ESrcsNotImplementedYet
};

/// server name
_LIT(KSrcsName,"obexserviceman");

// Server path

_LIT(KSrcsImg,"z:\\sys\\bin\\obexserviceman.exe");

// SRCS Server Uid
const TUid KSrcsUid={0x101F7C87};

// SRCS Server thread memory allocation.
// These are only used in WINS when the server is not running
// before the client tries to connect to it.
#ifdef __WINS__
const TInt KSrcsStackSize       =0x2000;			//  8KB
const TInt KSrcsMinHeapSize     =0x1000;		    //  4KB
const TInt KSrcsMaxHeapSize     =0x100000;		    //  1MB
#endif

// A version must be specifyed when creating a session with the server
const TUint KSrcsMajorVersionNumber=1;
const TUint KSrcsMinorVersionNumber=0;
const TUint KSrcsBuildVersionNumber=1;

// Opcodes used in message passing between client and server
enum TSrcsServRequest
{
    ESrcsBTServicesON,
    ESrcsBTServicesOFF,
    ESrcsStartUSB,
    ESrcsStopUSB,    
    ESrcsIrDAServicesON,
    ESrcsIrDAServicesOFF,    
    ESrcsCancelRequest
};

/**
* Define transports which are supported by SRCS
*/
enum TSrcsTransport
{
    ESrcsTransportBT,
    ESrcsTransportUSB,
    ESrcsTransportIrDA
};

/**
*	Transportation string definition
*/
_LIT8(KSrcsTransportBT,"OBEX/BT");
_LIT8(KSrcsTransportUSB,"OBEX/USB");
_LIT8(KSrcsTransportIrDA,"OBEX/IrDA");



#endif // SRCSCLSV_H

