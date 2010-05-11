/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/


#include "atcodec.h"
#include <mmtsy_names.h> // for etel
#include <bttypes.h>
#include <badesca.h>
#include "btmcprotocol.h"
#include "btmccallinghandler.h"
#include "btmcphonestatus.h"
#include "btmccallstatus.h"
#include "btmcprotocolstatus.h"
#include "btmcobserver.h"
#include "btmcmobileline.h"
#include "btmcnumber.h"
#include "btmcoperator.h"

#include "debug.h"

// Service Level Monitoring
const TInt KCmerFromHF = 0x01;
const TInt KCmerFromAG = 0x02;
const TInt KSLC_No3WayCalling = KCmerFromHF | KCmerFromAG;

const TInt KChldFromHF = 0x04;
const TInt KChldFromAG = 0x08;

const TInt KSLC_3WayCalling = KChldFromHF | KChldFromAG;

const TInt KBerUnknown = 99;

const TInt KServiceSlcTimer = 1;

const TInt KServiceGetSubscriber = 3;
const TInt KServiceGetOperator = 4;
const TInt KServiceCmdHandling = 5;

const TInt KQueryIMEI = 6;
const TInt KQueryIMSI = 7;

const TInt KServiceFirstHspCkpdTrap = 8;

CBtmcProtocol* CBtmcProtocol::NewL(
    MBtmcObserver& aObserver, TBtmcProfileId aProfile, const TDesC8& aBTDevAddr, TBool aAccessoryInitiated)
    {
    CBtmcProtocol* self=new (ELeave) CBtmcProtocol(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL(aProfile, aBTDevAddr, aAccessoryInitiated);
    CleanupStack::Pop(self);
    return self;
    }

CBtmcProtocol::~CBtmcProtocol()
    {
    TRACE_FUNC_ENTRY
    delete iCallingHandler;
    
    delete iCmdHanldingActive;
    delete iAtExt;
    
    delete iTimerActive;
    iTimer.Close();
    
    delete iProtocolStatus;
    delete iCallStatus;
    delete iPhoneStatus;
    
    delete iNumber;
    delete iOperator;
    
    delete iEtelQuery;
  
    iPhone.Close();
    iServer.UnloadPhoneModule(KMmTsyModuleName);
    iServer.Close();
    delete iOutgoPacketQueue;
    TRACE_FUNC_EXIT
    }

void CBtmcProtocol::NewProtocolDataL(const TDesC8& aData)
    {
    TRACE_FUNC
    TInt err = iInDataBuf.Append(aData);
    if (err)
        {
        CATResult* nok = CATResult::NewLC(EATERROR);
        SendResponseL(*nok);
        CleanupStack::PopAndDestroy(nok);
        }
    else if (!iCmdHanldingActive->IsActive())
        {
        TRequestStatus* sta = &iCmdHanldingActive->iStatus;
        *sta = KRequestPending;
        User::RequestComplete(sta, KErrNone);
        iCmdHanldingActive->GoActive();
        }
    }

void CBtmcProtocol::SendResponseL(const CATResult& aResult)
    {
    TRACE_FUNC
    iOutgoPacketQueue->InsertL(0, aResult.Des());
    iCredit = iOutgoPacketQueue->MdcaCount();
    DoSendProtocolDataL();
    }
    
void CBtmcProtocol::SendResponseL(const RPointerArray<CATResult>& aResults)
    {
    TRACE_FUNC
    TInt count = aResults.Count();
    for (TInt i = 0; i < count; i++)
        {
        iOutgoPacketQueue->InsertL(i, aResults[i]->Des());
        }
    iCredit = iOutgoPacketQueue->MdcaCount();
    DoSendProtocolDataL();
    }
    
void CBtmcProtocol::SendUnsoltResultL(const CATResult& aResult)
    {
    TRACE_FUNC_ENTRY
    if (aResult.Id() == EATVGS)
        {
        iOutgoPacketQueue->InsertL(0, aResult.Des());
        iCredit++;
        }
    else
        {
        iOutgoPacketQueue->AppendL(aResult.Des());
        if (!iCallingHandler->ActiveCmdHandling() && !iHandleCmdPending)
            {
            iCredit++;
            }
        }
    DoSendProtocolDataL();
    TRACE_FUNC_EXIT
    }
    
void CBtmcProtocol::CmdHandlingCompletedL()
    {
    TRACE_FUNC
    iHandleCmdPending = EFalse;
    TRequestStatus* sta = &iCmdHanldingActive->iStatus;
    *sta = KRequestPending;
    User::RequestComplete(sta, KErrNone);
    iCmdHanldingActive->GoActive();
    }

TBtmcProtocolStatus& CBtmcProtocol::ProtocolStatus()
    {
    return *iProtocolStatus;
    }

void CBtmcProtocol::VoiceRecognitionError()
    {
    iPhoneStatus->SetRecognitionInitiator(EBTMonoVoiceRecognitionDefaultInitiator);
    }

void CBtmcProtocol::HandleNrecCompletedL(TInt aErr)
    {
    TATId id = aErr ? EATERROR : EATOK;
    CATResult* nok = CATResult::NewLC(id);
    SendResponseL(*nok);
    CleanupStack::PopAndDestroy(nok);
    CmdHandlingCompletedL();
    }


void CBtmcProtocol::ActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    if (iPhoneStatus)
        iPhoneStatus->ActivateRemoteVolumeControl();
    else
        {
        TRACE_INFO(_L("WARNING, null vol handler!"))
        }
    }

