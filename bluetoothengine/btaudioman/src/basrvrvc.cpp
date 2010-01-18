/*
* Copyright (c) 2005-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of remote volume control.
*
*/


// INCLUDE FILES
#include "basrvrvc.h"
#include "basrvacc.h"
#include "basrvaccman.h"
#include "basrvpluginman.h"
#include "btaccPlugin.h"
#include "debug.h"

// ================= MEMBER FUNCTIONS =======================

CBasrvRvc* CBasrvRvc::New(CBasrvAcc& aAccMan)
    {
    CBasrvRvc* self = new CBasrvRvc(aAccMan);
    return self;
    }

CBasrvRvc::~CBasrvRvc()
    {
    CBTAccPlugin* plugin = NULL;
    plugin = iAcc.AccMan().PluginMan().Plugin(iRvcMaster);
    if (plugin)
        plugin->DeActivateRemoteVolumeControl();
    TRACE_FUNC
    }

void CBasrvRvc::Update(TInt aConnectedProfiles, TInt aProfileCarryingAudio)
    {
    TRACE_FUNC
    iConnectedProfiles = aConnectedProfiles;
    iProfileCarryingAudio = aProfileCarryingAudio;
    DoUpdateRvcMaster();
    }
    
CBasrvRvc::CBasrvRvc(CBasrvAcc& aAcc)
    : iAcc(aAcc)
    {
    TRACE_FUNC
    }

void CBasrvRvc::DoUpdateRvcMaster()
    {
    TRACE_FUNC
    TProfiles newMaster = EUnknownProfile;
    if ((iProfileCarryingAudio == EStereo) && (iConnectedProfiles & ERemConTG))
        newMaster = ERemConTG;
    else if (iProfileCarryingAudio & EAnyMonoAudioProfiles)
        newMaster = EAnyMonoAudioProfiles;
    TRACE_INFO((_L(" audio carrier 0x%02x,  conns 0x%04x"), iProfileCarryingAudio, iConnectedProfiles))
    TRACE_INFO((_L(" [RVC master update] current 0x%02x,  new 0x%02x"), iRvcMaster, newMaster))
    
    if (iRvcMaster != newMaster)
        {
        CBTAccPlugin* plugin = iAcc.AccMan().PluginMan().Plugin(iRvcMaster);
        if (plugin)
            {
            plugin->DeActivateRemoteVolumeControl();
            }
        plugin = iAcc.AccMan().PluginMan().Plugin(newMaster);
        if (plugin)
            {
            plugin->ActivateRemoteVolumeControl();
            TRACE_INFO((_L(" [RVC master update] 0x%02x took the role"), newMaster))
            }        
        iRvcMaster = newMaster;
        }
    }

//  End of File  
