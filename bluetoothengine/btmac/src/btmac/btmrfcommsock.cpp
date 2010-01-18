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
* Description:  BT RFComm socket
*
*/


#include <badesca.h>
#include "btmrfcommsock.h"
#include "btmsockobserver.h"
#include "debug.h"

// Constant for listening queue size
const TInt KListenQueSize = 1;

// ======== MEMBER FUNCTIONS ========

CBtmRfcommSock* CBtmRfcommSock::NewL(MBtmSockObserver& aObserver, RSocketServ& aServer)
    {
    CBtmRfcommSock* self = new (ELeave) CBtmRfcommSock(aObserver, aServer);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtmRfcommSock::~CBtmRfcommSock()
    {
	TRACE_FUNC_ENTRY
	iAda.Close();
    delete iDataSocket;
    delete iListenSocket;
    iInData.Close();
    iOutData.Close();
    delete iOutDataQueue;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CBtmRfcommSock::Connect
// ---------------------------------------------------------
//
void CBtmRfcommSock::ConnectL(TBTSockAddr& aAddr, TUint aService)
    {
    TRACE_FUNC_ENTRY
    TProtocolDesc pInfo;
    LEAVE_IF_ERROR(iServer.FindProtocol(TProtocolName(KRFCOMMDesC), pInfo));
    if (!iDataSocket)
        {
        iDataSocket = CBluetoothSocket::NewL(*this, iServer, pInfo.iSockType, pInfo.iProtocol);
        }
    LEAVE_IF_ERROR(iDataSocket->Connect(aAddr));
    iService = aService;
    iRemote = TBTDevAddr();
    iRemoteHasConnected = EFalse;
    TRACE_FUNC_EXIT
    }

// ---------------------------------------------------------
// CBtmRfcommSock::CancelActiveConnectSocket
// ---------------------------------------------------------
//
void CBtmRfcommSock::CancelConnect()
    {
    if (iDataSocket)
        {
        TRACE_FUNC_ENTRY
        iDataSocket->CancelConnect();
        TRACE_FUNC_EXIT
        }
    }

TBool CBtmRfcommSock::AccessoryInitiatedConnection()
	{
	return iRemoteHasConnected;
	}
	

TUint CBtmRfcommSock::ListenL(TUint aService, const TBTServiceSecurity& aSec, TUint aLastUsedPort)
    {
    TRACE_FUNC_ENTRY
    TRACE_ASSERT(!iListenSocket, EBTPanicRfcommAlreadyListen)
    TRACE_ASSERT(!iDataSocket, EBTPanicRfcommSockInuse)
	TProtocolDesc pInfo;
	LEAVE_IF_ERROR(iServer.FindProtocol(TProtocolName(KRFCOMMDesC), pInfo));
	iListenSocket = CBluetoothSocket::NewL(*this, iServer, pInfo.iSockType, pInfo.iProtocol);
	TRfcommSockAddr addr;
	addr.SetSecurity(aSec);
	
	TRACE_INFO((_L("Trying lastused port %d"), aLastUsedPort))
	addr.SetPort(aLastUsedPort);
	TInt error = iListenSocket->Bind(addr);
	if ( error )
	    {
	    TRACE_INFO((_L("Trying KRfcommPassiveAutoBind")))
	    addr.SetPort(KRfcommPassiveAutoBind);
	    LEAVE_IF_ERROR(iListenSocket->Bind(addr));
	    }	
	LEAVE_IF_ERROR(iListenSocket->Listen(KListenQueSize));
	iDataSocket = CBluetoothSocket::NewL(*this, iServer);
	LEAVE_IF_ERROR(iListenSocket->Accept(*iDataSocket));
	iService = aService;
	TRACE_FUNC_EXIT
	iRemote = TBTDevAddr();
	return iListenSocket->LocalPort();
    }

void CBtmRfcommSock::CancelListen()
    {
    if (iListenSocket)
        {
        TRACE_FUNC_ENTRY
        iListenSocket->CancelAccept();
        TRACE_FUNC_EXIT
        }
    }

void CBtmRfcommSock::Disconnect(RSocket::TShutdown aHow)
    {
    TRACE_FUNC_ENTRY
    if (iDataSocket)
        {
	    iAda.ActivateActiveRequester();
        iDataSocket->CancelWrite();
        iDataSocket->CancelRecv();
        iDataSocket->CancelIoctl();
        iDataSocket->Shutdown(aHow);
        }
    iRemote = TBTDevAddr();
    TRACE_FUNC_EXIT
    }

void CBtmRfcommSock::WriteL(const TDesC8& aData)
    {
    TRACE_FUNC_ENTRY
    TRACE_ASSERT(iDataSocket != NULL, EBTPanicObjectUninitialized)
    if (!iOutDataQueue)
        {
        iOutDataQueue = new (ELeave) CDesC8ArrayFlat(1);
        }
    iOutDataQueue->AppendL(aData);
    SendPacketL();
    TRACE_FUNC_EXIT
    }

void CBtmRfcommSock::CancelWrite()
    {
    if (iDataSocket)
        {
        TRACE_FUNC_ENTRY
        iDataSocket->CancelWrite();
        iOutDataQueue->Reset();
        TRACE_FUNC_EXIT
        }
    }

const TBTDevAddr& CBtmRfcommSock::Remote() const
    {
    return iRemote;
    }

TInt CBtmRfcommSock::RequestMasterRole()
    {
    TRACE_FUNC
    TRACE_ASSERT(iDataSocket, EBTPanicNullPointer)
    TUint32 basebandState = 0;
    TInt err = iDataSocket->PhysicalLinkState( basebandState );
    if (!err && (basebandState & ENotifySlave))
        {
        err = iDataSocket->RequestMasterRole();
        }
    return err;
    }

TBool CBtmRfcommSock::IsInSniff() const
    {
    return iInSniff;
    }

TInt CBtmRfcommSock::ActivateBasebandEventNotification(TInt aNotification)
    {
    TRACE_FUNC
    TRACE_ASSERT(iDataSocket, EBTPanicNullPointer)
    iBBNotificationMode = aNotification;
    return iDataSocket->ActivateBasebandEventNotifier(aNotification);
    }

void CBtmRfcommSock::SetObserver(MBtmSockObserver& aObserver)
    {
    TRACE_FUNC
    iObserver = &aObserver;
    }

TUint CBtmRfcommSock::Service() const
    {
    return iService;
    }

TInt CBtmRfcommSock::ActivateSniffRequester()
    {
    if (iDataSocket)
        return iAda.ActivateSniffRequester();
    else
        return KErrGeneral;
    }

void CBtmRfcommSock::RequestLinkToActiveMode()
    {
    if (iDataSocket)
        {
    	TInt err = iAda.ActivateActiveRequester();
    	TRACE_INFO((_L("ada.ActivateActiveRequester err %d"), err))    
        }
    TRACE_FUNC
    }

void CBtmRfcommSock::HandleConnectCompleteL(TInt aErr)
    {
    TRACE_FUNC_ENTRY
    TRACE_INFO((_L("err %d"), aErr))

    // Process the connect complete before issuing a receive request to ensure that
    // we are ready to process the data when it is received.
    iObserver->RfcommConnectCompletedL(aErr);

    if (!aErr)
        {
        TBTSockAddr sockAddr;
        iDataSocket->RemoteName(sockAddr);
        iRemote = sockAddr.BTAddr();
        iInData.ReAllocL(256);
        RequestMasterRole();
        TInt err = iAda.Open(iServer, iRemote);
        TRACE_INFO((_L("ada.Open err %d"), err))
        iDataSocket->ActivateBasebandEventNotifier(ENotifyAnyPowerMode | ENotifyAnyRole | 
                                                   ENotifySCOLinkUp | ENotifySCOLinkDown | 
                                                   ENotifySCOLinkError | ENotifyPhysicalLinkUp | 
                                                   ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError);
        ReceiveL();
        }
    TRACE_FUNC_EXIT
    }

void CBtmRfcommSock::HandleAcceptCompleteL(TInt aErr)
    {
    TRACE_FUNC_ENTRY

    iRemoteHasConnected = ETrue;

    // Process the accept complete before issuing a receive request to ensure that
    // we are ready to process the data when it is received.
    iObserver->RfcommAcceptCompletedL(aErr, iService);

   if (!aErr)
        {
        TBTSockAddr sockAddr;
        iDataSocket->RemoteName(sockAddr);
        iRemote = sockAddr.BTAddr();
        iInData.ReAllocL(256);
        RequestMasterRole();
        TInt err = iAda.Open(iServer, iRemote);
        TRACE_INFO((_L("ada.Open err %d"), err))
        iDataSocket->ActivateBasebandEventNotifier(ENotifyAnyPowerMode | ENotifyAnyRole |
                                                   ENotifySCOLinkUp | ENotifySCOLinkDown | 
                                                   ENotifySCOLinkError | ENotifyPhysicalLinkUp | 
                                                   ENotifyPhysicalLinkDown | ENotifyPhysicalLinkError);
        ReceiveL();
        }
    TRACE_FUNC_EXIT   
    }

void CBtmRfcommSock::HandleShutdownCompleteL(TInt aErr)
    {
    TRACE_FUNC
    iObserver->RfcommShutdownCompletedL(aErr);
    }

void CBtmRfcommSock::HandleSendCompleteL(TInt aErr)
    {
    TRACE_FUNC
    TRACE_INFO((_L("err %d"), aErr))
    iWriting = EFalse;
    if (!aErr)
        {
        SendPacketL();
        }
    else
        {
        iObserver->RfcommSendCompletedL(aErr);
        }
    }

void CBtmRfcommSock::HandleReceiveCompleteL(TInt aErr)
    {
    TRACE_FUNC
    TRACE_INFO((_L("err %d"), aErr))

    iObserver->RfcommReceiveCompletedL(aErr, iInData);
    if (!aErr)
        {
        ReceiveL();
        }
    }

void CBtmRfcommSock::HandleIoctlCompleteL(TInt aErr)
    {
    TRACE_FUNC
    TRACE_INFO((_L("KL2CAPEchoRequestIoctl callback code: %d"), aErr ))
    
    if (aErr)
        {
        iObserver->RfcommSendCompletedL(aErr);
        }
    }

void CBtmRfcommSock::HandleActivateBasebandEventNotifierCompleteL(
    TInt aErr, TBTBasebandEventNotification& aEventNotification)
    {
    TRACE_FUNC_ENTRY
    if (!aErr)
        {
    	switch(aEventNotification.EventType())
    		{
    		case ENotifyActiveMode:
    			{
    			TRACE_INFO(_L( "BB Notif: Active mode" ))
    			iInSniff = EFalse;
    			break;
    			}
    		case ENotifySniffMode:
    			{
    			TRACE_INFO(_L( "BB Notif: Sniff mode" ))
    			break;
    			}
    		case ENotifyMaster:
    			{
    			TRACE_INFO(_L( "BB Notif: Master role"))
    			break;
    			}
    		case ENotifySlave:
    			{
    			TRACE_INFO(_L("BB Notif: Slave role"))
    			break;
    			}
    		case ENotifySCOLinkUp:
    			{
    			TRACE_INFO(_L("BB Notif: SCO up"))
    			break;
    			}
    		case ENotifySCOLinkDown:
    			{
    			TRACE_INFO(_L("BB Notif: SCO down"))
    			
                TInt err = iDataSocket->Ioctl( KL2CAPEchoRequestIoctl, NULL, KSolBtL2CAP );
                
                TRACE_INFO((_L("KL2CAPEchoRequestIoctl return code: %d"), err ))
    			break;
    			}
    		case ENotifySCOLinkError:
    			{
    			TRACE_INFO(_L("BB Notif: SCO error"))
    			break;
    			}
    		case ENotifyPhysicalLinkUp:
    		    {
                TRACE_INFO(_L("BB Notif: Physical link up"))    		        
    		    break;
    		    }
    		case ENotifyPhysicalLinkDown:
    		    {
                TRACE_INFO(_L("BB Notif: Physical link down"))    		        
    		    break;
    		    }
    		case ENotifyPhysicalLinkError:
    		    {
                TRACE_INFO(_L("BB Notif: Physical link error"))    		        
    		    break;
    		    }
    		default:
    			{
    			TRACE_INFO((_L( "BB Notif: Uninteresting event (0x%08X)"), aEventNotification.EventType()))
    			}
    		};
        }
    TRACE_FUNC_EXIT
    }

void CBtmRfcommSock::ConstructL()
    {
    }

CBtmRfcommSock::CBtmRfcommSock(MBtmSockObserver& aObserver, RSocketServ& aServer)
    : iObserver(&aObserver), iServer(aServer), iInSniff(EFalse)
    {
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBtmAtPipe::HandleAccessoryDataEventL
// -----------------------------------------------------------------------------
//
void CBtmRfcommSock::SendPacketL()
    {
    if (!iWriting && iOutDataQueue && iOutDataQueue->MdcaCount() > 0)
        {
        if (iOutData.MaxLength() < iOutDataQueue->MdcaPoint(0).Length())
            {
            iOutData.ReAllocL(iOutDataQueue->MdcaPoint(0).Length());
            } 
        iOutData.Copy(iOutDataQueue->MdcaPoint(0));
        iDataSocket->Write(iOutData);
        iOutDataQueue->Delete(0);
        TRACE_INFO((_L8("[HFP] [O] %S"), &iOutData))
        iWriting = ETrue;
        }
    }

void CBtmRfcommSock::ReceiveL()
    {
    TRACE_FUNC
    TRACE_ASSERT(iDataSocket != NULL, EBTPanicObjectUninitialized)
    iInData.Zero();
    LEAVE_IF_ERROR(iDataSocket->RecvOneOrMore(iInData, 0, iXfrLength));
    }

void CBtmRfcommSock::CancelReceive()
    {
    if (iDataSocket)
        {
        TRACE_FUNC_ENTRY
        iDataSocket->CancelRecv();
        TRACE_FUNC_EXIT
        }
    }

void CBtmRfcommSock::SetService(TUint aService)
		{
		iService = aService;
		}