void CBtmcProtocol::DeActivateRemoteVolumeControl()
    {
    TRACE_FUNC
    if (iPhoneStatus)
        iPhoneStatus->DeActivateRemoteVolumeControl();
    else
        {
        TRACE_INFO(_L("WARNING, null vol handler!"))
        }
    }

TInt CBtmcProtocol::GetRemoteSupportedFeature()
    {
    return iProtocolStatus->iAccSupportedFeature;
    }

TBool CBtmcProtocol::ActiveChldHandling() const
    {
    return iCallingHandler->ActiveChldHandling();
    }

void CBtmcProtocol::SetHspRvcSupported(TBool aSupported)
    {
    TRACE_FUNC
    // Only update the supported feature field if this
    // is for HSP controlling to avoid denial of
    // HFP service
    if ( iProtocolStatus->iProfile == EBtmcHSP )
        {
        if ( aSupported )
            {
            // volume control must be enabled now:
            if ( !iPhoneStatus)
                {
                TRAP_IGNORE( iPhoneStatus = 
                    CBtmcPhoneStatus::NewL(*this, iPhone, iProtocolStatus->iProfile) );
                }
            if ( iPhoneStatus )
                {
                // We just do the best effort. If enabling volume control fails,
                // other functionalities on the HSP will still work:
                TRAP_IGNORE( iPhoneStatus->SetVolumeControlFeatureL(ETrue) );
                }
            }
        else
            {
            // When this serves the controlling of a HSP connection, iPhoneStatus is only
            // used for volume control, deleting it will disable volume control:
            delete iPhoneStatus;
            iPhoneStatus = NULL;
            }
        }
    }

void CBtmcProtocol::RequestCompletedL(CBtmcActive& aActive, TInt aErr)
    {
    TRACE_FUNC_ENTRY
    switch (aActive.ServiceId())
        {
        case KServiceSlcTimer:
            {
            if (aErr == KErrNone)
                {
                StopTimer(KServiceSlcTimer);
                iObserver.SlcIndicateL(EFalse);
                }
            break;
            }
        case KServiceGetSubscriber:
            {
            delete iNumber;
            iNumber = NULL;
            CmdHandlingCompletedL();
            break;
            }
        case KServiceGetOperator:
            {
            delete iOperator;
            iOperator = NULL;
            CmdHandlingCompletedL();
            break;
            }
        case KServiceCmdHandling:
            {
            TRAPD(err, DoHandleCommandL());
            if (err)
                {
                CATResult* nok = CATResult::NewLC(EATERROR);
                SendResponseL(*nok);
                CleanupStack::PopAndDestroy(nok);
                CmdHandlingCompletedL();
                }
            break;
            }
       	case KQueryIMSI:
       		{
       		TBuf8<RMobilePhone::KIMSISize> buf;
       		buf.Copy(iId);
            CATResult* cimi = CATResult::NewLC(EATCIMI, EATActionResult, TATParam(buf));
            SendResponseL(*cimi);
	        CleanupStack::PopAndDestroy(cimi);
	        CATResult* ok = CATResult::NewLC(EATOK);
	        SendResponseL(*ok);
	        CleanupStack::PopAndDestroy(ok);
	        CmdHandlingCompletedL();      		
					break;
       		}
       	case KServiceFirstHspCkpdTrap:
       	    {
       	    // No handling here. From now on, any incoming CKPD command 
       	    // will be processed normally.
       	    TRACE_INFO((_L("CKPD trapper terminated")));
       	    break;
       	    }
        default:
            break;
        }
    TRACE_FUNC_EXIT
    }

void CBtmcProtocol::CancelRequest(TInt aServiceId)
    {
    TRACE_FUNC_ENTRY
    if (aServiceId == KServiceSlcTimer || aServiceId == KServiceFirstHspCkpdTrap )
        {
        iTimer.Cancel();
        }
    TRACE_FUNC_EXIT
    }

CBtmcProtocol::CBtmcProtocol(MBtmcObserver& aObserver)
    : iObserver(aObserver),
      iSlcMask(0x00)
    {
    }

