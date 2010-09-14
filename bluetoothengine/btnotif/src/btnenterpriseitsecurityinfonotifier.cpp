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
* Description:  Bluetooth visibility timeout notifier class.
*
*/

// INCLUDE FILES
#include <BTNotif.rsg>								// Own resources
#include "btnenterpriseitsecurityinfonotifier.h"	// Own class definition
#include "btNotifDebug.h"							// Debugging macros
#include <tulstringresourcereader.h>
#include <e32cmn.h>
#include <dcmo.rsg>

_LIT(KDcmoResourceFileName, "z:dcmo.rsc");
_LIT(KNewLine, "\n");

//"Bluetooth" is 9 characters long
const TInt KDefaultBluetoothStringLength = 9;

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CBTEnterpriseItSecurityInfoNotifier::NewL
// Two-phased constructor.
// ----------------------------------------------------------
//
CBTEnterpriseItSecurityInfoNotifier* CBTEnterpriseItSecurityInfoNotifier::NewL()
    {
    CBTEnterpriseItSecurityInfoNotifier* self=new (ELeave) CBTEnterpriseItSecurityInfoNotifier();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// ----------------------------------------------------------
// CBTEnterpriseItSecurityInfoNotifier::CBTEnterpriseItSecurityInfoNotifier
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------
//
CBTEnterpriseItSecurityInfoNotifier::CBTEnterpriseItSecurityInfoNotifier() 
    {
    }

// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------
//
CBTEnterpriseItSecurityInfoNotifier::~CBTEnterpriseItSecurityInfoNotifier()
    {
    Cancel();
    }

// ----------------------------------------------------------
// CBTEnterpriseItSecurityInfoNotifier::RegisterL
// Register notifier.
// ----------------------------------------------------------
//
CBTEnterpriseItSecurityInfoNotifier::TNotifierInfo CBTEnterpriseItSecurityInfoNotifier::RegisterL()
    {
    iInfo.iUid = KBTEnterpriseItSecurityInfoNotifierUid;
    iInfo.iChannel = KBTEnterpriseItSecurityInfoNotifierUid;
    iInfo.iPriority = ENotifierPriorityHigh;
    return iInfo;
    }

// ----------------------------------------------------------
// CBTEnterpriseItSecurityInfoNotifier::StartL
// Synchronic notifier launch. Contructs and shows a global
// note when temp visibility expire, no parameters need here.
// ----------------------------------------------------------
//
TPtrC8 CBTEnterpriseItSecurityInfoNotifier::StartL(const TDesC8& /*aBuffer*/)
    {
    FLOG(_L("[BTNOTIF]\t CBTEnterpriseItSecurityInfoNotifier::StartL()"));
    
    ShowNoteAndCompleteL();

	FLOG(_L("[BTNOTIF]\t CBTEnterpriseItSecurityInfoNotifier::StartL() completed"));
    return TPtrC8(KNullDesC8);
    }

// ----------------------------------------------------------
// CBTEnterpriseItSecurityInfoNotifier::ProcessStartParamsL
// Mandatory for BT Notifiers when using asynchronous launch. 
// ----------------------------------------------------------
//
void CBTEnterpriseItSecurityInfoNotifier::ProcessStartParamsL()
    {
    //nothing to do here
    }

// ----------------------------------------------------------
// CBTEnterpriseItSecurityInfoNotifier::ShowNoteAndCompleteL
// Shows the notifier in backround 
// ----------------------------------------------------------
//
void CBTEnterpriseItSecurityInfoNotifier::ShowNoteAndCompleteL()
	{
	FLOG(_L("[BTNOTIF]\t CBTEnterpriseItSecurityInfoNotifier::ShowNoteAndComplete()"));
	
	//get full path to the DCMO resource file
	TParse* parser = new (ELeave) TParse;
	parser->Set(KDcmoResourceFileName(), &KDC_RESOURCE_FILES_DIR, NULL);
	CleanupStack::PushL(parser);
	TFileName* fileName = new (ELeave) TFileName;
	*fileName = parser->FullName();
	CleanupStack::PopAndDestroy(parser);
	CleanupStack::PushL(fileName);
	
	//create the resource reader object that we need to use several times
	CTulStringResourceReader* reader = CTulStringResourceReader::NewL(*fileName);
	CleanupStack::PushL(reader);
	
	//get pointer to the message part of the notifier
	TPtrC resourceString;
	resourceString.Set(reader->ReadResourceString(R_DM_RUN_TIME_VAR_DISABLE));

	//create descriptor with a max length to fit the localised "disabled" text + new line + "Bluetooth"
	RBuf content;
	content.CreateL(resourceString.Length() + KNewLine().Length() + KDefaultBluetoothStringLength);
	CleanupClosePushL(content);
	
	//add resource string and new line character to the content descriptor
	content.Append(resourceString);	
	content.Append(KNewLine());
	
	//get pointer to the Bluetooth name part of the notifier (can't assume this is actually "Bluetooth" in all languages)
	resourceString.Set(reader->ReadResourceString(R_DM_RUN_TIME_VAR_BLUETOOTH));
	
	//check that the resource string will fit into the content descriptor
	TInt requiredLength = content.Length() + resourceString.Length();
	if (requiredLength > content.MaxLength())
		{
		//allocate more space in the content descriptor
		content.ReAllocL(requiredLength);
		}
	
	//add resource string to the content descriptor
	content.Append(resourceString);	
	
	//display the notifier and complete
	iNotifUiUtil->ShowInfoNoteL(content, ECmdBTnotifUnavailable);
	CompleteMessage(KErrNone);
	
	//pop and destroy the content descriptor, resource reader and file name
	CleanupStack::PopAndDestroy(3, fileName);
	
	FLOG(_L("[BTNOTIF]\t CBTEnterpriseItSecurityInfoNotifier::ShowNoteAndComplete() complete"));
	}

void CBTEnterpriseItSecurityInfoNotifier::HandleGetDeviceCompletedL(const CBTDevice* /*aDev*/)
    {
    //nothing to do here
    }

// End of File
