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


// INCLUDE FILES
#include <e32cmn.h>
#include "BTAccSession.h"
#include "BTAccClientSrv.h"
#include "debug.h"
#include "BTAccServer.h"
#include "basrvaccman.h"
#include "BTAccInfo.h"
#include "btaccpanic.h"

typedef TPckgBuf<TBTDevAddr> TBTDevAddrPckgBuf;

// ================= MEMBER FUNCTIONS =======================
CBTAccSession* CBTAccSession::NewL(CBasrvAccMan& aAccMan)
    {
    return new (ELeave) CBTAccSession(aAccMan);
    }

CBTAccSession::CBTAccSession(CBasrvAccMan& aAccMan) 
    : iAccMan(aAccMan)
    {
    TRACE_FUNC
    }

// destructor
CBTAccSession::~CBTAccSession()
    {
    TRACE_FUNC
    if (iConnectMsg.Handle())
        {
        iAccMan.CancelConnect(iConnectingAddr);
        iConnectMsg.Complete(KErrAbort);
        }    
    if (iDisconnectMsg.Handle())
        {
        iDisconnectMsg.Complete(KErrAbort);
        }
    if (iDisconnectAllMsg.Handle())
        {
        iDisconnectAllMsg.Complete(KErrAbort);
        }
    if (iNotifyProfileMsg.Handle())
        {
        iNotifyProfileMsg.Complete(KErrAbort);
        }
    iProfileStatusCache.Close();
    
    //clear the accessory managers pointer to this session if it has one
    iAccMan.ClearProfileNotifySession(*this);
    
    //clear the servers reference to this session
    Server().ClientClosed(*this);
    }

void CBTAccSession::CreateL()
    {
    TRACE_FUNC
    //use CreateL instead of NewSessionL when using Server() to ensure the
    //session has been created correctly and Server() returns a valid pointer
    Server().ClientOpenedL(*this);
    }

void CBTAccSession::ConnectCompleted(TInt aErr, TInt aProfile, const RArray<TBTDevAddr>* aConflicts)
    {
    TRACE_FUNC
    if (iConnectMsg.Handle())
        {
        TRACE_INFO((_L("CBTAccSession::ConnectCompleted err %d"), aErr))
        if (aConflicts)
            {
            if (aErr == KErrNone)
                {
                //we need to error so the client knows there are conflicts which need handling
                aErr = KErrGeneral; 
                }
            
            const TInt KMaxNumberOfConflicts = 2;
            
            TBuf8<KBTDevAddrSize * KMaxNumberOfConflicts> buf;
            TInt count = aConflicts->Count();
            __ASSERT_DEBUG(count <= KMaxNumberOfConflicts, BTACC_PANIC(EMaxNumberOfConflictsExceeded));
            
            for (TInt i = 0; i < count; i++)
                {
                if(i >= KMaxNumberOfConflicts)
                    {
                    break; //prevent descriptor overflow
                    }
                buf.Append((*aConflicts)[i].Des());
                }
            
            if (buf.Length())
                {
                iConnectMsg.Write(1, buf);
                }
            }
        else if (aErr == KErrNone)
            {
            TPckgBuf<TInt> buf(aProfile);
            aErr = iConnectMsg.Write(1, buf);
            }
        iConnectMsg.Complete(aErr);
        }
    }
    
void CBTAccSession::DisconnectCompleted(TInt aProfile, TInt aErr)
    {
    TRACE_FUNC
    if (iDisconnectMsg.Handle())
        {
        TRACE_FUNC
        if (aErr == KErrNone)
            {
            TPckgBuf<TInt> buf(aProfile);
            aErr = iDisconnectMsg.Write(1, buf);
            }
        iDisconnectMsg.Complete(aErr);
        }
    }

void CBTAccSession::DisconnectAllCompleted(TInt aErr)
    {
    TRACE_FUNC
    if (iDisconnectAllMsg.Handle())
        {
        TRACE_FUNC
        TRACE_INFO((_L("ERR %d"), aErr))    
        iDisconnectAllMsg.Complete(aErr);
        }
    }
    
// ---------------------------------------------------------
// CBTAccSession::ConnectToAccessory
// ---------------------------------------------------------
void CBTAccSession::ConnectToAccessory(const RMessage2& aMessage)
    {
    TRACE_FUNC
    if (iConnectMsg.Handle())
        {
        aMessage.Complete(KErrServerBusy);
        }
    else
        {
        TInt err;
        TBTDevAddrPckgBuf pckg;
        iConnectMsg = aMessage;
        err = aMessage.Read(0, pckg);
        if (!err)
            {
            iConnectingAddr = pckg();
            TRAP(err, iAccMan.ConnectL(*this, pckg()));
            }
        if (err)
            {
            iConnectMsg.Complete(err);
            }
        }
    }
    
