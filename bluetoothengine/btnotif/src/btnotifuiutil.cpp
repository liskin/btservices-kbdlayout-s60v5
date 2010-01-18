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
* Description:  Declares Utility to show UI notes and queries. 
*
*/

#include <bautils.h>         // BAFL utils (for language file)
#include "btnotifuiutil.h"
#include "BTNotifDebug.h"    // Debugging macros
#include <stringloader.h>    // Localisation stringloader
#include <AknNoteWrappers.h> // Information note
#include <aknmessagequerydialog.h> // for the blocking query
#include <aknmediatorfacade.h>  // CoverUI 
#include <CoreApplicationUIsDomainPSKeys.h>  // Backlight control
#include <AknNotiferAppServerApplication.h>  // Application Key enable/disable

CBTNotifUIUtil* CBTNotifUIUtil::NewL( TBool aCoverDisplayEnabled )
    {
    CBTNotifUIUtil* self = new (ELeave) CBTNotifUIUtil( aCoverDisplayEnabled );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

CBTNotifUIUtil::CBTNotifUIUtil( TBool aCoverDisplayEnabled ) :
    iCoverDisplayEnabled( aCoverDisplayEnabled ), 
    iSystemCancel( EFalse ), 
    iAppKeyBlocked( EFalse )
    {
    }

void CBTNotifUIUtil::ConstructL()
    {
    iEikEnv = CEikonEnv::Static();

    TFileName filename;
    filename += KFileDrive;
    filename += KDC_RESOURCE_FILES_DIR; 
    filename += KResourceFileName;
    BaflUtils::NearestLanguageFile(iEikEnv->FsSession(),filename);
    iResourceFileFlag=iEikEnv->AddResourceFileL(filename);
    }

CBTNotifUIUtil::~CBTNotifUIUtil()
    {
    iSystemCancel = ETrue;
    delete iQueryDlg;
    delete iWaitDlg;
    // Activate apps key again before exit, in case any interruption happens before activation   
    if( iAppKeyBlocked )
        {
        FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil destructor Activate apps key before exiting. "));
        (void) ((CAknNotifierAppServerAppUi*)iEikEnv->EikAppUi())->SuppressAppSwitching(EFalse);    
        }
    iEikEnv->DeleteResourceFile(iResourceFileFlag);
    }
// ----------------------------------------------------------
// CBTNotifierBase::TurnLightsOn
// ----------------------------------------------------------
//
void CBTNotifUIUtil::TurnLightsOn()
    { 
    // Change the bit on and off. SysAp will detect that
    // the lights should be switched on for the specified time.
    //
    TInt err = KErrNone;
    
    err = RProperty::Set(KPSUidCoreApplicationUIs, KLightsControl, ELightsOn);
    err = RProperty::Set(KPSUidCoreApplicationUIs, KLightsControl, ELightsOff);
       
    if ( err != KErrNone )
        {
        FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifUIUtil::TurnLightsOn() RProperty::Set return error %d"), err ));
        }
           
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::TurnLightsOn() <<"));
    }

// ----------------------------------------------------------
// CBTNotifierBase::LocalEikonEnv
// ----------------------------------------------------------
//
CEikonEnv& CBTNotifUIUtil::LocalEikonEnv()
    {
    return *iEikEnv;
    }

// ----------------------------------------------------------
// CBTNotifierBase::ShowQueryL
// ----------------------------------------------------------
//
TInt CBTNotifUIUtil::ShowQueryL(TInt aPromptResource, TInt aExecuteResource, 
        TSecondaryDisplayBTnotifDialogs aDialogId, CAknQueryDialog::TTone aTone )
    {
    TInt ret;
    TBTDeviceName name( KNullDesC );
    if ( aPromptResource )
        {
        HBufC* prompt = StringLoader::LoadLC( aPromptResource );
        ret = ShowQueryL(*prompt, aExecuteResource, aDialogId, name, aTone );
        CleanupStack::PopAndDestroy( prompt );
        }
    else
        {
        ret = ShowQueryL(KNullDesC, aExecuteResource, aDialogId, name, aTone );
        }
    return ret;
    }

TInt CBTNotifUIUtil::ShowQueryL(const TDesC& aPrompt, TInt aExecuteResource, 
        TSecondaryDisplayBTnotifDialogs aDialogId, const TBTDeviceName& aDevNameInCoverUi, CAknQueryDialog::TTone aTone )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowQueryL >>"));
    __ASSERT_DEBUG( iQueryDlg == NULL, 
            User::Panic(_L("CBTNotifier iQueryDlg not released!"),KErrAlreadyExists));

    iQueryDlg = CAknQueryDialog::NewL(aTone);
    if( iCoverDisplayEnabled && aDialogId != ECmdBTnotifUnavailable)
        {
        // initializes cover support
        iQueryDlg->PublishDialogL(aDialogId, KUidCoverUiCategoryBTnotif );
        CoverUIDisplayL( iQueryDlg, aDevNameInCoverUi );
        }
 
    // Deactivate apps key
    // we can do nothing if this operation fails
    (void) ((CAknNotifierAppServerAppUi*)iEikEnv->EikAppUi())->SuppressAppSwitching(ETrue);     
    iAppKeyBlocked = ETrue;
    TurnLightsOn();
    iQueryDlg->SetFocus( ETrue );

    TInt keypress;
    if ( aPrompt.Length() )
        {
        keypress = iQueryDlg->ExecuteLD(aExecuteResource, aPrompt );
        }
    else
        {
        keypress = iQueryDlg->ExecuteLD( aExecuteResource );
        }
    
    iQueryDlg = NULL;
    if( !iSystemCancel )
        {    // Activate apps key when no external cancellation happens
        (void) ((CAknNotifierAppServerAppUi*)iEikEnv->EikAppUi())->SuppressAppSwitching(EFalse);
        iAppKeyBlocked = EFalse;
        }
    
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowQueryL keypress %d <<"), keypress ) );
    return keypress;
    }