void CBtmcProtocol::ConstructL(TBtmcProfileId aProfile, const TDesC8& aBTDevAddr, TBool aAccessoryInitiated)
    {
    TRACE_FUNC_ENTRY
    iProtocolStatus = new (ELeave) TBtmcProtocolStatus();
    iProtocolStatus->iProfile = aProfile;
    iAccessoryInitiated = aAccessoryInitiated;
    iProtocolStatus->iAGSupportedFeature = KBTAGSupportedFeatureV15;
    
    LEAVE_IF_ERROR(iServer.Connect())
    TInt err = iServer.LoadPhoneModule(KMmTsyModuleName);
    if (err != KErrNone && err != KErrAlreadyExists)
        LEAVE(err);
    LEAVE_IF_ERROR(iPhone.Open(iServer, KMmTsyPhoneName))

    iCallStatus = CBtmcCallStatus::NewL(*this, iPhone, aProfile);
    iProtocolStatus->iCallBits = iCallStatus->CallStatusL();
    
    iEtelQuery = CBtmcActive::NewL(*this, CActive::EPriorityStandard, KQueryIMEI);

    iPhone.GetPhoneId(iEtelQuery->iStatus, iIdentity);
    iEtelQuery->GoActive();
    
    switch (aProfile)
        {
        case EBtmcHFP0105:
            {
            TRACE_INFO((_L("constructing option HFP 1.5")))
            iPhoneStatus = CBtmcPhoneStatus::NewL(*this, iPhone, aProfile);
            StartTimerL(KServiceSlcTimer, KSlcTimeout);
            break;
            }
        case EBtmcHSP:
            {
            TRACE_INFO((_L("constructing option HSP")))
            if (aBTDevAddr.Length() != KBTDevAddrSize)
                {
                LEAVE(KErrBadDescriptor);
                }
            // HSP doesn't have an SLC protocol except the RFCOMM connection itself:
            iProtocolStatus->iSlc = ETrue;
            if ( iAccessoryInitiated && ( iProtocolStatus->iCallBits & KCallConnectedBit ) )
                {
                TRACE_INFO((_L("Incoming HSP connected, start CKPD trapper")));
                // In case of incoming HSP connection during a voice call, 
                // start timer to trap the CKPD command as part of the audio transfer of HSP.
                // If the first CKPD command is received during this period, it will be ignored.
                StartTimerL(KServiceFirstHspCkpdTrap, KFirstHspCkpdTimeout);
                }
            break;
            }
        default:
            LEAVE(KErrArgument);
        }
    TRAP_IGNORE(iAtExt = CHFPAtCmdHandler::NewL(*this));
    iOutgoPacketQueue = new (ELeave) CDesC8ArrayFlat(1);
    iCallingHandler = CBtmcCallingHandler::NewL(*this);
    iCmdHanldingActive = CBtmcActive::NewL(*this, CActive::EPriorityStandard, KServiceCmdHandling);
    if (iProtocolStatus->iSlc)
        {
        iCallStatus->ReportCallStatusL();
        }
    TRACE_FUNC_EXIT
    }

