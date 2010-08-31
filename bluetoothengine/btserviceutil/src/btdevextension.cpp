/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  an extended BT device offering properties of
* a Bluetooth device that may be needed by Bluetooth UIs
*
*/

#include <btservices/btdevextension.h>

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// C++ default constructor
// ---------------------------------------------------------------------------
//
CBtDevExtension::CBtDevExtension( TDefaultDevNameOption aNameOption ) 
: iNameOption( aNameOption )
    {
    }

// ---------------------------------------------------------------------------
// Symbian 2nd-phase constructor
// ---------------------------------------------------------------------------
//
void CBtDevExtension::ConstructL(  CBTDevice* aDev )
    {
    SetDeviceL( aDev );
    }

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
EXPORT_C CBtDevExtension* CBtDevExtension::NewLC( 
        CBTDevice* aDev, TDefaultDevNameOption aNameOption )
    {
    CBtDevExtension* self = NULL;
    self = new (ELeave) CBtDevExtension( aNameOption );
    CleanupStack::PushL( self );
    self->ConstructL( aDev );
    return self;
    }

// ---------------------------------------------------------------------------
// NewL
// ---------------------------------------------------------------------------
//
EXPORT_C CBtDevExtension* CBtDevExtension::NewLC( 
        const TInquirySockAddr& aAddr,
        const TDesC& aName,
        TDefaultDevNameOption aNameOption )
    {
    CBtDevExtension* self = new (ELeave) CBtDevExtension( aNameOption );
    CleanupStack::PushL( self );
    CBTDevice* dev = CBTDevice::NewLC( aAddr.BTAddr() );
    TBTDeviceClass cod( aAddr.MajorServiceClass(), 
            aAddr.MajorClassOfDevice(), aAddr.MinorClassOfDevice() );
    dev->SetDeviceClass( cod );
    if ( aName.Length() )
        {
        dev->SetDeviceNameL(  BTDeviceNameConverter::ToUTF8L( aName ) );
        }
    self->ConstructL( dev );
    CleanupStack::Pop( dev );
    return self;
    }