TInt CBTNotifUIUtil::ShowMessageQueryL(TDesC& aMessage, const TDesC& aHeader,
        TInt aResourceId, CAknQueryDialog::TTone aTone )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowMessageQueryL >>"));
    __ASSERT_DEBUG( iQueryDlg == NULL, 
            User::Panic(_L("CBTNotifier iQueryDlg not released!"),KErrAlreadyExists));

    iQueryDlg = CAknMessageQueryDialog::NewL( aMessage, aTone );
    iQueryDlg->PrepareLC( aResourceId );
     if( aHeader.Length() )
         {
         static_cast<CAknMessageQueryDialog*>(iQueryDlg)->Heading()->SetTextL( aHeader );
         }
    
    // Deactivate apps key
    // we can do nothing if this operation fails
    (void) ((CAknNotifierAppServerAppUi*)iEikEnv->EikAppUi())->SuppressAppSwitching(ETrue); 
    iAppKeyBlocked = ETrue;
    TurnLightsOn();
    iQueryDlg->SetFocus( ETrue );
    TInt keypress = iQueryDlg->RunLD();  
    iQueryDlg = NULL;
    
    if( !iSystemCancel )
        { // Activate apps key when no external cancellation happens
        (void) ((CAknNotifierAppServerAppUi*)iEikEnv->EikAppUi())->SuppressAppSwitching(EFalse);
        iAppKeyBlocked = EFalse;
        }
    
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowMessageQueryL keypress %d <<"), keypress ) );
    return keypress;
    }


TInt CBTNotifUIUtil::ShowTextInputQueryL(TDes& aText, TInt aExecuteResource, 
        TSecondaryDisplayBTnotifDialogs aDialogId, 
        CAknQueryDialog::TTone aTone )
    {
    return ShowTextInputQueryL(aText, KNullDesC, aExecuteResource, aDialogId, aTone);
    }

