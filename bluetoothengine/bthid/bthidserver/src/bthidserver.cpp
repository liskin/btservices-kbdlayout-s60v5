/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the implementation of application class
 *
*/

#include <e32svr.h>
#include <s32file.h>
#include <bt_sock.h>
#include <sysutil.h>
//#include <hal.h> // for checking the MachineUID. also link to hal.lib
#include <e32std.h>
//#include <btmanclient.h>
#include <btnotif.h>
#include "bthidserver.h"
#include "bthidsession.h"
#include "bthidclientsrv.h"
#include "bthidconnection.h"
#include "socketlistener.h"
#include "bthidtypes.h"
#include "bthiddevice.h"
#include "hiddescriptorlist.h"
#include "hiddescriptor.h"
#include "bthidserver.pan"
#include "debug.h"
#include "debugconfig.h"
#include "hidgeneric.h"
#include "hidlayoutids.h"
#include "bthidPsKey.h"
#include "hidsdpclient.h"


#ifndef DBG
#ifdef _DEBUG
#define DBG(a) a
#else
#define DBG(a)
#endif
#endif

//File store location
_LIT(KFileStore,"C:\\private\\2001E301\\bthiddevices.dat");

/**  PubSub key read and write policies */
_LIT_SECURITY_POLICY_C2( KBTHIDPSKeyReadPolicy, 
                          ECapabilityLocalServices, ECapabilityReadDeviceData );
_LIT_SECURITY_POLICY_C2( KBTHIDPSKeyWritePolicy, 
                          ECapabilityLocalServices, ECapabilityWriteDeviceData );

// A version number to use when storing device information
// Only for future proofing.
const TInt KDataFileVersionNumber = 1;

CBTHidServer::CBTHidServer() :
    CGenericServer(CActive::EPriorityStandard)
    {
    // Implementation not required
    }

CBTHidServer::~CBTHidServer()
    {
    delete iShutDownTimer;

    delete iControlListener;

    delete iInterruptListener;

    delete iBTConnIndex;

    delete iMasterContIndex; // Causes deletion of iBTConnContainer

    if (iTempInterrupt)
        {
        iTempInterrupt->Close();
        delete iTempInterrupt;
        }

    if (iTempControl)
        {
        iTempControl->Close();
        delete iTempControl;
        }

    iFs.Close();
    iSocketServ.Close();

    iReqs.ResetAndDestroy();
    
    delete iGenHID;
    
    delete iHidSdpClient;
    

    }

CBTHidServer* CBTHidServer::NewL()
    {
    CBTHidServer* self = CBTHidServer::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CBTHidServer* CBTHidServer::NewLC()
    {
    CBTHidServer* self = new (ELeave) CBTHidServer;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

const TInt KShutdownDelay = 5000000;

void CBTHidServer::StartShutdownTimerIfNoSession()
    {
    if (!ConnectionCount()
            && (!iShutDownTimer || !iShutDownTimer->IsActive()))
        {
        if (!iShutDownTimer)TRAP_IGNORE(iShutDownTimer = CPeriodic::NewL(CActive::EPriorityStandard));
        if (iShutDownTimer)
            iShutDownTimer->Start(KShutdownDelay, 0, TCallBack(
                    CBTHidServer::TimerFired, this));
            TRACE_FUNC
        }
    }

TInt CBTHidServer::TimerFired(TAny* /*aThis*/)
    {
        TRACE_STATIC_FUNC
    CActiveScheduler::Stop();
    return KErrNone;
    }

void CBTHidServer::CancelShutdownTimer()
    {
        TRACE_FUNC
    delete iShutDownTimer;
    iShutDownTimer = NULL;
    }

void CBTHidServer::ConstructL()
    {

        TRACE_INFO(_L("CBTHidServer::ConstructL()..."));
    iMasterContIndex = CObjectConIx::NewL();

    iBTConnContainer = iMasterContIndex->CreateL();

    iBTConnIndex = CObjectIx::NewL();

        // Connect to the file server
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConstructL(): Connecting to file server"));
    User::LeaveIfError(iFs.Connect());

    // Make the data storage path (if it doesn't exist)
    // If we can't create it, we can still make connections. They just won't
    // persist.
    iFs.MkDirAll(KFileStore);
        // Connect to the socket server.
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConstructL(): Connecting to socket server"));
    User::LeaveIfError(iSocketServ.Connect());

    // Create initial sockets to accept a connection on the control
    // and interrupt channels
    iTempControl = new (ELeave) RSocket;
    iTempInterrupt = new (ELeave) RSocket;

    // Set the security required for incoming connections on the
    // control and interrupt channels. This is handled in socket level now.

    // Create Socket listeners for the control and interrupt channel
    // ETrue, authorisation from user for incoming connection is asked
    iControlListener = CSocketListener::NewL(iSocketServ, KL2CAPHidControl,
            *this, ETrue);

    //no authorisation needs to be asked,
    //since it is asked during Control channel re-connection.
    iInterruptListener = CSocketListener::NewL(iSocketServ,
            KL2CAPHidInterrupt, *this, EFalse);

        // Request to accept connections into the sockets just created
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConstructL(): AcceptingConnections..."));
    User::LeaveIfError(iControlListener->AcceptConnection(*iTempControl));
    User::LeaveIfError(iInterruptListener->AcceptConnection(*iTempInterrupt));

        // Create the generic HID:
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConstructL(): Creating Generic HID"));
    iGenHID = CGenericHid::NewL(this);

        // Load details of any virtually-cabled devices.
        // Trap the error, but we can live with failure to load stored
        // information. The file may be corrupt and we don't want this
        // to prevent us using the application
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConstructL(): Loading virtually cabled devices"));
    TRAPD( err, LoadVirtuallyCabledDevicesL(KFileStore) );
    if (KErrNone != err)
        {
        err = err;
            TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConstructL(): Loading virtually cabled devices FAILED"));
        }

    TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConstructL(): Starting the server"));
    
    
    StartL(KBTHidSrvName);

    iActiveState = EFalse;

        TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConstructL(): Server Started."));

    }

CSession2* CBTHidServer::NewSessionL(const TVersion& aVersion,
        const RMessage2& /*aMessage*/) const
    {
    // check we're the right version
    if (!User::QueryVersionSupported(TVersion(KBTHIDServMajorVersionNumber,
            KBTHIDServMinorVersionNumber, KBTHIDServBuildVersionNumber),
            aVersion))
        {
        User::Leave(KErrNotSupported);
        }
    const_cast<CBTHidServer*> (this)->CancelShutdownTimer();
    // make new session
    return CBTHidServerSession::NewL(*const_cast<CBTHidServer*> (this));
    }

void CBTHidServer::InformClientsOfStatusChange(
        const CBTHidDevice& aDeviceDetails, TBTHidConnState aState)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::InformClientsOfStatusChange, state=%d"),aState) );
        if (aState == EBTDeviceConnected || 
	     aState == EBTDeviceLinkRestored || 
		 aState == EBTDeviceLinkLost     || 
		 aState == EBTDeviceDisconnected || 
		 aState == EBTDeviceConnectedFromRemote)
        {
        iLastUsedAddr = aDeviceDetails.iAddress;
        iActiveState = ETrue;
        }
    else
        {
        iActiveState = EFalse;
        }
    InformStatusChange(aDeviceDetails.iAddress, aState);
    GlobalNotify(aDeviceDetails.iAddress, aState);
    }

void CBTHidServer::InformStatusChange(const TBTDevAddr& aAddress,
        TBTHidConnState aState)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::InformStatusChange, state=%d"),aState) );
    THIDStateUpdateBuf updateBuf;
    THIDStateUpdate& update = updateBuf();

    update.iDeviceAddress = aAddress;
    update.iState = aState;

    // Send to all clients
    iSessionIter.SetToFirst();
    for (;;)
        {
        CBTHidServerSession* session;
        session = reinterpret_cast<CBTHidServerSession*> (iSessionIter++);
        if (!session)
            {
            break;
            }

        session->InformStatusChange(updateBuf);
        }
    }

