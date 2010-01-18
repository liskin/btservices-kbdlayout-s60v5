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
* Description:  Remote Volume Control
*
*/


#ifndef C_BTASRVRVC_H
#define C_BTASRVRVC_H

#include <e32base.h> 
#include "btaccTypes.h"

class CBasrvAcc;

/**
* This class is responsible for deciding which profile should take care of remote volume control
* based on the supported profiles in the remote side and the current audio path situation.
*/
class CBasrvRvc : public CBase
    {
public:
    static CBasrvRvc* New(CBasrvAcc& aAcc);

    ~CBasrvRvc();

    void Update(TInt aConnectedProfiles, TInt aProfileCarryingAudio);
    
private:

    /**
    * Default constructor
    */
    CBasrvRvc(CBasrvAcc& aAcc);

    void DoUpdateRvcMaster();

private:
    CBasrvAcc& iAcc;
    
    TInt iConnectedProfiles;
    
    TProfiles iRvcMaster; // the current profile which is mastering RVC
    TInt iProfileCarryingAudio; // The latest (last if no BT audio is opne) profile that carries BT audio
    
    };

#endif      // C_BTASRVRVC_H

// End of File
