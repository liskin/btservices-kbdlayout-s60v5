// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

#include "avrcplog.h"
#include "avrcprouter.h"
#include "avrcputils.h"
#include "controlcommand.h"
#include "passthroughhelper.h"
#include "controlbearer.h"


CPassthroughHelper* CPassthroughHelper::NewL(CRcpRouter& aRouter, MRemConControlCommandInterface& aCommandInterface, CDeltaTimer& aTimer)
	{
	CPassthroughHelper* helper = new(ELeave)CPassthroughHelper(aRouter, aCommandInterface, aTimer);
	CleanupStack::PushL(helper);
	helper->ConstructL();
	CleanupStack::Pop(helper);
	return helper;
	}

CPassthroughHelper::~CPassthroughHelper()
	{
	}
	
CPassthroughHelper::CPassthroughHelper(CRcpRouter& aRouter, MRemConControlCommandInterface& aCommandInterface, CDeltaTimer& aTimer)
	: iRouter(aRouter)
	, iCommandInterface(aCommandInterface)
	, iTimer(aTimer)
	{
	}

void CPassthroughHelper::ConstructL()
	{
	}

void CPassthroughHelper::Disconnect()
	{
	LOG_FUNC
	
	// We handle the last command as a special case to ensure the presses and
	// releases are balanced from a clients' point of view.
	if(iPreviousPassthrough)
		{
		iRouter.RemoveFromSendQueue(*iPreviousPassthrough);
		BalanceHandledCommand(*iPreviousPassthrough);
		}
	}

/** Sends a response to the remote device.

Because of the 100ms timeout for responses we send a response to
passthrough commands as soon as we receive them.  This means the real
response from RemCon is currently ignored.  A real response is
only sent if this is a vendor dependent command.

@param aCommand The command to respond to.
@param aErr The result of handling the command.  KErrNone if successful.
			KErrNotSupported if this operation is not supported.
*/		
void CPassthroughHelper::Respond(CControlCommand& aCommand, TInt aErr)
	{
	LOG_FUNC
	aCommand.SetResponseType(aErr);
	iRouter.AddToSendQueue(aCommand);
	}

/** To be called on completion of command handling.

This aggregates the handler's tidying up of a finished
command.

@param aCommand The command to tidy up.
*/	
void CPassthroughHelper::HandledCommand(CControlCommand& aCommand)
	{
	LOG_FUNC

	// If this was the previous passthrough reset it to NULL
	if(iPreviousPassthrough == &aCommand)
		{
		iPreviousPassthrough = NULL;
		}
	
	aCommand.CancelTimer(iTimer);
	aCommand.DecrementUsers();
	}