void CBTHidServer::GlobalNotify(const TBTDevAddr& aDeviceAddr,
        TBTHidConnState aState)
    {
    switch (aState)
        {
        case EBTDeviceLinkRestored:
        case EBTDeviceConnectedFromRemote:
            {
            HandleAsyncRequest(aDeviceAddr, EBTConnected);
            break;
            }

        case EBTDeviceLinkLost:
        case EBTDeviceDisconnected:
        case EBTDeviceUnplugged:
            {
            HandleAsyncRequest(aDeviceAddr, EBTDisconnected);
            break;
            }
        default:
            //No need to bother
            break;
        }
    }

TInt CBTHidServer::HandleAsyncRequest(const TBTDevAddr& aDeviceAddr,
        TInt aNote)
    {
      TRAPD(err, HandleAsyncRequestL(aDeviceAddr, aNote));
      return err;
    }

void CBTHidServer::HandleAsyncRequestL(const TBTDevAddr& aDeviceAddr,
        TInt aNote)
    {
    CBTHidNotifierHelper* notifier = CBTHidNotifierHelper::NewL(*this, aNote, aDeviceAddr);
    CleanupStack::PushL(notifier);
    
    iReqs.AppendL(notifier);
    CleanupStack::Pop(notifier);
    
    if (iReqs.Count() == 1) // only display our notifier if there's nothing already showing
        {
        notifier->Start();
        }
    }

void CBTHidServer::NotifierRequestCompleted()
    {
    delete iReqs[0];
    iReqs.Remove(0);

    if (iReqs.Count())
        {
        iReqs[0]->Start();
        }
    }

void CBTHidServer::GenericHIDConnectL(CBTHidConnection* aConnection,
        TBool aStartDriver)
    {
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::GenericHIDConnectL"));
    CBTHidDevice& devDetails = aConnection->DeviceDetails();

    // Search for the first report descriptor and give this
    // to Generic HID
    TBool foundRepDesc = EFalse;
    TInt i = 0;
    while ((i < devDetails.iDescList->DescriptorCount()) && (!foundRepDesc))
        {
        // Get the next descriptor.
        const CHidDescriptor& desc =
                (*(aConnection->DeviceDetails().iDescList))[i];

        if (desc.DescriptorType() == CHidDescriptor::EReportDescriptor)
            {
            foundRepDesc = ETrue;
            User::LeaveIfError(iGenHID->ConnectedL(aConnection->ConnID(),
                    desc.RawData()));

            // Try to start the driver if required.
            if (aStartDriver)
                {
                User::LeaveIfError(iGenHID->DriverActive(
                        aConnection->ConnID(), CHidTransport::EActive));
                }
            }

        i++;
        }

    // If we didn't find a report descriptor, the device information is corrupt
    if (!foundRepDesc)
        {
        User::Leave(KErrCorrupt);
        }
    }

void CBTHidServer::IncrementSessions()
    {
    iSessionCount++;
    CancelShutdownTimer();
    }

void CBTHidServer::DecrementSessions()
    {
    iSessionCount--;
    __ASSERT_DEBUG(iSessionCount >= 0, PanicServer(EMainSchedulerError));

    if (iSessionCount <= 0)
        {
        iSessionCount = 0;
        StartShutdownTimerIfNoSession();
        }
    }

TInt CBTHidServer::RunError(TInt aError)
    {
    if (aError == KErrBadDescriptor)
        {
        // A bad descriptor error implies a badly programmed client,
        // so panic it;
        // otherwise report the error to the client
        PanicClient(Message(), EBadDescriptor);
        }
    else
        {
        Message().Complete(aError);
        }

    // The leave will result in an early return from CServer::RunL(), skipping
    // the call to request another message. So do that now in order to keep the
    // server running.
    ReStart();

    return KErrNone; // handled the error fully
    }

void CBTHidServer::PanicClient(const RMessage2& aMessage, TInt aPanic)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::PanicClient(%d)"), aPanic) );
    aMessage.Panic(KBTHIDServer, aPanic);
    }

void CBTHidServer::PanicServer(TInt aPanic)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::PanicServer( %d )"), aPanic) );
    User::Panic(KBTHIDServer, aPanic);
    }

void CBTHidServer::ShutdownListeners(TInt aError)
    {
        // Shutdown listeners and close accepting sockets
        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::ShutdownListeners(%d)"), aError) )

    (void) aError;

    iControlListener->Cancel();
    iInterruptListener->Cancel();

    if (iTempInterrupt)
        {
        iTempInterrupt->Close();
        }

    if (iTempControl)
        {
        iTempControl->Close();
        }
    }

TUint CBTHidServer::ConnectionCount()
    {
    return iBTConnContainer->Count();
    }

CBTHidDevice& CBTHidServer::ConnectionDetailsL(TInt aConnID)
    {
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    return connection->DeviceDetails();
    }

TBTEngConnectionStatus CBTHidServer::ConnectStatus(const TBTDevAddr& aAddress)
    {
    TInt i = 0;
    TBool foundItem = EFalse;
    TBTEngConnectionStatus retVal = EBTEngNotConnected;
    TInt BTConnectionObjCount = iBTConnContainer->Count();

        TRACE_INFO(_L("[BTHID]\tCBTHidServer::ConnectStatus()"));
    while ((i < BTConnectionObjCount) && (!foundItem))
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            CBTHidDevice& devDetails = connection->DeviceDetails();

            if (devDetails.iAddress == aAddress)
                {
                foundItem = ETrue;
                TBTConnectionState HidConnectionStatus =
                        connection->ConnectStatus();
                if ( (EFirstConnection   == HidConnectionStatus) || 
				     (EConnecting        == HidConnectionStatus) || 
                     (EHIDReconnecting   == HidConnectionStatus) || 
                     (EHostReconnecting  == HidConnectionStatus) || 
                     (EHIDInitConnecting == HidConnectionStatus) )
                    {
                    retVal = EBTEngConnecting;
                    }
                if (EConnected == HidConnectionStatus)
                    {
                    retVal = EBTEngConnected;
                    }
                }
            }

        i++;
        }

    return retVal;
    }

