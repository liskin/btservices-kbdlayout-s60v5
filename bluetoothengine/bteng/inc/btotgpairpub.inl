/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implements getting/setting outgoing pair status
*               from/to P&S KBTOutgoingPairing from btengprivatepskeys.h.
*
*/

#include "btengprivatepskeys.h"
#include <e32property.h>

// ----------------------------------------------------------
// Get outgoing pair status from PS
// ----------------------------------------------------------
//
inline void OutgoPairingProperty(RProperty& aProperty, 
        TBTDevAddr& aAddr, TBTOutgoingPairMode& aMode)
    {
    TBuf8<sizeof( TBTOutgoingPairProperty )> propDes;
    TInt err = aProperty.Get( propDes );
    if ( !err && propDes.Length() == sizeof( TBTOutgoingPairProperty ) )
        {
        TBTOutgoingPairProperty prop;
        TPckgC<TBTOutgoingPairProperty> tmpPckg( prop );
        tmpPckg.Set( propDes );
        aAddr = tmpPckg().iAddr;
        aMode = tmpPckg().iMode;
        }
    else
        {
        aMode = EBTOutgoingPairNone;
        }
    }

// ----------------------------------------------------------
// Get outgoing pair status from PS
// ----------------------------------------------------------
//
inline void OutgoPairingProperty(TBTDevAddr& aAddr, TBTOutgoingPairMode& aMode)
    {
    RProperty property;
    TInt err = property.Attach( 
                KPSUidBluetoothEnginePrivateCategory, KBTOutgoingPairing );
    if ( !err )
        {
        OutgoPairingProperty( property, aAddr, aMode );
        }
    else
        {
        aMode = EBTOutgoingPairNone;
        }
    property.Close();
    }

// ----------------------------------------------------------
// Tells if another outgoing pairing is ongoing with a device
// other than the specified one.
// ----------------------------------------------------------
//
inline TBool OtherOutgoPairing( const TBTDevAddr& aAddr )
    {
    TBTDevAddr outpaddr;
    TBTOutgoingPairMode mode;
    OutgoPairingProperty( outpaddr, mode );
    return mode != EBTOutgoingPairNone && outpaddr != aAddr;
    }

// ----------------------------------------------------------
// Gets the status of outgoing pair with the specified device.
// ----------------------------------------------------------
//
inline TBTOutgoingPairMode OutgoPairingMode(
        RProperty& aProperty, const TBTDevAddr& aAddr )
    {
    TBTDevAddr addr;
    TBTOutgoingPairMode mode;
    OutgoPairingProperty(aProperty, addr, mode );
    return ( addr == aAddr ) ? mode : EBTOutgoingPairNone;
    }

// ----------------------------------------------------------
// Gets the status of outgoing pair with the specified device.
// ----------------------------------------------------------
//
inline TBTOutgoingPairMode OutgoPairingMode(const TBTDevAddr& aAddr)
    {
    TBTDevAddr addr;
    TBTOutgoingPairMode mode;
    OutgoPairingProperty( addr, mode );
    return ( addr == aAddr ) ? mode : EBTOutgoingPairNone;
    }

// ----------------------------------------------------------
// publish outgoing pairing status to PS
// ----------------------------------------------------------
//
inline void SetOutgoPairProperty(RProperty& aProperty, 
        const TBTDevAddr& aAddr,
        TBTOutgoingPairMode aMode )
    {
    if ( aMode == EBTOutgoingPairNone )
        {
        (void) aProperty.Set( KNullDesC8 );
        return;
        }
    TPckgBuf<TBTOutgoingPairProperty> tmpPckg;
    tmpPckg().iAddr = aAddr;
    tmpPckg().iMode = aMode;
    (void) aProperty.Set( tmpPckg );
    }

// ----------------------------------------------------------
// Locally instantiate a RProperty and Set Outgoing Pair Property
// ----------------------------------------------------------
//
inline void SetOutgoPairProperty(const TBTDevAddr& aAddr,
        TBTOutgoingPairMode aMode )
    {
    RProperty property;
    TInt err = property.Attach( 
                KPSUidBluetoothEnginePrivateCategory, KBTOutgoingPairing );
    if ( !err )
        {
        SetOutgoPairProperty( property, aAddr, aMode );
        }
    property.Close();
    }
