/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Part of CBTInqUI class: use Active object to search BT devices.
*
*/
#include "btninqui.h"
#include "btNotifDebug.h"

/**  Identification for active object request */
const TInt KBTNotifInquiryNotifierReq = 20;

// ----------------------------------------------------------
// CBTInqUI::PrepareScannerL
// ----------------------------------------------------------
//
void CBTInqUI::CreateScannerL()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::CreateScannerL()"));
    
    __ASSERT_DEBUG( iScanner == NULL, User::Panic(_L("BTNotifInqUI - iScanner not released!"),KErrAlreadyExists));
        
    // Create scanner object which will do the actual search
    iScanner = CBTNotifActive::NewL( this, KBTNotifInquiryNotifierReq, CActive::EPriorityStandard );
    User::LeaveIfError( iSocketServer.Connect() );
    TProtocolDesc pInfo;
    User::LeaveIfError( iSocketServer.FindProtocol( _L("BTLinkManager"), pInfo));
    User::LeaveIfError( iHostResolver.Open(iSocketServer, pInfo.iAddrFamily, pInfo.iProtocol));
    IssueRequestL();

    FLOG(_L("[BTNOTIF]\t CBTInqUI::CreateScannerL() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::IssueRequestL
// Issue Bluetooth device inquiry/search itself.
// Reads also inquiry mode from shared data.
// ----------------------------------------------------------
//
void CBTInqUI::IssueRequestL()
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::IssueRequestL()"));
    
    if (reinterpret_cast<CBTNotifierBase*>(iDevSearchObserver)->AutoLockOnL())
        {
        // Cancel bt inquiry immediately if device is locked.
        FLOG(_L("[BTNOTIF]\t CBTInqUI::IssueRequestL() Device is locked"));
        Cancel();
        return;
        }
    
    TUint action = 0; 
    if( !iPageForName )
        {
        action = KHostResInquiry + KHostResEir + KHostResIgnoreCache;
        }
    else
        {
        TInt firstPartialNameDevIndex;
        if (HaveDevsWithPartialName(firstPartialNameDevIndex))
            {
            FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::IssueRequestL() looking up device index %d (have partial name already)"), firstPartialNameDevIndex));

            action = KHostResName + KHostResIgnoreCache;
            TInquirySockAddr sa;
            sa.SetBTAddr(iLastSeenDevicesArray->At(firstPartialNameDevIndex)->BDAddr());
            iInquirySockAddr.SetBTAddr( sa.BTAddr() );            
            }
        else if( iCurrentlyResolvingUnnamedDeviceIndex < iDevsWithoutName.Count() )
            {
            FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::IssueRequestL() looking up device index %d (currently anonymous)"), iCurrentlyResolvingUnnamedDeviceIndex));

            action = KHostResName + KHostResIgnoreCache;
            TInquirySockAddr& sa = TInquirySockAddr::Cast( iDevsWithoutName[iCurrentlyResolvingUnnamedDeviceIndex].iAddr );
            iInquirySockAddr.SetBTAddr( sa.BTAddr() );            
            }
        }
    if (action)
        {
        iInquirySockAddr.SetAction( action ); 
        iInquirySockAddr.SetIAC(KGIAC);
        iHostResolver.GetByAddress( iInquirySockAddr, iEntry, iScanner->RequestStatus() );
        iScanner->GoActive();
        }
    else
        {
        FLOG(_L("[BTNOTIF]\t CBTInqUI::RunL() All name inquiries complete ."));
        iPageForName = EFalse;
        iCurrentlyResolvingUnnamedDeviceIndex = 0;
        InquiryComplete( KErrNone );
        }
    FLOG(_L("[BTNOTIF]\t CBTInqUI::IssueRequestL() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::RequestCompletedL
// BT-device or timeout has been received. Inform caller for
// received device and issue next EIR/Name request.
// ----------------------------------------------------------
//
void CBTInqUI::RequestCompletedL( CBTNotifActive* aActive, TInt aId, TInt aStatus )
    {
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::RequestCompletedL() status: %d >>"), aStatus ));
    ASSERT( aId == KBTNotifInquiryNotifierReq);
    (void) aActive;

    TInt firstPartialNameDevIndex = -1;
    const TBool haveDevsWithPartialName = HaveDevsWithPartialName(firstPartialNameDevIndex);

    if( aStatus == KErrNone )
        {
        if( iPageForName )
            {
            FLOG(_L("[BTNOTIF]\t CBTInqUI::RequestCompletedL() in name request state."));

            if (haveDevsWithPartialName)
                {
                // We resolve names in chronological order so it must be the first device
                // with an incomplete name we can find on the last seen list.
                HandleUpdatedNameL(firstPartialNameDevIndex);
                }
            else
                {
                // Must be the current index in iDevsWithoutName.
                HandleFoundNameL();
                iCurrentlyResolvingUnnamedDeviceIndex++;
                }
            IssueRequestL();
            }
        else
            {
            HandleInquiryDeviceL();
            iHostResolver.Next( iEntry, iScanner->RequestStatus() );
            iScanner->GoActive();
            }
        }
    else if( aStatus == (KHCIErrorBase - EPageTimedOut) && iPageForName )
        {
        FLOG(_L("[BTNOTIF]\t CBTInqUI::RequestCompletedL() HCI:EPageTimeOut, page next one."));
        if (haveDevsWithPartialName)
            {
            // A device with incomplete name has probably gone out of range. Just removing
            // it from the UI may be a bit confusing, so let's just update its RSSI indicator
            // to minimum.
            PageTimeoutOnDeviceWithPartialNameL(firstPartialNameDevIndex);
            }
        else
            {
            iCurrentlyResolvingUnnamedDeviceIndex++;            
            }
        IssueRequestL();
        }
    else
        {
        if( !iPageForName && (iDevsWithoutName.Count() > 0 || haveDevsWithPartialName) )
            {
            FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::RequestCompletedL() have devs with incomplete name = %d, nameless devices %d, paging for name."), 
                    haveDevsWithPartialName, iDevsWithoutName.Count() ));
            iCurrentlyResolvingUnnamedDeviceIndex = 0;
            iPageForName = ETrue;
            IssueRequestL();
            }
        else
            {
            FLOG(_L("[BTNOTIF]\t CBTInqUI::RequestCompletedL() NameInquiryComplete with error or no device found."));
            HandleError( aActive, aId, aStatus );
            }
        }
    FLOG(_L("[BTNOTIF]\t CBTInqUI::RequestCompletedL() <<"));
    }

// ----------------------------------------------------------
// CBTInqUI::HandleError
// Inform UI from error occured.
// ----------------------------------------------------------
//
void CBTInqUI::HandleError( CBTNotifActive* aActive, TInt aId, TInt aError )
    {
    FLOG(_L("[BTNOTIF]\t CBTInqUI::HandleError()"));
    (void) aActive;
    (void) aId;
    iPageForName = EFalse;
    InquiryComplete( aError );
    FLOG(_L("[BTNOTIF]\t CBTInqUI::HandleError() completed"));
    }

// ----------------------------------------------------------
// CBTInqUI::DoCancelRequest
// ----------------------------------------------------------
//
void CBTInqUI::DoCancelRequest( CBTNotifActive* aActive, TInt aId )
    {
    (void) aActive;
    (void) aId;
    iHostResolver.Cancel();
    }
	
// ----------------------------------------------------------
// CBTInqUI::HandleInquiryDeviceL
// Inform of properties of the found BTdevice,   
// which passes the search filter. Its name would be retrived 
// later if not contained by the first round of inquiry.   
// ----------------------------------------------------------
//
void CBTInqUI::HandleInquiryDeviceL()
    {
    TInquirySockAddr& sa = TInquirySockAddr::Cast( iEntry().iAddr );
    
    if ( iDesiredDeviceClass.DeviceClass() == 0 || sa.MajorClassOfDevice() == iDesiredDeviceClass.MajorDeviceClass() )
        {
        FLOG(_L("[BTNOTIF]\t CBTInqUI::HandleInquiryDeviceL() The found device passes the search filter."));
#ifdef _DEBUG
    TBuf<12> devAddrString;
    sa.BTAddr().GetReadable(devAddrString);
    FTRACE(FPrint(_L("[BTNOTIF]\t BT Address: %S"), &devAddrString));
#endif
        TBTDeviceName devName;
        TBool isNameComplete(EFalse);
        TBool nameGotten = CheckEirDeviceName( iEntry, devName, isNameComplete );
        FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleInquiryDeviceL() EIR device name? %d, %S, complete = %d"), nameGotten, &devName, isNameComplete ));
        if( nameGotten )
            {
            DeviceAvailableL( iEntry(), devName, isNameComplete );
            }
        else
            {
            iDevsWithoutName.AppendL( iEntry() );
            }
        }
    }

// ----------------------------------------------------------
// CBTInqUI::HandleFoundNameL
// Inform of retrieved device name after 2nd inquiry.  
// ----------------------------------------------------------
//
void CBTInqUI::HandleFoundNameL()
    {
#ifdef _DEBUG
    TBuf<12> devAddrString;
    TInquirySockAddr& sa = TInquirySockAddr::Cast( iEntry().iAddr );
    sa.BTAddr().GetReadable(devAddrString);
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleFoundNameL() BT Address: %S"), &devAddrString));
#endif
    if( iEntry().iName != KNullDesC )
        {
        FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleFoundNameL() Name found: %S"), &(iEntry().iName) ));
        DeviceAvailableL( iDevsWithoutName[iCurrentlyResolvingUnnamedDeviceIndex], iEntry().iName, ETrue );
        }

    FLOG(_L("[BTNOTIF]\t CBTInqUI::HandleFoundNameL() Complete"));
    }

void CBTInqUI::HandleUpdatedNameL(TInt aLastSeenIndex)
    {
#ifdef _DEBUG
    TBuf<12> devAddrString;
    TInquirySockAddr& sa = TInquirySockAddr::Cast( iEntry().iAddr );
    sa.BTAddr().GetReadable(devAddrString);
    FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleUpdatedNameL() BT Address: %S"), &devAddrString));
#endif
    iLastSeenDevicesNameComplete[aLastSeenIndex] = ETrue;
    if( iEntry().iName != KNullDesC )
        {
        DeviceNameUpdatedL(iEntry(), aLastSeenIndex);
        }

    FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleUpdatedNameL() Complete")));
    }


// ----------------------------------------------------------
// CBTInqUI::CheckEirDeviceName
// Check if the retrieved the device info contains device name.  
// ----------------------------------------------------------
//
TBool CBTInqUI::CheckEirDeviceName( TNameEntry& aEntry, TBTDeviceName& aName, TBool& aIsComplete )
    {
    TBluetoothNameRecordWrapper eir( aEntry() );
    TInt length = eir.GetDeviceNameLength();

    TInt err( KErrNone );

    if( length > 0 )
        {            
        err = eir.GetDeviceName( aName, aIsComplete);
        return (!err);
        }
    return EFalse;
    }
// End of File