TBool CBTHidServer::DeviceExistInContainer(const TBTDevAddr& aAddress)
    {
    TInt i = 0;
    TBool foundItem = EFalse;
    TInt BTConnectionObjCount = iBTConnContainer->Count();

    TRACE_INFO(_L("[BTHID]\tCBTHidServer::DeviceExistInContainer()"));
    while ((i < BTConnectionObjCount) && (!foundItem))
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            CBTHidDevice& devDetails = connection->DeviceDetails();

            if (devDetails.iAddress == aAddress)
                {
                foundItem = ETrue;                
                }
            }
        i++;
        }

    return foundItem;
    }

TBool CBTHidServer::GetConnectionAddress(TDes8& aAddressBuf)
    {
    TInt i = 0;
    TBool retVal = EFalse;
    TInt BTConnectionObjCount = iBTConnContainer->Count();
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::IsAllowToConnect()"));
    while ((i < BTConnectionObjCount))
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            CBTHidDevice& devDetails = connection->DeviceDetails();
            if (connection->IsConnected())
                {
                retVal = ETrue;
                aAddressBuf.Append((devDetails.iAddress).Des());
                if (aAddressBuf.Length() == KBTDevAddrSize * 2)
                    break;
                }
            }
        i++;
        }

    return retVal;
    }

TBool CBTHidServer::IsAllowToConnectFromServerSide(TUint aDeviceSubClass)
    {
    TInt i = 0;
    TBool retVal = ETrue;
    TInt BTConnectionObjCount = iBTConnContainer->Count();

        TRACE_INFO(_L("[BTHID]\tCBTHidServer::IsAllowToConnectFromServerSide()"));
    while ((i < BTConnectionObjCount) && retVal)
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            CBTHidDevice& devDetails = connection->DeviceDetails();
            TBTConnectionState HidConnectionStatus =
                    connection->ConnectStatus();
            if (connection->IsConnected() || 
			      HidConnectionStatus == EHIDReconnecting || 
				  HidConnectionStatus == EHIDInitConnecting)
                {
                if ((IsKeyboard(aDeviceSubClass) && IsKeyboard(
                        devDetails.iDeviceSubClass)) || (IsPointer(
                        aDeviceSubClass) && IsPointer(
                        devDetails.iDeviceSubClass)))
                    {
                    retVal = EFalse;
                    iConflictAddr = devDetails.iAddress;
                    }
                }
            }
        i++;
        }

    return retVal;
    }

TBool CBTHidServer::IsKeyboard(TUint aDeviceSubClass)
    {
    TUint deviceSubClass = aDeviceSubClass;
    TBool retVal = EFalse;
    if ((deviceSubClass >> 2) & EMinorDevicePeripheralKeyboard)
        {
        retVal = ETrue;
        }
    return retVal;
    }

TBool CBTHidServer::IsPointer(TUint aDeviceSubClass)
    {
    TUint deviceSubClass = aDeviceSubClass;
    TBool retVal = EFalse;
    if ((deviceSubClass >> 2) & EMinorDevicePeripheralPointer)
        {
        retVal = ETrue;
        }
    return retVal;
    }

TBool CBTHidServer::IsAllowToConnectFromClientSide(TBTDevAddr aDevAddr)
    {
    TInt i = 0;
    TBool retVal = ETrue;
    TInt BTConnectionObjCount = iBTConnContainer->Count();

    TUint deviceSubClass = GetDeviceSubClass(aDevAddr);
    TRACE_INFO( (_L("[BTHID]\tCBTHidServer::IsAllowToConnectFromClientSide() BTConnectionObjCount = %d"), BTConnectionObjCount) );
    while ((i < BTConnectionObjCount) && retVal)
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            CBTHidDevice& devDetails = connection->DeviceDetails();
            TBTConnectionState HidConnectionStatus =
                    connection->ConnectStatus();
            if (connection->IsConnected() || 
			       HidConnectionStatus == EConnecting || 
				   HidConnectionStatus == EHIDInitConnecting)
                {
                if (devDetails.iAddress != aDevAddr)
                    {                    
                    if ((IsKeyboard(deviceSubClass) && IsKeyboard(
                            devDetails.iDeviceSubClass)) || (IsPointer(
                            deviceSubClass) && IsPointer(
                            devDetails.iDeviceSubClass)))
                        {
                        TRACE_INFO(_L("[BTHID]\tCBTHidServer::() NO connection allowed, connection exist already!"));
                        retVal = EFalse;
                        iConflictAddr = devDetails.iAddress;
                        }
                    }
                }
            }
        i++;
        }

    return retVal;
    }

TUint CBTHidServer::GetDeviceSubClass(TBTDevAddr aDevAddr)
    {
    TInt i = 0;
    TUint deviceSubClass = 0;
    TInt BTConnectionObjCount = iBTConnContainer->Count();

        TRACE_INFO(_L("[BTHID]\tCBTHidServer::GetMinorDeviceClass()"));
    while (i < BTConnectionObjCount)
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            CBTHidDevice& devDetails = connection->DeviceDetails();

            if (devDetails.iAddress == aDevAddr)
                {
                deviceSubClass = devDetails.iDeviceSubClass;
                }
            }
        i++;
        }
    return deviceSubClass;
    }

TBTDevAddr CBTHidServer::ConflictAddress()
    {
    return iConflictAddr;
    }

void CBTHidServer::CleanOldConnection(TInt aConnID)
    {
    TInt i = 0;
    TBTDevAddr currentAddr;
    TRACE_INFO( (_L("[BTHID]\tCBTHidServer::CleanOldConnection() aConnID[%d]"), aConnID) );
    CBTHidConnection* connection =
            static_cast<CBTHidConnection*>(iBTConnIndex->At(aConnID));
    TRACE_INFO( (_L("[BTHID]\tCBTHidServer::CleanOldConnection() aConnID[%d]"), aConnID) );
    __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));
    
    if (connection)
        {
        TRACE_INFO( _L("[BTHID]\tCBTHidServer::CleanOldConnection() if ") );
        CBTHidDevice& currentDetails = connection->DeviceDetails();
        currentAddr = currentDetails.iAddress;
        TRACE_INFO( _L("[BTHID]\tCBTHidServer::CleanOldConnection() if 1") );
        while (i < iBTConnContainer->Count())
            {
            TRACE_INFO( (_L("[BTHID]\tCBTHidServer::CleanOldConnection() i = [%d]"), i) );
            connection = static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

            if (connection)
                {
                CBTHidDevice& devDetails = connection->DeviceDetails();
                if (devDetails.iAddress == currentAddr && !(connection->IsConnected()))
                    {
                    TRACE_INFO( _L("[BTHID]\tCBTHidServer::CleanOldConnection() if 2") );
                    iGenHID->Disconnected(connection->ConnID());
                    iBTConnIndex->Remove(connection->ConnID());
                    }
                }
            i++;
            }
        }

    return;
    } 

