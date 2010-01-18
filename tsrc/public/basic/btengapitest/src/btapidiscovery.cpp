/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Used for testing BT API
*  Revision    : $Revision: $
*  Date        : $Date: $
*
*/


#include <btengdiscovery.h>
#include <btengconstants.h>

#include "btapidiscovery.h"
#include "bttestlogger.h"

_LIT8(KServiceIdNokiaPcSuite, "0x000050050000100080000002EE000001");

// Constructor
CBTApiDiscovery::CBTApiDiscovery( MBTTestObserver& aObserver )
    : iObserver( aObserver )
    {
    }

// Destructor
CBTApiDiscovery::~CBTApiDiscovery( )
    {
    delete iBTEngDiscovery;
    delete iRemoteDevice;
    }


// NewL
CBTApiDiscovery* CBTApiDiscovery::NewL (MBTTestObserver& aObserver)
    {
    CBTApiDiscovery* self = new (ELeave) CBTApiDiscovery(aObserver);

    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }


// Symbian 2nd phase constructor.
void CBTApiDiscovery::ConstructL()
    {
    iBTEngDiscovery = CBTEngDiscovery::NewL( this );
    iBTEngDiscovery->SetNotifier( this );
    iError = KErrNone;
    }
    
void CBTApiDiscovery::ServiceSearchComplete( const RSdpRecHandleArray& aResult, 
                                     TUint aTotalRecordsCount, TInt aErr )
    {
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiDevman::HandleGetDevicesComplete: %d" ), aErr );
    iError = aErr;
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        }
    }
    
void CBTApiDiscovery::AttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                       const RSdpResultArray& aAttr, 
                                       TInt aErr )
    {
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiDevman::HandleGetDevicesComplete: %d" ), aErr );
    iError = aErr;
    if(iSdpResults.Count() == 0)
        {
        // Store one set of SDP results for furher testing
        for(TInt i = 0; i < aAttr.Count(); i++)
            {
            iSdpResults.Append(aAttr[i]);
            }
        }
    if(iWaiter.IsStarted())
        {
        iWaiter.AsyncStop();
        }
    }
    
void CBTApiDiscovery::ServiceAttributeSearchComplete( TSdpServRecordHandle aHandle, 
                                              const RSdpResultArray& aAttr, 
                                              TInt aErr )
    {
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiDevman::HandleGetDevicesComplete: %d" ), aErr );
    iError = aErr;
    if(iSdpResults.Count() == 0)
        {
        // Store one set of SDP results for furher testing
        for(TInt i = 0; i < aAttr.Count(); i++)
            {
            iSdpResults.Append(aAttr[i]);
            }
        }
    if(iWaiter.IsStarted())
        {
        iWaiter.AsyncStop();
        }
    }
    
void CBTApiDiscovery::DeviceSearchComplete( CBTDevice* aDevice, TInt aErr )
    {
    iObserver.Logger().Log( CBtTestLogger::ETLDebug, _L( "CBTApiDevman::HandleGetDevicesComplete: %d" ), aErr );
    iError = aErr;
    if ( iWaiter.IsStarted() )
        {
        iWaiter.AsyncStop();
        }
    }

TInt CBTApiDiscovery::SearchRemoteDeviceL()
    {
    TInt err = KErrNone;
    CBTDevice* device = NULL;
    
    device = CBTDevice::NewLC();
    
    /*
    err = iBTEngDiscovery->SearchRemoteDevice( device );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        CleanupStack::PopAndDestroy( device );
        return err;
        }
    if ( iError )
        {
        CleanupStack::PopAndDestroy( device );
        return iError;
        }
    */
    err = iBTEngDiscovery->SearchRemoteDevice( device );
    if ( !err )
        {
        iBTEngDiscovery->CancelSearchRemoteDevice();
        }

    CleanupStack::PopAndDestroy( device );
    return err;
    }

