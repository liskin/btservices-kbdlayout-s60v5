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
    
    TUint action = 0; 
    if( !iPageForName )
        {
        action = KHostResInquiry + KHostResEir + KHostResIgnoreCache;
        }
    else
        {
        if( iIndex < iDevsWithoutName.Count() )
            {
            action = KHostResName + KHostResIgnoreCache;
            TInquirySockAddr& sa = TInquirySockAddr::Cast( iDevsWithoutName[iIndex].iAddr );
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
        iIndex = 0;
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
    
    if( aStatus == KErrNone )
        {
        if( iPageForName )
            {
            HandleFoundNameL();
            iIndex++;
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
        iIndex++;
        IssueRequestL();
        }
    else
        {
        if( !iPageForName && iDevsWithoutName.Count()>0 )
            {
            FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::RequestCompletedL() nameless devices %d, paging for name."), 
                    iDevsWithoutName.Count() ));
            iIndex = 0;
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
        TBool nameGotten = CheckEirDeviceName( iEntry, devName );
        FTRACE(FPrint(_L("[BTNOTIF]\t CBTInqUI::HandleInquiryDeviceL() EIR device name? %d, %S"), nameGotten, &devName ));
        if( nameGotten )
            {
            DeviceAvailableL( iEntry(), devName );
            }
        else
            {
            iDevsWithoutName.Append( iEntry() );
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

        DeviceAvailableL( iDevsWithoutName[iIndex], iEntry().iName );
        }
    
    FLOG(_L("[BTNOTIF]\t CBTInqUI::HandleFoundNameL() Complete"));
    }

// ----------------------------------------------------------
// CBTInqUI::CheckEirDeviceName
// Check if the retrieved the device info contains device name.  
// ----------------------------------------------------------
//
TBool CBTInqUI::CheckEirDeviceName( TNameEntry& aEntry, TBTDeviceName& aName )
    {
    TBluetoothNameRecordWrapper eir( aEntry() );
    TInt length = eir.GetDeviceNameLength();
    
    TBool isComplete( EFalse );
    TInt err( KErrNone );
        
    if( length > 0 )
        {            
        err = eir.GetDeviceName( aName, isComplete);
        return (!err);
        }
    return EFalse;
    }
// End of File