TInt CBTNotifUIUtil::ShowTextInputQueryL(TDes& aText, const TDesC& aPrompt, TInt aExecuteResource, 
        TSecondaryDisplayBTnotifDialogs aDialogId, 
        CAknQueryDialog::TTone aTone )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowTextInputQueryL >>"));
    
    __ASSERT_DEBUG( iQueryDlg == NULL, 
            User::Panic(_L("CBTNotifier iQueryDlg not released!"),KErrAlreadyExists));

    iQueryDlg = CAknTextQueryDialog::NewL( aText, aTone );
    if( iCoverDisplayEnabled && aDialogId != ECmdBTnotifUnavailable)
        {
        // initializes cover support
        iQueryDlg->PublishDialogL(aDialogId, KUidCoverUiCategoryBTnotif );
        }
    if ( aPrompt.Length() )
        {
        iQueryDlg->SetPromptL( aPrompt );
        }
    
    // Deactivate apps key
    // we can do nothing if this operation fails
    (void) ((CAknNotifierAppServerAppUi*)iEikEnv->EikAppUi())->SuppressAppSwitching(ETrue);   
    iAppKeyBlocked = ETrue;
    TurnLightsOn();
    iQueryDlg->SetFocus( ETrue );
    TInt keypress = iQueryDlg->ExecuteLD( aExecuteResource );
    iQueryDlg = NULL;
    
    if( !iSystemCancel )
        { // Activate apps key when no external cancellation happens
        (void) ((CAknNotifierAppServerAppUi*)iEikEnv->EikAppUi())->SuppressAppSwitching(EFalse);
        iAppKeyBlocked = EFalse;
        }
    
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowTextInputQueryL txt '%S' keypress %d <<"), 
            &aText, keypress ) );
    return keypress;    
    }

void CBTNotifUIUtil::ShowInfoNoteL(
        TInt aResourceId, TSecondaryDisplayBTnotifDialogs aDialogId )
    {
    // Load a string, create and show a note
    HBufC* stringHolder = StringLoader::LoadLC( aResourceId );
    ShowInfoNoteL( *stringHolder, aDialogId );
    CleanupStack::PopAndDestroy(); //stringHolder    
    }


void CBTNotifUIUtil::ShowInfoNoteL(
        const TDesC& aResource, TSecondaryDisplayBTnotifDialogs aDialogId )
    {
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowInfoNoteL  '%S' >> "), 
            &aResource ) ); 
    CAknInformationNote* note = new (ELeave) CAknInformationNote(); 
    if(iCoverDisplayEnabled && aDialogId != ECmdBTnotifUnavailable)
        {
        CleanupStack::PushL( note );
        // initializes cover support
        note->PublishDialogL(aDialogId, KUidCoverUiCategoryBTnotif ); 
        CleanupStack::Pop(note);
        }
    note->ExecuteLD( aResource );
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowInfoNoteL done <<"));
    }

void CBTNotifUIUtil::ShowConfirmationNoteL( TBool aWaitingDlg, const TDesC& aResource, 
        TSecondaryDisplayBTnotifDialogs aDialogId, const TBTDeviceName& aDevNameInCoverUi )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowConfirmationNoteL >>"));
    CAknConfirmationNote* note = new (ELeave) CAknConfirmationNote( aWaitingDlg );

    if(iCoverDisplayEnabled && aDialogId != ECmdBTnotifUnavailable)
        {
        CleanupStack::PushL( note );
        // initializes cover support
        note->PublishDialogL(aDialogId, KUidCoverUiCategoryBTnotif ); 
        CoverUIDisplayL( note, aDevNameInCoverUi );
        CleanupStack::Pop(note);
        }
    TurnLightsOn();
    note->ExecuteLD( aResource );
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowConfirmationNoteL done <<"));
    }

void CBTNotifUIUtil::ShowErrorNoteL( TBool aWaitingDlg, const TDesC& aResource, 
        TSecondaryDisplayBTnotifDialogs aDialogId, TBTDeviceName aDevNameInCoverUi )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowErrorNoteL >>"));
    CAknErrorNote* note = new (ELeave) CAknErrorNote( aWaitingDlg );
    if(iCoverDisplayEnabled && aDialogId != ECmdBTnotifUnavailable)
        {
        CleanupStack::PushL( note );
        // initializes cover support
        note->PublishDialogL(aDialogId, KUidCoverUiCategoryBTnotif ); 
        CoverUIDisplayL( note, aDevNameInCoverUi );
        CleanupStack::Pop(note);
        }
    TurnLightsOn();
    note->ExecuteLD( aResource );
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowErrorNoteL done <<"));
    }

