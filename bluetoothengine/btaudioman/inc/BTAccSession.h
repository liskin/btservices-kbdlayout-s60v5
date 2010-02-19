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
* Description: 
*     Server class creates the session. This class then recieves the messages from
*      client and forward them to server class to be handled. Messages are completed
*      with return values recieved from server. 
*
*/


#ifndef BTACCSESSION_H
#define BTACCSESSION_H

//  INCLUDES
#include <e32svr.h>   // access CSession2 declaration
#include <bttypes.h>
#include "btaccTypes.h"

// CLASS DECLARATION
class CBTAccServer;
class CBasrvAccMan;
class TAccInfo;
class CBasrvAudio4Dos;

/**
*  CBTSession class represent session on server side
*
*/
NONSHARABLE_CLASS(CBTAccSession) : public CSession2
    {
public:  // Constructors and destructor
    /**
    * Constructor.
    */
    static CBTAccSession* NewL(CBasrvAccMan& aAccMan);

    /**
    * Destructor.
    */
    ~CBTAccSession();
    
    void ConnectCompleted(TInt aErr, TInt aProfile, const RArray<TBTDevAddr>* aConflicts);
    
    void DisconnectCompleted(TInt aProfile, TInt aErr);

    void DisconnectAllCompleted(TInt aErr);

    void NotifyClientNewProfile(TInt aProfile, const TBTDevAddr& aAddr);
    void NotifyClientNoProfile(TInt aProfile, const TBTDevAddr& aAddr);
        
private:
    /**
    *  connect an accessory
    * @param  aMessage     Message from client containing BT Device address
    * @return KErrNone if successful otherwise one of the system wide error cdes
    */
    void ConnectToAccessory(const RMessage2& aMessage);
    
    /**
    *  cancel connecting
    */
    void CancelConnectToAccessory();

    /**
    *  disconnect an accessory
    * @param  aMessage     Message from client containing BT Device address
    * @return KErrNone if successful otherwise one of the system wide error cdes
    */
    void DisconnectAccessory(const RMessage2& aMessage);
    
    void GetConnections(const RMessage2& aMessage);
    
    void DisconnectAllGracefully(const RMessage2& aMessage);

    void NotifyProfileStatus(const RMessage2& aMessage);
    
    /**
    * 
    * @param
    * @return
    */
    void GetInfoOfConnectedAcc(const RMessage2& aMessage); 

    void IsConnected(const RMessage2& aMessage);     

private: // Functions from base classes
    void ServiceL(const RMessage2 &aMessage);

    CBTAccServer& Server();

    void CreateL();
    
private:
    /**
    * C++ default constructor.
    */
    CBTAccSession(CBasrvAccMan& aAccMan);

    void UpdateProfileStatusCache(const TProfileStatus& aStatus);

private:    // Data
    CBasrvAccMan& iAccMan;  // not own
    
    TBTDevAddr iConnectingAddr;
    RMessagePtr2 iConnectMsg;
    RMessagePtr2 iDisconnectMsg;
    RMessagePtr2 iDisconnectAllMsg;
    
    RArray<TProfileStatus> iProfileStatusCache;
    RMessagePtr2 iNotifyProfileMsg;
    
    CBasrvAudio4Dos* iAudio4Dos;
    };

#endif      // BTACCSESSION_H

// End of File