/** Handles incoming passthrough command.

How the command is handled is dependent on this and the previous command.

Previous command	This command 	Behaviour
Press_op1			Press_op1		discard
Press_op1			Release_op1		press_op1 hold timer not expired - generate click_op1
									press_op1 release timer not expired - generate release_op1
Press_op1			Press_op2		generate release_op1, start hold timer for press_op2
Press_op1			Release_op2		press_op1 hold timer not expired - generate click_op1, discard release_op2
									press_op1 release timer not expired - generate release_op1, discard release_op2
Release_op1			Press_op1		start hold timer for press_op1
Release_op1			Release_op1		discard
Release_op1			Press_op2		start hold timer for press_op2	
Release_op1			Release_op2		discard

@param aCommand The command to handle.
@leave System wide error code.
*/
void CPassthroughHelper::HandlePassthrough(CControlCommand& aCommand)
	{
	LOG_FUNC
	
	// We can't map Vendor Unique passthrough command to clicks as we do
	// not know how the button action is conveyed to the API.  All we can
	// do is try and ensure that we try to generate a release if one is 
	// missing by asking the convert.  We also need to consider them as
	// part of the passthrough history for how other passthrough commands
	// should be interpreted.
	TUint8 thisOpId;
	aCommand.Frame().OperationId(thisOpId);
	if(!aCommand.IsAvrcpPassthrough())
		{
		aCommand.SetClick(EFalse);
		}
	
	// Behaviour depends on previous command
	TUint8 prevOpId;
	if(iPreviousPassthrough)
		{
		__ASSERT_ALWAYS(iPreviousPassthrough->ButtonAct() == AVCPanel::EButtonPress,
	 		AvrcpUtils::Panic(EAvrcpPressNotPreviousPassthroughCommand));

		iPreviousPassthrough->Frame().OperationId(prevOpId);
		}
		
	if(aCommand.ButtonAct() == AVCPanel::EButtonPress)
		{
		// Respond now so we can decrement users of aCommand at will
		Respond(aCommand, KErrNone);		
			
		if(iPreviousPassthrough)
			{
			// previous is press, current is press
			if(prevOpId == thisOpId)
				{
				// prevCommand is for same opId as this one.  Drop this
				// command and reset release timer for prevCommand.
				aCommand.DecrementUsers();
				
				iPreviousPassthrough->CancelTimer(iTimer);
				// If still flagged as a click then the user won't have been notified
				// of the press yet - so do it now.
				if(iPreviousPassthrough->Click())
					{
					iPreviousPassthrough->SetClick(EFalse);
					iPreviousPassthrough->SetCoreButtonAction(ERemConCoreApiButtonPress, ETrue);
					iCommandInterface.MrcciNewCommand(*iPreviousPassthrough, iPreviousPassthrough->ClientId());
					}
				StartReleaseTimer(*iPreviousPassthrough);
				}
			else if(iPreviousPassthrough->Click())
				{
				// prevCommand is click, Hold timer not expired.  Generate click
				// to RemCon then start waiting for click.
				iPreviousPassthrough->CancelTimer(iTimer);
				iPreviousPassthrough->SetCoreButtonAction(ERemConCoreApiButtonClick, ETrue);
				iCommandInterface.MrcciNewCommand(*iPreviousPassthrough, iPreviousPassthrough->ClientId());
				
				HandledCommand(*iPreviousPassthrough);
									
				// Start waiting for click (core api) or release (vendor unique)
				NewPress(aCommand);
				}
			else
				{
				// prevCommand is not click, Release timer not expired.  Generate
				// release to RemCon then start waiting for click.
				BalanceHandledCommand(*iPreviousPassthrough);
						
				// Start waiting for click (core api) or release (vendor unique)
				NewPress(aCommand);
				}
			}
		else
			{
			// No previous command, current is press
			
			// Start waiting for click (core api) or release (vendor unique)
			NewPress(aCommand);
			}	
		}
	else if(iPreviousPassthrough)
		{
		// previous is press, current is release
		if(prevOpId == thisOpId)
			{
			Respond(aCommand, KErrNone);
					
			if(iPreviousPassthrough->Click())
				{
				// Button release for same opId with hold timeout
				aCommand.DecrementUsers(); // Drop this command.
				
				// Cancel hold timer and send the previous command as a click.
				iPreviousPassthrough->CancelTimer(iTimer); 
				iPreviousPassthrough->SetCoreButtonAction(ERemConCoreApiButtonClick, ETrue);
				iCommandInterface.MrcciNewCommand(*iPreviousPassthrough, iPreviousPassthrough->ClientId());
				
				HandledCommand(*iPreviousPassthrough);
				}
			else
				{
				// Button release for same opId, hold timeout expired
				iPreviousPassthrough->CancelTimer(iTimer);
				HandledCommand(*iPreviousPassthrough);
				
				if(aCommand.IsAvrcpPassthrough())
					{
					aCommand.SetCoreButtonAction(ERemConCoreApiButtonRelease, ETrue);
					}
				iCommandInterface.MrcciNewCommand(aCommand, aCommand.ClientId());
				aCommand.DecrementUsers();
				}
			}
		else
			{
			// Drop this release and continue waiting for release to
			// prevCommand.
			Respond(aCommand, KErrNone);
			aCommand.DecrementUsers();
			}
		}
	else
		{
		// No prevCommand, this is a release
		// To get here either:
		// - this is a spurious release
		// - we've given up waiting for this
		
		// Because responses must be made within 100ms we respond before waiting
		// to see what RemCon thinks.
		Respond(aCommand, KErrNone);
		aCommand.DecrementUsers();
		}
	}

void CPassthroughHelper::NewPress(CControlCommand& aCommand)
	{
	LOG_FUNC
	__ASSERT_DEBUG(!iPreviousPassthrough, AVRCP_PANIC(EPreviousPassthroughNonNullReplacing));
	
	iPreviousPassthrough = &aCommand;
	
	if(aCommand.IsAvrcpPassthrough())
		{
		StartHoldTimer(aCommand);
		}
	else
		{
		iCommandInterface.MrcciNewCommand(aCommand, aCommand.ClientId());
		StartReleaseTimer(aCommand);
		}
	}

/** To be called on completion of command handling for an unbalanced press.

This is a special version of the HandledCommand which which assumes the
command has been notified to the user and so balances the notification
of a press with a generated release command.

@param aCommand The command to tidy up.
*/
void CPassthroughHelper::BalanceHandledCommand(CControlCommand& aCommand)
	{
	LOG_FUNC
	// CancelTimer also frees the timer entries, so needs to be called
	// here even though there's no pending entry.
	aCommand.CancelTimer(iTimer);
	
	// If this was the previous passthrough reset it to NULL
	if(iPreviousPassthrough == &aCommand)
		{
		iPreviousPassthrough = NULL;
		}
	
	// Here we handle the case that a press has been informed but a release has yet
	// to be reported, and if it hasn't we provide the notification.
	if(aCommand.ButtonAct() == AVCPanel::EButtonPress && !aCommand.Click())
		{
		if(aCommand.IsAvrcpPassthrough())
			{
			// FIXME I think we might need a new transaction id here?
			TRAPD(err, aCommand.ReSetCoreButtonActionL(ERemConCoreApiButtonRelease, ETrue));
			if (err == KErrNone)
				{
				iCommandInterface.MrcciNewCommand(aCommand, aCommand.ClientId());
				}
				else
				{
				// just silently drop this command, do not send command to remconserv
				}
			}
		else
			{
			// Need see if we can get a valid release from the converter.  Pass
			// it the same AVC Frame as the press, but with the button action 
			// flipped to release.  If we can't do this then there's not a
			// lot we can do.  
			// FIXME
			}
		}
	aCommand.iHandlingLink.Deque();
	aCommand.DecrementUsers();
	}


