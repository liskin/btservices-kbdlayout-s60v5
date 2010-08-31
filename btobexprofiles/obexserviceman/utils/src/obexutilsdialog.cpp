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
* Description: 
*
*/


// INCLUDE FILES
#include "obexutilsdialog.h"
#include "obexutilsdialogtimer.h"
#include "obexutilsdebug.h"
#include <hbdevicenotificationdialogsymbian.h>
#include <btservices/bluetoothdevicedialogs.h>
#include <hbtextresolversymbian.h>

_LIT(KSendingDialog, "com.nokia.hb.btdevicedialog/1.0");
_LIT(KCurrentFileIndex,"currentFileIdx" );
_LIT(KTotalFileCount, "totalFilesCnt");
_LIT(KDestinationName, "destinationName");
_LIT(KFileName,"fileName");
_LIT(KFileSizeTxt,"fileSzTxt");
_LIT(KFileSize,"fileSz");
_LIT(KProgressValue,"progressValue");

_LIT(KSendingCancelledText, "txt_bt_dpophead_sending_cancelled");
_LIT(KDeviceText,"txt_bt_dpopinfo_sent_to_1");

const TInt KMaxDescriptionLength = 256;
const TInt KMinStringSize = 10;
const TInt KMinFileSize = 1024;

_LIT(KLocFileName, "btdialogs_");     
_LIT(KPath, "z:/resource/qt/translations/"); 

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CObexUtilsDialog::CObexUtilsDialog( MObexUtilsDialogObserver* aObserverPtr ) : 
    iDialogObserverPtr( aObserverPtr )
    {
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::NewL
// -----------------------------------------------------------------------------
EXPORT_C CObexUtilsDialog* CObexUtilsDialog::NewL( MObexUtilsDialogObserver* aObserverPtr )
    {
    CObexUtilsDialog* self = new ( ELeave ) CObexUtilsDialog( aObserverPtr );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return( self );
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::NewLC
// -----------------------------------------------------------------------------
EXPORT_C CObexUtilsDialog* CObexUtilsDialog::NewLC( MObexUtilsDialogObserver* aObserverPtr )
    {
    CObexUtilsDialog* self = new ( ELeave ) CObexUtilsDialog( aObserverPtr );
    CleanupStack::PushL( self );
    self->ConstructL();
    return( self );
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::ConstructL
// Symbian OS default constructor can leave.
// -----------------------------------------------------------------------------
void CObexUtilsDialog::ConstructL()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::ConstructL()"));

    if (!iDialogObserverPtr)
        {
        // The observer pointer was not given as an argument.
        //
        User::Leave(KErrArgument);
        }
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::ConstructL() completed"));
    } 

// -----------------------------------------------------------------------------
// Destructor
// -----------------------------------------------------------------------------
EXPORT_C CObexUtilsDialog::~CObexUtilsDialog()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::~CObexUtilsDialog()"));
    iDeviceName.Close();
    delete iWaitDialog;
    delete iProgressDialog;
    delete iObexDialogTimer;
    delete iMessageBox;
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::~CObexUtilsDialog() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::LaunchProgressDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::LaunchProgressDialogL(
    MObexUtilsProgressObserver* aObserverPtr, TInt aFileCount, 
    const TDesC& aDeviceName, TInt aTimeoutValue )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchProgressDialogL()"));
    
    TBuf<KMinStringSize> key;
    if ( aObserverPtr )
        {
        // The observerPtr was given, so store it and start a timer
        //
        iProgressObserverPtr = aObserverPtr;

        if ( !iObexDialogTimer )
            {
            iObexDialogTimer = CObexUtilsDialogTimer::NewL( this );
            }
        iObexDialogTimer->Cancel();
        iObexDialogTimer->SetTimeout( aTimeoutValue );
        }

    iFileCount = aFileCount;
    iDeviceName.Close();
    iDeviceName.CreateL(aDeviceName);
    iProgressDialog = CHbDeviceDialogSymbian::NewL();
    
    CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
    CleanupStack::PushL(map);
    TInt data = TBluetoothDialogParams::ESend;
    key.Num(TBluetoothDialogParams::EDialogType);
    AddDataL( map, key, &data, CHbSymbianVariant::EInt );
    User::LeaveIfError(iProgressDialog->Show(KSendingDialog(),*map,this));
    CleanupStack::PopAndDestroy(map);    

    if ( iProgressObserverPtr )
        {
        iObexDialogTimer->Tickle();
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchProgressDialogL() completed"));    
    }


EXPORT_C void CObexUtilsDialog::UpdateProgressNoteL( TInt aFileSize,TInt aFileIndex, const TDesC& aFileName )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressNoteL()"));  
    HBufC* key = HBufC::NewL(KMaxDescriptionLength);
    CleanupStack::PushL(key);
    
    CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
    CleanupStack::PushL(map);
    
    iFileIndex = aFileIndex+1;
    key->Des().Copy(KCurrentFileIndex());
    AddDataL( map, *key, &iFileIndex, CHbSymbianVariant::EInt );
    
    key->Des().Copy(KTotalFileCount());
    AddDataL( map, *key, &iFileCount, CHbSymbianVariant::EInt );

    key->Des().Copy(KDestinationName());
    AddDataL( map, *key, &iDeviceName, CHbSymbianVariant::EDes );
    
    key->Des().Copy(KFileName());
    AddDataL( map, *key, &aFileName, CHbSymbianVariant::EDes );
    
    // todo: localiation is needed for code below:
    HBufC* value = HBufC::NewL(KMaxDescriptionLength);
    CleanupStack::PushL(value);
    key->Des().Copy(KFileSizeTxt());
    value->Des().Zero();
    if(aFileSize < KMinFileSize)
        {
        value->Des().AppendNum(aFileSize);
        value->Des().Append(_L(" Bytes"));
        }
    else
        {
        TInt filesize =  aFileSize/KMinFileSize;
        value->Des().AppendNum(filesize);
        value->Des().Append(_L(" KB"));
        }
    AddDataL( map, *key, value, CHbSymbianVariant::EDes );
    CleanupStack::PopAndDestroy( value );
    
    key->Des().Copy(KFileSize());
    AddDataL( map, *key, &aFileSize, CHbSymbianVariant::EInt );
    TInt ret = iProgressDialog->Update(*map);
    CleanupStack::PopAndDestroy(map);
    CleanupStack::PopAndDestroy(key);
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressNoteL()Completed"));  
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::LaunchWaitDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::LaunchWaitDialogL( const TDesC& aDisplayText )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchWaitDialogL()"));

    if ( iWaitDialog || iProgressDialog )
        {
        // Allow only one dialog at a time
        //
        User::Leave( KErrInUse );
        }
    iWaitDialog = CHbDeviceProgressDialogSymbian::NewL(CHbDeviceProgressDialogSymbian::EWaitDialog,this);
    iWaitDialog->SetTextL(aDisplayText);
    iWaitDialog->ShowL();

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchWaitDialogL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::CancelWaitDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::CancelWaitDialog()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::CancelWaitDialogL()"));

    if( iWaitDialog )
        {
        iWaitDialog->Close();
        delete iWaitDialog;
        iWaitDialog = NULL;
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::CancelWaitDialogL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::CancelProgressDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::CancelProgressDialog()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::CancelProgressDialogL()"));

    if( iProgressDialog )
        {        
        iProgressDialog->Cancel();
        delete iProgressDialog;
        iProgressDialog = NULL;
        delete iObexDialogTimer;
        iObexDialogTimer = NULL;
        }  
        
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::CancelProgressDialogL() completed"));  
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::UpdateProgressDialogL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::UpdateProgressDialogL( TInt aProgressValue  )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressDialogL()"));

    if ( iProgressDialog )
        {
        HBufC* key = HBufC::NewL(KMaxDescriptionLength);
        CleanupStack::PushL(key);

        CHbSymbianVariantMap* map = CHbSymbianVariantMap::NewL();
        CleanupStack::PushL(map);
        
        key->Des().Copy(KProgressValue());
        AddDataL( map, *key, &aProgressValue, CHbSymbianVariant::EInt );

        HBufC* value = HBufC::NewL(KMaxDescriptionLength);
        CleanupStack::PushL(value);
        key->Des().Copy(KCurrentFileIndex());
        value->Des().AppendNum(iFileIndex);
        AddDataL( map, *key, value, CHbSymbianVariant::EDes );
        CleanupStack::PopAndDestroy(value);
        
        TInt ret = iProgressDialog->Update(*map);
        
        CleanupStack::PopAndDestroy(map);
        CleanupStack::PopAndDestroy(key);    
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressDialogL() completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::UpdateProgressDialog
// -----------------------------------------------------------------------------
void CObexUtilsDialog::UpdateProgressDialog()
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressDialog()"));

    TRAPD( ignoredError, UpdateProgressDialogL( 
        iProgressObserverPtr->GetProgressStatus()) );
        
    if (ignoredError != KErrNone)
        {
        FLOG(_L("Ignore this error"));
        }

    if ( iObexDialogTimer )
        {
        iObexDialogTimer->Tickle();
        }

    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::UpdateProgressDialog() completed"));
    }
    
// -----------------------------------------------------------------------------
// CObexUtilsDialog::LaunchQueryDialogL
// -----------------------------------------------------------------------------
//
EXPORT_C void CObexUtilsDialog::LaunchQueryDialogL( const TDesC& aConfirmText )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsDialog::LaunchQueryDialogL()"));
    if ( iMessageBox )
        {
        //todo: Already one dialog is displayed, currently not supported for 
        //multiple dialogs at the same time.
        User::Leave( KErrGeneral );
        }
    iMessageBox = CreateAndShowMessageBoxL( CHbDeviceMessageBoxSymbian::EQuestion,
            aConfirmText, this, 0 );
    }
    

// -----------------------------------------------------------------------------
// CObexUtilsUiDialog::ShowErrorNoteL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::ShowErrorNoteL( const TDesC& aTextId )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsUiDialog::ShowErrorNoteL"));
    if ( iMessageBox )
        {
        //todo: Already one dialog is displayed, currently not supported for 
        //multiple dialogs at the same time.
        User::Leave( KErrGeneral );
        }
    iMessageBox = CreateAndShowMessageBoxL( CHbDeviceMessageBoxSymbian::EWarning,
                aTextId, this, 0 );
    FLOG(_L("[OBEXUTILS]\t CObexUtilsUiDialog::ShowErrorNoteL completed"));
    }

// -----------------------------------------------------------------------------
// CObexUtilsUiDialog::ShowInformationNoteL
// -----------------------------------------------------------------------------
EXPORT_C void CObexUtilsDialog::ShowInformationNoteL( const TDesC& aTextId )
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsUiDialog::ShowInformationNoteL"));
    if ( iMessageBox )
        {
        //todo: Already one dialog is displayed, currently not supported for 
        //multiple dialogs at the same time.
        User::Leave( KErrGeneral );
        }
    iMessageBox = CreateAndShowMessageBoxL( CHbDeviceMessageBoxSymbian::EInformation,
                    aTextId, this, 0 );
    FLOG(_L("[OBEXUTILS]\t CObexUtilsUiDialog::ShowInformationNoteL completed"));
    }