// ---------------------------------------------------------
// CBTAccSession::ConnectToAccessory
// ---------------------------------------------------------
void CBTAccSession::CancelConnectToAccessory()
    {
    TRACE_FUNC
     if (iConnectMsg.Handle())
        {
        iAccMan.CancelConnect(iConnectingAddr);
        iConnectMsg.Complete(KErrCancel);
        }
    }
    
// ---------------------------------------------------------
// CBTAccSession::DisconnectAccessory
// ---------------------------------------------------------
void CBTAccSession::DisconnectAccessory(const RMessage2& aMessage)
    {
    TRACE_FUNC
    if (iDisconnectMsg.Handle() || iDisconnectAllMsg.Handle())
        {
        aMessage.Complete(KErrServerBusy);
        }
    else
        {
        iDisconnectMsg = aMessage;
        TBTDevAddrPckgBuf pckg;
        TInt err = aMessage.Read(0, pckg);
        if (!err)
            {
            TRAP(err, iAccMan.DisconnectL(*this, pckg()));
            }
        if (err)
            {
            iDisconnectMsg.Complete(err);
            }
        }
    }

void CBTAccSession::GetConnections(const RMessage2& aMessage)
    {
    TRACE_FUNC
    TProfiles profile = static_cast<TProfiles>(aMessage.Int1());
    RPointerArray<const TAccInfo> infos;
    TInt ret = iAccMan.AccInfos(infos);
    
    if (ret == KErrNone)
        {
        RBuf8 buf;
        
        //get the number of connected accessories
        TInt accessoryCount = infos.Count();
        
        //create a buffer the size of either the client side buffer or the
        //maximum amount of addresses (number of connected accessories)
        TInt min = Min(aMessage.GetDesMaxLength(0) / KBTDevAddrSize, accessoryCount);        
        ret = buf.Create(min * KBTDevAddrSize);
        
        //if the buffer was created successfully
        if (ret == KErrNone)
            {
            //iterate through the accessory info array and append the device
            //addresses to the buffer
            for (TInt i = 0; i < min; i++)
                {
                if (infos[i]->iConnProfiles & profile)
                    {
                    buf.Append(infos[i]->iAddr.Des());
                    }
                }
            }
        
        //finished with the accessory info array
        infos.Close();
        
        if(buf.Length() <= 0)
            {
            ret = KErrNotFound;
            }
        
        //if there is no error and the buffer has something in,
        //write the buffer to the message
        if (ret == KErrNone)
            {
            ret = aMessage.Write(0, buf);
            
            if (ret == KErrNone)
                {
                ret = accessoryCount;
                }
            }
        }
    
    //complete the message with either the number of addresses (num of
    //connected accessories) or a system-wide error code
    aMessage.Complete(ret);
    }

void CBTAccSession::DisconnectAllGracefully(const RMessage2& aMessage)
    {
    TRACE_FUNC
    if (iDisconnectMsg.Handle() || iDisconnectAllMsg.Handle())
        {
        aMessage.Complete(KErrServerBusy);
        }
    else
        {
        iDisconnectAllMsg = aMessage;
        TRAPD(err, iAccMan.DisconnectAllL(*this));
        if (err)
            {
            iDisconnectAllMsg.Complete(err);
            }
        }
    }

void CBTAccSession::NotifyProfileStatus(const RMessage2& aMessage)
    {
    TRACE_FUNC
    iAccMan.SetProfileNotifySession(*this);    
    if (iNotifyProfileMsg.Handle())
        {
        aMessage.Complete(KErrInUse);
        }
    else
        {
        if (iProfileStatusCache.Count())
            {
            TProfileStatusPckgBuf buf(iProfileStatusCache[0]);
            TInt err = aMessage.Write(0, buf);
            aMessage.Complete(err);
            iProfileStatusCache.Remove(0);
            }
        else
            {
            iNotifyProfileMsg = aMessage;
            }
        }
    }

// ---------------------------------------------------------
// CBTAccSession::GetBTAccInfo
// ---------------------------------------------------------
void CBTAccSession::GetInfoOfConnectedAcc( const RMessage2& aMessage )
    {
    TRACE_FUNC
    TAccInfo info;
    TPckg<TAccInfo> infoPckg(info);
    TInt err = aMessage.Read(0, infoPckg);
    if (!err)
        {
        const TAccInfo* infoptr = iAccMan.AccInfo(info.GetBDAddress());
        if (infoptr)
            {
            TPtr8 ptr((TUint8*)infoptr, sizeof(TAccInfo), sizeof(TAccInfo));
            err = aMessage.Write(0, ptr);
            }
        else
            {
            err = KErrNotFound;
            }
        }
    aMessage.Complete(err);
    }
    
