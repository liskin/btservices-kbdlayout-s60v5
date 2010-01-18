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
* Description:  The observer of CBtsacActive's request events
*
*/


#ifndef M_BTSACACTIVEOBSERVER_H
#define M_BTSACACTIVEOBSERVER_H

//  INCLUDES
#include <e32base.h>

class CBtsacActive;

/**
 * The observer of CBtsacActive's request events
 *
 * This class defines the interface to handle a async request events from CBtsacActive.
 *
 * @since S60 v3.1
 */
class MBtsacActiveObserver
    {
    
public:

    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void RequestCompletedL(CBtsacActive& aActive) = 0;
    
    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void CancelRequest(CBtsacActive& aActive) = 0;
    
    };

#endif  // M_BTSACACTIVEOBSERVER_H
