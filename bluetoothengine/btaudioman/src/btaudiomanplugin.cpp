/*
* Copyright (c) 2006-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Bluetooth Audio Manager ECom plug-in class definition.
*
*/


#include <ecom/ecom.h>
#include <bttypes.h>
#include "btaudiomanplugin.h"
#include "btaccTypes.h"
#include "debug.h"

enum TRequestId
    {
    ERequestConnect = 1,
    ERequestDisconnect = 2,
    ENotifyProfileStatusChange = 3,
    ERequestDisconnectAll,
    };

CBtAudioManPlugin* CBtAudioManPlugin::NewL()
    {
    CBtAudioManPlugin* self = new (ELeave) CBtAudioManPlugin();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

CBtAudioManPlugin::~CBtAudioManPlugin()
    {
    delete iActive4ClientReq;
    delete iActive4ProfileStatus;
    iClient.Close();
    TRACE_FUNC
    }

void CBtAudioManPlugin::SetObserver( MBTEngPluginObserver* aObserver )
    {
    iObserver = aObserver;
    }

void CBtAudioManPlugin::GetSupportedProfiles( RProfileArray& aProfiles )
    {
    aProfiles.Reset();
    aProfiles.Append(EBTProfileHSP);
    aProfiles.Append(EBTProfileHFP);
    aProfiles.Append(EBTProfileA2DP);
    }

TBool CBtAudioManPlugin::IsProfileSupported(const TBTProfile aProfile ) const
    {
    return (aProfile == EBTProfileHSP ||
        aProfile == EBTProfileHFP  ||
        aProfile == EBTProfileA2DP);
    }

TInt CBtAudioManPlugin::Connect( const TBTDevAddr& aAddr )
    {
    TRACE_FUNC
    TInt err = PrepareAsyncRequest(aAddr, ERequestConnect);
    if(!err)
        {
        iDiagnostic.Zero();
        iClient.ConnectToAccessory(iActive4ClientReq->iStatus, iBTDevAddrPckgBuf, iDiagnostic);
        iActive4ClientReq->GoActive();
        }
    return err;
    }

void CBtAudioManPlugin::CancelConnect( const TBTDevAddr& aAddr )
    {
    if (iBTDevAddrPckgBuf() == aAddr &&
        iActive4ClientReq->IsActive() &&
        iActive4ClientReq->RequestId() == ERequestConnect )
        {
        TRACE_INFO(_L("CBtAudioManPlugin::CancelConnect KErrCancel"))
        iActive4ClientReq->Cancel();
        if (iObserver)
            {
            iObserver->ConnectComplete(aAddr, EBTProfileHFP, KErrCancel);
            }
        }
    else
        {
        TRACE_INFO(_L("CBtAudioManPlugin::CancelConnect KErrNotFound"))
        if (iObserver)
            {
            iObserver->ConnectComplete(aAddr, EBTProfileHFP, KErrNotFound);
            }
        }

    }

TInt CBtAudioManPlugin::Disconnect( const TBTDevAddr& aAddr, TBTDisconnectType /*aDiscType*/ )
    {
    TInt req = ERequestDisconnect;
    if (aAddr == TBTDevAddr())
        {
        req = ERequestDisconnectAll;
        }
    TInt err = PrepareAsyncRequest(aAddr, req);
    if (!err)
        {
        if (req == ERequestDisconnect)
            {
            iClient.DisconnectAccessory(iActive4ClientReq->iStatus, iBTDevAddrPckgBuf, iDiagnostic);
            }
        else // if (req == ERequestDisconnectAll)
            {
            iClient.DisconnectAllGracefully(iActive4ClientReq->iStatus);
            }
        iActive4ClientReq->GoActive();
        }
    return err;
    }

void CBtAudioManPlugin::GetConnections( RBTDevAddrArray& aAddrArray, TBTProfile aConnectedProfile )
    {
    aAddrArray.Reset();
    TProfiles profile = EUnknownProfile;
    
    if (aConnectedProfile == EBTProfileHSP || aConnectedProfile == EBTProfileHFP)
        {
        profile = EAnyMonoAudioProfiles;
        }
    else if (aConnectedProfile == EBTProfileA2DP)
        {
        profile = EStereo;
        }
    else
        {
        return;
        }
    
    //guess there are 2 addresses as a 'best guess'
    TInt numAddresses = 2;
    TInt count = numAddresses;
    RBuf8 addrbuf;
    TPtrC8 ptr(addrbuf);
    
    do
        {
        //if this is > 1st time round, the buffer must be deleted and the
        //count might have changed
        addrbuf.Close();
        numAddresses = count;
        
        //create a buffer using the 'best guess' size
        TInt err = addrbuf.Create(KBTDevAddrSize * numAddresses);
        
        if (err != KErrNone)
            {
            //we can't do anything if the buffer fails to create, just return
            //and ignore the error as there is no error path (maybe this will
            //change in the future)
            return;
            }
        
        //get the number of connections and the addresses. count can be either
        //the number of connections or a system-wide error code
        count = iClient.GetConnections(addrbuf, profile);
        
        if (count < KErrNone)
            {
            //error occurred so we're finished with the buffer
            addrbuf.Close();
            
            //ignore the error as there is no error path (maybe this will
            //change in the future)
            return;
            }
        }
    //iterate if the number of connections is greater than our 'best guess' or
    //maybe another connection was established while this was taking place
    while (count > numAddresses);
    
    //iterate through the addresses buffer
    while (ptr.Length() >= KBTDevAddrSize)
        {
        //append each address to the device address array
        TInt err = aAddrArray.Append(TBTDevAddr(ptr.Left(KBTDevAddrSize)));
        
        if (err == KErrNone)
            {
            //shift the pointer along to the next address  
            ptr.Set(ptr.Mid(KBTDevAddrSize));
            }
        else
            {
            //error occurred so we're finished with the buffer
            addrbuf.Close();
            
            //ignore the error as there is no error path (maybe this will
            //change in the future)
            return;
            }
        }
    
    //now finished with the address buffer
    addrbuf.Close();
    }

TBTEngConnectionStatus CBtAudioManPlugin::IsConnected( const TBTDevAddr& aAddr )
    {
    TBTEngConnectionStatus stat = (TBTEngConnectionStatus) iClient.IsConnected(aAddr);
    TRACE_INFO((_L("IsConnected %d"), stat))
    return stat;
    }

void CBtAudioManPlugin::RequestCompletedL(CBasrvActive& aActive)
    {
    TRACE_FUNC
    TInt result = aActive.iStatus.Int();
    switch (aActive.RequestId())
        {
        case ENotifyProfileStatusChange:
            {
            // Notify any observer if one is present, and we got valid data (i.e. no error)
            if (result == KErrNone && iObserver)
                {
                ReportProfileConnectionEvents(iProfileStatus.iAddr, iProfileStatus.iProfiles, iProfileStatus.iConnected);
                }
            // Handle resubscribing for future notifications
            static const TInt KMaxFailedNotifyConnectionStatusAttempts = 3;
            if (result == KErrNone)
                {
                iNotifyConnectionStatusFailure = 0; // reset failure count
                NotifyConnectionStatus();
                }
            else if (result != KErrServerTerminated && iNotifyConnectionStatusFailure < KMaxFailedNotifyConnectionStatusAttempts)
                {
                TRACE_ERROR((_L8("Connection Status Notification failed (transiently): %d"), result))
                ++iNotifyConnectionStatusFailure;
                NotifyConnectionStatus();
                }
            else
                {
                // The server has died unexpectedly, or we've failed a number of times in succession so we cannot re-subscribe. 
                // The lack of state here makes it diffcult to know how to report (handle?) this.  However, we are in
                // no worse situation than before, plus the issue is now logged.
                TRACE_ERROR((_L8("Connection Status Notification failed (terminally): %d"), result))
                iClient.Close(); // clean the handle, this might kill outstanding disconnect/connect requests
                                 // but it looks like the server is hosed anyway...
                // In keeping with the existing logic we don't attempt to re-start the server.  That will only happen when an
                // active request is made.
                }
            break;
            }
        case ERequestConnect:
            {
            if (result != KErrNone) // might have conflicts, decode iDiagnostic
                {
                if (iDiagnostic.Length() >= KBTDevAddrSize)
                    {
                    RBTDevAddrArray array;
                    CleanupClosePushL(array);
                    TPtrC8 ptr(iDiagnostic);
                    while (ptr.Length() >= KBTDevAddrSize)
                        {
                        array.AppendL(TBTDevAddr(ptr.Left(KBTDevAddrSize)));
                        #ifdef _DEBUG
                        const TPtrC8 myPtr(array[array.Count() - 1].Des());
                        #endif
                        TRACE_INFO((_L8("conflict <%S>"), &myPtr))
                        ptr.Set(ptr.Mid(KBTDevAddrSize));
                        }
                    iObserver->ConnectComplete(iBTDevAddrPckgBuf(), EBTProfileHFP, result, &array);
                    CleanupStack::PopAndDestroy(&array);
                    }
                else
                    {
                    iObserver->ConnectComplete(iBTDevAddrPckgBuf(), EBTProfileHFP, result);
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
                ReportProfileConnectionEvents(iBTDevAddrPckgBuf(), profile, ETrue);
                }
            break;
            }
        case ERequestDisconnect:
            {
            if (result != KErrNone)
                {
                iObserver->DisconnectComplete(iBTDevAddrPckgBuf(), EBTProfileHFP, result);
                }
            else
                {
                TInt profile = 0;
                if (iDiagnostic.Length() >= sizeof(TInt))
                    {
                    TPckg<TInt> pckg(profile);
                    pckg.Copy(iDiagnostic.Mid(0, sizeof(TInt))); 
                    }
                ReportProfileConnectionEvents(iBTDevAddrPckgBuf(), profile, EFalse);
                }
            break;
            }
        case ERequestDisconnectAll:
            {
            iObserver->DisconnectComplete(iBTDevAddrPckgBuf(), EBTProfileHFP, result);
            break;
            }
        }
    }
    
void CBtAudioManPlugin::CancelRequest(CBasrvActive& aActive)
    {
    if (aActive.RequestId() == ENotifyProfileStatusChange)
        {
        iClient.CancelNotifyConnectionStatus();
        }
    else if (aActive.RequestId() == ERequestConnect)
        {
        iClient.CancelConnectToAccessory();
        }
    }

CBtAudioManPlugin::CBtAudioManPlugin() : iProfileStatusPckg(iProfileStatus)
    {
    TRACE_FUNC
    }

void CBtAudioManPlugin::ConstructL()
    {
    LEAVE_IF_ERROR(iClient.Connect());
    // Create the handler for profile notifications
    iActive4ProfileStatus = CBasrvActive::NewL(*this, CActive::EPriorityStandard, ENotifyProfileStatusChange);
    NotifyConnectionStatus();
    // Create the handler for active requests (connect, disconnect, etc.)
    iActive4ClientReq = CBasrvActive::New(*this, CActive::EPriorityStandard, ERequestConnect);
    }

void CBtAudioManPlugin::NotifyConnectionStatus()
    {
    iClient.NotifyConnectionStatus(iProfileStatusPckg, iActive4ProfileStatus->iStatus);
    iActive4ProfileStatus->GoActive();
    }

TInt CBtAudioManPlugin::ReconnectIfNeccessary()
    {
    TInt err = KErrNone;
    if (iClient.Handle() == KNullHandle)
        {
        TRACE_INFO((_L8("Handle to Audio Man Server is not valid, connecting again...")))
        err = iClient.Connect();
        TRACE_INFO((_L8("... reconnection result = %d"), err))
        if(!err)
            {
            // Now reconnected, we should start status notifications again...
            NotifyConnectionStatus();
            }
        }
    return err;
    }

TInt CBtAudioManPlugin::PrepareAsyncRequest(const TBTDevAddr& aAddr, TInt aRequestId)
    {
    TInt err = KErrNone;
    err = ReconnectIfNeccessary();
    if (!err && iActive4ClientReq->IsActive())
        {
        // I would normally expect KErrInUse, so as to distinguish this failure from running out of message slots
        err = KErrServerBusy;
        }
    if (!err)
        {
        iBTDevAddrPckgBuf() = aAddr;
        iActive4ClientReq->SetRequestId(aRequestId);
        }
    return err;    
    }

void CBtAudioManPlugin::ReportProfileConnectionEvents(const TBTDevAddr& aAddr, const TInt aProfiles, TBool aConnected)
    {
    TRACE_FUNC
    TRACE_INFO((_L("status %d profiles 0x%04X"), aConnected, aProfiles))
    TBTEngConnectionStatus status = IsConnected(aAddr);
    if (iObserver)
        {
        if (aConnected)
            {
            if (aProfiles & EHFP)
                {
                iObserver->ConnectComplete(aAddr, EBTProfileHFP, KErrNone);
                }
            if (aProfiles & EHSP)
                {
                iObserver->ConnectComplete(aAddr, EBTProfileHSP, KErrNone);
                }
            if (aProfiles & EStereo)
                {
                iObserver->ConnectComplete(aAddr, EBTProfileA2DP, KErrNone);
                }
            }
        else
            {
            if (status != EBTEngConnected)
                {
                if (aProfiles & EHFP)
                    {
                    iObserver->DisconnectComplete(aAddr, EBTProfileHFP, KErrNone);
                    }
                if (aProfiles & EHSP)
                    {
                    iObserver->DisconnectComplete(aAddr, EBTProfileHSP, KErrNone);
                    }
                if (aProfiles & EStereo)
                    {
                    iObserver->DisconnectComplete(aAddr, EBTProfileA2DP, KErrNone);
                    }
                }
            }
        }
    }

//
