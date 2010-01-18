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
    return HandleAsyncRequest(aAddr, ERequestConnect);    
    }

void CBtAudioManPlugin::CancelConnect( const TBTDevAddr& aAddr )
    {
    if (iBTDevAddrPckgBuf() == aAddr &&
        iActive4ClientReq &&
        iActive4ClientReq->IsActive() &&
        iActive4ClientReq->RequestId() == ERequestConnect )
        {
        TRACE_INFO(_L("CBtAudioManPlugin::CancelConnect KErrCancel"))
        delete iActive4ClientReq;
        iActive4ClientReq = NULL;
        if (iObserver)
            {
            iObserver->ConnectComplete(iBTDevAddrPckgBuf(), 
                EBTProfileHFP, KErrCancel);
            }
        }
    else
        {
        TRACE_INFO(_L("CBtAudioManPlugin::CancelConnect KErrNotFound"))
        if (iObserver)
            {
            iObserver->ConnectComplete(aAddr , 
                EBTProfileHFP, KErrNotFound);
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
    return HandleAsyncRequest(aAddr, req);
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
    switch (aActive.RequestId())
        {
        case ENotifyProfileStatusChange:
            {
            if (aActive.iStatus == KErrNone && iObserver)
                {
                ReportProfileConnectionEvents(iProfileStatus.iAddr, iProfileStatus.iProfiles,
                    iProfileStatus.iConnected);
                iClient.NotifyConnectionStatus(iProfileStatusPckg, aActive.iStatus);
                aActive.GoActive();
                }
            break;
            }
        case ERequestConnect:
            {
            if (iActive4ClientReq->iStatus.Int() != KErrNone) // might have conflicts, decode iDiagnostic
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
                    iObserver->ConnectComplete(iBTDevAddrPckgBuf(), 
                        EBTProfileHFP, iActive4ClientReq->iStatus.Int(), &array);
                    CleanupStack::PopAndDestroy(&array);
                    }
                else
                    {
                    iObserver->ConnectComplete(iBTDevAddrPckgBuf(), 
                        EBTProfileHFP, iActive4ClientReq->iStatus.Int());
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
            delete iActive4ClientReq;
            iActive4ClientReq = NULL;
            break;
            }
        case ERequestDisconnect:
            {
            if (iActive4ClientReq->iStatus.Int() != KErrNone)
                {
                iObserver->DisconnectComplete(iBTDevAddrPckgBuf(), 
                        EBTProfileHFP, iActive4ClientReq->iStatus.Int());
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
            delete iActive4ClientReq;
            iActive4ClientReq = NULL;
            break;
            }
        case ERequestDisconnectAll:
            {
            iObserver->DisconnectComplete(iBTDevAddrPckgBuf(), 
                        EBTProfileHFP, iActive4ClientReq->iStatus.Int());
			break;
            }
        }
    }
    
void CBtAudioManPlugin::CancelRequest(CBasrvActive& aActive)
    {
    if (aActive.RequestId() == ENotifyProfileStatusChange )
        {
        iClient.CancelNotifyConnectionStatus();
        }
    else if (aActive.RequestId() == ERequestConnect )
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
    iActive4ProfileStatus = CBasrvActive::NewL(*this, CActive::EPriorityStandard, ENotifyProfileStatusChange);
    iClient.NotifyConnectionStatus(iProfileStatusPckg, iActive4ProfileStatus->iStatus);
    iActive4ProfileStatus->GoActive();
    }

TInt CBtAudioManPlugin::HandleAsyncRequest(const TBTDevAddr& aAddr, TInt aRequestId)
    {
    TInt err = KErrNone;
    if (! iClient.Handle() )
        {
        err = iClient.Connect();
        }
    if ( err )
        {
        return err;
        }
    if ( iActive4ClientReq )
        {
        err = KErrServerBusy;
        }
    if (!err)
        {
        iActive4ClientReq = CBasrvActive::New(*this, CActive::EPriorityStandard, aRequestId);
        if (iActive4ClientReq)
            {
            iBTDevAddrPckgBuf() = aAddr;
            if (aRequestId == ERequestConnect)
                {
                iDiagnostic.Zero();
                iClient.ConnectToAccessory(iActive4ClientReq->iStatus, iBTDevAddrPckgBuf, iDiagnostic);
                }
            else if (aRequestId == ERequestDisconnect)
                {
                iClient.DisconnectAccessory(iActive4ClientReq->iStatus, iBTDevAddrPckgBuf, iDiagnostic);
                }
            else // if (aRequestId == ERequestDisconnectAll)
            	{
            	iClient.DisconnectAllGracefully(iActive4ClientReq->iStatus);
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
			if(	status != EBTEngConnected )
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
