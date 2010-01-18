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
*                This class communicates with Nokia "Accessory Framework" 
*                to for e.g., get permission to attach/detach a remote bluetooth
*                device and also recieves requests to open/close audio connections.
*  Version     : %version:  2.2.3 %
*
*/


// INCLUDE FILES
#include "BTAccInfo.h"
#include "basrvaccfwconn.h"
#include "debug.h"

#include <AccPolCommonNameValuePairs.h>

//  CONSTANTS

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBasrvAccfwConn::CBasrvAccfwConn
// C++ default constructor can NOT contain any code that
// might leave.
// -----------------------------------------------------------------------------
//
CBasrvAccfwConn::CBasrvAccfwConn() 
    {
    }

// ---------------------------------------------------------
// Destructor
// ---------------------------------------------------------
//
CBasrvAccfwConn::~CBasrvAccfwConn()
    {
    iAccessoryBTControlSession.CloseSubSession();
    iAccessoryServerSession.Close();
    TRACE_FUNC
   }

// ---------------------------------------------------------
// CBasrvAccfwIf::NewL
// ---------------------------------------------------------
//
CBasrvAccfwIf* CBasrvAccfwIf::NewL(const TAccInfo* /*aInfo*/)
{
  return CBasrvAccfwConn::NewL();
}

// ---------------------------------------------------------
// CBasrvAccfwConn::NewL
// ---------------------------------------------------------
//
CBasrvAccfwIf* CBasrvAccfwConn::NewL()
    {
    CBasrvAccfwConn* self = new(ELeave) CBasrvAccfwConn();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }


// ---------------------------------------------------------
// ConstructL
// ---------------------------------------------------------
//
void CBasrvAccfwConn::ConstructL()
    {
    User::LeaveIfError( iAccessoryServerSession.Connect() );
    User::LeaveIfError( iAccessoryBTControlSession.CreateSubSession( iAccessoryServerSession ) );
    TRACE_FUNC
    }
    
// ---------------------------------------------------------
// CBasrvAccfwConn::CompleteReq
// ---------------------------------------------------------
//
void CBasrvAccfwConn::CompleteReq(TBool /*aUse*/)    
    {
    // Dos related implementation    
    }

// ---------------------------------------------------------
// CBasrvAccfwConn::AttachAccessory
// ---------------------------------------------------------
//
void CBasrvAccfwConn::AttachAccessory(const TBTDevAddr& aBDAddr, TRequestStatus &aStatus)
    {
    TRACE_INFO_SEG(
        {
           TBuf<12> buf;
        aBDAddr.GetReadable(buf);
        TRACE_INFO((_L("CBasrvAccfwConn::AttachAccessory, BTAddr: %S"), &buf))
        });
    iAccessoryBTControlSession.ConnectAccessory( aStatus, aBDAddr );                                                   
    }

// ---------------------------------------------------------
// CBasrvAccfwConn::CancelAttachAccessory
// ---------------------------------------------------------
//
void CBasrvAccfwConn::CancelAttachAccessory(const TBTDevAddr& /*aBDAddr*/)
    {
    iAccessoryBTControlSession.CancelConnectAccessory();                                                   
    TRACE_FUNC
    }
    
// ---------------------------------------------------------
// CBasrvAccfwConn::DetatchAccessory
// ---------------------------------------------------------
//
void CBasrvAccfwConn::DetatchAccessory( const TBTDevAddr& aBDAddr, TRequestStatus &aStatus )
    {
    TRACE_INFO_SEG(
        {
           TBuf<12> buf;
        aBDAddr.GetReadable(buf);
        TRACE_INFO((_L("CBasrvAccfwConn::DetatchAccessory, BTAddr: %S"), &buf))
        });
    iAccessoryBTControlSession.DisconnectAccessory( aStatus, aBDAddr );                                           
    }
    
// -----------------------------------------------------------------------------
// CBasrvAccfwConn::NotifyBTAudioLinkOpenReq
// -----------------------------------------------------------------------------
//
void CBasrvAccfwConn::NotifyBTAudioLinkOpenReq(TBTDevAddr& aBDAddress, 
                                                   TRequestStatus& aStatus, TAccAudioType& aType )
    {
    TRACE_FUNC
    iAccessoryBTControlSession.NotifyBluetoothAudioLinkOpenReq( aStatus, aBDAddress, aType ); 
    }

// -----------------------------------------------------------------------------
// CBasrvAccfwConn::NotifyBTAudioLinkCloseReq
// -----------------------------------------------------------------------------
//
void CBasrvAccfwConn::NotifyBTAudioLinkCloseReq(TBTDevAddr& aBDAddress, 
                                                    TRequestStatus& aStatus, TAccAudioType& aType )
    {
    TRACE_FUNC
    iAccessoryBTControlSession.NotifyBluetoothAudioLinkCloseReq( aStatus,  aBDAddress, aType );
    }

// -----------------------------------------------------------------------------
// CBasrvAccfwConn::CancelNotifyBTAudioLinkOpenReq
// -----------------------------------------------------------------------------
//
void CBasrvAccfwConn::CancelNotifyBTAudioLinkOpenReq()
    {
    iAccessoryBTControlSession.CancelNotifyBluetoothAudioLinkOpenReq(); 
    TRACE_FUNC
    }

