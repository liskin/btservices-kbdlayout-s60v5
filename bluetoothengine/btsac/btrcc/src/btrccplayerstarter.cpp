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
* Description:  This class launchs music player, then send play command
*                to the player at 2 seconds timeout.
*
*/


// INCLUDE FILES
#include <apacmdln.h>
#include <apgcli.h>
#include <e32property.h>
#include <remconcoreapitarget.h>
#include "btrccplayerstarter.h"
#include "btaudioremconpskeys.h"
#include "debug.h"

// MODULE DATA STRUCTURES

// mpx playeris name is mpx.exe in 5.0.
// it is musicplayer.exe in 3.2.
_LIT(KMpxPlayerExeName, "mpx.exe");

_LIT(KMpxPlayerSearchPatternBySID, "*102072c3*");

static const TInt KPlayCmdToPlayerDelay = 6000000; // value will be tuned later 

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CPlayerStarter::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CPlayerStarter* CPlayerStarter::NewL()
	{
	CPlayerStarter* self = new (ELeave) CPlayerStarter();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

void CPlayerStarter::SetCoreTarget(CRemConCoreApiTarget& aTarget)
    {
    iCoreTarget = &aTarget;
    }

// -----------------------------------------------------------------------------
// CPlayerStarter::CPlayerStarter
// C++ constructor.
// -----------------------------------------------------------------------------
//
CPlayerStarter::CPlayerStarter()
  :CActive(EPriorityNormal)
    {
    CActiveScheduler::Add(this);
    }

// -----------------------------------------------------------------------------
// CPlayerStarter::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CPlayerStarter::ConstructL()
    {
    TRACE_FUNC
    iTimer.CreateLocal();
    }

// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CPlayerStarter::~CPlayerStarter()
	{
	TRACE_FUNC
	// Starting player is not finished yet, send play command. Just wish the player 
	// can receive it.
	if (IsActive() && iState != ERespondPlayNok)
	    {
        RProperty::Set(KBTAudioRemCon, KBTAudioPlayerControl, 
                EBTAudioResumePlayer);	    
	    }
	Cancel();
	iTimer.Close();
	}

// -----------------------------------------------------------------------------
// CPlayerStarter::RunL
// -----------------------------------------------------------------------------
//
void CPlayerStarter::RunL()
    {
    TRACE_INFO((_L("CPlayerStarter::RunL() state %d, status %d"), iState, iStatus.Int()));
    if (iStatus != KErrNone)
        {
        iState = EIdle;
        return;
        }
    switch (iState)
        {
        case ERespondPlayOk:
            {
            // Response OK has been sent to the remote device, start timer waiting
            // for player's initialization.
            iTimer.After(iStatus, KPlayCmdToPlayerDelay);
            SetActive();
            iState = EPlayMusicTiming;
            TRACE_INFO(_L("schedule PLAY command ..."));
            break;
            }
        case EPlayMusicTiming:
            {
            // Player should have fully up and running. Send PLAY command to it
            // through our internal Play API.
            RProperty::Set(KBTAudioRemCon, KBTAudioPlayerControl, 
                    EBTAudioResumePlayer);
            TRACE_INFO(_L("PLAY command sent to btmonobearer"));
            iState = EIdle;
            break;
            }
        default:
            {
            iState = EIdle;
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CPlayerStarter::RunError
// -----------------------------------------------------------------------------
//
TInt CPlayerStarter::RunError(TInt aError)
    {
    TRACE_FUNC
    (void) aError;
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CPlayerStarter::DoCancel
// -----------------------------------------------------------------------------
//
void CPlayerStarter::DoCancel()
    {
    TRACE_FUNC
    switch (iState)
        {
        case ERespondPlayOk:
        case ERespondPlayNok:
            {
            // This cancel won't cancel the response sending to the remote 
            // device. (RemCon FW's cancel mechanism is only completing client's request.
            // the real operation in its server side won't be stopped.)
            iCoreTarget->Cancel();
            break;
            }
        case EPlayMusicTiming:
            {
            iTimer.Cancel();
            break;
            }
        }
    }
// ---------------------------------------------------------------------------
// A 'play' command has been received.
// ---------------------------------------------------------------------------
//
void CPlayerStarter::MrccatoPlay(
    TRemConCoreApiPlaybackSpeed /* aSpeed */,
    TRemConCoreApiButtonAction aButtonAct )
    {
    TRACE_FUNC
    if ( ( aButtonAct == ERemConCoreApiButtonClick ) ||
        ( aButtonAct == ERemConCoreApiButtonPress ) )
        {
        StartPlayIfNeeded();
        }
    }

// ---------------------------------------------------------------------------
// A command has been received.
// ---------------------------------------------------------------------------
//
void CPlayerStarter::MrccatoCommand(
    TRemConCoreApiOperationId aOperationId,
    TRemConCoreApiButtonAction aButtonAct )
    {
    TRACE_INFO((_L("CPlayerStarter::MrccatoCommand opId %d, button %d"), 
        aOperationId, aButtonAct));
    switch (aOperationId)
        {
        case ERemConCoreApiPlay:
            {
            if ( ( aButtonAct == ERemConCoreApiButtonClick ) ||
                ( aButtonAct == ERemConCoreApiButtonPress ) )
                {
                StartPlayIfNeeded();
                }
            break;
            }
        case ERemConCoreApiStop:
        case ERemConCoreApiPause:
            {
            if ( ( aButtonAct == ERemConCoreApiButtonClick ) ||
                ( aButtonAct == ERemConCoreApiButtonPress ) )
                {
                // if starting player is ongoing, possibly PLAY command has been scheduled
                // need to abort it.
                Cancel();
                }
            break;
            }
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CPlayerStarter::StartPlayIfNeeded
// -----------------------------------------------------------------------------
//
void CPlayerStarter::StartPlayIfNeeded() 
    {
    TRACE_FUNC
    // unfinished business ongoing, eat the event.
    if (IsActive())
        {
        return;
        }
    if (!IsMusicPlayerRunning())
         {
         TRAPD(err, LaunchMusicPlayerL());
         // Send the response of play command to remote device
         iCoreTarget->PlayResponse(iStatus, err);
         SetActive();
         iState = (err) ? ERespondPlayNok : ERespondPlayOk;
         }
    }

TBool CPlayerStarter::IsMusicPlayerRunning()
    {
    TRACE_FUNC
    // Music player is running if we can find a thread whose name contains 
    // S60 Music Player's SID.
    TFindThread findt(KMpxPlayerSearchPatternBySID);
    TFullName result;
    TBool running(EFalse);
    if (!findt.Next(result))
        {
        TRACE_INFO((_L("Thread '%S'is found"), &result));
        running = ETrue;
        }
    return running;
    }

void CPlayerStarter::LaunchMusicPlayerL()
    {
    RApaLsSession ls;
    User::LeaveIfError(ls.Connect());
    CleanupClosePushL(ls);
    CApaCommandLine* cmd = CApaCommandLine::NewL();
    CleanupStack::PushL(cmd);
    cmd->SetExecutableNameL( KMpxPlayerExeName );
    cmd->SetCommandL( EApaCommandOpen );
    User::LeaveIfError(ls.StartApp( *cmd ));
    CleanupStack::PopAndDestroy(2); // cmd, ls
    }


//  End of File  
