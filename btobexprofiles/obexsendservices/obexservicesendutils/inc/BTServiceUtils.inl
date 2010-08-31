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
* Description:  Inline methods for BTSU.
*
*/



// INCLUDE FILES
#include <e32base.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CleanupResetAndDestroy<T>::PushL
// -----------------------------------------------------------------------------
//
template <class T> inline void CleanupResetAndDestroy<T>::PushL( T& aRef )
    {
    CleanupStack::PushL( TCleanupItem( &ResetAndDestroy, &aRef ) );
    }

// -----------------------------------------------------------------------------
// CleanupResetAndDestroy<T>::ResetAndDestroy
// -----------------------------------------------------------------------------
//
template <class T> void CleanupResetAndDestroy<T>::ResetAndDestroy( TAny *aPtr )
    {
    ( STATIC_CAST( T*, aPtr) )->ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CleanupResetAndDestroyPushL
// -----------------------------------------------------------------------------
//
template <class T> inline void CleanupResetAndDestroyPushL( T& aRef )
    {
    CleanupResetAndDestroy<T>::PushL( aRef );
    }
            
// End of File