void CBTAccSession::IsConnected(const RMessage2& aMessage)
    {
    TRACE_FUNC
    TBTDevAddrPckgBuf pckg;
    TInt err = aMessage.Read(0, pckg);
    
    if (err == KErrNone)
        {
        aMessage.Complete(iAccMan.ConnectionStatus4Client(pckg()));
        }
    else
        {
        aMessage.Complete(err);
        }
    }

void CBTAccSession::NotifyClientNewProfile(TInt aProfile, const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TProfileStatus newp;
    newp.iAddr = aAddr;
    newp.iConnected = ETrue;
    newp.iProfiles = aProfile;
    if (iNotifyProfileMsg.Handle())
        {
        TProfileStatusPckgBuf buf(newp);
        TInt err = iNotifyProfileMsg.Write(0, buf);
        iNotifyProfileMsg.Complete(err);        
        }
    else
        {
        UpdateProfileStatusCache(newp);
        }
    }
    
void CBTAccSession::NotifyClientNoProfile(TInt aProfile, const TBTDevAddr& aAddr)
    {
    TRACE_FUNC
    TProfileStatus newp;
    newp.iAddr = aAddr;
    newp.iConnected = EFalse;
    newp.iProfiles = aProfile;
    if (iNotifyProfileMsg.Handle())
        {
        TProfileStatusPckgBuf buf(newp);
        TInt err = iNotifyProfileMsg.Write(0, buf);
        iNotifyProfileMsg.Complete(err);        
        }
    else
        {
        UpdateProfileStatusCache(newp);
        }
    }

// ---------------------------------------------------------
// Server
// Return a reference to CBTServer
// ---------------------------------------------------------
//
CBTAccServer& CBTAccSession::Server()
    {
    return *static_cast<CBTAccServer*>(const_cast<CServer2*>(CSession2::Server()));
    }

// ---------------------------------------------------------
// ServiceL
// service a client request; test the opcode and then do
// appropriate servicing
// ---------------------------------------------------------
//
void CBTAccSession::ServiceL(const RMessage2& aMessage)
    {
    TRACE_FUNC
    TRACE_INFO((_L("CBTAccSession::DispatchMessageL func %d"), aMessage.Function()))
    switch (aMessage.Function())
        {
        case EBTAccSrvConnectToAccessory:
            {
            ConnectToAccessory(aMessage);
            break;
            }
        case EBTAccSrvCancelConnectToAccessory:
            {
            CancelConnectToAccessory();
            aMessage.Complete(KErrNone);
            break;
            }

        case EBTAccSrvDisconnectAccessory:
            {
            DisconnectAccessory(aMessage);
            break;
            }
        case EBTAccSrvGetConnections:
            {
            GetConnections(aMessage);
            break;
            }
        case EBTAccSrvDisconnectAllGracefully:
            {
            DisconnectAllGracefully(aMessage);
            break;
            }
        case EBTAccSrvNotifyConnectionStatus:
            {
            NotifyProfileStatus(aMessage);
            break;
            }
        case EBTAccSrvCancelNotifyConnectionStatus:
            {
            if (iNotifyProfileMsg.Handle())
                {
                iNotifyProfileMsg.Complete(KErrCancel);
                }
            aMessage.Complete(KErrNone);
            break;
            }
        case EBTAccSrvConnectionStatus:
            {
            IsConnected(aMessage);
            break;
            }
        case EBTAccSrvGetInfoOfConnectedAcc:
            {
            GetInfoOfConnectedAcc(aMessage);
            break;
            }
        default:
            {
            PanicClient(aMessage, EBTAccBadRequest);
            break;
            }
        }
    }

void CBTAccSession::UpdateProfileStatusCache(const TProfileStatus& aStatus)
    {
    TRACE_FUNC
    TInt count = iProfileStatusCache.Count();
    for (TInt i = 0; i < count; i++)
        {
        if (iProfileStatusCache[i].iAddr == aStatus.iAddr && 
            iProfileStatusCache[i].iProfiles == aStatus.iProfiles)
            {
            iProfileStatusCache[i].iConnected = aStatus.iConnected;
            return;
            }
        }
    iProfileStatusCache.Append(aStatus);
    }

// End of File