void CObexUtilsDialog::ProgressDialogCancelled(const CHbDeviceProgressDialogSymbian* aDialog)
    {
    FLOG(_L("[BTSU]\t CObexUtilsDialog::ProgressDialogCancelled(), cancelled by user"));
    (void) aDialog;
    if ( iDialogObserverPtr )
        {
        iDialogObserverPtr->DialogDismissed(ECancelButton);
        }
    delete iWaitDialog;
    iWaitDialog = NULL;
    }

// -----------------------------------------------------------------------------
// CObexUtilsDialog::MessageBoxClosed
// -----------------------------------------------------------------------------
void  CObexUtilsDialog::MessageBoxClosed(const CHbDeviceMessageBoxSymbian *aMessageBox, 
        CHbDeviceMessageBoxSymbian::TButtonId aButton)
    {
    FLOG(_L("[OBEXUTILS]\t CObexUtilsUiDialog::MessageBoxClosed"));
    (void)aMessageBox;
    delete iMessageBox;
    iMessageBox = NULL;
    if(iDialogObserverPtr)
        {
        //ETrue if user selects Yes, otherwise EFalse.
         iDialogObserverPtr->DialogDismissed( 
                 (aButton == CHbDeviceMessageBoxSymbian::EAcceptButton) ? 
                     EYesButton : ENoButton  );
        }
    FLOG(_L("[OBEXUTILS]\t CObexUtilsUiDialog::MessageBoxClosed completed"));
    }

