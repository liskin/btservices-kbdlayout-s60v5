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
* Description:  Implementation of an accessory management.
*
*/


// INCLUDE FILES
#include <centralrepository.h>
#include <btengdomaincrkeys.h>

#include "basrvaccman.h"
#include "debug.h"

void CBasrvAccMan::AudioToPhone(CBasrvAudio4Dos& /*aAudio4Dos*/)
    {
    }
    
void CBasrvAccMan::AudioToAccessory(CBasrvAudio4Dos& /*aAudio4Dos*/)
    {
    }
    
void CBasrvAccMan::ConstructL()
    {
    CRepository* cenrep = NULL;
    TRAP_IGNORE(cenrep = CRepository::NewL(KCRUidBluetoothEngine));
    TInt avrcpVol = EBTAvrcpVolCTNotSupported;
    TInt autoDisconnect = EBTDisconnectIfAudioOpenFails;
    if (cenrep)
        {
        cenrep->Get(KBTAvrcpVolCTLV, avrcpVol);
        cenrep->Get(KBTDisconnectIfAudioOpenFailsLV, autoDisconnect);
        delete cenrep;
        }
    iAvrcpVolCTSupported = (avrcpVol == EBTAvrcpVolCTSupported) ? ETrue : EFalse;
    iDisconnectIfAudioOpenFails = (autoDisconnect == EBTDisconnectIfAudioOpenFails) ? ETrue : EFalse;
    TRACE_INFO((_L("[AVRCP_Vol_CT] %d [DisconnectIfAudioOpenFails] %d"), 
        iAvrcpVolCTSupported, iDisconnectIfAudioOpenFails))
    }

void CBasrvAccMan::DestructVariant()
    {
    }
