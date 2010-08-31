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
* Description:  obexservicemanager client class definition
*
*/


#ifndef OBEXSMCLIENT_H
#define OBEXSMCLIENT_H

//  INCLUDES
#include <e32std.h>
#include <locodbearer.h>
#include "usbobex.h"

/// server name
_LIT(KSrcsName,"obexserviceman");

// Server path

_LIT(KSrcsImg,"z:\\sys\\bin\\obexserviceman.exe");

// A version must be specifyed when creating a session with the server
const TUint KSrcsMajorVersionNumber=1;
const TUint KSrcsMinorVersionNumber=0;
const TUint KSrcsBuildVersionNumber=1;

// SRCS Server Uid
const TUid KSrcsUid={0x101F7C87};

// Opcodes used in message passing between client and server
enum TSrcsServRequest
{
    ESrcsBTServicesON,
    ESrcsBTServicesOFF,
    ESrcsStartUSB,
    ESrcsStopUSB,    
    ESrcsIrDAServicesON,
    ESrcsIrDAServicesOFF,    
    ESrcsCancelRequest
};




/**
* Client side thread starting function
* Start the server when client tries to contact it if it is not allready running
*/ 
TInt StartThread();

/**
 *  The client class. An RSessionBase sends messages to the server with the function
 *  RSessionBase::SendReceive(); specifying an opcode and and array of argument pointers.
 *  Functions in derived classes, such as RSrcs::InitialiseServices(), are wrappers for different calls to
 *  SendReceive().
 */
NONSHARABLE_CLASS (RObexSMServer): public RSessionBase
    {
public:  // Constructor

    /**
    * C++ default constructor.
    */
    RObexSMServer();

public:     
    
   /**
    * Turns BT services ON/OFF depending on given parameter.
    *
    * @since S60 v3.2
    * @param    TBool aState Boolean value to toggle services.
    * @return   TInt indicating the success of call.
    */
    TInt ManageServices(TLocodBearer aBearer, TBool aBearStatus,TRequestStatus &aStatus );
                                   

public: // Functions from base classes

   /**
    * From RSessionBase informs current version    
    *
    * @since S60 v3.2
    * @return   Version information about the server
    */
    TVersion Version() const;

   /**
    * From RSessionBase connect to the server   
    *
    * @since S60 v3.2    
    */
    TInt Connect();
    
   /**
    * Cancel pending request
    *
    * @since S60 v3.2       
    */ 
    void CancelRequest();   
    

private:    // Data

    RProcess iProses;
    };


#endif  // OBEXSMCLIENT_H
