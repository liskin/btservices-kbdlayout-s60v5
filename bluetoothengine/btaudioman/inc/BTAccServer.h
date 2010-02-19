/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*                 Server class is responsible for creating sessions and then handle
*                messages from the session class.
*
*/


#ifndef BTACC_SERVER_H
#define BTACC_SERVER_H

//  INCLUDES
#include <e32svr.h> 

// CONSTANTS

// FORWARD DECLARATION
class CBasrvAccMan;
class TAccInfo;
class CBTAccSession;

/**
*  CBTAccServer server core class
*/
NONSHARABLE_CLASS(CBTAccServer) : public CPolicyServer
    {
public:  // Constructors and destructor
    /**
    * Two-phased constructor.
    */
    static CBTAccServer* NewLC();

    /**
    * Destructor.
    */
    ~CBTAccServer();
    
    void StartShutdownTimerIfNoSessions();
    
    void ClientOpenedL(CBTAccSession& aSession);
        
    void ClientClosed(CBTAccSession& aSession);
        
private: // Functions from base classes
    /**
    * Creates a new session to client
    * @param aVersion the version
    * @param aMessage not used
    * return the new created session
    */
    CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;
           
    /**
    * Constructor
    * @param aPriority This AO's priority
    */
    CBTAccServer(TInt aPriority);

    /**
    * Two-Phase constructor
    */
    void ConstructL();

    void CancelShutdownTimer();
            
    static TInt TimerFired(TAny* aThis);

private:    // Data
    CBasrvAccMan* iAccMan;

    CPeriodic* iTimer;
    
    RPointerArray<CBTAccSession> iSessions;        
    };

#endif      // BTACC_SERVER_H

// End of File
