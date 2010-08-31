/*
* Copyright (c) 2007-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Profile controller interface
*
*/



#ifndef C_BTPROFILECONTROLLER_H
#define C_BTPROFILECONTROLLER_H


#include <e32base.h> 

#include "BTServiceClient.h"


class CBTServiceClient;

/**
 *  ProfileControllerIntreface definition 
 * 
 *  @endcode
 *
 *  @lib ?library
 *  @since S60 v.3.2
 */
NONSHARABLE_CLASS (CBTSController) : public CBase
    {

public: 

    CBTSController(); 
    /**
     * Destructor.
     */
    virtual ~CBTSController();

    /**
     * Abort command to client
     *
     * @since S60 v.3.2     
     * @return None
     */
     void Abort( );
     
     
     virtual void SendUnSupportedFiles();

protected:
    
    /**
     * Create client
     *
     * @since S60 v.3.2     
     * @return None
     */
     void CreateClientL(MBTServiceClientObserver* aObserver,
                       const TBTDevAddr& aRemoteDevice,
                       const TUint aRemotePort,
                       RArray<CObexHeader*> aHeaderList );    


protected: // data

    /**
     * Service client
     * Not own. 
     */
    CBTServiceClient*           iClient;

    };


#endif // C_BTPROFILECONTROLLER_H