TInt CBTHidServer::NewConnectionL(TBTConnectionState aConnectionStatus)
    {
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::NewConnectionL"));
     __ASSERT_DEBUG( aConnectionStatus == EConnecting || aConnectionStatus == EHIDInitConnecting ,
                               CBTHidServer::PanicServer(EBadState));
     
    CBTHidConnection *connection = CBTHidConnection::NewLC(iSocketServ,
            *this, aConnectionStatus);
    // Add to the connection container object.
    iBTConnContainer->AddL(connection);
    CleanupStack::Pop(); // connection

    // Now add the object to the index to get an id.
    // We can't let this just leave since we have already inserted the
    // connection object into the container.
    TInt id = 0;
    TRAPD( res,
            id = iBTConnIndex->AddL(connection);
            connection->SetConnID(id);
    )

    if (res != KErrNone)
        {
        // Couldn't make an index entry.
        // Close the connection object, causing it to be removed from the
        // container
        connection->Close();
        User::Leave(res);
        }

    return id;
    }

void CBTHidServer::DoFirstConnectionL(TInt aConnID)
    {
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::DoFirstConnectionL"));
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    CBTHidDevice &devDetails = ConnectionDetailsL(aConnID);
    DBG(TUint DeviceClass = devDetails.iDeviceSubClass;

        DBG(RDebug::Print(_L("[BTHID]\tCBTHidServer::DoFirstConnectionL iDeviceSubClass  = %d"), DeviceClass));
        )
        
    
    
    if (!IsAllowToConnectFromServerSide(devDetails.iDeviceSubClass))
        {
        User::Leave(KErrAlreadyExists);
        }

    connection->ConnectL();
    }

void CBTHidServer::DeleteNewConnection(TInt aConnID)
    {
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::DeleteNewConnection"));
    iBTConnIndex->Remove(aConnID);
    }

/*Asks the server to disconnect (virtual cable unplug) a device totally,
 * remove the connection entry from the connection container.
 */
void CBTHidServer::CloseBluetoothConnection(const TBTDevAddr& aAddress)

    {
    TInt i = 0;
    TBool foundItem = EFalse;

        TRACE_INFO(_L("[BTHID]\tCBTHidServer::CloseBluetoothConnection"));
    while ((i < iBTConnContainer->Count()) && (!foundItem))
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        CBTHidDevice& devDetails = connection->DeviceDetails();

        if (devDetails.iAddress == aAddress)
            {
            foundItem = ETrue;
            // Inform the Generic HID of the disconnection.
            iGenHID->Disconnected(connection->ConnID());

            // Get it to disconnect if its connected.
            connection->Disconnect();
            
            InformClientsOfStatusChange(connection->DeviceDetails(),
                EBTDeviceDisconnected);
            
            // Delete the connection object.
            iBTConnIndex->Remove(connection->ConnID());

            // Update the stored devices, as we could have power off
            // and no clean shutdown.
            // Use the non-leaving version.
            StoreVirtuallyCabledDevices(KFileStore);
            }

        i++;
        }

    }

/*Asks the server to disconnect all the devices totally,
 * remove the connection entries from the connection container.
 */
void CBTHidServer::CloseAllBluetoothConnection()
    {
    TInt i = 0;

        TRACE_INFO(_L("[BTHID]\tCBTHidServer::CloseAllBluetoothConnection"));

    while (i < iBTConnContainer->Count())
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);
        if (connection)
            {
            if (connection->IsConnected())
                {
                // Inform the Generic HID of the disconnection.
                iGenHID->Disconnected(connection->ConnID());

                // Get it to disconnect if its connected.
                connection->Disconnect();
                
                InformClientsOfStatusChange(connection->DeviceDetails(),
                    EBTDeviceDisconnected);
                 
                // Delete the connection object.
                iBTConnIndex->Remove(connection->ConnID());

                // Update the stored devices, as we could have power off
                // and no clean shutdown.
                // Use the non-leaving version.
                StoreVirtuallyCabledDevices(KFileStore);
                }
            }

        i++;
        }
    }

void CBTHidServer::DisconnectDeviceL(const TBTDevAddr& aAddress)
    {
    TInt i = 0;
    TBool foundItem = EFalse;

        TRACE_INFO(_L("[BTHID]\tCBTHidServer::DisconnectDeviceL"));
    while ((i < iBTConnContainer->Count()) && (!foundItem))
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        CBTHidDevice& devDetails = connection->DeviceDetails();

        if (devDetails.iAddress == aAddress)
            {
            foundItem = ETrue;

            // Drop the bluetooth connection.
            connection->DropConnection();

            // Stop the driver.
            iGenHID->DriverActive(connection->ConnID(),
                    CHidTransport::ESuspend);

            InformClientsOfStatusChange(connection->DeviceDetails(),
                    EBTDeviceDisconnected);
            //Microsoft Keyboard & other "Unsecure" devices
            CheckAndSetControlListenerSecurityL(
                    connection->DeviceDetails().iUseSecurity);
            }

        i++;
        }
    }

void CBTHidServer::DisconnectAllDeviceL()
    {
    TInt i = 0;

        TRACE_INFO(_L("[BTHID]\tCBTHidServer::DisconnectAllDeviceL"));
    while (i < iBTConnContainer->Count())
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            if (connection->IsConnected())
                {
                CBTHidDevice& devDetails = connection->DeviceDetails();

                // Drop the bluetooth connection.
                connection->DropConnection();

                // Stop the driver.
                iGenHID->DriverActive(connection->ConnID(),
                        CHidTransport::ESuspend);

                InformClientsOfStatusChange(devDetails,
                        EBTDeviceDisconnected);
                //Microsoft Keyboard & other "Unsecure" devices
                CheckAndSetControlListenerSecurityL(
                        devDetails.iUseSecurity);
                }

            i++;
            }
        }
    }

