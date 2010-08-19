/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  type declaration.
*
*/


#ifndef BTSACDEFS_H
#define BTSACDEFS_H

const TInt KBTDevAddrReadable = KBTDevAddrSize * 2;

const TUint8 KRequestNone 	  = 0x00; 
const TUint8 KConnectReq 	  = 0x01; 
const TUint8 KDisconnectReq	  = 0x02;
const TUint8 KOpenAudioReq	  = 0x04;
const TUint8 KCloseAudioReq	  = 0x08;
const TUint8 KCompleteAllReqs = 0xff;


const TInt KRequestIdWaitRemoteConfStart = 0xfffb;
const TInt KRequestIdGavdpError = 0xfffc;
const TInt KRequestIdTimer = 0xfffd;
const TInt KRequestIdSelfComplete = 0xfffe;

enum TBTConnType
	{
	EOutgoingConn,
	EOutgoingConnNoDiscovery,
	EOutgoingConnWithAudio,
	EOutgoingConnWithAudioNoDiscovery,
	EIncomingConn,
	EConnExists
	};

enum TAudioOpenedBy
	{
	EAudioOpenedByNone,
	EAudioOpenedByAFW,
	EAudioOpenedByAcc	
	};

enum TStreamConfiguredBy
	{
	EStreamConfiguredBySrc,
	EStreamConfiguredBySink
	};

enum TBTSACResetGavdp
    {
    EDontResetGavdp,
    EResetGavdp
    };

//Panic codes
enum TBTPanicCode 
    {
	EBTPanicNullPointer = 33400,
    EBTPanicSocketExists 
    };
    
#endif      // BTSACDEFS_H
            