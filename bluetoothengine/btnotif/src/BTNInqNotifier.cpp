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
* Description:  Declares Bluetooth device inquiry notifier class.
*
*/


// INCLUDE FILES
#include <eikenv.h>          // Eikon environment

#include "btninqnotifier.h"  // Own class definition
#include "btNotifDebug.h"    // Debugging macros
#include <StringLoader.h>    // Localisation stringloader
#include <BTNotif.rsg>          // Own resources
    

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTInqNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTInqNotifier* CBTInqNotifier::NewL()
    {
    CBTInqNotifier* self=new (ELeave) CBTInqNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------
// CBTInqNotifier::CBTInqNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTInqNotifier::CBTInqNotifier()
    {
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTInqNotifier::~CBTInqNotifier()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqNotifier::~CBTInqNotifier()"));
    delete iUi;
    iUi = NULL;
    FLOG(_L("[BTNOTIF]\t CBTInqNotifier::~CBTInqNotifier() completed"));
    }

// ----------------------------------------------------------
// CBTInqNotifier::RegisterL
// ----------------------------------------------------------
//
CBTInqNotifier::TNotifierInfo CBTInqNotifier::RegisterL()
    {
    
    iInfo.iUid=KDeviceSelectionNotifierUid;
    iInfo.iChannel=KBTInquiryChannel;
    iInfo.iPriority=ENotifierPriorityVHigh; // User input level
    return iInfo;
    }

// ----------------------------------------------------------
// CBTInqNotifier::ProcessStartParamsL
// Initialize parameters and launch device search.
// ----------------------------------------------------------
//
void CBTInqNotifier::ProcessStartParamsL()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqNotifier::ProcessStartParamsL()"));

    TBTDeviceClass cod;
    TBTDeviceSelectionParamsPckg pckg;
    pckg.Copy(*iParamBuffer); 

    LaunchInquiryL(pckg().DeviceClass());
    
    FLOG(_L("[BTNOTIF]\t CBTInqNotifier::ProcessStartParamsL() completed "));
    }

// ----------------------------------------------------------
// CBTInqNotifier::NotifyDeviceSearchCompleted
// Notified by InquiryUI, return to caller by completing message
// ----------------------------------------------------------
//
void CBTInqNotifier::NotifyDeviceSearchCompleted(TInt aErr, const TBTDeviceResponseParams& aDevice)
    {
    FTRACE(FPrint( _L("[BTNOTIF]\t CBTInqNotifier::NotifyDeviceSearchCompleted aErr %d"), aErr ));
    CompleteMessage(TBTDeviceResponseParamsPckg( aDevice ), aErr);
    }

// ----------------------------------------------------------
// CBTInqNotifier::Cancel
// Release all own resources (member variables)
// ----------------------------------------------------------
//
void CBTInqNotifier::Cancel()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqNotifier::Cancel()"));
    if( iUi )
        {
        iUi->Cancel();
        delete iUi;
        iUi = NULL;
        }
    CBTNotifierBase::Cancel();
    FLOG(_L("[BTNOTIF]\t CBTInqNotifier::Cancel() completed"));
    }

// ----------------------------------------------------------
// Ensure BT is ON; there are not too many connections; InqUi 
// is launched successfully.
// ----------------------------------------------------------
//
void CBTInqNotifier::LaunchInquiryL(const TBTDeviceClass& aDesiredDevice)
    {
    // Turn BT on if it is not yet.
    if ( !CheckAndSetPowerOnL() )
        {
        CompleteMessage(KErrGeneral);
        return;
        }

    TInt linkCount;
    User::LeaveIfError(RProperty::Get(KPropertyUidBluetoothCategory, 
            KPropertyKeyBluetoothGetPHYCount, linkCount));
    if ( linkCount > 6 )   //support only piconet
        {
        iNotifUiUtil->ShowInfoNoteL( R_BT_BUSY_NOTE, ECmdBTnotifUnavailable );
        User::Leave(KErrInUse);
        }
    
    // launch Inquiry UI
  
    iUi = new (ELeave) CBTInqUI( this, iNotifUiUtil, aDesiredDevice);
   

    iUi->ConstructL();
    }

// End of File
