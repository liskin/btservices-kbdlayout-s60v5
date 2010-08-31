/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: class for prompting user to authorize a service
*              connection request.
*
*/

#include <hb/hbcore/hbsymbianvariant.h>
#include <btengconnman.h>
#include "btnotifserviceauthorizer.h"
#include "btnotifsecuritymanager.h"
#include "bluetoothtrace.h"
#include "btnotifclientserver.h"
#include "bluetoothnotification.h"
#include "btnotifconnectiontracker.h"
#include "btnotificationmanager.h"
#include "btnotifserver.h"
#include "btnotifutil.h"

//
// SDP UUID Constants - Short form
// Taken from Bluetooth Profile specification v1.1
// These are used when registering the service to
// local SDP database and when searching the service
// information from remote device.
const TUint KBTSdpDun                   = 0x1103;
const TUint KBTSdpGenericTelephony      = 0x1204;
const TUint KBTSdpFax                   = 0x1111;
const TUint KBTSdpObjectPush            = 0x1105;
const TUint KBTSdpFileTransfer          = 0x1106;
const TUint KBTSdpHeadSet               = 0x1108;
const TUint KBTSdpGenericNetworking     = 0x1201;
const TUint KBTSdpBasicImaging          = 0x111b;


CBTNotifServiceAuthorizer* CBTNotifServiceAuthorizer::NewL(
                                        CBTNotifSecurityManager& aParent)
    {
    CBTNotifServiceAuthorizer* me = new (ELeave) CBTNotifServiceAuthorizer(aParent);
    CleanupStack::PushL(me);
    me->ConstructL();
    CleanupStack::Pop(me);
    return me;
    }

CBTNotifServiceAuthorizer::~CBTNotifServiceAuthorizer()
    {
    iParams.Close();
    if( iNotification )
        {
        // Clear the notification callback, we cannot receive them anymore.
        iNotification->RemoveObserver();
        iNotification->Close(); // Also dequeues the notification from the queue.
        iNotification = NULL;
        }
    if ( !iNotifierMessage.IsNull() )
        {
        iNotifierMessage.Complete( KErrServerTerminated );
        }
    }

CBTNotifServiceAuthorizer::CBTNotifServiceAuthorizer(
                                        CBTNotifSecurityManager& aParent)
:iParent(aParent)
    {
    }

void CBTNotifServiceAuthorizer::ConstructL()
    {
    }

void CBTNotifServiceAuthorizer::StartNotifierL(const RMessage2& aMessage)
    {
    if (!iNotifierMessage.IsNull())
        {
        if(aMessage.Function() == EBTNotifCancelNotifier)
            {
            TInt err = iNotification->Close();
            iNotifierMessage.Complete(KErrCancel);
            aMessage.Complete(err);
            return;
            }
        BOstrace0(TRACE_DEBUG,DUMMY_DEVLIST,"[BTNotif]:We are busy");
        User::Leave(KErrServerBusy );
        }

    iParams.ReAllocL( aMessage.GetDesLengthL( EBTNotifSrvParamSlot ) );
    aMessage.ReadL( EBTNotifSrvParamSlot, iParams );
    
    TBTAuthorisationParams params;
    TPckgC<TBTAuthorisationParams> paramsPckg( params );
    paramsPckg.Set( iParams );
    
    iServiceId = paramsPckg().iUid.iUid;
    
    const CBtDevExtension* dev = NULL;
    dev = iParent.BTDevRepository().Device(paramsPckg().iBDAddr);

    if(dev && dev->Device().GlobalSecurity().Banned() )
        {
        // If the device is banned, service connection from
        // this device is not allowed:
        BOstrace0(TRACE_DEBUG,DUMMY_DEVLIST,"[BTNotif]:Device is banned");
        aMessage.Complete( KErrCancel);
        return;    
        }

    if(dev && dev->Device().GlobalSecurity().NoAuthorise())
        {
        // If the device is a trusted one, no need to pop up query messages.
        TPckgBuf<TBool> answer;
        answer() = ETrue;
        aMessage.Write(EBTNotifSrvReplySlot, answer);
        aMessage.Complete(KErrNone);
        return;
        }
    
    // User must namually authorize this request. 
    // Get needed info for the dialog:
    iPairedDevice = (dev == NULL ) ? EFalse : dev->IsUserAwareBonded();
    iDeviceClass = (dev == NULL ) ? 0 : dev->Device().DeviceClass().DeviceClass();
    TBTNotifUtil::GetDeviceUiNameL(iCurrentDeviceName, 
            dev, paramsPckg().iName, paramsPckg().iBDAddr );

    TBool autoAuthorize;
    PrepareNotificationL(autoAuthorize,
            TBluetoothDialogParams::EUserAuthorization, 
            EAuthorization, iPairedDevice);
    if ( autoAuthorize ) 
        {
        TPckgBuf<TBool> answer; 
        answer() = ETrue;
        aMessage.Write(EBTNotifSrvReplySlot, answer);
        aMessage.Complete(KErrNone);
        }
    else 
        {
        iNotification->ShowL();
        // we do not save the message until all leavable functions have executed successfully.
        // This makes sure the iNotifierMessage has a valid handle.
        iNotifierMessage = aMessage;
        }
    }

