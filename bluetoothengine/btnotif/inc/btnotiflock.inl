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
* Description:  Implements getting/setting notifier locks on BT devices
*               from/to P&S KBTNotifierLocks from btengprivatepskeys.h.
*
*/

#include "btengprivatepskeys.h"
#include <e32property.h>

// ----------------------------------------------------------
// Initialize members to zero
// ----------------------------------------------------------
//
inline TBTNotifLock::TBTNotifLock()
    : iAddr(TBTDevAddr()), iLocks(EBTNotifierLockNone)
    {
    }

// ----------------------------------------------------------
// initialize members to the specified values
// ----------------------------------------------------------
//
inline TBTNotifLock::TBTNotifLock(const TBTDevAddr& aAddr, TInt aLocks)
    : iAddr( aAddr ), iLocks( aLocks )
    {
    }

// ----------------------------------------------------------
// returns the address
// ----------------------------------------------------------
//
inline const TBTDevAddr& TBTNotifLock::Addr() const
    {
    return iAddr;
    }

// ----------------------------------------------------------
// returns the lock as value
// ----------------------------------------------------------
//
inline TInt TBTNotifLock::Locks() const
    {
    return iLocks;
    }

// ----------------------------------------------------------
// returns the lock as modifiable reference
// ----------------------------------------------------------
//
inline TInt& TBTNotifLock::Locks()
    {
    return iLocks;
    }

// ----------------------------------------------------------
// Locally instantiate a RProperty and invoke GetNotifLocks
// ----------------------------------------------------------
//
inline void TBTNotifLockPublish::GetNotifLocks( TInt& aLocks, const TBTDevAddr& aAddr )
    {
    RProperty property;
    TInt err = property.Attach( 
                KPSUidBluetoothEnginePrivateCategory, KBTNotifierLocks );
    if ( !err )
        {
        GetNotifLocks( property, aLocks, aAddr );
        }
    property.Close();
    }

// ----------------------------------------------------------
// Extracts from PS key and returns the lock value for
// the specified device
// ----------------------------------------------------------
//
inline void TBTNotifLockPublish::GetNotifLocks(RProperty& aProperty,
        TInt& aLocks, const TBTDevAddr& aAddr )
    {
    aLocks = EBTNotifierLockNone;
    TInt infoSize( sizeof( TBTNotifLock ) );
    TBuf8<sizeof( TBTNotifLock ) * 8> infoDes;
    TInt err = aProperty.Get( infoDes );
    if ( err || infoDes.Length() < infoSize )
        {
        // zero length of the PS content indicates no lock set at all. 
        return;
        }
    TInt infoCount = infoDes.Length() / infoSize;
    
    TPckgBuf<TBTNotifLock> tmpPckg;
    for ( TInt i = 0; i < infoCount; i++ )
        {
        tmpPckg.Copy(infoDes.Mid( i * infoSize, infoSize ));
        if ( tmpPckg().Addr() == aAddr )
            {
            // found the locks for the device, writes to client.
            aLocks = tmpPckg().Locks();
            return;
            }
        }
    }

// ----------------------------------------------------------
// Locally instantiate a RProperty and invoke AddNotifLocks
// ----------------------------------------------------------
//
inline void TBTNotifLockPublish::AddNotifLocks( TInt aLocks, const TBTDevAddr& aAddr )
    {
    RProperty property;
    TInt err = property.Attach( 
                KPSUidBluetoothEnginePrivateCategory, KBTNotifierLocks );
    if ( !err )
        {
        AddNotifLocks( property, aLocks, aAddr );
        }
    property.Close();
    }

// ----------------------------------------------------------
// Find the lock for the device from PS key and updates its value if
// needed.
// ----------------------------------------------------------
//
inline void TBTNotifLockPublish::AddNotifLocks( RProperty& aProperty,
        TInt aLocks, const TBTDevAddr& aAddr )
    {
    TBuf8<sizeof( TBTNotifLock ) * 8> infoDes;
    TInt infoSize( sizeof( TBTNotifLock ) );
    TInt err = aProperty.Get( infoDes );   
    if ( err )
        {
        return;
        }
    TInt infoCount = infoDes.Length() / infoSize;
    TPckgBuf<TBTNotifLock> tmpPckg;

    for ( TInt i = 0; i < infoCount; i++ )
        {
        tmpPckg.Copy(infoDes.Mid( i * infoSize, infoSize ));
        if ( tmpPckg().Addr() == aAddr )
            {
            TInt newOps = tmpPckg().Locks() | aLocks;
            // update the value only if it is really changed:
            if ( tmpPckg().Locks() != newOps )
                {
                tmpPckg().Locks() = newOps;
                infoDes.Replace( i * infoSize, infoSize, tmpPckg );
                (void) aProperty.Set( infoDes );
                }
            return;
            }
        }
    // no lock for the device so far, append it to the end:
    tmpPckg() = TBTNotifLock( aAddr, aLocks );
    if ( infoCount )
        {
        infoDes.Append( tmpPckg );
        (void) aProperty.Set( infoDes );        
        }
    (void) aProperty.Set( tmpPckg ); 
    }

// ----------------------------------------------------------
// Locally instantiate a RProperty and invoke DeleteNotifLocks
// ----------------------------------------------------------
//
inline void TBTNotifLockPublish::DeleteNotifLocks( TInt aLocks, const TBTDevAddr& aAddr )
    {
    RProperty property;
    TInt err = property.Attach( 
                KPSUidBluetoothEnginePrivateCategory, KBTNotifierLocks );
    if ( !err )
        {
        DeleteNotifLocks( property, aLocks, aAddr );
        }
    property.Close();
    }

// ----------------------------------------------------------
// Find the lock for the device from PS key and updates its value if
// needed.
// ----------------------------------------------------------
//
inline void TBTNotifLockPublish::DeleteNotifLocks( RProperty& aProperty,
        TInt aLocks, const TBTDevAddr& aAddr )
    {
    TBuf8<sizeof( TBTNotifLock ) * 8> infoDes;
    TInt infoSize( sizeof( TBTNotifLock ) );
    TInt err = aProperty.Get( infoDes );
    if ( err || infoDes.Length() < infoSize )
        {
        return;
        }
    TInt infoCount = infoDes.Length() / infoSize;
    TPckgBuf<TBTNotifLock> tmpPckg;
    for ( TInt i = 0; i < infoCount; i++ )
        {
        tmpPckg.Copy( infoDes.Mid( i * infoSize, infoSize ) );
        if ( tmpPckg().Addr() == aAddr )
            {
            TInt newOps = tmpPckg().Locks() & ~aLocks;
            // update PS only if the value is changed:
            if ( tmpPckg().Locks() != newOps )
                {
                tmpPckg().Locks() = newOps;
                if ( tmpPckg().Locks() == 0)
                    {
                    // no lock for this device anymore, remove from PS:
                    infoDes.Delete( i * infoSize, infoSize );
                    }
                else
                    {
                    // Update the lock value:
                    infoDes.Replace( i * infoSize, infoSize, tmpPckg );
                    }
                (void) aProperty.Set( infoDes );
                }
            return;
            }
        }
    }