TInt CBTApiDiscovery::RegisterSdpRecord()
    {
    TInt err = KErrNone;
    TUUID service = EBTProfileSAP;
    
    err = iBTEngDiscovery->RegisterSdpRecord( service, 0, iHandle1 );
    if( err )
        {
        return err;
        }
    if ( iHandle1 == 0 )
        {
        return KErrNotFound;
        }
        
    // Test 128 bit UUID
    TLex8 lex;
	TUint32 HH; TUint32 HL; TUint32 LH; TUint32 LL;
	lex = KServiceIdNokiaPcSuite().Mid(2, 8);
	lex.Val(HH, EHex);
	lex = KServiceIdNokiaPcSuite().Mid(10, 8);
	lex.Val(HL, EHex);
	lex = KServiceIdNokiaPcSuite().Mid(18, 8);
	lex.Val(LH, EHex);
	lex = KServiceIdNokiaPcSuite().Mid(26, 8);
	lex.Val(LL, EHex);
    service = TUUID(HH, HL, LH, LL);
    
    err = iBTEngDiscovery->RegisterSdpRecord( service, 0, iHandle2 );
    if( err )
        {
        return err;
        }
    if ( iHandle2 == 0 )
        {
        return KErrNotFound;
        }
    return err;
    }

TInt CBTApiDiscovery::DeleteSdpRecord()
    {
    TInt err = KErrNone;
    
    if ( iHandle1 == 0 || iHandle2 == 0 )
        {
        return KErrArgument;
        }
    
    err = iBTEngDiscovery->DeleteSdpRecord( iHandle1 );
    if ( err )
        {
        return err;
        }
        
    err = iBTEngDiscovery->DeleteSdpRecord( iHandle2 );

    return err;    
    }
    
TInt CBTApiDiscovery::RemoteSdpQueryL( TBTDevAddr& aAddr )
    {
    TInt err = KErrNone;
    TUUID service = EBTProfileHSP;
    TBTDevAddr btAddr;
    delete iRemoteDevice;
    iRemoteDevice = NULL;
    
    /*
    iRemoteDevice = CBTDevice::NewL();
    
    err = iBTEngDiscovery->SearchRemoteDevice( iRemoteDevice );
    
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
    if ( iError )
        {
        return iError;
        }
    */
    
    btAddr = aAddr; // iRemoteDevice->BDAddr();
    
    err = iBTEngDiscovery->RemoteSdpQuery( btAddr, service );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
    if ( iError )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLError, _L( "RemoteSdpQuery failed!: %d" ), iError ); 
        // return iError;
        }
    
    err = iBTEngDiscovery->RemoteSdpQuery( btAddr, iHandle1, KSdpAttrIdProtocolDescriptorList );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
    if ( iError )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLError, _L( "RemoteSdpQuery failed!: %d" ), iError );
        // return iError;
        }

    err = iBTEngDiscovery->RemoteSdpQuery( btAddr, service, KSdpAttrIdProtocolDescriptorList );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
    if ( iError )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLError, _L( "RemoteSdpQuery failed!: %d" ), iError );
        // return iError;
        }
    
    err = iBTEngDiscovery->RemoteProtocolChannelQuery( btAddr, service );
    if ( !err )
        {
        iWaiter.Start();
        }
    else
        {
        return err;
        }
        
    if ( iError )
        {
        iObserver.Logger().Log( CBtTestLogger::ETLError, _L( "RemoteProtocolChannelQuery failed!: %d" ), iError );
        // return iError;
        }
    
    err = iBTEngDiscovery->RemoteProtocolChannelQuery( btAddr, service );
    iBTEngDiscovery->CancelRemoteSdpQuery();

    delete iRemoteDevice;
    iRemoteDevice = NULL;
    
    return err; 
    }
    
TInt CBTApiDiscovery::ParseSdpAttrValues()
    {
    if (iSdpResults.Count() == 0)
        {
        TBTEngSdpAttrValue attrValue;
        attrValue.iAttrValue.iValNumeric = 0;
        iSdpResults.Append( attrValue );
        }
    
    TSdpElementType type;
    TInt channel = KErrNotFound;
    TBool check = ETrue;
    
    for(TInt i = 0; i < iSdpResults.Count(); i++)
        {
        CBTEngDiscovery::ParseNextSdpAttrValueType( iSdpResults, i, type );
        if(type == ETypeNil ||
              type == ETypeUint ||
              type == ETypeUUID ||
              type == ETypeString ||
              type == ETypeBoolean ||
              type == ETypeDES ||
              type == ETypeDEA ||
              type == ETypeURL ||
              type == ETypeEncoded)
            {
            
            }
        else
            {
            
            }
        
        }

    CBTEngDiscovery::ParseRfcommChannel( iSdpResults, channel );
    if (channel > 0)
        {
        // Ok
        return KErrNone;
        }
    else
        {
        // Failed
        return KErrGeneral;
        }
    }