void CBTHidServer::CheckAndSetControlListenerSecurityL(TBool aSec)
    {

        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::CheckAndSetControlListenerSecurityL(%d)"), aSec) );

    //Checking for Microsoft Keyboard & other "Unsecure" devices

    if (!aSec) //Security is set on in Constructor of listener. This overrides that setting.
        {
        delete iControlListener;
        iControlListener = NULL;
        if (iTempControl)
            iTempControl->Close();
        iControlListener = CSocketListener::NewL(iSocketServ,
                KL2CAPHidControl, *this, EFalse); //iAuthorisationFlag); We need authorisation, unless otherwise stated.
        User::LeaveIfError(iControlListener->AcceptConnection(*iTempControl));
        }

    }

// from MBTConnectionObserver
void CBTHidServer::HandleControlData(TInt aConnID, const TDesC8& aBuffer)
    {
    iGenHID->DataIn(aConnID, CHidTransport::EHidChannelCtrl, aBuffer);
    }

void CBTHidServer::HandleCommandAck(TInt aConnID, TInt aStatus)
    {
    iGenHID->CommandResult(aConnID, aStatus);
    }

void CBTHidServer::HandleInterruptData(TInt aConnID, const TDesC8& aBuffer)
    {
    iGenHID->DataIn(aConnID, CHidTransport::EHidChannelInt, aBuffer);
    }

void CBTHidServer::FirstTimeConnectionComplete(TInt aConnID, TInt aStatus)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::FirstTimeConnectionComplete(%d)"), aStatus));
    TInt error = aStatus;

    CBTHidConnection* connection =
            static_cast<CBTHidConnection*> (iBTConnIndex->At(aConnID));
    __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));

    if (error == KErrNone)
        {
        TBool genHidConnected = EFalse;

            TRAP( error,
                    // Inform the Generic HID of the Connection
                    GenericHIDConnectL(connection, ETrue);

                    // Record that we got as far as informing the Generic HID.
                    genHidConnected = ETrue;

                    // Try to start monitoring the channels.
                    connection->StartMonitoringChannelsL();
            )

        if (error != KErrNone)
            {
            // If we informed the Generic HID of the connection, then
            // we must also disconnect.
            if (genHidConnected)
                {
                iGenHID->Disconnected(aConnID);
                }

            // Delete the connection object.
            iBTConnIndex->Remove(aConnID);
            }
        else
            {
            // Update the stored devices, as we could have power off
            // and no clean shutdown.
            // Use the non-leaving version.
            TRACE_INFO( _L("[BTHID]\tCBTHidServer::FirstTimeConnectionComplete() before CleanOldConnection") );
            CleanOldConnection(aConnID);
            TRACE_INFO( _L("[BTHID]\tCBTHidServer::FirstTimeConnectionComplete() after CleanOldConnection") );
            StoreVirtuallyCabledDevices(KFileStore);
            }
        }
    else
        {
        iBTConnIndex->Remove(aConnID);
        }

    // Report the connection result to the sessions.
    iSessionIter.SetToFirst();
    for (;;)
        {
        CBTHidServerSession* session;
        session = reinterpret_cast<CBTHidServerSession*> (iSessionIter++);
        if (!session)
            {
            break;
            }

        session->InformConnectionResult(aConnID, error);
        }
    }

void CBTHidServer::FirstTimeConnectionCompleteFromRemote(TInt aConnID, TInt aStatus)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::FirstTimeConnectionCompleteFromRemote(%d)"), aStatus));
    TInt error = aStatus;

    CBTHidConnection* connection =
            static_cast<CBTHidConnection*> (iBTConnIndex->At(aConnID));
    __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));

    if (error == KErrNone)
        {
        TBool genHidConnected = EFalse;

            TRAP( error,
                    // Inform the Generic HID of the Connection
                    GenericHIDConnectL(connection, ETrue);

                    // Record that we got as far as informing the Generic HID.
                    genHidConnected = ETrue;

                    // Try to start monitoring the channels.
                    connection->StartMonitoringChannelsL();
            )

        if (error != KErrNone)
            {
            // If we informed the Generic HID of the connection, then
            // we must also disconnect.
            if (genHidConnected)
                {
                iGenHID->Disconnected(aConnID);
                }

            // Delete the connection object.
            //Quietly refuse the remote initialized connection in case of error. 
            //No need to bother user.
            iBTConnIndex->Remove(aConnID);
            }
        else
            {
            // Update the stored devices, as we could have power off
            // and no clean shutdown.
            // Use the non-leaving version.
            CleanOldConnection(aConnID);
            StoreVirtuallyCabledDevices(KFileStore);
            InformClientsOfStatusChange(connection->DeviceDetails(),
                    EBTDeviceConnectedFromRemote);
            }
        }
    else
        {
        //Quietly refuse the remote initialized connection in case of error.
        //No need to bother user.
        iBTConnIndex->Remove(aConnID);
        }
    }

void CBTHidServer::StartSDPSearch(TInt aConnID)
    {
    iConnID = aConnID;
    TRACE_INFO( (_L("[BTHID]\tCBTHidServer::StartSDPSearch aConnID= (%d)"), aConnID));
    CBTHidConnection* connection =
            static_cast<CBTHidConnection*> (iBTConnIndex->At(iConnID));
    __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));
    
    TRAPD( res,
                // Retrieve the hid device object for this new connection
                CBTHidDevice &devDetails =
                ConnectionDetailsL(iConnID);

                // Create a new HID Sdp Client
                // Its only used here so it doesn't matter if we leave.
                delete iHidSdpClient;
                iHidSdpClient = 0;
                //Create a new hid sdp client using the hid device object.
                iHidSdpClient = CHidSdpClient::NewL(devDetails, *this);

                // Start the hid sdp client
                iHidSdpClient->StartL();
        )
    
    if (res != KErrNone)
        {
        // Get the server to delete the new connection object
        DeleteNewConnection(iConnID);
        }
    }

void CBTHidServer::HidSdpSearchComplete(TInt aResult)
    {
    TRACE_FUNC(_L("[BTHID]\tCBTHidServer::HidSdpSearchComplete"));
    // This is a callback from the Hid SDP client so we can't delete it here
    // Get it to destroy itself when its convenient.
    iHidSdpClient->Kill();
    // Deleted outside destructor.
    iHidSdpClient = 0;

    // If the SDP search was a success
    if (aResult == KErrNone)
        {       
        // Try to connect to the device as a HID
        CBTHidConnection* connection =
                static_cast<CBTHidConnection*> (iBTConnIndex->At(iConnID));
        __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));
        if (connection)
            {
            CBTHidDevice& devDetails = connection->DeviceDetails();
            
            //Only after SDP search complete, do we know the CoD which is needed
            //to tell if the incoming connection is allowed or not.
            //ETrue , establish the connection.
            //EFalse, refuse the remote connecion sliently
            if (IsAllowToConnectFromClientSide(devDetails.iAddress))
                {
                FirstTimeConnectionCompleteFromRemote(iConnID, aResult);
                }
            else
                {
                FirstTimeConnectionCompleteFromRemote(iConnID, KErrAlreadyExists);
                }
            }
        
        }
    }