//------------------------------------------------------------------------------------
// Timer functions
//------------------------------------------------------------------------------------

/** Starts the hold timer (only applicable to press commands).

This is the timer to determine whether a command will be passed to RemCon
as a click.  If the timer expires before a matching release is received
the press and release will be sent separately.  Otherwise,  a click is 
sent.

@param aCommand The command to start the timer for.
*/	
void CPassthroughHelper::StartHoldTimer(CControlCommand& aCommand)
	{
	LOG_FUNC
	
	// These cannot fail as we use placement new	
	TAvrcpTimerExpiryInfo* timerInfo = new(aCommand.TimerExpiryInfo())TAvrcpTimerExpiryInfo(this, aCommand);
	
	TCallBack callback(HoldExpiry, timerInfo);
	TDeltaTimerEntry* timerEntry = new(aCommand.TimerEntry())TDeltaTimerEntry(callback);
	
	iTimer.Queue(KRcpHoldThreshold, *timerEntry);
	}

/** Callback when hold timer expires.

This is a static forwarding function.

@param aExpiryInfo The information used by the real HoldExpiry to
					deal with the timer expiry.
*/	
TInt CPassthroughHelper::HoldExpiry(TAny* aExpiryInfo)
	{
	LOG_STATIC_FUNC
	TAvrcpTimerExpiryInfo *timerInfo = reinterpret_cast<TAvrcpTimerExpiryInfo*>(aExpiryInfo);
	(reinterpret_cast<CPassthroughHelper*>(timerInfo->iHandler))->HoldExpiry(timerInfo->iCommand);
	
	return KErrNone;
	}
	
/** Deals with expiry of hold timer.

1) This is not a click.  Set click to false for this command.
2) Inform RemCon of available press command
3) Start release timer.

@param aCommand	The CControlCommand that has expired.
*/
void CPassthroughHelper::HoldExpiry(CControlCommand& aCommand)
	{
	LOG_FUNC
	__ASSERT_DEBUG((aCommand.ButtonAct() == AVCPanel::EButtonPress), AvrcpUtils::Panic(EAvrcpHoldExpiryForRelease));
	
	// We haven't received a release soon enough to treat this as
	// a click.  Send the press on to RemCon and wait for a release.
	// CancelTimer also frees the timer entries, so needs to be called
	// here even though there's no pending entry.
	aCommand.CancelTimer(iTimer);
	aCommand.SetClick(EFalse);
	aCommand.SetCoreButtonAction(ERemConCoreApiButtonPress, ETrue);
	iCommandInterface.MrcciNewCommand(aCommand, aCommand.ClientId());
	StartReleaseTimer(aCommand);
	}

/** Starts the release timer (only applicable to press commands).

If a release is not received quickly enough to treat this press as
a click, the release timer is started.  2s after receiving a button
press we should assume the release if we haven't received one.

@param aCommand The command to start the timer for.
*/		
void CPassthroughHelper::StartReleaseTimer(CControlCommand& aCommand)
	{
	TAvrcpTimerExpiryInfo* timerInfo = new(aCommand.TimerExpiryInfo())TAvrcpTimerExpiryInfo(this, aCommand);

	TCallBack callback(ReleaseExpiry, timerInfo);
	TDeltaTimerEntry* timerEntry = new(aCommand.TimerEntry())TDeltaTimerEntry(callback);
	
	iTimer.Queue(KRcpIncomingButtonReleaseTimeout, *timerEntry);
	}

/** Callback when release timer expires.

This is a static forwarding function.

@param aExpiryInfo The information used by the real ReleaseExpiry to
					deal with the timer expiry.
*/	
TInt CPassthroughHelper::ReleaseExpiry(TAny* aExpiryInfo)
	{
	LOG_STATIC_FUNC
	TAvrcpTimerExpiryInfo *timerInfo = reinterpret_cast<TAvrcpTimerExpiryInfo*>(aExpiryInfo);
	(reinterpret_cast<CPassthroughHelper*>(timerInfo->iHandler))->ReleaseExpiry(timerInfo->iCommand);
	
	return KErrNone;
	}

/** Deals with expiry of release timer.

1) Generate release for this command.
2) Inform RemCon of available release command.

@param aCommand	The CControlCommand that has expired.
*/
void CPassthroughHelper::ReleaseExpiry(CControlCommand& aCommand)
	{
	LOG_FUNC
	__ASSERT_DEBUG((aCommand.ButtonAct() == AVCPanel::EButtonPress), AvrcpUtils::Panic(EAvrcpReleaseExpiryForRelease));
	__ASSERT_DEBUG(!aCommand.Click(), AvrcpUtils::Panic(EAvrcpReleaseExpiryForClick));
	
	// We haven't received a release within the allotted time.  Assume
	// one and generate it to RemCon.
	BalanceHandledCommand(aCommand);
	}