// -----------------------------------------------------------------------------
// CBasrvAccfwConn::CancelNotifyBTAudioLinkCloseReq
// -----------------------------------------------------------------------------
//
void CBasrvAccfwConn::CancelNotifyBTAudioLinkCloseReq()
    {
    iAccessoryBTControlSession.CancelNotifyBluetoothAudioLinkCloseReq();
    TRACE_FUNC
    }


// -----------------------------------------------------------------------------
// CBasrvAccfwConn::AudioConnectionOpenComplete
// -----------------------------------------------------------------------------
//
void CBasrvAccfwConn::AudioConnectionOpenCompleteL( const TBTDevAddr& aBDAddress, 
                                                        TInt aResp, TInt aLatency )
   {

   TAccValueTypeTInt value; 
    value.iValue = aLatency;

    TRACE_INFO_SEG(
        {
           TBuf<12> buf;
        aBDAddress.GetReadable(buf);
        TRACE_INFO((_L("CBasrvAccfwConn::AudioConnectionOpenCompleteL, BTAddr: %S LATENCY: %d"), &buf, value.iValue))
        });
   iAccessoryBTControlSession.BluetoothAudioLinkOpenRespL( aBDAddress, aResp ) ;
    TAccPolNameRecord nameRecord;
    TRAPD(ret, nameRecord.SetNameL(KAccAudioLatency));
    if ( !ret )
        {
        TRAP_IGNORE(iAccessoryBTControlSession.AccessoryValueChangedNotifyL(aBDAddress, nameRecord, value));
        }
   }


// -----------------------------------------------------------------------------
// CBasrvAccfwConn::AudioConnectionCloseComplete
// -----------------------------------------------------------------------------
//
void CBasrvAccfwConn::AudioConnectionCloseCompleteL( const TBTDevAddr& aBDAddress,
                                                         TInt aResp )
    {
    TRACE_INFO_SEG(
        {
           TBuf<12> buf;
        aBDAddress.GetReadable(buf);
        TRACE_INFO((_L("CBasrvAccfwConn::AudioConnectionCloseCompleteL, BTAddr: %S"), &buf))
        });
    iAccessoryBTControlSession.BluetoothAudioLinkCloseRespL( aBDAddress, aResp ) ;
                                                  
    }


// -----------------------------------------------------------------------------
// CBasrvAccfwConn::NotifyAudioLinkOpen
// -----------------------------------------------------------------------------
//
void CBasrvAccfwConn::NotifyAudioLinkOpenL( const TBTDevAddr& aBDAddress, TAccAudioType aType )
    {
    TRACE_INFO_SEG(
        {
           TBuf<12> buf;
        aBDAddress.GetReadable(buf);
        TRACE_INFO((_L("CBasrvAccfwConn::NotifyAudioLinkOpenL, BTAddr: %S"), &buf))
        });
    iAccessoryBTControlSession.BluetoothAudioLinkOpenedNotifyL( aBDAddress, aType ) ;
    }


// -----------------------------------------------------------------------------
// CBasrvAccfwConn::NotifyAudioLinkClose
// -----------------------------------------------------------------------------
//
void CBasrvAccfwConn::NotifyAudioLinkCloseL( const TBTDevAddr& aBDAddress, TAccAudioType aType )
    {
    TRACE_INFO_SEG(
        {
           TBuf<12> buf;
        aBDAddress.GetReadable(buf);
        TRACE_INFO((_L("CBasrvAccfwConn::NotifyAudioLinkCloseL, BTAddr: %S"), &buf))
        });
    iAccessoryBTControlSession.BluetoothAudioLinkClosedNotifyL( aBDAddress, aType ) ;
    }

// -----------------------------------------------------------------------------
// CBasrvAccfwConn::DisableNREC
// -----------------------------------------------------------------------------
//
TInt CBasrvAccfwConn::DisableNREC(const TBTDevAddr& aBDAddress)
    {
    TRACE_INFO_SEG(
        {
           TBuf<12> buf;
        aBDAddress.GetReadable(buf);
        TRACE_INFO((_L("CBasrvAccfwConn::DisableNREC, BTAddr: %S"), &buf))
        });
    
    TAccValueTypeTBool value; 
    value.iValue = ETrue; 
    TAccPolNameRecord nameRecord;
    
    // Echo cancellation
    TRAPD(ret, nameRecord.SetNameL(KAccAcousticEchoControl));
    if (!ret)
        {
        TRAP(ret, iAccessoryBTControlSession.AccessoryValueChangedNotifyL( aBDAddress, 
                                                                 nameRecord,
                                                                 value )); 
        }
    if (!ret)
        {
        // Noise Reduction
        TRAP(ret, nameRecord.SetNameL(KAccNoiseReduction));
        if (!ret)
            {
            TRAP(ret, iAccessoryBTControlSession.AccessoryValueChangedNotifyL( aBDAddress, 
                                                                     nameRecord, 
                                                                        value )); 
            }
        }
    return ret; 
    }

// End of file