void CBTHidServer::LinkLost(TInt aConnID)
    {
        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::LinkLost(%d)"), aConnID));
    // Stop the driver.
    iGenHID->DriverActive(aConnID, CHidTransport::ESuspend);

    CBTHidConnection* connection =
            static_cast<CBTHidConnection*> (iBTConnIndex->At(aConnID));
    __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));

    // Inform clients of the change in status of this connection.
    InformClientsOfStatusChange(connection->DeviceDetails(),
            EBTDeviceLinkLost);
    }

void CBTHidServer::LinkRestored(TInt aConnID)
    {
    CBTHidConnection* connection =
            static_cast<CBTHidConnection*> (iBTConnIndex->At(aConnID));
    __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));

    // Inform the Generic HID of the reconnection
    TInt error = iGenHID->DriverActive(aConnID, CHidTransport::EActive);

    // If there was no error, try to start monitoring the channels.
    if (error == KErrNone)
        {
            // Try to start monitoring channels.
            TRAP( error, connection->StartMonitoringChannelsL(); )
        }

    // Report new connection status.
    if (error == KErrNone)
        {
        InformClientsOfStatusChange(connection->DeviceDetails(),
                EBTDeviceLinkRestored);
        }
    else
        {
        InformClientsOfStatusChange(connection->DeviceDetails(),
                EBTDeviceUnplugged);
        // Inform the Generic HID of the disconnection.
        iGenHID->Disconnected(aConnID);
        // Delete the connection object.
        iBTConnIndex->Remove(aConnID);

        // Update the stored devices, as we could have power off
        // and no clean shutdown.
        // Use the non-leaving version.
        StoreVirtuallyCabledDevices(KFileStore);
        }
    }

void CBTHidServer::Disconnected(TInt aConnID)
    {
    CBTHidConnection *connection =
            static_cast<CBTHidConnection*> (iBTConnIndex->At(aConnID));
    __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));

    // Stop the driver.
    iGenHID->DriverActive(aConnID, CHidTransport::ESuspend);

    // Report new connection status.
    InformClientsOfStatusChange(connection->DeviceDetails(),
            EBTDeviceDisconnected);

        //Check if no security is needed for listening socket
        //Microsoft Keyboard & other "Unsecure" devices need this.

        // possible leave is sign of severe problems in BT stack. So no reason to handle leave.
        TRAP_IGNORE( CheckAndSetControlListenerSecurityL(connection->DeviceDetails().iUseSecurity) );

    }

void CBTHidServer::Unplugged(TInt aConnID)
    {
    CBTHidConnection *connection =
            static_cast<CBTHidConnection*> (iBTConnIndex->At(aConnID));
    __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));

    // Report new connection status.
    InformClientsOfStatusChange(connection->DeviceDetails(),
            EBTDeviceUnplugged);

    // Inform the Generic HID of the disconnection.
    iGenHID->Disconnected(aConnID);

    iBTConnIndex->Remove(aConnID);

    // Update the stored devices, as we could have power off
    // and no clean shutdown.
    // Use the non-leaving version.
    StoreVirtuallyCabledDevices(KFileStore);
    }

//from MListenerObserver
void CBTHidServer::SocketAccepted(TUint aPort, TInt aErrorCode)
    {
    TBTSockAddr sockAddr;
    TBTDevAddr devAddr;

        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::SocketAccepted, port=%d, error code=%d"), aPort, aErrorCode) );

    // Check error code
    if (aErrorCode != KErrNone)
        {
        // If we do get an error there isn't much we can about it.
        // Just tidy up.

        ShutdownListeners(aErrorCode);
        }
    else
        {
        TInt i = 0;
        
        TInt err = KErrNone;

        TInt connectingID = 0;
        // Check which port has accepted a connection
        switch (aPort)
            {
            // The HID Control Channel
            case KL2CAPHidControl:
                // Get the BT address of the device that has connected
                iTempControl->RemoteName(sockAddr);
                devAddr = sockAddr.BTAddr();
                // incoming HID connection is allowed
                if (!DeviceExistInContainer(devAddr))
                    {                    
                    TRAPD( res,                            
                           // to be created as New if device not yet listed in container
                           connectingID = NewConnectionL(EHIDInitConnecting);
                    
                           // Retrieve the hid device object for this new connection
                           CBTHidDevice &devDetails =
                           ConnectionDetailsL(connectingID);

                           // Fill in the information we got from the client
                           devDetails.iAddress = devAddr;
                           devDetails.iUseSecurity = ETrue;
                           )

                     if (res != KErrNone && connectingID != 0)
                         {
                         // Get the server to delete the new connection object
                         DeleteNewConnection(connectingID);
                         }
                     }

                if (IsAllowToConnectFromClientSide(devAddr))
                    {
                    TInt count = iBTConnContainer->Count();
                    while ((i < count) && (iTempControl))
                        {
                        CBTHidConnection
                                * connection =
                                        static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);
                        __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));
                        connection->OfferControlSocket(devAddr, iTempControl);
                        i++;
                        }

                    }
                else
                    {
                    InformStatusChange(devAddr, EBTDeviceAnotherExist);
                    }

                // The next socket to accept into
                if (iTempControl)
                    {
                    // Reuse this socket
                    iTempControl->Close();
                    err = KErrNone;
                    }
                else
                    {
                    // Socket ownership has been transferred so create a new
                    // socket
                    //TRAP( err, iTempControl = new (ELeave) RSocket; )
                    iTempControl = new RSocket;
                    }

                // Created a socket to accept into so accept next connection
                if (err == KErrNone)
                    {
                    err = iControlListener->AcceptConnection(*iTempControl);
                    }

                // If we failed to allocate a new RSocket or failed
                // in the AcceptConnectionL call it means we can no longer
                // accept connections from a device.
                if (err != KErrNone)
                    {
                        TRACE_INFO(_L("[BTHID]\tCBTHidServer::SocketAccepted, control channel failed, shutdown listener"));
                    ShutdownListeners(err);
                    }
                
                break;

                // The HID Interrupt Channel
            case KL2CAPHidInterrupt:
                // Get the BT address of the device that has connected
                iTempInterrupt->RemoteName(sockAddr);
                devAddr = sockAddr.BTAddr();
                if (IsAllowToConnectFromClientSide(devAddr))
                    {
                    TInt count = iBTConnContainer->Count();
                    while ((i < count) && (iTempInterrupt))
                        {
                        CBTHidConnection
                                *connection =
                                        static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);
                        __ASSERT_ALWAYS(connection, PanicServer(EInvalidHandle));
                        connection->OfferInterruptSocket(devAddr,
                                iTempInterrupt);
                        i++;
                        }
                    }
                else
                    {
                    //Commented for avoiding display the same notes twice for same device
                    //because of two channels(Control+Interrupt) has been rejected
                    //InformStatusChange(devAddr, EBTDeviceAnotherExist);
                    }

                // The next socket to accept into
                if (iTempInterrupt)
                    {
                    // Reuse this socket
                    iTempInterrupt->Close();
                    err = KErrNone;
                    }
                else
                    {
                    // Socket ownership has been transferred so create a new
                    // socket
                    //TRAP( err, iTempInterrupt = new (ELeave) RSocket; )
                    iTempInterrupt = new RSocket;
                    }

                // Created a socket to accept into so accept next connection
                if (err == KErrNone)
                    {
                    err = iInterruptListener->AcceptConnection(
                            *iTempInterrupt);
                    }

                // If we failed to allocate a new RSocket or failed
                // in the AcceptConnectionL call it means we can no longer
                // accept connections from a device.
                if (err != KErrNone)
                    {
                        TRACE_INFO(_L("[BTHID]\tCBTHidServer::SocketAccepted, interrupt channel failed, shutdown listener"));
                    ShutdownListeners(err);
                    }
                break;

            default:
                PanicServer(EInvalidHandle);
                break;
            }
        }

    }

