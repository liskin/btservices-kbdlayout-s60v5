/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Contains BTSU wide definitions.
*
*/


// INCLUDE FILES
#include "BTServiceUtils.h"

// CONSTANTS

// ============================= LOCAL FUNCTIONS ===============================
//

// -----------------------------------------------------------------------------
// BTSUPanic
// Issues a Panic using the panic codes and name defined by this subsystem.
// Returns: None.
// -----------------------------------------------------------------------------
//
void BTSUPanic( TBTSUPanicCode aPanic )
    {
    User::Panic( KBTSUModuleName, aPanic );
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================

//  End of File  
