/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Hid ECom plug-in class definition.
 *
*/


#include <ecom/ecom.h>
#include <ecom/implementationproxy.h>
#include <btmanclient.h>
#include "bthidengplugin.h"
#include "bthidclient.h"

#include "debug.h"

//Used to identify the type of request to the AO
enum TRequestId
    {
    ERequestConnect = 1,
    ERequestDisconnect = 2,
    ENotifyProfileStatusChange = 3,
    ERequestDisconnectAll,
    };

CBTHidPlugin::CBTHidPlugin()
    {
    }

CBTHidPlugin* CBTHidPlugin::NewL()
    {
    CBTHidPlugin* self = new (ELeave) CBTHidPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

void CBTHidPlugin::ConstructL()
    {
        TRACE_FUNC
        LEAVE_IF_ERROR( iClient.Connect() );

    iActive4ProfileStatus = CGenericActive::NewL(*this,
            CActive::EPriorityStandard, ENotifyProfileStatusChange);
    iDiagnosticAddress.Zero();
    iClient.NotifyStatusChange(iHIDStateUpdatePckg, iDiagnosticAddress,
            iActive4ProfileStatus->iStatus);
    iActive4ProfileStatus->GoActive();

        TRACE_FUNC_EXIT
    }

CBTHidPlugin::~CBTHidPlugin()
    {
    delete iActive4ClientReq;
    delete iActive4ProfileStatus;
    if (iClient.Handle())
        {
        TRequestStatus req = KRequestPending;
        iClient.DisconnectAllGracefully(req);
        User::WaitForRequest(req);
        }
    iClient.Close();
        TRACE_FUNC
    }

void CBTHidPlugin::SetObserver(MBTEngPluginObserver* aObserver)
    {
    iObserver = aObserver;
    }

void CBTHidPlugin::GetSupportedProfiles(RProfileArray& aProfiles)
    {
    aProfiles.Reset();
    aProfiles.Append(EBTProfileHID);
    }

TBool CBTHidPlugin::IsProfileSupported(const TBTProfile aProfile) const
    {
    return (aProfile == EBTProfileHID);
    }

TInt CBTHidPlugin::Connect(const TBTDevAddr& aAddr)
    {
        TRACE_FUNC
    return HandleAsyncRequest(aAddr, ERequestConnect);
    }

void CBTHidPlugin::CancelConnect(const TBTDevAddr& aAddr)
    {
        TRACE_FUNC
    (void) aAddr;
    iClient.CancelConnectDevice();
    }

TInt CBTHidPlugin::Disconnect(const TBTDevAddr& aAddr,
        TBTDisconnectType aDiscType)
    {
        TRACE_FUNC
    (void) aDiscType;
    return HandleAsyncRequest(aAddr, ERequestDisconnect);
    }

void CBTHidPlugin::GetConnections(RBTDevAddrArray& aAddrArray,
        TBTProfile aConnectedProfile)
    {
    if (aConnectedProfile == EBTProfileHID)
        {
        TBuf8<KBTDevAddrSize * 2> addrbuf;
        iClient.GetConnections(addrbuf, aConnectedProfile);
        TPtrC8 ptr(addrbuf);
        while (ptr.Length() >= KBTDevAddrSize)
            {
            aAddrArray.Append(TBTDevAddr(ptr.Left(KBTDevAddrSize)));
            ptr.Set(ptr.Mid(KBTDevAddrSize));
            }
        }
    }

void CBTHidPlugin::HandleNotifyProfileStatusChange(CGenericActive& aActive)
    {
    if (aActive.iStatus.Int() == KErrNone && iObserver)
        {
        TBool retStatus = EFalse;

        THIDStateUpdate& HIDStateUpdate = iHIDStateUpdatePckg();

        if (HIDStateUpdate.iState == EBTDeviceConnected
                || HIDStateUpdate.iState == EBTDeviceLinkRestored
                || HIDStateUpdate.iState == EBTDeviceConnectedFromRemote)
            {
            retStatus = ETrue;
            ReportProfileConnectionEvents(HIDStateUpdate.iDeviceAddress,
                    retStatus);
            }

        if (HIDStateUpdate.iState == EBTDeviceDisconnected
                || HIDStateUpdate.iState == EBTDeviceLinkLost
                || HIDStateUpdate.iState == EBTDeviceUnplugged)
            {
            retStatus = EFalse;
            ReportProfileConnectionEvents(HIDStateUpdate.iDeviceAddress,
                    retStatus);
            }
        iDiagnosticAddress.Zero();
        iClient.NotifyStatusChange(iHIDStateUpdatePckg, iDiagnosticAddress,
                aActive.iStatus);
        aActive.GoActive();
        }
    else
        {
        if (aActive.iStatus.Int() != KErrNone)
            {
            THIDStateUpdate& HIDStateUpdate = iHIDStateUpdatePckg();
            if (iDiagnosticAddress.Length() >= KBTDevAddrSize)
                {
                RBTDevAddrArray array;
                TPtrC8 ptr(iDiagnosticAddress);
                while (ptr.Length() >= KBTDevAddrSize)
                    {
                        TRAP_IGNORE(array.AppendL(TBTDevAddr(ptr.Left(KBTDevAddrSize))););
#ifdef _DEBUG
                    const TPtrC8 myPtr(array[array.Count() - 1].Des());
#endif
                        TRACE_INFO((_L8("conflict <%S>"), &myPtr))
                    ptr.Set(ptr.Mid(KBTDevAddrSize));
                    }

                iBTDevAddrPckgBuf() = HIDStateUpdate.iDeviceAddress;
                iObserver->ConnectComplete(iBTDevAddrPckgBuf(),
                        EBTProfileHID, aActive.iStatus.Int(), &array);
                array.Close();
                }
            else
                {
                iObserver->ConnectComplete(HIDStateUpdate.iDeviceAddress,
                        EBTProfileHID, aActive.iStatus.Int());
                }
            iClient.NotifyStatusChange(iHIDStateUpdatePckg,
                    iDiagnosticAddress, aActive.iStatus);
            aActive.GoActive();
            }
        }
    }

void CBTHidPlugin::HandelRequestConnectComplete()
    {
    if (iActive4ClientReq->iStatus.Int() != KErrNone) // might have conflicts, decode iDiagnostic
        {
        if (iDiagnostic.Length() >= KBTDevAddrSize)
            {
            RBTDevAddrArray array;
            TPtrC8 ptr(iDiagnostic);
            while (ptr.Length() >= KBTDevAddrSize)
                {
                    TRAP_IGNORE(array.AppendL(TBTDevAddr(ptr.Left(KBTDevAddrSize))););
#ifdef _DEBUG
                const TPtrC8 myPtr(array[array.Count() - 1].Des());
#endif
                    TRACE_INFO((_L8("conflict <%S>"), &myPtr))
                ptr.Set(ptr.Mid(KBTDevAddrSize));
                }
            iObserver->ConnectComplete(iBTDevAddrPckgBuf(), EBTProfileHID,
                    iActive4ClientReq->iStatus.Int(), &array);
            array.Close();
            }
        else
            {
            iObserver->ConnectComplete(iBTDevAddrPckgBuf(), EBTProfileHID,
                    iActive4ClientReq->iStatus.Int());
            }
        }
    else
        {
        TInt profile = 0;
        if (iDiagnostic.Length() >= sizeof(TInt))
            {
            TPckg<TInt> pckg(profile);
            pckg.Copy(iDiagnostic.Mid(0, sizeof(TInt)));
            }
        ReportProfileConnectionEvents(iBTDevAddrPckgBuf(), ETrue);
        }
    delete iActive4ClientReq;
    iActive4ClientReq = NULL;
    }

void CBTHidPlugin::HandelRequestDisconnectComplete()
    {
    if (iActive4ClientReq->iStatus.Int() != KErrNone)
        {
        iObserver->DisconnectComplete(iBTDevAddrPckgBuf(), EBTProfileHID,
                iActive4ClientReq->iStatus.Int());
        }
    else
        {
        TInt profile = 0;
        if (iDiagnostic.Length() >= sizeof(TInt))
            {
            TPckg<TInt> pckg(profile);
            pckg.Copy(iDiagnostic.Mid(0, sizeof(TInt)));
            }
        ReportProfileConnectionEvents(iBTDevAddrPckgBuf(), EFalse);
        }
    delete iActive4ClientReq;
    iActive4ClientReq = NULL;
    }

void CBTHidPlugin::RequestCompletedL(CGenericActive& aActive)
    {
        TRACE_FUNC
    switch (aActive.RequestId())
        {
        case ENotifyProfileStatusChange:
            {
            HandleNotifyProfileStatusChange(aActive);
            break;
            }
        case ERequestConnect:
            {
            HandelRequestConnectComplete();
            break;
            }
        case ERequestDisconnect:
            {
            HandelRequestDisconnectComplete();
            break;
            }
        case ERequestDisconnectAll:
            {
            iObserver->DisconnectComplete(iBTDevAddrPckgBuf(), EBTProfileHID,
                    iActive4ClientReq->iStatus.Int());
            break;
            }
        }
    }

TBTEngConnectionStatus CBTHidPlugin::IsConnected(const TBTDevAddr& aAddr)
    {
    TInt stat = 0;
    stat = iClient.IsConnected(aAddr);
        TRACE_INFO((_L("CBTHidPlugin::IsConnected() = %d"), stat))
    return ((TBTEngConnectionStatus) stat);
    }

void CBTHidPlugin::CancelRequest(CGenericActive& aActive)
    {
    if (aActive.RequestId() == ENotifyProfileStatusChange)
        {
        iClient.CancelNotifyStatusChange();
        }
    else
        {
        if (aActive.RequestId() == ERequestConnect)
            {
            iClient.CancelConnectDevice();
            }
        }
    }

TInt CBTHidPlugin::HandleAsyncRequest(const TBTDevAddr& aAddr,
        TInt aRequestId)
    {
    TInt err = KErrNone;
    if (!iClient.Handle())
        {
        err = iClient.Connect();
        }
    if (err)
        {
        return err;
        }
    if (iActive4ClientReq)
        {
        err = KErrServerBusy;
        }
    if (!err)
        {
        iActive4ClientReq = CGenericActive::New(*this,
                CActive::EPriorityStandard, aRequestId);
        if (iActive4ClientReq)
            {
            iBTDevAddrPckgBuf() = aAddr;
            if (aRequestId == ERequestConnect)
                {
                iDiagnostic.Zero();
                iClient.ConnectDevice(iBTDevAddrPckgBuf, iDiagnostic,
                        iActive4ClientReq->iStatus);
                }
            else
                {
                iClient.DisconnectDevice(iBTDevAddrPckgBuf, EBTDiscImmediate,
                        iActive4ClientReq->iStatus);
                }
            iActive4ClientReq->GoActive();
            }
        else
            {
            err = KErrNoMemory;
            }
        }
    return err;
    }

void CBTHidPlugin::ReportProfileConnectionEvents(const TBTDevAddr& aAddr,
        TBool aConnected)
    {
        TRACE_FUNC
        TRACE_INFO((_L("CBTHidPlugin::ReportProfileConnectionEvents() == %d"), aConnected))

    if (iObserver)
        {
        if (aConnected)
            {
            iObserver->ConnectComplete(aAddr, EBTProfileHID, KErrNone);
            }
        else
            {
            iObserver->DisconnectComplete(aAddr, EBTProfileHID, KErrNone);
            }
        }
    }

//End of File
