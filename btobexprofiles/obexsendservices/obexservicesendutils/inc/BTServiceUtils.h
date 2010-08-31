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
* Description:  Contains BTSU wide definitions.
*
*/


#ifndef BT_SERVICE_UTILS_H
#define BT_SERVICE_UTILS_H

// INCLUDES
#include <e32std.h>

// CONSTANTS

// Name of this component
//
_LIT( KBTSUModuleName, "Bluetooth Service Utils" );

const TInt KBTSUArrayGranularity     = 1;
const TInt KBTSUMaxStringLength      = 255;
const TInt KBTSUTextBufferMaxSize    = 512;
const TInt KBTSUDataBufferMaxSize    = 102400;  // 100K 
const TInt KBTSUDataBufferExpandSize = 4;
const TInt KBTSUEqualStrings         = 0; // The ok-result of TDesC::Compare

_LIT( KBTSProtocol, "RFCOMM" ); // The Bluetooth transport layer

// Obex header HI values
//
const TUint8 KBTSUNameHeader        = 0x01;
const TUint8 KBTSUImageHandleHeader = 0x30;
const TUint8 KBTSUTypeHeader        = 0x42;
const TUint8 KBTSULengthHeader      = 0xC3;
const TUint8 KBTSUTargetHeader      = 0x46;
const TUint8 KBTSUAppParamsHeader   = 0x4C;
const TUint8 KBTSUImgDescriptorHeader = 0x71;


// DATA TYPES

enum TBTSUPanicCode 
    {
    EBTSUPanicNullPointer = 45000,
    EBTSUPanicExistingObject,
    EBTSUPanicObjectActive,
    EBTSUPanicUnhandledCase,
    EBTSUPanicInternalError,
    EBTSUPanicOutOfRange,
    EBTSUPanicResponseAlreadyPresent,
    EBTSUPanicNoBufferEvenThoughCountNotZero
    };

enum TBTServiceStatus
    {
    EBTSNoError = 100,
    EBTSNoFiles,
    EBTSAbort,
    EBTSConnectingFailed,
    EBTSGettingFailed,
    EBTSPuttingFailed,
    EBTSNoSuitableProfiles,
    EBTSUserCancel,
    EBTSBIPOneNotSend,
    EBTSBIPSomeSend,
    EBTSBIPNoneSend
    };
    
 struct  TBTSUImageCap
 	{
 	TDesC* iEncoding;
 	TSize  iMinPixelSize;
 	TSize  iMaxPixelSize;
 	TInt   iMaxByteSize;	
 	};


// FUNCTION PROTOTYPES

/**
* Calls the Symbian OS Panic function specifying this component's name as the
* panic category and aPanic as the panic code.
* @param aPanic The panic code.
* @return None.
*/
void BTSUPanic( TBTSUPanicCode aPanic );


// CLASS DECLARATION

/**
*  A cleanup template class for objects that need ResetAndDestroy call.
*/
template <class T> class CleanupResetAndDestroy
    {
    public:
        inline static void PushL( T& aRef );
    private:
        static void ResetAndDestroy( TAny *aPtr );
    };

template <class T> inline void CleanupResetAndDestroyPushL( T& aRef );

#include "BTServiceUtils.inl"

#endif      // BT_SERVICE_UTILS_H
            
// End of File
