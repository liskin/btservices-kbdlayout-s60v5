/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Active object helper class inline function definitions.
*
*/

// -----------------------------------------------------------------------------
// Get the identifier of this instance.
// -----------------------------------------------------------------------------
//
inline TInt CBTNotifActive::RequestId()
    {
    return iRequestId;
    }


// -----------------------------------------------------------------------------
// Set the identifier of this instance.
// -----------------------------------------------------------------------------
//
inline void CBTNotifActive::SetRequestId( TInt aId )
    {
    iRequestId = aId;
    }


// -----------------------------------------------------------------------------
// Activate the active object.
// -----------------------------------------------------------------------------
//
inline void CBTNotifActive::GoActive()
    {
    SetActive();
    }


// -----------------------------------------------------------------------------
// Cancel an outstanding request.
// -----------------------------------------------------------------------------
//
inline void CBTNotifActive::CancelRequest()
    {
    Cancel();
    }


// -----------------------------------------------------------------------------
// Get a reference to the active object request status.
// -----------------------------------------------------------------------------
//
inline TRequestStatus& CBTNotifActive::RequestStatus()
    {
    return iStatus;
    }
