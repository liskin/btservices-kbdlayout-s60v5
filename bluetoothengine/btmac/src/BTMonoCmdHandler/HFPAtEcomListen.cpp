/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  ECom plugin install/uninstall/version listener
*
*/


#include "HFPAtEcomListen.h"
#include "debug.h"

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CHFPAtEcomListen* CHFPAtEcomListen::NewL(
    RATExt* aAtCmdExt,
    MHFPAtEcomListen* aCallback )
    {
    CHFPAtEcomListen* self = NewLC( aAtCmdExt, aCallback );
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// Two-phased constructor.
// ---------------------------------------------------------------------------
//
CHFPAtEcomListen* CHFPAtEcomListen::NewLC(
    RATExt* aAtCmdExt,
    MHFPAtEcomListen* aCallback )
    {
    CHFPAtEcomListen* self = new (ELeave) CHFPAtEcomListen( aAtCmdExt,
                                                            aCallback );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor.
// ---------------------------------------------------------------------------
//
CHFPAtEcomListen::~CHFPAtEcomListen()
    {
    TRACE_INFO( _L("CHFPAtEcomListen::~CHFPAtEcomListen()") );
    ResetData();
    TRACE_INFO( _L("CHFPAtEcomListen::~CHFPAtEcomListen() complete") );
    }

// ---------------------------------------------------------------------------
// Resets data to initial values
// ---------------------------------------------------------------------------
//
void CHFPAtEcomListen::ResetData()
    {
    TRACE_INFO( _L("CHFPAtEcomListen::ResetData()") );
    // APIs affecting this:
    // IssueRequest()
    Stop();
    // Internal
    Initialize();
    TRACE_INFO( _L("CHFPAtEcomListen::ResetData() complete") );
    }

// ---------------------------------------------------------------------------
// Starts waiting for ECom plugin install/uninstall/version status changes
// ---------------------------------------------------------------------------
//
TInt CHFPAtEcomListen::IssueRequest()
    {
    TRACE_INFO( _L("CHFPAtEcomListen::IssueRequest()") );
    if ( iEcomListenState != EHFPStateIdle )
        {
        TRACE_INFO( _L("CHFPAtEcomListen::IssueRequest() (not ready) complete") );
        return KErrNotReady;
        }
    iStatus = KRequestPending;
    iAtCmdExt->ReceiveEcomPluginChange( iStatus, iPluginUidPckg, iEcomTypePckg );
    SetActive();
    iEcomListenState = EHFPStateEcomListening;
    TRACE_INFO( _L("CHFPAtEcomListen::IssueRequest() complete") );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// Stops waiting for Ecom plugin install/uninstall/version status changes
// ---------------------------------------------------------------------------
//
TInt CHFPAtEcomListen::Stop()
    {
    TRACE_INFO( _L("CHFPAtEcomListen::Stop()") );
    if ( iEcomListenState != EHFPStateEcomListening )
        {
        TRACE_INFO( _L("CHFPAtEcomListen::Stop() (not ready) complete" ));
        return KErrNotReady;
        }
    iAtCmdExt->CancelReceiveEcomPluginChange();
    Cancel();
    iEcomListenState = EHFPStateIdle;
    TRACE_INFO( _L("CHFPAtEcomListen::Stop() complete") );
    return KErrNone;
    }

// ---------------------------------------------------------------------------
// CHFPAtEcomListen::CHFPAtEcomListen
// ---------------------------------------------------------------------------
//
CHFPAtEcomListen::CHFPAtEcomListen( RATExt* aAtCmdExt,
                                    MHFPAtEcomListen* aCallback ) :
    CActive( EPriorityHigh ),
    iAtCmdExt( aAtCmdExt ),
    iCallback( aCallback ),
    iPluginUidPckg( iPluginUid ),
    iEcomTypePckg( EEcomTypeUninstall )
    {
    Initialize();
    }

// ---------------------------------------------------------------------------
// CHFPAtEcomListen::ConstructL
// ---------------------------------------------------------------------------
//
void CHFPAtEcomListen::ConstructL()
    {
    TRACE_INFO( _L("CHFPAtEcomListen::ConstructL()") );
    if ( !iAtCmdExt || !iCallback )
        {
        User::Leave( KErrGeneral );
        }
    CActiveScheduler::Add( this );
    TRACE_INFO( _L("CHFPAtEcomListen::ConstructL() complete") );
    }

// ---------------------------------------------------------------------------
// Initializes this class
// ---------------------------------------------------------------------------
//
void CHFPAtEcomListen::Initialize()
    {
    // Don't initialize iAtCmdExt here (it is set through NewL)
    // Don't initialize iCallback here (it is set through NewL)
    iEcomListenState = EHFPStateIdle;
    iPluginUid = TUid::Null();
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called when plugin installed, uninstalled or changed
// ---------------------------------------------------------------------------
//
void CHFPAtEcomListen::RunL()
    {
    TRACE_INFO( _L("CHFPAtEcomListen::RunL()") );
    iEcomListenState = EHFPStateIdle;
    TInt retTemp = iStatus.Int();
    if ( retTemp != KErrNone )
        {
        //TRACE_INFO( _L("CHFPAtEcomListen::RunL() (ERROR) complete (%d)"), retTemp );
        return;
        }
    IssueRequest();
    // Now the operation is either unintall or install of plugin
    // Notify parent and ATEXT
    if ( iEcomTypePckg() == EEcomTypeUninstall )
        {
        iCallback->NotifyPluginUninstallation( iPluginUid );
        }
    else if ( iEcomTypePckg() == EEcomTypeInstall )
        {
        iCallback->NotifyPluginInstallation( iPluginUid );
        }
    else
        {
        TRACE_INFO( _L("CHFPAtEcomListen::RunL() (not supported) complete") );
        }
    TRACE_INFO( _L("CHFPAtEcomListen::RunL() complete") );
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Gets called on cancel
// ---------------------------------------------------------------------------
//
void CHFPAtEcomListen::DoCancel()
    {
    TRACE_INFO( _L("CHFPAtEcomListen::DoCancel()") );
    TRACE_INFO( _L("CHFPAtEcomListen::DoCancel() complete") );
    }