// ---------------------------------------------------------------------------
// NewLC
// ---------------------------------------------------------------------------
//
EXPORT_C CBtDevExtension* CBtDevExtension::NewLC( 
        const TBTDevAddr& aAddr,
        const TDesC& aName,
        TDefaultDevNameOption aNameOption )
    {
    CBtDevExtension* self = new (ELeave) CBtDevExtension( aNameOption );
    CleanupStack::PushL( self );
    CBTDevice* dev = CBTDevice::NewLC( aAddr );
    if ( aName.Length() )
        {
        dev->SetDeviceNameL(  BTDeviceNameConverter::ToUTF8L( aName ) );
        }
    self->ConstructL( dev );
    CleanupStack::Pop( dev );
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
EXPORT_C CBtDevExtension::~CBtDevExtension()
    {
    iAlias.Close();
    delete iDev;
    }

// ---------------------------------------------------------------------------
// IsBonded
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CBtDevExtension::IsBonded( const TBTNamelessDevice &dev )
    {
    // IsValidPaired tells if the paired bit of dev is valid
    // and IsPaired tells if the device is paired or not:
    return dev.IsValidPaired() && 
           dev.IsPaired() && 
           // Authentication due to OBEX cases e.g. file transfer, is not 
           // considered as bonded in Bluetooth UI:
           dev.LinkKeyType() != ELinkKeyUnauthenticatedUpgradable;
    }

// ---------------------------------------------------------------------------
// IsJustWorksBonded
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CBtDevExtension::IsJustWorksBonded( const TBTNamelessDevice &dev )
    {
    return IsBonded( dev ) && 
         dev.LinkKeyType() == ELinkKeyUnauthenticatedNonUpgradable;
    }

// ---------------------------------------------------------------------------
// IsUserAwareBonded
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CBtDevExtension::IsUserAwareBonded( const TBTNamelessDevice &dev )
    {
    if ( IsJustWorksBonded( dev ) )
        {
        // Just Works bonded devices can happen without user awareness.
        // For example, authentication due to an incoming service connection request 
        // from a device without IO.
        // We use cookies to identify if this JW pairing is user aware or not:
        TInt32 cookie = dev.IsValidUiCookie() ? dev.UiCookie() : EBTUiCookieUndefined;
        return (cookie & EBTUiCookieJustWorksPaired );        
        }
    // Pairing in other mode than Just Works are always user-aware:
    return IsBonded( dev );
    }

// ---------------------------------------------------------------------------
// IsHeadset
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CBtDevExtension::IsHeadset( const TBTDeviceClass &aCod )
    {
    if ( ( aCod.MajorServiceClass() & EMajorServiceAudioService ) &&
         ( aCod.MajorDeviceClass() & EMajorDeviceAudioDevice ) )
        {
        if (aCod.MinorDeviceClass() == EMinorDeviceAVHandsfree ||
            aCod.MinorDeviceClass() == EMinorDeviceAVCarAudio)
            {
            // This is the typical CoD setting used by carkits:
            return false;
            }
        return true;
        }
    return false;
    }

// ---------------------------------------------------------------------------
// 
// ---------------------------------------------------------------------------
//
EXPORT_C const TDesC& CBtDevExtension::Alias() const
    {
    return iAlias;
    }

// ---------------------------------------------------------------------------
// Addr()
// ---------------------------------------------------------------------------
//
EXPORT_C const TBTDevAddr& CBtDevExtension::Addr() const
    {
    return iDev->BDAddr();
    }

// ---------------------------------------------------------------------------
// Device
// ---------------------------------------------------------------------------
//
EXPORT_C const CBTDevice& CBtDevExtension::Device() const
    {
    return *iDev;
    }



// ---------------------------------------------------------------------------
// IsUserAwareBonded
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CBtDevExtension::IsUserAwareBonded() const
    {
    return IsUserAwareBonded( iDev->AsNamelessDevice() );
    }

// ---------------------------------------------------------------------------
// ServiceConnectionStatus()
// ---------------------------------------------------------------------------
//
EXPORT_C TBTEngConnectionStatus CBtDevExtension::ServiceConnectionStatus() const
    {
    return iServiceStatus;
    }

// ---------------------------------------------------------------------------
// SetDeviceL
// ---------------------------------------------------------------------------
//
EXPORT_C void CBtDevExtension::SetDeviceL( CBTDevice* aDev )
    {
    delete iDev;
    iDev = aDev;
    // It is possible that the client set a NULL object to us.
    if ( !iDev )
        {
        iDev = CBTDevice::NewL();
        }
    // No optimization here. If client sets an identical device instance
    // We will still execute these steps:
    UpdateNameL();
    //UpdateServiceStatusL();
    }

EXPORT_C CBtDevExtension* CBtDevExtension::CopyL()
    {
    CBtDevExtension* newDev = CBtDevExtension::NewLC( NULL );
    CBTDevice* dev = iDev->CopyL();
    CleanupStack::PushL( dev );
    newDev->SetDeviceL( dev );
    CleanupStack::Pop( dev);
    newDev->SetServiceConnectionStatus( ServiceConnectionStatus() );
    CleanupStack::Pop( newDev );
    return newDev;
    }

// ---------------------------------------------------------------------------
// ServiceConnectionStatus()
// ---------------------------------------------------------------------------
//
void CBtDevExtension::SetServiceConnectionStatus(
        TBTEngConnectionStatus aStatus )
    {
    iServiceStatus = aStatus;
    }

// ---------------------------------------------------------------------------
// UpdateL()
// ---------------------------------------------------------------------------
//
void CBtDevExtension::UpdateNameL()
    {
    // UI takes friendly name for displaying if it is available
    iAlias.Zero();
    if ( iDev->IsValidFriendlyName() && iDev->FriendlyName().Length() != 0 )
        {
        iAlias.ReAllocL(iDev->FriendlyName().Length());
        iAlias.Append(iDev->FriendlyName());
        }
    // otherwise, device name, if it is available, will be displayed
    else if ( iDev->IsValidDeviceName() && iDev->DeviceName().Length() != 0 )
        {
        TBTDeviceName name = BTDeviceNameConverter::ToUnicodeL(iDev->DeviceName());
        iAlias.ReAllocL(name.Length());
        iAlias.Append(name);
        }
    if ( iAlias.Length() == 0 && 
        ( iNameOption == EColonSeperatedBDAddr || iNameOption == EPlainBDAddr ) )
        {
        // Name for display is still missing. We need to make one for user to 
        // identify it.
        FormatAddressAsNameL();
        }
    }

// ---------------------------------------------------------------------------
// FormatAddressAsNameL()
// ---------------------------------------------------------------------------
//
void CBtDevExtension::FormatAddressAsNameL()
    {
    // readable format of BD_ADDR is double size of BD ADDR size,
    // and plus the seperators.
    iAlias.ReAllocL( KBTDevAddrSize * 3 );
    _LIT(KColon, ":");
    if ( iNameOption == EColonSeperatedBDAddr )
        {
        iDev->BDAddr().GetReadable( iAlias, KNullDesC, KColon, KNullDesC );         
        }
    else
        {
        iDev->BDAddr().GetReadable( iAlias );
        }
    }
