/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Base class for sync commands. 
*               
*/

#ifndef ATCMDSYNCBASE_H
#define ATCMDSYNCBASE_H

#include <e32base.h>
#include <etelmm.h>
#include "atcommandparser.h"
#include "atcmdbase.h"
#include "atmisccmdplugin.h"

/**
 *  Abstract base class for sync AT command handler interface.
 *  It can not be instantiated individually as it doesn't fully
 *  implement MATCmdBase.
 */
NONSHARABLE_CLASS( CATCmdSyncBase ) : public CBase,
                                      public MATCmdBase
    {
public:
    virtual ~CATCmdSyncBase() {};
    CATCmdSyncBase(MATMiscCmdPlugin* aCallback, TAtCommandParser& aATCmdParser, RMobilePhone& aPhone);

    // From MATCmdBase
    virtual void HandleCommandCancel();
    
protected:
    MATMiscCmdPlugin* iCallback;
    TAtCommandParser& iATCmdParser;
    RMobilePhone& iPhone;
    };

#endif // ATCMDSYNCBASE_H
