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
* Description:  CObexSM class definition 
                 definitions.
*
*/



#ifndef OBEXSM_H
#define OBEXSM_H

//  INCLUDES

#include <e32svr.h>
#include "obexserviceman.h"
#include "SrcsServiceManager.h"

// FORWARD DECLARATIONS


/**
 *  CSrcs server core class
 *
 */
class CObexSM : public CPolicyServer
    {
        
public: 

   /**
    * Two-phased constructor.
    */
    static CObexSM* NewL();

   /**
    * Destructor.
    */
    ~CObexSM();

public: // New functions

   /**
    * Initializes all server database classes
    *
    * @since s60 3.2
    */
    void InitialiseServerL();

   /**
    * Manages Services that are handled by SRCS 
    *
    * @since s60 3.2
    * @param aTransport the used transport media
    * @param aState Boolean ON/OFF
    * @param aObserver the request completing observer
    * @param aMessage the message to be completed
    * @return TInt indicating the success of call.
    */
    TInt ManageServices(TSrcsTransport aTransport,TBool aState, MObexSMRequestObserver* aObserver, 
                        const RMessage2& aMessage);

    

public: // Functions from base classes


   /**
    * From CServer NewSessionL.
    *
    * @since s60 3.2
    * @param aVersion version number
    * @param aMessage the connect message
    * @return Pointer to session
    */
    CSession2* NewSessionL( const TVersion& aVersion, const RMessage2& aMessage ) const;
    
   /**
    * Increase session count
    *
    * @since s60 3.2    
    */
    void IncSessionCount();        
    
   /**
    * Decrese session count and shutdown server when all session are closed.
    *
    * @since s60 3.2        
    */    
    void DecSessionCount();

private:

   /**
    * Default constructor
    */
    CObexSM( TInt aPriority );

   /**
    * Two-Phase constructor
    */
    void ConstructL();

private:    // Data
    CSrcsServiceManager*    iServiceHandler;            // Service Handler
    TInt                    iSessionCount;
    };

#endif // OBEXSM_H

// End of File
