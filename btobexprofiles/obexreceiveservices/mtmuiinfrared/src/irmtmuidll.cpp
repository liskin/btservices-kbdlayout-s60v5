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

#include "irmtmuidata.h"
#include "irmtmui.h"
#include "debug.h"

// ================= OTHER EXPORTED FUNCTIONS ==============

//
// ---------------------------------------------------------
// NewMtmUiL Creates new MTM UI
// Returns: CBaseMtmUi pointer to created mtm
//
EXPORT_C CBaseMtmUi* NewMtmUiL( CBaseMtm& aMtm, CRegisteredMtmDll& aRegisteredDll )
	{
    FLOG( _L( "CBaseMtmUi* NewMtmUiL\t" ));
	return CIrMtmUi::NewL( aMtm, aRegisteredDll );
	}

//
// ---------------------------------------------------------
// NewMtmUiL Creates new MTM UI Data layer
// Returns: CBaseMtmUiData pointer to created mtm
//
EXPORT_C CBaseMtmUiData* NewMtmUiDataLayerL( CRegisteredMtmDll& aRegisteredDll )
	{    
    FLOG( _L( "CBaseMtmUiData* NewMtmUiDataLayerL\t" ));
	return CIrMtmUiData::NewL( aRegisteredDll );
	}

// End of File
