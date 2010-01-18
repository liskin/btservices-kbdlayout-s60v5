/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Engine ECom plug-in interface for DosServer audio routing.
*
*/


#include <ecom/ecom.h>

inline CBTAudio4Dos* CBTAudio4Dos::NewL(TUid aImplementationUid)
    {
    CBTAudio4Dos* self = reinterpret_cast<CBTAudio4Dos*>(
        REComSession::CreateImplementationL(
            aImplementationUid, _FOFF(CBTAudio4Dos, iInstanceUid))
        );
    return self;    
    }

inline CBTAudio4Dos::~CBTAudio4Dos()
    {
    REComSession::DestroyedImplementation(iInstanceUid);
    }

