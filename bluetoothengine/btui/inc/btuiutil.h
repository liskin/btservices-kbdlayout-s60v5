/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  
 *   
 */

#ifndef BTUIUTIL_H
#define BTUIUTIL_H

#include <qglobal.h>
#include <bt_sock.h>

/*!
  Converts a QString which contains a BT device address in readable format to
  Symbian native TBTDevAddr type.
 */
inline void addrReadbleStringToSymbian( const QString &readable, TBTDevAddr &addr)
{
    TBuf<KBTDevAddrSize * 2> buffer(readable.utf16());
    addr.SetReadable( buffer );
}

/*!
  Converts a Symbian native TBTDevAddr to 
  QString which contains the BT device address in readable format.
 */
inline void addrSymbianToReadbleString( QString &readable, const TBTDevAddr &addr)
{
    TBuf<KBTDevAddrSize * 2> buffer;
    addr.GetReadable( buffer );
    readable = QString::fromUtf16( buffer.Ptr(), buffer.Length() );
}

/*!
  Decide the device name to display from the device information, and 
  converts the name if necessary. If the device doesn't have a valid name,
  the given default name will be used.
*/
inline void getDeviceDisplayName( QString& dispName, const CBTDevice& device ,
    const TDesC& defaultName )
{
    // friendly name is preferred if available
    if( device.IsValidFriendlyName() && device.FriendlyName().Length()){
        dispName = QString::fromUtf16( 
                device.FriendlyName().Ptr(), device.FriendlyName().Length() );
    }
    // next preferred is actual device name
    else if( device.IsValidDeviceName() && device.DeviceName().Length() ) {
        dispName = QString::fromUtf8( 
                (char*) device.DeviceName().Ptr(), device.DeviceName().Length() );
    }
    else {
        // finally, use default name if nothing else is available
        dispName = QString::fromUtf16( 
                defaultName.Ptr(), defaultName.Length() );
    }
}

/*!
  Guess if the given Class of Device indicates an Audio/Video device (headset and carkit)
  or not.
  Computer device supporting audio is not considered as AV device.
*/
inline bool isAVDevice( const TBTDeviceClass &cod )
{
    int majorDevCls = cod.MajorDeviceClass();
    int minorDevCls = cod.MinorDeviceClass();
    return ( ( majorDevCls == EMajorDeviceAV ) 
        || ( cod.MajorServiceClass() == EMajorServiceRendering 
        && majorDevCls != EMajorDeviceImaging ) );
}

/*!
  Guess if the given Class of Device indicates an input device (keyboard and mouse)
  or not.
*/
inline bool isHIDDevice( const TBTDeviceClass &cod )
{
    int majorDevCls = cod.MajorDeviceClass();
    int minorDevCls = cod.MinorDeviceClass();
    return ( ( majorDevCls == EMajorDevicePeripheral ) &&
        ( minorDevCls == EMinorDevicePeripheralKeyboard || 
        minorDevCls == EMinorDevicePeripheralPointer ) );
}

/*!
  Tells if the given device is bonded.
*/
inline bool isBonded( const CBTDevice &dev )
{
    // todo: this has not addresses Just Works pairing mode yet.
    return dev.IsValidPaired() && dev.IsPaired() &&
        dev.LinkKeyType() != ELinkKeyUnauthenticatedUpgradable;
}

/*!
  Tells if the given device supports file transfer.
*/
inline bool supportsFileTransfer( const TBTDeviceClass &cod )
{
    int majorDevCls = cod.MajorDeviceClass();
    return ( majorDevCls == EMajorDevicePhone 
            || majorDevCls == EMajorDeviceComputer ); 
}

#endif // BTUIMODELUTIL_H