void CBtmcProtocol::DoHandleCommandL()
    {
    TRACE_FUNC
    if (iHandleCmdPending)
        {
        return;
        }
    TBuf8<KMaxATSize> cmddes;
    if (iInDataBuf.NextCommand(cmddes))
        {
        return;
        }
    if (cmddes.Length() == 0)
        {
        return;
        }
    TRACE_INFO_SEG(
        {
        TBuf8<KMaxATSize> buf;
        buf = cmddes;
        buf.Trim();
        Trace(_L8("[HFP] [I] %S"), &buf);
        })

    CATCommand* cmd = NULL;
    TRAPD(err, cmd = CATCommand::NewL(cmddes));
    if (err)
        {
      	if(iAtExt)
        	{
      		iAtExt->HandleCommand(cmddes, _L8("\n\rERROR\n\r"));
        	return;
        	}
        CATResult* nok = CATResult::NewLC(EATERROR);
        SendResponseL(*nok);
        CleanupStack::PopAndDestroy(nok);
        return;
        }
    CleanupStack::PushL(cmd);
    iHandleCmdPending = ETrue;
	TATId id = cmd->Id();
    if (id == EATA || 
        id == EATD2 || 
        id == EATD1 ||
        (id == EATCHLD && cmd->Type() != EATTestCmd)||
        id == EATCHUP || 
        id == EATVTS || 
        (id == EATBVRA && cmd->Type() == EATWriteCmd) ||
        id == EATBLDN )
    	{
    	if (id == EATBVRA)
    	    {
    	    TRACE_ASSERT(cmd->ParamNum() == 1, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(cmd->Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            TBTMonoVoiceRecognitionInitiator initor;
            initor = (value) ? EBTMonoVoiceRecognitionActivatedByRemote
                : EBTMonoVoiceRecognitionDeactivatedByRemote;
            iPhoneStatus->SetRecognitionInitiator(initor);
    	    }
    	iCallingHandler->HandleCallingCmdL(*cmd);
    	}
    else if ( id == EATCKPD )
        {
        if ( iTimerActive && iTimerActive->IsActive() )
            {
            // incoming HSP was just connected and the first CKPD is received, 
            // ignore this command as audio establishment will be done via Acc FW 
            StopTimer( KServiceFirstHspCkpdTrap );
            TRACE_INFO((_L("First CKPD, do nothing")));
            CATResult* nok = CATResult::NewLC( EATOK );
            SendResponseL(*nok);
            CleanupStack::PopAndDestroy(nok);
            CmdHandlingCompletedL();
            }
        else
            {
            iCallingHandler->HandleCallingCmdL(*cmd);
            }
        }
    else if (id == EATNREC && cmd->Type() == EATWriteCmd)
        {
	    TRACE_ASSERT(cmd->ParamNum() == 1, KErrArgument);
        TATParam param = TATParam();
        LEAVE_IF_ERROR(cmd->Parameter(0, param))
        TInt value;
        LEAVE_IF_ERROR(param.Int(value))
        if (!value &&
        	ServiceLevelConnected() )
            {
            //Always respond OK to +NREC=0
            HandleNrecCompletedL(0);
            }
        else
            {
            LEAVE(KErrArgument);
            }
        }
    else
        {
        // for HSP profile we support only VGS & VGM commands
        // CKPD is handled elsewhere
        if(iProtocolStatus->iProfile == EBtmcHSP &&
           ((cmd->Id() != EATVGS &&
             cmd->Id() != EATVGM)) )
            {
            LEAVE(KErrNotSupported);
            }

        switch (cmd->Type())
            {
            case EATTestCmd:
                {
                HandleTestCommandL(*cmd);
                break;
                }
            case EATReadCmd:
                {
                HandleReadCommandL(*cmd);
                break;
                }
            case EATWriteCmd:
                {
                HandleWriteCommandL(*cmd);
                break;
                }
            case EATActionCmd:
                {
                HandleActionCommandL(*cmd);
                break;
                }
            default:
                LEAVE(KErrNotSupported);
            }
        }
    CleanupStack::PopAndDestroy(cmd);

    if (!iProtocolStatus->iSlc && ServiceLevelConnected())
        {
        StopTimer(KServiceSlcTimer);
        if(iPhoneStatus)
            {
            iPhoneStatus->SetVolumeControlFeatureL(iProtocolStatus->iAccSupportedFeature & KHfFeatureBitVolumeControl);
            iPhoneStatus->SetVoiceRecognitionControlL(iProtocolStatus->iAccSupportedFeature & KHfFeatureBitVoiceRecognition);
            }
        iProtocolStatus->iSlc = ETrue;
        iCallStatus->ReportCallStatusL();
        iObserver.SlcIndicateL(ETrue);
        }
    }

// -----------------------------------------------------------------------------
// CBtmcProtocol::HandleTestCommand
// 
// Test Command handled by this method:
//    CHLD, BVRA, CIND, CCWA, CLIP
// -----------------------------------------------------------------------------
//
void CBtmcProtocol::HandleTestCommandL(const CATCommand& aCmd)
    {
    TRACE_FUNC
    RATResultPtrArray resarr;
    ATObjArrayCleanupResetAndDestroyPushL(resarr);
    
    TATParam param;
    switch (aCmd.Id())
        {
        case EAT:
            {
            break;
            }
        case EATCHLD:
            {
            param = TATParam(KDesTestCodeCHLDv15);
            iSlcMask = KSLC_3WayCalling;
            break;
            }
        case EATBVRA:
            {
            param = TATParam(KDesTestCodeBVRA);
            break;
            }
        case EATCIND:
            {
            param = TATParam(KDesTestCodeCINDv15);
            break;
            }
        case EATCCWA:
            {
            param = TATParam(KDesTestCodeCCWA);
            break;
            }
        case EATCLIP:
            {
            param = TATParam(KDesTestCodeCLIP);
            break;
            }
        case EATCREG:
	        {
        	param = TATParam(KDesTestCodeCCWA); // it is the same (0,1)
        	break;
    	    }
    	  case EATCOLP:
    	  	{
        	param = TATParam(KDesTestCodeCCWA); // it is the same (0,1)
        	break;    	  		
    	  	}
        default:
            LEAVE(KErrNotFound);
        }
    if (param.Type() != EATNullParam)
        {
        CATResult* code = CATResult::NewL(aCmd.Id(), EATTestResult, param);
        CleanupStack::PushL(code);
        resarr.AppendL(code);
        CleanupStack::Pop(code);
        }
    CATResult* ok = CATResult::NewL(EATOK);
    CleanupStack::PushL(ok);
    resarr.AppendL(ok);
    CleanupStack::Pop(ok);
    SendResponseL(resarr);
    CleanupStack::PopAndDestroy(&resarr);
    CmdHandlingCompletedL();
    }

// -----------------------------------------------------------------------------
// CBtmcProtocol::HandleReadCommand
// 
// Read Command handled by this method:
//     CIND, CLIP, COPS
// -----------------------------------------------------------------------------
//
void CBtmcProtocol::HandleReadCommandL(const CATCommand& aCmd)
    {
    TRACE_FUNC
    RATResultPtrArray resarr;
    ATObjArrayCleanupResetAndDestroyPushL(resarr);
    CATResult* code = NULL;
    RATParamArray params;
    CleanupClosePushL(params);
    switch (aCmd.Id())
        {
        case EATCIND:
            {
            // Network status
            if(!iPhoneStatus)
                {
                LEAVE(KErrNotSupported);
                }

            RMobilePhone::TMobilePhoneRegistrationStatus net = 
                    iPhoneStatus->NetworkStatus();

            if (net == RMobilePhone::ERegisteredOnHomeNetwork || 
                    net == RMobilePhone::ERegisteredRoaming)
                {
                LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoATNetworkAvailable)))
                }
            else
                {
                LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoATNetworkUnavailable)))
                }

            // call status
            if ((iProtocolStatus->iCallBits & KCallConnectedBit) ||
                    (iProtocolStatus->iCallBits & KCallHoldBit))
                {
                LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoATCallActive)))
                }
            else
                {
                LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoATNoCall)))
                }

            // Call setup status
            TInt callSetupInd = EBTMonoATNoCallSetup;

            if (iProtocolStatus->iCallBits & KCallRingingBit)
                {
                callSetupInd = EBTMonoATCallRinging;
                }
            else if (iProtocolStatus->iCallBits & KCallDiallingBit)
                {
                callSetupInd = EBTMonoATCallDialling;
                }
            else if (iProtocolStatus->iCallBits & KCallConnectingBit)
                {
                callSetupInd = EBTMonoATCallConnecting;
                }
            LEAVE_IF_ERROR(params.Append(TATParam(callSetupInd)))
            // call_setup == callsetup
            LEAVE_IF_ERROR(params.Append(TATParam(callSetupInd)))

            // Call held status
            TInt callHeldInd = EBTMonoATNoCallHeld;
            if( (iProtocolStatus->iCallBits & KCallHoldBit) && (iProtocolStatus->iCallBits & KCallConnectedBit) )
                {
                callHeldInd = EBTMonoATCallHeldAndActive;
                }
            else if(iProtocolStatus->iCallBits & KCallHoldBit)
                {
                callHeldInd = EBTMonoATCallHeldOnly;
                }
            LEAVE_IF_ERROR(params.Append(TATParam(callHeldInd)))
            // signal status
            LEAVE_IF_ERROR(params.Append(TATParam(iPhoneStatus->GetSignalStrength())))
            // roaming status
            if(net == RMobilePhone::ERegisteredRoaming)
                {
                LEAVE_IF_ERROR(params.Append(TATParam(1)))
                }
            else 
                {
                LEAVE_IF_ERROR(params.Append(TATParam(0)))
                }
            // battery charge
            LEAVE_IF_ERROR(params.Append(TATParam(iPhoneStatus->GetBatteryCharge())))

            code = CATResult::NewL(EATCIND, EATReadResult, &params);
            break;
            }
        case EATCLIP:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(iProtocolStatus->iCallerIdNotif)))
            LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoATCallerIdNetworkServiceUnknown)))
            code = CATResult::NewL(EATCLIP, EATReadResult, &params);
            break;
            }
        case EATCOPS:
            {
            iOperator = CBtmcOperator::NewL(*this, *this, CActive::EPriorityStandard, KServiceGetOperator);
            iOperator->GoActive();
            break;
            }
        case EATCREG:
            {
            if(!iPhoneStatus)
                {
                LEAVE(KErrNotSupported);
                }

            RMobilePhone::TMobilePhoneRegistrationStatus net = 
                    iPhoneStatus->NetworkStatus();
            switch(net)
                {
                case RMobilePhone::ERegistrationUnknown:
                    {
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregEnableUnsolicited)))
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregNetworkServiceUnknown)))
                    code = CATResult::NewL(EATCREG, EATReadResult, &params);
                    break;
                    }
                case RMobilePhone::ENotRegisteredEmergencyOnly:
                case RMobilePhone::ENotRegisteredNoService:
                    {
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregEnableUnsolicited)))
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregNetworkServiceNotRegistered)))
                    code = CATResult::NewL(EATCREG, EATReadResult, &params);
                    break;
                    }
                case RMobilePhone::ENotRegisteredSearching:
                case RMobilePhone::ERegisteredBusy:
                    {
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregEnableUnsolicited)))
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregNetworkServiceNotRegisteredSearching)))
                    code = CATResult::NewL(EATCREG, EATReadResult, &params);
                    break;
                    }
                case RMobilePhone::ERegisteredOnHomeNetwork:
                    {
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregEnableUnsolicited)))
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregNetworkServiceHomeNetwork)))
                    code = CATResult::NewL(EATCREG, EATReadResult, &params);
                    break;
                    }
                case RMobilePhone::ERegistrationDenied:
                    {
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregEnableUnsolicited)))
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregNetworkServiceRegistrationDenied)))
                    code = CATResult::NewL(EATCREG, EATReadResult, &params);
                    break;
                    }
                case RMobilePhone::ERegisteredRoaming:
                    {
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregEnableUnsolicited)))
                    LEAVE_IF_ERROR(params.Append(TATParam(EBTMonoCregNetworkServiceRegisteredRoaming)))
                    code = CATResult::NewL(EATCREG, EATReadResult, &params);
                    break;
                    }
                default:
                    TRACE_INFO(_L("Error: default in CREG"));
                    break;  
                };
            break;  
            }        
        case EATCOLP:
            {
            LEAVE_IF_ERROR(params.Append(TATParam(TInt(iProtocolStatus->iOutgoingCallNotif))))
            code = CATResult::NewL(EATCOLP, EATReadResult, &params);
            break;
            }
        default:
            LEAVE(KErrNotSupported);
        }
    CleanupStack::PopAndDestroy(&params);
    if (code)
        {     
        CleanupStack::PushL(code);
        resarr.AppendL(code);
        CleanupStack::Pop(code);
        CATResult* ok = CATResult::NewL(EATOK);
        CleanupStack::PushL(ok);
        resarr.AppendL(ok);
        CleanupStack::Pop(ok);
        SendResponseL(resarr);
        CmdHandlingCompletedL();
        }
    CleanupStack::PopAndDestroy(&resarr);
    }

