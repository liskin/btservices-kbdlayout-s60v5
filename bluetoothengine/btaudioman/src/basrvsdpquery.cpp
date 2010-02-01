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
* Description: 
*       This class handles asynchronous SDP queries
*
*/


// INCLUDE FILES
#include "basrvsdpquery.h"
#include "btaccTypes.h"
#include "debug.h"

enum TQueryRequestId
    {
    ECreateView = 70,
    EGetResponse = 71   
    };


// ================= MEMBER FUNCTIONS =======================

CBasrvSdpQuery* CBasrvSdpQuery::NewL(MBasrvSdpQuerier& aQuerier)
	{
	CBasrvSdpQuery* self = new (ELeave) CBasrvSdpQuery(aQuerier);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

CBasrvSdpQuery::~CBasrvSdpQuery()
	{
	if (iSdpAgent)
		{
		iSdpAgent->Cancel();
		}
    delete iSdpAgent;
    delete iSdpSearchPattern;
    iServiceAttrs.Close();
    delete iResponse;
   	iReg.Close();
    delete iActive;
   	iRegServ.Close();
	}

void CBasrvSdpQuery::QueryAccInfoL(const TBTDevAddr& aAddr, TBool aTolerateSdpError)
    {
    if (iSdpAgent)
        {
		User::Leave(KErrInUse);
        }
    iAddr = aAddr;
    iServiceAttrs.Reset();
    TBTSdpAttrInt SF;        //TBTSupportedFeature SF; 
    SF.iAttrValue = 0;
    SF.iUUID = KAudioSinkUUID;  
    SF.iAttrID = KSdpAttrIdSupportedFeatures;        
    iServiceAttrs.Append(SF); // Advanced Audio Distribution Profile - BT Stereo Audio
    
    SF.iUUID = 0x111e;   // HFP
    SF.iAttrID = KSdpAttrIdSupportedFeatures;
    iServiceAttrs.Append(SF); // BT Handsfree Profile - BT Mono Audio
    
    SF.iUUID = 0x1108;   // HSP
    SF.iAttrID = 0x0302;  // Atti id of remote volume control
    iServiceAttrs.Append(SF); // BT Headset Profile - BT Mono Audio 

    SF.iUUID = KAVRemoteControlTargetUUID;
    SF.iAttrID = KSdpAttrIdSupportedFeatures;
    iServiceAttrs.Append(SF);  // Audio Video Remote Control Profile
    
    SF.iUUID = KAVRemoteControlTargetUUID;
    SF.iAttrID = KSdpAttrIdBluetoothProfileDescriptorList;
    iServiceAttrs.Append(SF);  // Audio Video Remote Control Profile, to find out avrcp 1.4 support    

    
    /*****************************************************
     *   Series 60 Customer
     *   You may want to consider modifying the following
     *   code to better meet your needs.
     *****************************************************/
    // Perform remote SDP query to find out if information about the exact 
    // model of the accessory is available in the SDP registry. NOTE that 
    // this query if using a proprietary UUID which will only exist in some
    // devices of a certain manufacturer. For other devices this query does
    // not find a value.
    //
    TUUID serviceUuid = TUUID((0x00005555),(0x00001000),(0x80000002),(0xEE000001)); // model number
    SF.iUUID = serviceUuid;  
    SF.iAttrID = 0x0300; // Attri id of device model number
    iServiceAttrs.Append(SF); 
    iCursor = 0;
    iTolerateSdpError = aTolerateSdpError;
    iSdpAgent = CSdpAgent::NewL(*this, iAddr);
    iSdpSearchPattern = CSdpSearchPattern::NewL();
    DoNextServiceRecordRequestL();
    }

void CBasrvSdpQuery::NextRecordRequestComplete(TInt aError,
                                             TSdpServRecordHandle aHandle,
                                             TInt aTotalRecordsCount)
    {
    TRAP_IGNORE(NextRecordRequestCompleteL(aError, aHandle, aTotalRecordsCount));
    }

void CBasrvSdpQuery::AttributeRequestResult(
    TSdpServRecordHandle aHandle,
    TSdpAttributeID aAttrID,
    CSdpAttrValue* aAttr)
    {
    TRAP_IGNORE(AttributeRequestResultL(aHandle, aAttrID, aAttr));
    }

void CBasrvSdpQuery::AttributeRequestComplete(TSdpServRecordHandle aHandle, TInt aError)
    {
    TRAP_IGNORE(AttributeRequestCompleteL(aHandle, aError));
    }

void CBasrvSdpQuery::RequestCompletedL(CBasrvActive& aActive)
    {
    switch (aActive.RequestId())
        {
        case ECreateView:
            {
            iResponse = NULL;
            if (aActive.iStatus > KErrNone)
                {
    		    iResponse = CBTRegistryResponse::NewL(iReg);	
    		    aActive.SetRequestId(EGetResponse);    
    		    iResponse->Start(aActive.iStatus);
        	    aActive.GoActive();
                }
            else
                {
                QueryCompleteL(KErrNone);
                }
            break;
            }
        case EGetResponse:
            {
            QueryCompleteL(KErrNone);
            break;
            }
        }
    }
    
void CBasrvSdpQuery::CancelRequest(CBasrvActive& /*aActive*/)
    {
    }

void CBasrvSdpQuery::ConstructL()
	{
	}

CBasrvSdpQuery::CBasrvSdpQuery(MBasrvSdpQuerier& aQuerier)
	: iQuerier(aQuerier)
	{
	}

void CBasrvSdpQuery::QueryCompleteL(TInt aErr)
    {
    iSdpAgent->Cancel();
    delete iSdpAgent;
    iSdpAgent = NULL;
    delete iSdpSearchPattern;
    iSdpSearchPattern = NULL;
    TBTDeviceClass cod = TBTDeviceClass();
	if ( iResponse && iResponse->Results().Count() > 0 )
    	{
    	cod = (iResponse->Results())[0]->DeviceClass();
    	}
    if (!aErr)
        {
        TAccInfo info;
        MakeAccInfo(info);
        iQuerier.GetAccInfoCompletedL(aErr, &info, &cod);
        }
    else
        {
        iQuerier.GetAccInfoCompletedL(aErr, NULL, &cod);
        }
    }

void CBasrvSdpQuery::DoNextServiceRecordRequestL()
    {
    if (iCursor >= iServiceAttrs.Count())
        {
        TInt err = iRegServ.Connect();
        if (!err)
            err = iReg.Open(iRegServ);
        if (err)
            {
            QueryCompleteL(KErrNone);
            }
        else
            {
            iActive = CBasrvActive::NewL(*this, CActive::EPriorityStandard, ECreateView);
            TBTRegistrySearch pattern;
        	pattern.FindAddress(iAddr);
    	    iReg.CreateView(pattern, iActive->iStatus);
    	    iActive->GoActive();
            }
        }
    else
        {
        iSdpSearchPattern->Reset();
        iSdpSearchPattern->AddL(iServiceAttrs[iCursor].iUUID);
        iSdpAgent->SetRecordFilterL(*iSdpSearchPattern);
        iSdpAgent->NextRecordRequestL();
        }
    }

void CBasrvSdpQuery::NextRecordRequestCompleteL(TInt aError,
                                             TSdpServRecordHandle aHandle,
                                             TInt aTotalRecordsCount)
    {
    TRACE_INFO((_L("NextRecord err <%d>, Handle <0x%x>, Records Count <%d>"), 
            aError, aHandle, aTotalRecordsCount));

    if(aError == KErrEof || (!aError && aTotalRecordsCount == 0))
        {
        iServiceAttrs.Remove(iCursor);
        DoNextServiceRecordRequestL();
        }
    else if(aError)
        {
        if (!iTolerateSdpError)
            {
            QueryCompleteL(aError);
            }
        else
            {
            for (TInt i = iServiceAttrs.Count() - 1; i >= iCursor; i--)
                {
                iServiceAttrs.Remove(i);
                }
            DoNextServiceRecordRequestL();
            }
        }
    else if(aTotalRecordsCount)
        {
        iSdpAgent->AttributeRequestL(aHandle, iServiceAttrs[iCursor].iAttrID);
        }
    }

void CBasrvSdpQuery::AttributeRequestResultL(
    TSdpServRecordHandle aHandle,
    TSdpAttributeID aAttrID,
    CSdpAttrValue* aAttr)
    {
    TRACE_INFO((_L("AttributeRequestResult, attr type %d"), aAttr->Type()));
    
    // Adds the attribute values synchronously to the result array.
    TRAP_IGNORE( aAttr->AcceptVisitorL( *this ) );
    
    TRACE_INFO((_L("handle <0x%x>, attr id <0x%04x>, value <%d>"), aHandle, aAttrID, iServiceAttrs[iCursor].iAttrValue));
    (void) aHandle;
    (void) aAttrID;
    delete aAttr;
    }

void CBasrvSdpQuery::AttributeRequestCompleteL(TSdpServRecordHandle aHandle, TInt aError)
    {
    TRACE_INFO((_L("AttributeRequestCompleteL err %d"), aError)); (void) aError;
    iSdpAgent->Cancel();
    TInt currentCursor = iCursor;
    iCursor++;
    
    if(iServiceAttrs[currentCursor].iUUID == KAVRemoteControlTargetUUID &&
       iServiceAttrs[currentCursor].iAttrID == KSdpAttrIdSupportedFeatures)
        {
        // We have some other information (avrcp version) in the same record
        // so lets get the information without quering the same record again.
        iSdpAgent->AttributeRequestL(aHandle, iServiceAttrs[iCursor].iAttrID);
        }
    else
        {        
        DoNextServiceRecordRequestL();
        }
    }

 void CBasrvSdpQuery::VisitAttributeValueL( CSdpAttrValue &aValue, TSdpElementType aType )
     {
     switch ( aType )
         {
         case ETypeUint:
             {
             TRACE_INFO((_L("attr value %d"), aValue.Uint()));
             iServiceAttrs[iCursor].iAttrValue = aValue.Uint();
             break;
             }
         case ETypeInt:
             {
             TRACE_INFO((_L("attr value %d"), aValue.Int()));
             iServiceAttrs[iCursor].iAttrValue = aValue.Int();
             break;
             }
         case ETypeBoolean:
             {
             TRACE_INFO((_L("attr value %d"), aValue.Bool()));
             iServiceAttrs[iCursor].iAttrValue = aValue.Bool();
             break;
             }
         }     
     }

 void CBasrvSdpQuery::StartListL( CSdpAttrValueList &aList )
     {
     (void) aList;
     }

 void CBasrvSdpQuery::EndListL()
     {
     
     }

void CBasrvSdpQuery::MakeAccInfo(TAccInfo& aInfo)
    {
    aInfo.SetBDAddress(iAddr);
    if (iServiceAttrs.Count())
        {
        aInfo.SetCapturingSupport(EFalse); // default indication as of CR 403-6409 CMLA change
        TUUID serviceUuid = TUUID((0x00005555),(0x00001000),(0x80000002),(0xEE000001)); // model number
        TUint8 suppProfiles = 0x00;
        for ( TUint ii = 0 ; ii < iServiceAttrs.Count() ; ++ii )
            {
            if ( iServiceAttrs[ii].iUUID == KAudioSinkUUID )
                {
                TRACE_INFO((_L("Accessory is A2DP Sink.")))
                suppProfiles |= EStereo;
                aInfo.SetSupportedFeature((TUint16)iServiceAttrs[ii].iAttrValue, TAccInfo::EStereoIndex);
                }
            else if ( iServiceAttrs[ii].iUUID == 0x111e )  // HFP
                {
                suppProfiles |= EHFP;
                TRACE_INFO((_L("Accessory supports Handsfree profile.")))                    
                aInfo.SetSupportedFeature((TUint16)iServiceAttrs[ii].iAttrValue, TAccInfo::EHFPIndex);
                }
            else if ( iServiceAttrs[ii].iUUID == 0x1108 ) // HSP
                {
                suppProfiles |= EHSP;
                TRACE_INFO((_L("Accessory supports Headset profile.")))                                        
                
                if (iServiceAttrs[ii].iAttrValue)
                    {
                    TRACE_INFO((_L("HSP Remote volume control is supported.")))
                    // Server is going to feed 0x10 to supported features for headset profile.
                    // The SDP of HSP doesn't really use supported feature attribute.
                    // We save RVC support in supported feature field for internal future use only.
                    // This approach is consistent with Accessory framework API.
                    aInfo.SetSupportedFeature(KHspRemoteVolumeControlSupport, TAccInfo::EHSPIndex);
                    }
                }
            else if ( iServiceAttrs[ii].iUUID == KAVRemoteControlTargetUUID )
                {
                if(iServiceAttrs[ii].iAttrID == KSdpAttrIdSupportedFeatures)
                    {
                    suppProfiles |= ERemConTG; // 0x08 remote control profile
                    TRACE_INFO((_L("Accessory is AVRCP TG, features 0x%02X"), iServiceAttrs[ii].iAttrValue))                            
                    aInfo.SetSupportedFeature(iServiceAttrs[ii].iAttrValue, TAccInfo::ERemConTGIndex);  // remote control = Position 3
                    }
                else // KSdpAttrIdBluetoothProfileDescriptorList, Avrcp version
                    {
                    if(iServiceAttrs[ii].iAttrValue == 0x0104)
                        {
                        aInfo.iAvrcpVersion = TAccInfo::EAvrcpVersion14;
                        }
                    else
                        {
                        aInfo.iAvrcpVersion = TAccInfo::EAvrcpVersionNo14;
                        }                    
                    }
                }
            else if (iServiceAttrs[ii].iUUID == serviceUuid)
                {
                aInfo.SetModel(iServiceAttrs[ii].iAttrValue);    
                TRACE_INFO((_L("Accessory is proprietary. Model: %d"), iServiceAttrs[ii].iAttrValue ))
                }
            }
        aInfo.SetSuppportedProfiles(suppProfiles);
        }    
    }