//from MTransportLayer
TUint CBTHidServer::CountryCodeL(TInt aConnID)
    {
    // Identify the connection object.
    CBTHidConnection* conn = IdentifyConnectionL(aConnID);
    // Retrieve its device details.
    const CBTHidDevice& device = conn->DeviceDetails();

    //return the country code
    return device.iCountryCode;
    }

TUint CBTHidServer::VendorIdL(TInt aConnID)
    {
    // Identify the connection object.
    CBTHidConnection* conn = IdentifyConnectionL(aConnID);
    // Retrieve its device details.
    const CBTHidDevice& device = conn->DeviceDetails();

    //return the Vendor Id.
    return device.iVendorID;
    }

TUint CBTHidServer::ProductIdL(TInt aConnID)
    {
    // Identify the connection object.
    CBTHidConnection* conn = IdentifyConnectionL(aConnID);
    // Retrieve its device details.
    const CBTHidDevice& device = conn->DeviceDetails();

    //return the Product Id.
    return device.iProductID;
    }

void CBTHidServer::GetProtocolL(TInt aConnID, TUint16 /*aInterface*/)
    {
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    // Bluetooth HID only has one interface. We don't need the interface param.
    connection->GetProtocolL();
    }

void CBTHidServer::SetProtocolL(TInt aConnID, TUint16 aValue, TUint16 /*aInterface*/)
    {
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    // Bluetooth HID only has one interface. We don't need the interface param.
    connection->SetProtocolL(aValue);
    }

void CBTHidServer::GetReportL(TInt aConnID, TUint8 aReportType,
        TUint8 aReportID, TUint16 /*aInterface*/, TUint16 aLength)
    {
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    // Bluetooth HID only has one interface. We don't need the interface param.
    connection->GetReportL(aReportType, aReportID, aLength);
    }

void CBTHidServer::SetReportL(TInt aConnID, TUint8 aReportType,
        TUint8 aReportID, TUint16 /*aInterface*/, const TDesC8& aReport)
    {
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    // Bluetooth HID only has one interface. We don't need the interface param
    connection->SetReportL(aReportType, aReportID, aReport);
    }

void CBTHidServer::DataOutL(TInt aConnID, TUint8 aReportID,
        TUint16 /*aInterface*/, const TDesC8& aReport)
    {
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    // Bluetooth HID only has one interface. We don't need the interface param
    connection->DataOutL(aReportID, aReport); //we disregard DATA Input, DATA Feature and DATA Other
    }

void CBTHidServer::GetIdleL(TInt aConnID, TUint8 /*aReportID*/, TUint16 /*aInterface*/)
    {
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    // Bluetooth HID only has one interface. We don't need the interface param.
    // Bluetooth HID doesn't specify Report ID.
    connection->GetIdleL();
    }

void CBTHidServer::SetIdleL(TInt aConnID, TUint8 aDuration,
        TUint8 /*aReportID*/, TUint16 /*aInterface*/)
    {
    CBTHidConnection *connection = IdentifyConnectionL(aConnID);

    // Bluetooth HID only has one interface. We don't need the interface param.
    // Bluetooth HID doesn't specify Report ID.
    connection->SetIdleL(aDuration);
    }

CBTHidConnection* CBTHidServer::IdentifyConnectionL(TInt aConnID) const
    {
    CBTHidConnection *connection =
            static_cast<CBTHidConnection*> (iBTConnIndex->At(aConnID));

    if (!connection)
        {
        User::Leave(KErrNotFound);
        }

    return connection;
    }

void CBTHidServer::LoadVirtuallyCabledDevicesL(const TDesC& aStoreName)
    {
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::LoadVirtuallyCabledDevicesL"));
    // Create the parsed name and open the store
    TParse parsedName;
    User::LeaveIfError(iFs.Parse(aStoreName, parsedName));

    CFileStore* store = CDirectFileStore::OpenLC(iFs, parsedName.FullName(),
            EFileRead);

    // Open the data stream inside the store
    RStoreReadStream stream;
    stream.OpenLC(*store, store->Root());

    // Read the version number of the data file.
    // Its not used in this version.
    stream.ReadInt32L();

    // Get the number of devices to load.
    TInt count = stream.ReadInt32L();
    TInt i;
    for (i = 0; i < count; i++)
        {
        // Create the connection object. Ok to leave.
        CBTHidConnection* connection = CBTHidConnection::NewLC(iSocketServ,
                *this, ENotConnected);

        stream >> connection->DeviceDetails();
        // Check if no security is needed, then override earlier Control listener with unsecure one.
        // This is needed for Microsoft Keyboard fix.
        CheckAndSetControlListenerSecurityL(
                connection->DeviceDetails().iUseSecurity);

        // Add to the connection container object.
        iBTConnContainer->AddL(connection);
        CleanupStack::Pop(); // connection

        // We can't let this just leave since we have already inserted the
        // connection object into the container.
        TBool connectionIndexed = EFalse;
        TRAPD( res,
                // Now add the object to the index to get an id.
                TInt id = iBTConnIndex->AddL(connection);
                connection->SetConnID(id);
                connectionIndexed = ETrue;
                connection->ReconnectL();

                // Inform the Generic HID of the connection, but don't
                // start the driver yet. This will be done in LinkRestored.
                GenericHIDConnectL(connection, EFalse);
        )

        if (res != KErrNone)
            {
            if (connectionIndexed)
                {
                // Connection object was added to the index, but reconnection
                // failed.
                iBTConnIndex->Remove(connection->ConnID());
                }
            else
                {
                // Couldn't make an index entry.
                // Close the connection object, causing it to be removed from
                // the container.
                connection->Close();
                }
            }
        }

    // Delete all the remaining stuff on the cleanup stack
    // (store and stream)
    CleanupStack::PopAndDestroy(2);
    }