// -----------------------------------------------------------------------------
// CBtmcProtocol::HandleWriteCommand
// 
// Write Command handled by this method:
//    CHLD BVRA, CCWA, CLIP, VGS, VGM, CMEE, COPS
// -----------------------------------------------------------------------------
//
void CBtmcProtocol::HandleWriteCommandL(const CATCommand& aCmd)
    {
    TRACE_FUNC
    RATResultPtrArray resarr;
    ATObjArrayCleanupResetAndDestroyPushL(resarr);    
    CATResult* code = NULL;
    switch (aCmd.Id())
        {
        case EATCMER:
            {
            TRACE_ASSERT(aCmd.ParamNum() >= 4, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(aCmd.Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            if (value == 3)
                {
                param = TATParam();
                LEAVE_IF_ERROR(aCmd.Parameter(3, param))
                LEAVE_IF_ERROR(param.Int(value))
                iProtocolStatus->iIndicatorNotif = (value == 1) ? 
                    KIndAllActivated : KIndAllDeActivated;
                TRACE_INFO((_L("Indicators enabled: %d"), value))
                }
            iSlcMask = KSLC_No3WayCalling;
            break;
            }
        case EATBRSF:
            {
            TRACE_ASSERT(aCmd.ParamNum() == 1, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(aCmd.Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            iProtocolStatus->iAccSupportedFeature = value;
            code = CATResult::NewL(EATBRSF, EATWriteResult, 
                TATParam(iProtocolStatus->iAGSupportedFeature));
            break;
            }
        case EATCCWA:
            {
            TRACE_ASSERT(aCmd.ParamNum() == 1, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(aCmd.Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            iProtocolStatus->iCallWaitingNotif = (TBTMonoATCallWaitingNotif)value;
            break;
            }
        case EATCLIP:
            {
            TRACE_ASSERT(aCmd.ParamNum() == 1, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(aCmd.Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            iProtocolStatus->iCallerIdNotif = (TBTMonoATCallerIdNotif) value ;
            break;
            }
        case EATVGS:
            {
            TRACE_ASSERT(aCmd.ParamNum() == 1, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(aCmd.Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            // In HFP, AT+VGS is not valid before service level connection
            // has established.
            // In HSP, AT+VGS is allowed at any time.
            if (!iPhoneStatus && iProtocolStatus->iProfile == EBtmcHSP )
                {
                iPhoneStatus = CBtmcPhoneStatus::NewL(*this, iPhone, iProtocolStatus->iProfile);
                iPhoneStatus->SetVolumeControlFeatureL(ETrue);
                }
            if(iPhoneStatus)
                {
                iPhoneStatus->SetSpeakerVolumeL(value);
                }
            break;
            }
        case EATVGM:
            {
            TRACE_ASSERT(aCmd.ParamNum() == 1, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(aCmd.Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            break;
            }
        case EATCMEE:
            {
            TRACE_ASSERT(aCmd.ParamNum() == 1, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(aCmd.Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            iProtocolStatus->iAdvancedErrorCode = value;
            break;
            }
        case EATCOPS:
            {
            break;
            }
        case EATBIA:
        	{
        	for(TInt i=0; i<aCmd.ParamNum(); i++)
	        	{
	        	TATParam param = TATParam();
	        	aCmd.Parameter(i, param);
	        	if(param.Type() == EATNullParam)
	        		continue;
	        	else
		        	{
					TInt value;
					param.Int(value);		        	
	        		SetIndicatorL(i+1, value);
		        	}
	        	}
	        break;
        	}
        case EATCREG:
        	{
            TRACE_ASSERT(aCmd.ParamNum() == 1, KErrArgument);
            TATParam param = TATParam();
            LEAVE_IF_ERROR(aCmd.Parameter(0, param))
            TInt value;
            LEAVE_IF_ERROR(param.Int(value))
            iProtocolStatus->iNetworkRegStatusNotif = TBool(value);        	
            break;
        	}
        case EATCOLP:
        	{
          TRACE_ASSERT(aCmd.ParamNum() == 1, KErrArgument);
          TATParam param = TATParam();
          LEAVE_IF_ERROR(aCmd.Parameter(0, param))
          TInt value;
          LEAVE_IF_ERROR(param.Int(value))
          iProtocolStatus->iOutgoingCallNotif = TBool(value);
          break;
        	}
        default:
            LEAVE(KErrNotSupported);
        }

    if (code)
        {
        CleanupStack::PushL(code);
        resarr.AppendL(code);
        CleanupStack::Pop(code);
        }
    CATResult* ok = CATResult::NewL(EATOK);
    CleanupStack::PushL(ok);
    resarr.AppendL(ok);
    CleanupStack::Pop(ok);
    SendResponseL(resarr);
    CleanupStack::PopAndDestroy(&resarr);
    CmdHandlingCompletedL();    
    }

// -----------------------------------------------------------------------------
// CBtmcProtocol::HandleActionCommandL
// 
// Action Command handled by this method:
//    CLCC CNUM
// -----------------------------------------------------------------------------
//
void CBtmcProtocol::HandleActionCommandL(const CATCommand& aCmd)
    {
    TRACE_FUNC
    RATResultPtrArray resarr;
    ATObjArrayCleanupResetAndDestroyPushL(resarr);    
    CATResult* code = NULL;
    RATParamArray params;
    CleanupClosePushL(params);
    switch (aCmd.Id())
        {
        case EATCNUM:
            {
            iNumber = CBtmcNumber::NewL(*this, *this, CActive::EPriorityStandard, KServiceGetSubscriber);
            iNumber->GoActive();
            break;
            }
        case EATCLCC:
            {
            iCallStatus->HandleClccL();
            break;
            }
        case EATCSQ:
            {
            TRACE_INFO(_L("Requesting Signal strength"));
            LEAVE_IF_ERROR(params.Append(TATParam(iPhoneStatus->GetRssiStrength())))
            LEAVE_IF_ERROR(params.Append(TATParam(KBerUnknown)))
            code = CATResult::NewL(EATCSQ, EATActionResult, &params);
            TRACE_INFO(_L("done"));
            break;
            }
        case EATCGMI:
            {
            TBuf8<RMobilePhone::KPhoneManufacturerIdSize> buf;
            buf.Copy(iIdentity.iManufacturer);
            LEAVE_IF_ERROR(params.Append(TATParam(buf)))
            code = CATResult::NewL(EATCGMI, EATActionResult, &params);
            break;
            }
        case EATCGMM:
            {
            TBuf8<RMobilePhone::KPhoneModelIdSize> buf;
            buf.Copy(iIdentity.iModel);
            LEAVE_IF_ERROR(params.Append(TATParam(buf)))
            code = CATResult::NewL(EATCGMM, EATActionResult, &params);
            break;      
            }
        case EATCGMR:
            {
            TBuf8<RMobilePhone::KPhoneRevisionIdSize> buf;
            buf.Copy(iIdentity.iRevision);
            LEAVE_IF_ERROR(params.Append(TATParam(buf)))
            code = CATResult::NewL(EATCGMR, EATActionResult, &params);
            break;      
            }
        case EATCIMI:
            {
            iEtelQuery->SetServiceId(KQueryIMSI);
            iPhone.GetSubscriberId(iEtelQuery->iStatus, iId);
            iEtelQuery->GoActive();
            break;
            }
        case EATCGSN:
            {
            TBuf8<RMobilePhone::KPhoneSerialNumberSize> buf;
            buf.Copy(iIdentity.iSerialNumber);
            LEAVE_IF_ERROR(params.Append(TATParam(buf)))
            code = CATResult::NewL(EATCGSN, EATActionResult, &params);
            break;
            }
        default:
            LEAVE(KErrNotSupported);
        }    
    CleanupStack::PopAndDestroy(&params);
    if (code)
        {
        CleanupStack::PushL(code);
        resarr.AppendL(code);
        CleanupStack::Pop(code);
        CATResult* ok = CATResult::NewL(EATOK);
        CleanupStack::PushL(ok);
        resarr.AppendL(ok);
        CleanupStack::Pop(ok);
        SendResponseL(resarr);
        CmdHandlingCompletedL();
        }
    CleanupStack::PopAndDestroy(&resarr);
    }

void CBtmcProtocol::DoSendProtocolDataL()
    {
    TRACE_INFO((_L("credit %d"), iCredit))
    TInt count = iOutgoPacketQueue->MdcaCount();
    for (TInt i = 0; iCredit >0 && i < count; i++)
        {
        iCredit--;
        TBuf8<KMaxATSize> buf;
        buf.Copy(iOutgoPacketQueue->MdcaPoint(0));
        iObserver.SendProtocolDataL(buf);
        iOutgoPacketQueue->Delete(0);
        }
    }

void CBtmcProtocol::StartTimerL(TInt aService, TInt aTimeout)
    {
    if (!iTimerActive)
        {
        TRACE_FUNC_ENTRY
        iTimerActive = CBtmcActive::NewL(*this, CActive::EPriorityStandard, aService);
        LEAVE_IF_ERROR(iTimer.CreateLocal());
        iTimer.After(iTimerActive->iStatus, aTimeout);
        iTimerActive->GoActive();
        TRACE_FUNC_EXIT
        }
    else
        {
        TRACE_WARNING(_L("WARNING, timer is active already"))
        }
    }

void CBtmcProtocol::StopTimer(TInt aService)
    {
    if (iTimerActive && iTimerActive->ServiceId() == aService)
        {
        TRACE_FUNC_ENTRY
        delete iTimerActive;
        iTimerActive = NULL;
        iTimer.Close();
        TRACE_FUNC_EXIT
        }
    }
    
// -----------------------------------------------------------------------------
// CBtmcProtocol::IsServiceLevelEstablished
// -----------------------------------------------------------------------------
//
TBool CBtmcProtocol::ServiceLevelConnected() const
    {
    TRACE_FUNC
    TRACE_INFO((_L("SLC 0x%02x"), iSlcMask))
    if ((iProtocolStatus->iAccSupportedFeature & 0x02) && 
        (iProtocolStatus->iAGSupportedFeature & 0x01) &&
        (iSlcMask == KSLC_3WayCalling))
        {
        return ETrue;
        }
    if ((!(iProtocolStatus->iAccSupportedFeature & 0x02) ||
         !(iProtocolStatus->iAGSupportedFeature & 0x01)) &&
        (iSlcMask == KSLC_No3WayCalling))
        {
        return ETrue;
        }
    return EFalse;
    }

void CBtmcProtocol::SetIndicatorL(TInt aIndicator, TInt aValue)
	{
	TInt indBit( 0 );
	switch(aIndicator)
		{
		case EBTMonoATNetworkIndicator:
		    indBit = KIndServiceBit;
			break;
		case EBTMonoATCallIndicator:
		case EBTMonoATCallSetupIndicator:
		case EBTMonoATCall_SetupIndicator:
		case EBTMonoATCallHeldIndicator:
		    // call, call setup, call held indicators are mandatory.
		    break;
		case EBTMonoATSignalStrengthIndicator:
		    indBit = KIndSignalBit;
			break;
		case EBTMonoATRoamingIndicator:
		    indBit = KIndRoamBit;
			break;
		case EBTMonoATBatteryChargeIndicator:
		    indBit = KIndChargeBit;
			break;
		default:
			LEAVE(KErrArgument)
		}
	if ( indBit )
	    {
	    if ( aValue )
	        {
            iProtocolStatus->iIndicatorNotif |= indBit;
            }
        else
            {
            iProtocolStatus->iIndicatorNotif &= ~indBit;
            }
	    }
	}
void CBtmcProtocol::UnsolicitedResultFromATExtL(TInt aErr, const TDesC8& aAT)
    {
		TRACE_FUNC
		if(aErr)
			{
			delete iAtExt;
			iAtExt = NULL;
			}
		else
			{
	    iOutgoPacketQueue->AppendL(aAT);
	    if (!iCallingHandler->ActiveCmdHandling() && !iHandleCmdPending)
		    {
		    iCredit++;
		    }
    	DoSendProtocolDataL();
  		}
    }

void CBtmcProtocol::ATExtHandleCommandCompletedL(TInt aErr, const TDesC8& aReply)
		{
		TRACE_FUNC
		if(aErr)
			{
	    CATResult* nok = CATResult::NewLC(EATERROR);
	    SendResponseL(*nok);
	    CleanupStack::PopAndDestroy(nok);
			}
		else
			{
	    iOutgoPacketQueue->AppendL(aReply);
	    if (!iCallingHandler->ActiveCmdHandling() && !iHandleCmdPending)
		    {
		    iCredit++;
		    }
	    DoSendProtocolDataL();
			}
		}


// End of file