// ----------------------------------------------------------
// CBTNotifUIUtil::IsQueryReleased
// Check if iQueryDlg is NULL
// ----------------------------------------------------------
// 
TBool CBTNotifUIUtil::IsQueryReleased()
    {
    return ( iQueryDlg ) ? EFalse : ETrue;   
    }

// ----------------------------------------------------------
// CBTNotifUIUtil::UpdateQueryDlgL
// Update Query with specified info, called by UpdateL()
// ----------------------------------------------------------
// 
void CBTNotifUIUtil::UpdateQueryDlgL( TDesC& aMessage )
    {
    __ASSERT_DEBUG( iQueryDlg != NULL, 
                User::Panic(_L("CBTNotifier iQueryDlg not exist to update!"),KErrNotFound));
    iQueryDlg->SetPromptL( aMessage );
    iQueryDlg->DrawDeferred();
    }

// ----------------------------------------------------------
// CBTNotifUIUtil::UpdateMessageQueryDlgL
// Update message query when RNotifier::UpdateL() is called
// ----------------------------------------------------------
// 
void CBTNotifUIUtil::UpdateMessageQueryDlgL( TDesC& aMessage )
    {
    __ASSERT_DEBUG( iQueryDlg != NULL, 
                User::Panic(_L("CBTNotifier CAknTextQueryDialog iQueryDlg not exist to update!"),KErrNotFound));
    static_cast<CAknMessageQueryDialog*>(iQueryDlg)->SetMessageTextL( aMessage );
    iQueryDlg->DrawDeferred();
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::UpdateMessageQueryDlg "));
    }

// ----------------------------------------------------------
// CBTNotifUIUtil::UpdateCoverUiL
// Update secondary display 
// ----------------------------------------------------------
// 
void CBTNotifUIUtil::UpdateCoverUiL( const TDesC8& aMessage )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::UpdateCoverUiL >>"));
    __ASSERT_DEBUG( iQueryDlg != NULL, 
                    User::Panic(_L("CBTNotifier iQueryDlg not exist to update!"),KErrNotFound));
    if(iCoverDisplayEnabled )
        {                                        
        CAknMediatorFacade* coverData = AknMediatorFacade(iQueryDlg);
        if(coverData)
            { 
            coverData->ResetBuffer();
            coverData->BufStream() << aMessage; // insert your buffer here
            coverData->BufStream().CommitL();
            coverData->PostUpdatedDataL();
            }
        }
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::UpdateCoverUiL done <<"));
    }

void CBTNotifUIUtil::DismissDialog()
    {
    if( !IsQueryReleased() )
        {
        delete iQueryDlg;
        iQueryDlg = NULL;
        }
    }

void CBTNotifUIUtil::CoverUIDisplayL( const MObjectProvider* aMop, 
        const TBTDeviceName& aDevNameInCoverUi )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::CoverUIDisplayL() >>"));
    
    CAknMediatorFacade* covercl = AknMediatorFacade( aMop ); 
    if( covercl && aDevNameInCoverUi.Length() )
        {
        covercl->BufStream() << aDevNameInCoverUi;
        covercl->BufStream().CommitL(); // no more data to send so commit buf
        }
    
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::CoverUIDisplayL() <<"));
    }

TInt CBTNotifUIUtil::ShowWaitDlgL( TInt aResourceId )
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowWaitDlgL() >>"));
    iWaitDlg = new(ELeave) CAknWaitDialog((REINTERPRET_CAST(CEikDialog**,&iWaitDlg)), ETrue );
    TInt ret = iWaitDlg->ExecuteLD(aResourceId);
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::ShowWaitDlgL() <<"));
    return ret;
    }

void CBTNotifUIUtil::CompleteWaitDlgL()
    {
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::CompleteWaitDlgL() >>"));
    if(iWaitDlg)
        {
        iWaitDlg->ProcessFinishedL();
        }
    FLOG(_L("[BTNOTIF]\t CBTNotifUIUtil::CompleteWaitDlgL() <<"));
    }