void CBTHidServer::StoreVirtuallyCabledDevicesL(const TDesC& aStoreName)
    {
        TRACE_INFO(_L("[BTHID]\tCBTHidServer::StoreVirtuallyCabledDevicesL"));
    // Parse the filestore name into a usable version.
    TParse parsedName;
    User::LeaveIfError(iFs.Parse(aStoreName, parsedName));

    // Count the number of devices that we will need to store.
    // Also count the accumulated disk size require for each one.
    TInt numVirtuallyCabled = 0;
    TInt requiredDiskSize = 0;

    TInt i;

    for (i = 0; i < iBTConnContainer->Count(); i++)
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            CBTHidDevice &device = connection->DeviceDetails();
            if (device.iVirtuallyCabled)
                {
                numVirtuallyCabled++;
                // Increase the required size.
                requiredDiskSize += device.DiskSize();
                }
            }
        }

    // Check for required disk size
    // NOTE !! Be careful when changing functionality in here

    // We have the number of bytes required to store all the device objects
    // we want to store

    // First thing written will be the file version which is 4 bytes
    // Then the number of virtually cabled devices which is 4 bytes
    requiredDiskSize += 8;

        TRACE_INFO( (_L("[BTHID]\tCBTHidServer::StoreVirtuallyCabledDevicesL: Required disk size %d bytes"),
                        requiredDiskSize) );
    // Now check that the required number of bytes will not take us into
    // the critical disk space area.
    if (SysUtil::DiskSpaceBelowCriticalLevelL(&iFs, requiredDiskSize, EDriveC))
        {
        User::Leave(KErrDiskFull);
        }

    // Start the write for real.

    CFileStore* store = CDirectFileStore::ReplaceLC(iFs,
            parsedName.FullName(), EFileWrite);
    store->SetTypeL(KDirectFileStoreLayoutUid);

    // Create the stream
    RStoreWriteStream stream;
    TStreamId id = stream.CreateLC(*store);

    // Write the version number of the data file
    stream.WriteInt32L(KDataFileVersionNumber);

    // Write the number of devices we are going to store.
    stream.WriteInt32L(numVirtuallyCabled);

    // Externalise each device object that is virtually cabled.
    for (i = 0; i < iBTConnContainer->Count(); i++)
        {
        CBTHidConnection *connection =
                static_cast<CBTHidConnection*> ((*iBTConnContainer)[i]);

        if (connection)
            {
            CBTHidDevice &device = connection->DeviceDetails();
            if (device.iVirtuallyCabled)
                {
                stream << device;
                }
            }

        }

    // Commit the changes to the stream
    stream.CommitL();
    CleanupStack::PopAndDestroy();

    // Set the stream in the store and commit the store
    store->SetRootL(id);
    store->CommitL();
    CleanupStack::PopAndDestroy();
    }

void CBTHidServer::StoreVirtuallyCabledDevices(const TDesC& aStoreName)
    {
        TRAP_IGNORE( StoreVirtuallyCabledDevicesL(aStoreName);)
    }

void RunServerL()
    {
        TRACE_FUNC_ENTRY
    User::RenameThread(KBTHidSrvName);
    // Create and install the active scheduler for this thread.
    CActiveScheduler* scheduler = new (ELeave) CActiveScheduler();
    CleanupStack::PushL(scheduler);
    CActiveScheduler::Install(scheduler);
    // create the server (leave it on the cleanup stack)
    CBTHidServer* btHidServer = CBTHidServer::NewLC();
    // Initialisation complete, now signal the client
    RProcess::Rendezvous(KErrNone);
        // The server is not up and running.
        TRACE_INFO( ( _L( "[BTHID]\t BTHID server now up and running" ) ) )
    // The active scheduler runs during the lifetime of this thread.
    CActiveScheduler::Start();
    // Cleanup the server and scheduler.
    CleanupStack::PopAndDestroy(btHidServer);
    CleanupStack::PopAndDestroy(scheduler);
        TRACE_FUNC_EXIT
    }

GLDEF_C TInt E32Main()
    {
    __UHEAP_MARK;
        TRACE_FUNC_ENTRY
    CTrapCleanup* cleanup = CTrapCleanup::New();
    TInt err = KErrNoMemory;
    if (cleanup)
        {
            TRAP( err, RunServerL() );
        delete cleanup;
        }
    __UHEAP_MARKEND;
    return err;
    }

// CBTHidNotifierHelper

CBTHidNotifierHelper* CBTHidNotifierHelper::NewL(CBTHidServer& aHidServer, TInt aNote, const TBTDevAddr& aDeviceAddr)
    { 
    CBTHidNotifierHelper* self = new(ELeave) CBTHidNotifierHelper(aHidServer, aNote, aDeviceAddr);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBTHidNotifierHelper::~CBTHidNotifierHelper()
    {
    Cancel();
    iNotifier.Close();
    }   

void CBTHidNotifierHelper::Start()
    {   
    TRACE_INFO(_L("CBTHidNotifierHelper::Start()..."));
    
    // Simple async handling
    iNotifier.StartNotifierAndGetResponse(iStatus,
                    KBTGenericInfoNotifierUid, iGenericInfoNotifierType,
                    iNoResult);
    
    // assert object is not already active
    __ASSERT_DEBUG(!IsActive(), CBTHidServer::PanicServer(ENotifierAlreadyActive));
    
    SetActive();
    }           

void CBTHidNotifierHelper::DoCancel()
    {
    TRACE_INFO(_L("CBTHidNotifierHelper::DoCancel()..."));
    
    iNotifier.CancelNotifier(KBTGenericInfoNotifierUid);
    }

CBTHidNotifierHelper::CBTHidNotifierHelper(CBTHidServer& aHidServer, TInt aNote, const TBTDevAddr& aDeviceAddr)
    : CActive(CActive::EPriorityStandard),
    iHidServer(aHidServer)
    {
    TRACE_INFO(_L("CBTHidNotifierHelper::CBTHidNotifierHelper()..."));
    
    iGenericInfoNotifierType().iMessageType = (TBTGenericInfoNoteType) aNote;
    iGenericInfoNotifierType().iRemoteAddr.Copy(aDeviceAddr.Des());
    
    CActiveScheduler::Add(this);
    }
    
void CBTHidNotifierHelper::ConstructL()
    {
    TRACE_INFO(_L("CBTHidNotifierHelper::ConstructL()..."));
    
    User::LeaveIfError(iNotifier.Connect());
    }
    
void CBTHidNotifierHelper::RunL()
    {
    TRACE_INFO(_L("CBTHidNotifierHelper::RunL()..."));
    
    iHidServer.NotifierRequestCompleted();
    }
