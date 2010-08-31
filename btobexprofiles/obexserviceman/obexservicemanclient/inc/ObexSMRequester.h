/*
* Copyright (c) 2006-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  obexservicemanager client request class
*
*/


#ifndef OBEXSMREQUESTER_H
#define OBEXSMREQUESTER_H

#include <locodservicepluginobserver.h>

#include <e32def.h>
#include "ObexSMPlugin.h"

class MObexSMRequestObserver;


/**
 *  Active object request class
 *
 *  Its instance will be used to control Asynchronous request
 *  from ECom plugin to obex service manager server
 *
 *  @lib obexserviceman.lib
 *  @since S60 3.2
 */
NONSHARABLE_CLASS(CObexSMRequester) : public CActive
    {

public:

    /**
     * Two-phased constructor.
     *
     * @param aObserver the request completing observer
     * @param aBearer the bearer identification
     * @param aBearStatus the status of this bearer, ETrue
                 if it is available EFalse otherwise.
     */
    static CObexSMRequester* NewL(MObexSMRequestObserver* aObserver,
                                TLocodBearer aBearer, TBool aBearStatus); 

    /**
     * Two-phased constructor.
     *
     * @param aObserver the request completing observer
     * @param aBearer the bearer identification
     * @param aBearStatus the status of this bearer, ETrue
                 if it is available EFalse otherwise.
     */
    static CObexSMRequester* NewLC(MObexSMRequestObserver* aObserver,
                                TLocodBearer aBearer, TBool aBearStatus); 
    /**
     * Destructor.
     */
    virtual ~CObexSMRequester();

    /**
     * Request function   
     *   
     * @since S60 3.2     
     */
    void ActivateRequest();
    /**
     * Get Request Status     
     *
     * @since S60 3.2    
     * @return the requested status 
     */
    TBool GetStatus();
    
    /**
     * Get the requested bearer
     *
     * @since S60 3.2    
     * @return the requested bearer
     */
    TLocodBearer GetBearer();

protected:

    // from base class CActive
    /**
     * From CActive. 
     * Cancel current activity.
     *
     * @since S60 3.2     
     */
    void DoCancel();

    /**
     * From CActive. 
     * Called when read or write operation is ready.
     *
     * @since S60 3.2     
     */
    void RunL();

    /**
     * From CActive. 
     * Called if RunL() leaves. Retry listening after error.
     *
     * @since S60 3.x
     * @param aError the error code
     */
    TInt RunError( TInt aError );

private:


    CObexSMRequester(MObexSMRequestObserver* aObserver, TLocodBearer aBearer, TBool aBearStatus);

    void ConstructL();

private: // data    

    MObexSMRequestObserver* iObserver;
    TLocodBearer            iBearer;
    TBool                   iBearStatus; 
    };


#endif //  OBEXSMREQUESTER_H
