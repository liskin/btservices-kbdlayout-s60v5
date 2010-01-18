/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  P&S key as a tunnel for btsac and btaudioservice.
*
*/


#ifndef BTAUDIOREMCONPSKEYS_H
#define BTAUDIOREMCONPSKEYS_H

const TUid KBTAudioRemCon = {0x10208971};

const TUint KBTAudioPlayerControl = 0x01;

enum TBTAudioPlayerControlValue
    {
    EBTAudioUndefined,
    EBTAudioPausePlayer,
    EBTAudioResumePlayer
    };

#endif
            
// End of File