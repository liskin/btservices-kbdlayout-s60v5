/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*       This class implements RemCon bearer pulgin interface.
*
*/

#include "playercontrolistener.h"
#include <btaudioremconpskeys.h>
#include "debug.h"
    
CBTAudioPlayerControlListener* CBTAudioPlayerControlListener::NewL(MBmbPlayerControl& aControl)
    {
    CBTAudioPlayerControlListener* self = new(ELeave) CBTAudioPlayerControlListener(aControl);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
    }

CBTAudioPlayerControlListener::~CBTAudioPlayerControlListener()
    {
   	Cancel();
	iProperty.Close();
    }

CBTAudioPlayerControlListener::CBTAudioPlayerControlListener(MBmbPlayerControl& aControl) 
    : CActive(CActive::EPriorityStandard), iControl(aControl)
    {
    CActiveScheduler::Add(this);
    }

void CBTAudioPlayerControlListener::ConstructL()
    {
	TRACE_FUNC
    LEAVE_IF_ERROR(iProperty.Attach(KBTAudioRemCon, KBTAudioPlayerControl));
	Start();
    }

void CBTAudioPlayerControlListener::RunL()
    {
    TRACE_FUNC
    TRACE_INFO((_L8("listener Status %d"), iStatus.Int()))
    TInt err = iStatus.Int();
    if (iStatus == KErrNone || iStatus == KErrNotFound )
        Start();
	if(!err)
	    {
	    TInt value = 0;
    	err = iProperty.Get(value);
    	if (!err)
    	    {
    	    if (value == EBTAudioPausePlayer)
    	        iControl.Pause();
    	    else if (value == EBTAudioResumePlayer)
    	        iControl.Play();
    	    }
	    }
    }

void CBTAudioPlayerControlListener::DoCancel()
    {
    iProperty.Cancel();
    }
    
void CBTAudioPlayerControlListener::Start()
    {
    iProperty.Subscribe(iStatus);
    SetActive();  
    TRACE_FUNC
    }
