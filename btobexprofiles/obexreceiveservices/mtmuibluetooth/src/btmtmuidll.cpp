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
* Description: 
*     DLL interface
*
*/


// INCLUDE FILES

#include "btmtmuidata.h"
#include "btmtmui.h"
#include "btmtmuidebug.h"

// ================= OTHER EXPORTED FUNCTIONS ==============

// ---------------------------------------------------------
// NewBtMtmUiL Creates new MTM UI (BT)
// Returns: CBaseMtmUi pointer to created mtm
// ---------------------------------------------------------
//
EXPORT_C CBaseMtmUi* NewBtMtmUiL( CBaseMtm& aMtm, CRegisteredMtmDll& aRegisteredDll )
	{
    FLOG( _L( "CBtMtmUi: NewBtMtmUiL\t" ) );
	return CBtMtmUi::NewL( aMtm, aRegisteredDll );
	}

// ---------------------------------------------------------
// NewBtMtmUiDataLayerL Creates new MTM UI data layer (BT)
// Returns: CBaseMtmUiData pointer to created mtm
// ---------------------------------------------------------
//
EXPORT_C CBaseMtmUiData* NewBtMtmUiDataLayerL( CRegisteredMtmDll& aRegisteredDll )
	{
    FLOG( _L( "CBtMtmUi: NewBtMtmUiDataLayerL\t" ) );
	return CBtMtmUiData::NewL( aRegisteredDll );
	}

// End of File
