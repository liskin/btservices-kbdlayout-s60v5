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
* Description:  Methods for BT notifier
*
*/


// INCLUDE FILES

#include <eikenv.h>

#include "btnauthnotifier.h"            // All notifiers
#include "btnpinnotifier.h"
#include "btninqnotifier.h"
#include "btnobexpinnotifier.h"
#include "btnpwrnotifier.h"
#include "btNotifDebug.h"
#include "BTNGenericInfoNotifier.h"   
#include "BTNGenericQueryNotifier.h"   
#include "btnpbappinnotifier.h"
#include "btnpaireddevsettnotifier.h"
#include "btnssppasskeyentrynotifier.h"
#include "btnumcmpnotifier.h"
#include "btnenterpriseitsecurityinfonotifier.h"

// CONSTANTS
const TInt KBTNotifierArrayIncrement = 8;


// ================= EXPORTED FUNCTIONS ====================

// ---------------------------------------------------------
//
// Instantiate notifiers
//
// ---------------------------------------------------------

LOCAL_C void CreateBTNotifiersL( CArrayPtrFlat<MEikSrvNotifierBase2>* aNotifiers )
    {
    FLOG(_L("[BTNOTIF]\t CreateBTNotifiersL"));

    CBTInqNotifier* inquiryNotifier = CBTInqNotifier::NewL();
    CleanupStack::PushL( inquiryNotifier );
    aNotifiers->AppendL( inquiryNotifier );
    CleanupStack::Pop( inquiryNotifier );

    CBTPinNotifier* pinNotifier = CBTPinNotifier::NewL();
    CleanupStack::PushL( pinNotifier );
    aNotifiers->AppendL( pinNotifier );
    CleanupStack::Pop( pinNotifier );

    CBTAuthNotifier* authNotifier = CBTAuthNotifier::NewL();
    CleanupStack::PushL( authNotifier );
    aNotifiers->AppendL( authNotifier );
    CleanupStack::Pop( authNotifier );

    CBTObexPinNotifier* obexNotifier = CBTObexPinNotifier::NewL();
    CleanupStack::PushL( obexNotifier );
    aNotifiers->AppendL( obexNotifier );
    CleanupStack::Pop( obexNotifier );

    CBTPwrNotifier* pwrNotifier = CBTPwrNotifier::NewL();
    CleanupStack::PushL( pwrNotifier );
    aNotifiers->AppendL( pwrNotifier );
    CleanupStack::Pop( pwrNotifier );
	
	CBTGenericInfoNotifier* infoNotifier = CBTGenericInfoNotifier::NewL();
	CleanupStack::PushL( infoNotifier );
	aNotifiers->AppendL( infoNotifier );
	CleanupStack::Pop( infoNotifier );

	CBTGenericQueryNotifier* queryNotifier = CBTGenericQueryNotifier::NewL();
	CleanupStack::PushL( queryNotifier );
	aNotifiers->AppendL( queryNotifier );
	CleanupStack::Pop( queryNotifier );
	
    CBTPBAPPinNotifier* pbapNotifier = CBTPBAPPinNotifier::NewL();
    CleanupStack::PushL( pbapNotifier );
    aNotifiers->AppendL( pbapNotifier );
    CleanupStack::Pop( pbapNotifier );

    CBTPairedDevSettNotifier* pdsNotifier = CBTPairedDevSettNotifier::NewL();
    CleanupStack::PushL( pdsNotifier );
    aNotifiers->AppendL( pdsNotifier );
    CleanupStack::Pop( pdsNotifier );

    CBTSSPPasskeyEntryNotifier* sspPinNotifier = CBTSSPPasskeyEntryNotifier::NewL();
    CleanupStack::PushL( sspPinNotifier );
    aNotifiers->AppendL( sspPinNotifier );
    CleanupStack::Pop( sspPinNotifier );

    CBTNumCmpNotifier* numCmpNotifier = CBTNumCmpNotifier::NewL();
    CleanupStack::PushL(numCmpNotifier);
    aNotifiers->AppendL( numCmpNotifier );
    CleanupStack::Pop( numCmpNotifier );

    CBTEnterpriseItSecurityInfoNotifier* enterpriseItSecurityNotifier = CBTEnterpriseItSecurityInfoNotifier::NewL();
    CleanupStack::PushL(enterpriseItSecurityNotifier);
    aNotifiers->AppendL(enterpriseItSecurityNotifier);
    CleanupStack::Pop(enterpriseItSecurityNotifier);
    
    FLOG(_L("[BTNOTIF]\t CreateBTNotifiersL completed"));
    }

// ---------------------------------------------------------
//
// Lib main entry point: Creates a notifiers array.
//
// ---------------------------------------------------------

EXPORT_C CArrayPtr<MEikSrvNotifierBase2>* NotifierArray()
    {
    FLOG(_L("[BTNOTIF]\t NotifierArray"));
    
    CArrayPtrFlat<MEikSrvNotifierBase2>* notifiers = NULL;

	notifiers = 
	           new CArrayPtrFlat<MEikSrvNotifierBase2>( KBTNotifierArrayIncrement );
    if( notifiers )
        {
        TRAPD( err, CreateBTNotifiersL( notifiers ));
        if( err )
            {
            FTRACE(FPrint(_L("[BTNOTIF]\t Notifier creation failure! Error code: %d"), err));
            TInt count = notifiers->Count();
            while(count--)
                (*notifiers)[count]->Release();
            delete notifiers;
            notifiers = NULL;
            }
            
        FLOG(_L("[BTNOTIF]\t NotifierArray completed"));
        }
    return notifiers;
    }


//  End of File



