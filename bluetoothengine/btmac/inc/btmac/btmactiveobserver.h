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
* Description:  The observer of CBtmActive's request events
*
*/


#ifndef M_BTMACTIVEOBSERVER_H
#define M_BTMACTIVEOBSERVER_H

//  INCLUDES
#include <e32base.h>

class CBtmActive;

/**
 * The observer of CBtmActive's request events
 *
 * This class defines the interface to handle a async request events from CBtmActive.
 *
 * @since S60 v3.1
 */
class MBtmActiveObserver
    {
    
public:

    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void RequestCompletedL(CBtmActive& aActive) = 0;
    
    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void CancelRequest(CBtmActive& aActive) = 0;
    
    };

#endif  // M_BTMACTIVEOBSERVER_H
