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
* Description: 
*     BT HID settings manager
*
*/


// INCLUDE FILES

#include <e32property.h>
#include <e32debug.h>
#include <centralrepository.h>
#include "btengprivatecrkeys.h"
#include "bthidsettings.h"
#include "debug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBtHidSettings::CBtHidSettings
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C CBtHidSettings::CBtHidSettings()
    {}

// -----------------------------------------------------------------------------
// CBtHidSettings::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
EXPORT_C void CBtHidSettings::ConstructL()
    {}

// -----------------------------------------------------------------------------
// CBtHidSettings::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CBtHidSettings* CBtHidSettings::NewL()
    {
    CBtHidSettings* self = CBtHidSettings::NewLC();
    CleanupStack::Pop();

    return self;
    }


// -----------------------------------------------------------------------------
// CBtHidSettings::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CBtHidSettings* CBtHidSettings::NewLC()
    {
    CBtHidSettings* self = new( ELeave ) CBtHidSettings;

    CleanupStack::PushL( self );
    self->ConstructL();

    return self;
    }


// Destructor
EXPORT_C CBtHidSettings::~CBtHidSettings()
    {}

// ----------------------------------------------------
// CBtHidSettings::LoadLayoutSetting()
// ----------------------------------------------------
//
EXPORT_C THidKeyboardLayoutId CBtHidSettings::LoadLayoutSetting()
    {
    TInt layout;

    TInt err = GetValue( KBtHidKeyboardLayout, layout );
    if( KErrNone != err)
        {
        TRACE_INFO( (_L("[HID]\tGetValue() error (%d)\n"), err));
        layout = 0;
        }

    // Convert the Int to the enum.
    THidKeyboardLayoutId layoutID =
        static_cast<THidKeyboardLayoutId>(layout);

    TRACE_INFO( (_L("[HID]\tCBtHidSettings::LoadLayoutSetting()(%d)\n"),
                 layoutID));
    return layoutID;
    }

// ----------------------------------------------------
// CBtHidSettings::SaveLayoutSettingL
// ----------------------------------------------------
//
EXPORT_C void CBtHidSettings::SaveLayoutSettingL(THidKeyboardLayoutId aNewValue) const
    {
    User::LeaveIfError( SetValue( KBtHidKeyboardLayout, aNewValue ) );
    }

// -----------------------------------------------------------------------------
// CBtHidSettings::SetValue
// Sets integer value to CenRep
// -----------------------------------------------------------------------------
//
TInt CBtHidSettings::SetValue( const TUint32 aKey, const TInt aValue ) const
    {
    TRACE_INFO(_L("[BTHID]\t CBtHidSettings::SetValue"));
    CRepository* cenrep = NULL;
    TRAPD(status, cenrep = CRepository::NewL(KCRUidBTEngPrivateSettings));
    if( status == KErrNone)
        {
        status = cenrep->Set( aKey, aValue );
        delete cenrep;
        }
    if( status != KErrNone)
        {
        TRACE_INFO(_L("Cenrep fail"));
        }
    return status;
    }


// -----------------------------------------------------------------------------
// CBtHidSettings::GetValue
// Gets integer value from CenRep
// -----------------------------------------------------------------------------
//
TInt CBtHidSettings::GetValue( const TUint32 aKey, TInt& aValue )
    {
    TRACE_INFO(_L("[BTHID]\t CBtHidSettings::GetValue"));
    CRepository* cenrep = NULL;
    TRAPD( status , cenrep = CRepository::NewL(KCRUidBTEngPrivateSettings));
    if( status == KErrNone)
        {
        status = cenrep->Get( aKey, aValue );
        delete cenrep;
        }
    if( status != KErrNone)
        {
        TRACE_INFO(_L("Cenrep fail"));
        }

    TRACE_FUNC_EXIT
    return status;
    }

// End of File



