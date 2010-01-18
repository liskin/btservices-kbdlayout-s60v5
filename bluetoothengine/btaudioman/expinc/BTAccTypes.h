/*
* Copyright (c) 2005-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*                Type of profiles supported in bluetooth audio accessories
*
*/


#ifndef BTAUDIOPROFILES_H
#define BTAUDIOPROFILES_H

#include <bttypes.h>

enum TProfiles
    {
    ERemConCT   = 0x10, // AVRCP Controller
    ERemConTG   = 0x08, // AVRCP Target
    EStereo     = 0x04, // A2DP
    EHSP        = 0x02, // HFP
    EHFP        = 0x01, // HSP
    EUnknownProfile = 0x00,
    EAnyMonoAudioProfiles = EHFP | EHSP,
    EAnyAudioProfiles = EStereo | EAnyMonoAudioProfiles,
    EAnyRemConProfiles = ERemConTG | ERemConCT,
    EAnyAccessoryProfiles = EAnyRemConProfiles | EAnyAudioProfiles,
    };


class TProfileStatus
    {
public:    
    TBTDevAddr iAddr;
    TInt iProfiles;
    TBool iConnected;    
    };

typedef TPckgBuf<TProfileStatus> TProfileStatusPckgBuf;
typedef TPckg<TProfileStatus> TProfileStatusPckg;


#endif      // BTAUDIOPROFILES_H   
            
// End of File