void CObexUtilsDialog::ProgressDialogClosed(const CHbDeviceProgressDialogSymbian* aDialog)
    {
    (void) aDialog;
    }

void CObexUtilsDialog::DataReceived(CHbSymbianVariantMap& aData)
    {
    (void) aData;
    }

void CObexUtilsDialog::DeviceDialogClosed(TInt aCompletionCode)
    {
    FLOG(_L("[BTSU]\t CObexUtilsDialog::DeviceDialogClosed()"));   
    (void) aCompletionCode;

    delete iObexDialogTimer;
    iObexDialogTimer = NULL;
    delete iProgressDialog;
    iProgressDialog = NULL;  
    
    TBool ok = HbTextResolverSymbian::Init(KLocFileName, KPath);
    if(ok)
        {
        TRAP_IGNORE(
                 HBufC* deviceName = HbTextResolverSymbian::LoadLC(KDeviceText,iDeviceName);
                 HBufC* sendText = HbTextResolverSymbian::LoadLC(KSendingCancelledText);
                 CHbDeviceNotificationDialogSymbian::NotificationL(
                         KNullDesC, deviceName->Des(), sendText->Des());
                 CleanupStack::PopAndDestroy( sendText );		 
                 CleanupStack::PopAndDestroy( deviceName );
                );
        }
    else
        {
        CHbDeviceNotificationDialogSymbian::NotificationL(
                KNullDesC, KDeviceText(), KSendingCancelledText());
        }
    if ( iDialogObserverPtr )
        {
        iDialogObserverPtr->DialogDismissed(ECancelButton);
        }
    }

void CObexUtilsDialog::AddDataL(CHbSymbianVariantMap* aMap, const TDesC& aKey, 
        const TAny* aData, CHbSymbianVariant::TType aDataType)
    {
    CHbSymbianVariant* value = CHbSymbianVariant::NewL(aData, aDataType);
    CleanupStack::PushL( value );
    User::LeaveIfError( aMap->Add( aKey, value ) ); // aMap takes the ownership of value
    CleanupStack::Pop( value );
    }

CHbDeviceMessageBoxSymbian* CObexUtilsDialog::CreateAndShowMessageBoxL(
        CHbDeviceMessageBoxSymbian::TType aType,
        const TDesC& aText, MHbDeviceMessageBoxObserver* aObserver,
        TInt aTimeout )
    {
    CHbDeviceMessageBoxSymbian* messageBox =
            CHbDeviceMessageBoxSymbian::NewL( aType );
    CleanupStack::PushL(messageBox);
    //ToDo: Need to use localised strings.
    messageBox->SetTextL(aText);
    messageBox->SetObserver(aObserver);
    messageBox->SetTimeout(aTimeout);
    messageBox->ShowL();
    CleanupStack::Pop(messageBox);
    return messageBox;
    }

//  End of File  
