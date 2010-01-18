/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Utilities that may be useful for components of
*               Bluetooth Engine subsystem
*
*/

#ifndef BTENGUTIL_H
#define BTENGUTIL_H

#include <btdevice.h>
#include <btengconstants.h>

/**
  Tells if the given device has been paired regardless of user awareness.
  (User aware paired devices are those seen in the paired device view of
  Bluetooth application.)
*/
inline TBool IsPaired( const TBTNamelessDevice &dev )
{
    // IsValidPaired tells if the paired bit of dev is valid
    // and IsPaired tells if the device is paired or not:
    return dev.IsValidPaired() && 
           dev.IsPaired() && 
           // Authentication due to OBEX cases e.g. file transfer, is not 
           // considered as paired in Bluetooth engine scope:
           dev.LinkKeyType() != ELinkKeyUnauthenticatedUpgradable;
}

/**
  Tells if the given device has been paired with Just Works model.
*/
inline TBool IsJustWorksPaired( const TBTNamelessDevice &dev )
{
    return IsPaired( dev ) && 
         dev.LinkKeyType() == ELinkKeyUnauthenticatedNonUpgradable;
}

/**
  Tells if the given device has been paired under user awareness.
  (User aware paired devices are those seen in the paired device view of
  Bluetooth application.)
*/
inline TBool IsUserAwarePaired( const TBTNamelessDevice &dev )
{
    if ( IsJustWorksPaired( dev ) )
        {
        // Just Works paired devices can happen without user awareness.
        // For example, paired due to an incoming service connection request 
        // from a device without IO.
        // We use cookies to identify if this JW pairing is user aware or not:
        TInt32 cookie = dev.IsValidUiCookie() ? dev.UiCookie() : EBTUiCookieUndefined;
        return (cookie & EBTUiCookieJustWorksPaired );        
        }
    // Pairing in other mode than Just Works are always user-aware:
    return IsPaired( dev );
}

#endif /*BTENGUTIL_H*/