void CBTNotifServiceAuthorizer::MBRDataReceived( CHbSymbianVariantMap& aData )
    {
    // "actionResult" will be true if the user clicks 'Yes' on the dialog and false, if he/she clicks 'No'
    // "iCheckBoxState" will be set to true of the checkbox is checked, else false.
    if(aData.Keys().MdcaPoint(0).Compare(_L("actionResult")) == 0)
        {
        TBTAuthorisationParams params;
        TPckgC<TBTAuthorisationParams> paramsPckg(params);
        paramsPckg.Set(iParams);

        TPckgBuf<TBool> answer;
        TInt val = *(static_cast<TInt*>(aData.Get(_L("actionResult"))->Data()));
        if(val)
            {
            answer() = ETrue;
            if(iCheckBoxState)
                {
                // Set the device "Trusted" property
                iParent.TrustDevice(paramsPckg().iBDAddr);
                }
             }
        else
            {
            answer() = EFalse;
            if(iCheckBoxState)
                {
                // If the device is paried, unpair it as well.
                if(iPairedDevice)
                    {
                    iParent.UnpairDevice(paramsPckg().iBDAddr);
                    }
                
                //Set the device "Blocked" property
                iParent.BlockDevice(paramsPckg().iBDAddr,ETrue);
                }
            }
        if ( !iNotifierMessage.IsNull() )
            {
            iNotifierMessage.Write(EBTNotifSrvReplySlot, answer);
            iNotifierMessage.Complete(KErrNone);
            }
        }
    else if(aData.Keys().MdcaPoint(0).Compare(_L("checkBoxState")) == 0)
        {
        iCheckBoxState = *(static_cast<TInt*>(aData.Get(_L("checkBoxState"))->Data()));
        }
    }

void CBTNotifServiceAuthorizer::MBRNotificationClosed( TInt aError, const TDesC8& aData )
    {
    (void) aError;
    (void) aData;
    iNotification->RemoveObserver();
    iNotification = NULL;
    }

void CBTNotifServiceAuthorizer::PrepareNotificationL(TBool& aAutoAuthorize,
        TBluetoothDialogParams::TBTDialogType aType,
    TBTDialogResourceId aResourceId, TBool aPaired)
    {
    iNotification = iParent.ConnectionTracker().NotificationManager()->GetNotification();
    User::LeaveIfNull( iNotification ); // For OOM exception, leaves with KErrNoMemory
    iNotification->SetObserver( this );
    iNotification->SetNotificationType( aType, aResourceId );
    TInt err = KErrNone;
    aAutoAuthorize = EFalse;
    
    //Set the dialog title based on the service IDs
    switch(iServiceId)
        {
        case KBTSdpObjectPush:
        case KBTSdpBasicImaging:
            {
            if(aPaired)
                {
                err = iNotification->SetData( TBluetoothDialogParams::EDialogTitle, TBluetoothDialogParams::EReceiveFromPairedDevice);
                // In case of receiving a msg from a paired deivce, the checkbox is checked by default.
                iCheckBoxState = ETrue;
                User::LeaveIfError(err);
                }
            else
                {
                err = iNotification->SetData( TBluetoothDialogParams::EDialogTitle, TBluetoothDialogParams::EReceive);
                iCheckBoxState = EFalse;
                User::LeaveIfError(err);
                }
            }
            break;
            
        case KBTSdpFax:
        case KBTSdpDun:
        case KBTSdpFileTransfer:
        case KBTSdpHeadSet:
        case KBTSdpGenericTelephony:
        case KBTSdpGenericNetworking:
            {
            err = iNotification->SetData( TBluetoothDialogParams::EDialogTitle, TBluetoothDialogParams::EConnect);
            // In case of an incoming connection, the checkbox is checked by default.
            iCheckBoxState = ETrue;
            User::LeaveIfError(err);
            }
            break;
            
        default:
            {
            TBTAuthorisationParams params;
            TPckgC<TBTAuthorisationParams> paramsPckg(params);
            paramsPckg.Set(iParams);
            
            // In this case, if there already exists a connection to an audio device, then we simply accept
            // the incoming connection without querying the user.
            // If there is no existing connection, then we pop up a query message.
            if(IsExistingConnectionToAudioL(paramsPckg().iBDAddr))
                {
                aAutoAuthorize = ETrue;
                return;
                }
            else
                {
                err = iNotification->SetData( TBluetoothDialogParams::EDialogTitle, TBluetoothDialogParams::EConnect);
                // In case of an incoming connection, the checkbox is checked by default.
                iCheckBoxState = ETrue;
                User::LeaveIfError(err);
                }
            }
            break;
        }
    
    //Add the device name 
    err = iNotification->SetData( TBluetoothDeviceDialog::EDeviceName, iCurrentDeviceName );
    User::LeaveIfError(err);
    //Add the device class
    err = iNotification->SetData( TBluetoothDeviceDialog::EDeviceClass, iDeviceClass );
    User::LeaveIfError(err);
    
    }

TBool CBTNotifServiceAuthorizer::IsExistingConnectionToAudioL(const TBTDevAddr& aDevAddr)
    {
    CBTEngConnMan* connMan = CBTEngConnMan::NewL();
    TBTEngConnectionStatus conntatus(EBTEngNotConnected);
    (void) connMan->IsConnected(aDevAddr,conntatus);      
    delete connMan;
    return (conntatus==EBTEngConnected || conntatus==EBTEngConnecting);
    }

