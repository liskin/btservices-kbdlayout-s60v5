/*
* ============================================================================
*  Name        : btnotifwrapperproxy.cpp
*  Part of     : bluetoothengine / btnotifwrapper
*  Description : ECOM plug-in entry implementation.
*
*  Copyright © 2009 Nokia Corporation and/or its subsidiary(-ies).
*  All rights reserved.
*  This component and the accompanying materials are made available
*  under the terms of "Eclipse Public License v1.0"
*  which accompanies this distribution, and is available
*  at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
*  Initial Contributors:
*  Nokia Corporation - initial contribution.
*
*  Contributors:
*  Nokia Corporation
* ============================================================================
* Template version: 4.1
*/


#include <ecom/implementationproxy.h>
#include <btmanclient.h>
#include <btextnotifiers.h>
#ifdef SYMBIAN_ENABLE_SPLIT_HEADERS
#include <btextnotifierspartner.h>
#endif
#include <btnotifierapi.h>
//#include <networking/panuiinterfaces.h>

#include "btnotifwrapper.h"

/**  Granularity for constructing the array of notifiers */
const TInt KBTNotifWrapperArraySize = 11;
/** Channel for discovery notifiers */
const TUid KBTDiscoveryChannel = { 0x00000601 };
/** Channel for authentication and authorisation notifiers */
const TUid KBTAuthChannel = {0x00000602};
/** Channel for OBEX passkey notifiers */
//const TUid KBTObexAuthChannel = {0x00000603};
/** Channel for power mode query */
//const TUid KBTPowerModeChannel = {0x00000605};
/** Channel for query notifiers */
//const TUid KBTQueryChannel = {0x00000606};
/** Channel for power mode query */
//const TUid KBTInfoChannel = {0x00000610};


// ======== LOCAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// ?description
// ---------------------------------------------------------------------------
//
void CreateAndAppendNotifierLC( CArrayPtr<MEikSrvNotifierBase2>& aArray,
    const TUid& aUid, const TUid& aChannel )
    {
    CBTNotifWrapper* notifier = CBTNotifWrapper::NewLC( aUid, aChannel );
    aArray.AppendL( notifier );
    }

// ---------------------------------------------------------------------------
// ?description
// ---------------------------------------------------------------------------
//
CArrayPtr<MEikSrvNotifierBase2>* CreateNotifierArrayL()
    {
    CArrayPtrFlat<MEikSrvNotifierBase2>* notifiers = 
        new( ELeave ) CArrayPtrFlat<MEikSrvNotifierBase2>( KBTNotifWrapperArraySize );
    CleanupStack::PushL( notifiers );
    // Create all the notifiers:
    // Connection authorization notifier
    CreateAndAppendNotifierLC( *notifiers, KBTManAuthNotifierUid, KBTAuthChannel );
    // Old and new PIN notifiers
    CreateAndAppendNotifierLC( *notifiers, KBTManPinNotifierUid, KBTAuthChannel );
    CreateAndAppendNotifierLC( *notifiers, KBTPinCodeEntryNotifierUid, KBTAuthChannel );
    // Secure simple pairing notifiers
    CreateAndAppendNotifierLC( *notifiers, KBTNumericComparisonNotifierUid, KBTAuthChannel );
    CreateAndAppendNotifierLC( *notifiers, KBTPasskeyDisplayNotifierUid, KBTAuthChannel );
    
    // todo: add
    // KBTUserConfirmationNotifierUid for incoming JW dedicated bonding.
    
    CreateAndAppendNotifierLC( *notifiers, KDeviceSelectionNotifierUid, KBTDiscoveryChannel );

    /*
     * todo:
     * Other notifiers to be migrated:
     * 
     * existing stack notifiers:
     * CreateAndAppendNotifierL( aArray, KPbapAuthNotifierUid, KBTObexPINChannel );
     * 
     * S60 SDK API:
     * CreateAndAppendNotifierL( aArray, KPowerModeSettingNotifierUid, KBTPowerModeChannel );
     *
     * S60-defined platform:
     * CreateAndAppendNotifierL( aArray, KBTObexPasskeyQueryNotifierUid, KBTObexPINChannel );
     * CreateAndAppendNotifierL( aArray, KBTGenericInfoNotifierUid, KBTInfoChannel );
     * CreateAndAppendNotifierL( aArray, KBTGenericQueryNotifierUid, KBTQueryChannel );
     * 
     * new (PAN-related):
     * CreateAndAppendNotifierL( aArray, KBTPanDeviceSelectionNotifierUid, KBTDiscoveryChannel );
     * CreateAndAppendNotifierL( aArray, KBTPanNapUplinkAuthorisationNotifierUid, KBTAuthChannel );
     */
    CleanupStack::Pop( notifiers->Count() + 1, notifiers );  // Each notifier + notifier array itself
    return notifiers;
    }

// ---------------------------------------------------------------------------
// ECOM entry point.
// ---------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
    IMPLEMENTATION_PROXY_ENTRY( 0x20026FEE, CreateNotifierArrayL )
    };

// ======== GLOBAL FUNCTIONS ========

// ---------------------------------------------------------------------------
// ECOM factory method.
// ---------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
    }
