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
* Description:  This module is created for variant that doesn't support 
*                 Bluetooth SIM Access Profile. 
*
*/

	
#include "BTUIMainView.h"
//#include "BTUIModel.h" 

// ----------------------------------------------------
// CBTUIMainView::SetSapStatusL
// ----------------------------------------------------
//
void CBTUIMainView::SetSapStatusL( TBTSapMode /*aValue*/ )
    {

    }

// ----------------------------------------------------------
// CBTUIMainView::AskSapModeDlgL
// ----------------------------------------------------------
//
void CBTUIMainView::AskSapModeDlgL()
    {
    
    }
// ----------------------------------------------------------
// CBTUIMainView::GetSapStatusL
// ----------------------------------------------------------

TInt CBTUIMainView::GetSapStatusL()
	{
	return EBTSapDisabled;	
	}

// End of File

