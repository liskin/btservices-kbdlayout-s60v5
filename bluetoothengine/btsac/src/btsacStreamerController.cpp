/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation for CBTSACStreamerController class. This class constructs and
*				 communicates with 'BT Audio Streamer' component to do RTP streaming.
*
*/


// INCLUDE FILES
#include "btsacStreamerController.h"
#include "debug.h"

// Subband codec-specific values
// in bluetoothAV.h
using namespace SymbianSBC;      


// CONSTANTS
_LIT(KBTAADLL, "btaudioadaptation.dll");

//const TInt KUpgradeTimerDelay = 120000000;	// 2 minutes. How often we sill try to upgrade back to a better quality audio. 
const TInt KRetryTimerDelay = 2000000;		// 2 seconds. If something fails, how soon should we retry. 
const TInt KStabilizationDelay = 2000000;	// 2 seconds. Wait this long after bitpool change then start mononitor packet drops again
const TInt KDataCollectDelay = 600000; 		// Time (600ms) to collect packet drop data

// All tables below have to formed such a way that highest max bitpool value has to be in position 0 and etc.
const TInt KMaxBitpoolValues[] = {59, 48, 40, 34}; // Max bitpool values
const TInt KDeviationValues[] = {1000, 100000, 300000, 300000}; // maximum time (ys) between first and last packet dropped event

const TInt KNumOfBitpoolValues = sizeof(KMaxBitpoolValues)/sizeof(TInt);
#define MAXBITPOOLVALUEMAX KMaxBitpoolValues[0]
#define MAXBITPOOLVALUEMIN KMaxBitpoolValues[KNumOfBitpoolValues-1]

const float KReconfigureLimitFactor = 0.02; // 2% of during KDataCollectDelay transferred data
const float KLimitCheckInterval = ((float)KDataCollectDelay/1000000.0); // Time (seconds) to collect packet drop data

// MODULE DATA STRUCTURES

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBTSACStreamerController::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBTSACStreamerController* CBTSACStreamerController::NewL(MBTAudioAdaptationObserver& aObserver )
    {
    CBTSACStreamerController* self = new (ELeave) CBTSACStreamerController (aObserver);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

    
// -----------------------------------------------------------------------------
// Destructor.
// -----------------------------------------------------------------------------
//
CBTSACStreamerController::~CBTSACStreamerController()
    {
    TRACE_FUNC
    Cancel();
    if(iAudioInput)
	    {
	    iAudioInput->Stop();
	    iAudioInput->Disconnect();
	    }
    delete iPacketDropIoctl;
    delete iEncoderCI;
    delete iBTStreamer;
    delete iAudioInput;
    iLib.Close();
	iTimer.Close(); 
	iBitpoolData.Close();
    TRACE_INFO((_L("CBTSACStreamerController::~CBTSACStreamerController() completed")))
    }

// -----------------------------------------------------------------------------
// CBTSACStreamerController::CBTSACStreamerController
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CBTSACStreamerController::CBTSACStreamerController(MBTAudioAdaptationObserver& aObserver)
    : CActive( EPriorityNormal ), iObserver(aObserver)
    {
	CActiveScheduler::Add(this);
    }
    
// -----------------------------------------------------------------------------
// CBTSACStreamerController::ConstructL
// Symbian 2nd phase constructor.
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::ConstructL( )
    {
    TRACE_FUNC

	User::LeaveIfError(iTimer.CreateLocal()); 
	iThread = RThread();

	iBTStreamer = CBTAudioStreamer::NewL();

    LEAVE_IF_ERROR( iLib.Load( KBTAADLL ) );
    typedef TInt (*TOurLibraryFunction2)(MBTAudioStreamObserver&, MBTAudioErrorObserver&);
    TOurLibraryFunction2 newL2 = reinterpret_cast<TOurLibraryFunction2>(iLib.Lookup(1));
    iAudioInput = (CBTAudioStreamInputBase*)newL2(*iBTStreamer, *this);       

	TInt ret;
	ret = iAudioInput->Connect();
	if (ret) 
		{
		TRACE_INFO((_L("CBTSACStreamerController::ConstructL() Audio Input Connect Error %d"), ret))
		User::Leave(ret); 
		}

	ret = iAudioInput->SetFormat(KMMFFourCCCodeSBC);	
	if (ret)
   		{
 		TRACE_INFO((_L("CBTSACStreamerController::ConstructL() Audio Input SetFormat Error %d"), ret))
  		User::Leave(ret);
 		}

    iEncoderCI = reinterpret_cast<CSbcEncoderIntfc *> (iAudioInput->EncoderInterface(KUidSbcEncoderIntfc));
	User::LeaveIfNull(iEncoderCI);

	InitializeBitpoolDataL();
    iState = EStateIdle;
      
    TRACE_INFO((_L("CBTSACStreamerController::ConstructL() completed")))
    }
    
// -----------------------------------------------------------------------------
// CBTSACStreamerController::StartStream
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::StartStream(RSocket& aSocket, TInt aFrameLength )
	{
	TRACE_FUNC
	
	if (!(iBTStreamer) || !(iAudioInput) )    
		{
		return KErrNotFound;	
		}
    
    if (iState == EStateError)
    	{
    	iState = EStateIdle;
    	TInt err = iAudioInput->Connect();
		if (!err) 
			{
			err = iAudioInput->SetFormat(KMMFFourCCCodeSBC);
			}    	
		if (err)
	   		{
	   		TRACE_INFO((_L("CBTSACStreamerController::StartStream() Audio Input initialization Error %d"), err))
	  		return err;
	 		}
    	}
    
    if (iState == EStateIdle)    
    	{
	    TInt err = KErrNone;
	    TRAP( err, iBTStreamer->StartL(aSocket, aFrameLength, iAudioInput, iCurrentBitrate));

	    if ( err == KErrNone )
	    	{
	    	err = iAudioInput->Start(); 	
	    	}

		if ( err == KErrNone )
			{			
			TRACE_INFO((_L("CBTSACStreamerController::StartStream() Sending packet drop IOCTL")))
    		TRAP( err, iPacketDropIoctl = CActivePacketDropIoctl::NewL(*this, aSocket));
    		
    		if( err == KErrNone )
    			{
    			iPacketDropDeviation = 0;
				iTotalNbrOfDroppedPackets = 0;
    			iState = EStateStreaming;
				
				if(iCurrentBitpoolData->iMaxBitpoolValue < iMatchCap.MaxBitpoolValue())
					{
					TRACE_INFO((_L("CBTSACStreamerController::StartStream(), upgrade bitrate.")))
					// Streaming was stopped when the bitrate was lower than maximum, so upgrade it for starters.
					// iPacketDropIoctl->Start will be called after self completion.
					
					// Update current bitpool data to be highest possible. Highest possible bitpool value can be found from position 
					iCurrentBitpoolData = GetBitpoolData(0);
					if(iCurrentBitpoolData)
						{
						DoSelfComplete(EUpgradeBitrate, KErrNone);
						}
					else
						{
						err = KErrArgument;
						}
					}
				else if(iMatchCap.MaxBitpoolValue() > iBitpoolData[iBitpoolData.Count() - 1].iMaxBitpoolValue)
					{
					// Don't start receive packet drop events if max bitpool value is not bigger than min
					// bitpool value (we can't downgrade bitpool, we already have lowest possible value).
					DoSelfComplete(EStartStabilizingTimer, KErrNone);
					}
				TRACE_INFO((_L("CBTSACStreamerController::StartStream() completed")))
    			}
	    	}
	    if( err )
	    	{
	    	// In case someting went wrong stop both audio adaptation and streamer
	    	TRACE_INFO((_L("CBTSACStreamerController::StartStream() *Error: %d"), err))
	    	iAudioInput->Stop();
	    	iAudioInput->Disconnect();
	    	iBTStreamer->Stop();
      	
	    	//make sure iAudioInput->Connect() & SetFormat() get called in StartStream()
	    	iState = EStateError;
	    	}
	    return err;
    	}
    else
    	{
    	return KErrAlreadyExists;
    	}
	}
        
// -----------------------------------------------------------------------------
// CBTSACStreamerController::StopStream
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::StopStream()
	{
	TRACE_FUNC
    Cancel();
   	if (!(iBTStreamer) || !(iAudioInput))    
		{
		return KErrNotFound;	
		}
    else if (iState != EStateIdle )
    	{
      	iAudioInput->Stop();
      	iBTStreamer->Stop();
	   	delete iPacketDropIoctl; 
	   	iPacketDropIoctl = NULL; 
	   	iState = EStateIdle;    
	 	return KErrNone;	
    	}
    else
    	{
    	return KErrAlreadyExists; 
    	}
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::ResetAudioInput
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::ResetAudioInput()
	{
	TRACE_FUNC
	iAudioInput->Stop();
	iAudioInput->Disconnect();
    TInt err = iAudioInput->Connect();
	if (!err) 
		{
		err = iAudioInput->SetFormat(KMMFFourCCCodeSBC);
		}    	
	if (err)
		{
		TRACE_INFO((_L("CBTSACStreamerController::ResetAudioInput() Audio Input initialization Error %d"), err))
		}	
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::FillCapabilities
// -----------------------------------------------------------------------------
//	
TInt CBTSACStreamerController::FillCapabilities( TSBCCodecCapabilities& aSbc )
	{
	TRACE_FUNC

	TUint8 itemp = 0x00; 
    // Sampling Frequency
    TInt ret;
	RArray<TUint> SupportedSamplingFrequencies; 
	ret = iEncoderCI->GetSupportedSamplingFrequencies(SupportedSamplingFrequencies); 
	if ( !ret)
		{
		 itemp = 0x00; 
	
		 for (TInt i=0; i<SupportedSamplingFrequencies.Count(); i++ )
		 	{
			 if ( SupportedSamplingFrequencies[i] == TUint(16000) )
			 	{
				itemp = E16kHz;	// 	E16kHz		= 0x8 
				continue; 
			 	}
			 if ( SupportedSamplingFrequencies[i] == TUint(32000) )
			 	{
			 	itemp |= E32kHz; //	E32kHz		= 0x4
			 	continue; 
			 	}
			 if ( SupportedSamplingFrequencies[i] == TUint(44100) )
			 	{
			 	itemp |= E44100Hz;	// E44100Hz	= 0x2
			 	continue; 
				}
			 if ( SupportedSamplingFrequencies[i] == TUint(48000) )
			 	{
			 	itemp |= E48kHz;	// E48kHz	= 0x1
			 	continue; 
			 	}
		 	}
		 aSbc.SetSamplingFrequencies(itemp);
		 TRACE_INFO((_L("CBTSACStreamerController::FillCapabilities() Encoder Sampling Frequencies: %d"), itemp))
		}
	else
		{
		return ret; 
		}
	
	// Channel mode
    RArray<CSbcEncoderIntfc::TSbcChannelMode> SupportedChannelModes; 
    ret = iEncoderCI->GetSupportedChannelModes(SupportedChannelModes);
	if ( !ret )
		{
		 itemp = 0x00; 
	
		 for (TInt i=0; i<SupportedChannelModes.Count(); i++ )
		 	{
			 if ( SupportedChannelModes[i] ==  CSbcEncoderIntfc::ESbcChannelMono)
			 	{
				itemp = EMono;	// 	EMono			= 0x8	 		 		
				continue;		 		
			 	}
			 if ( SupportedChannelModes[i] == CSbcEncoderIntfc::ESbcChannelDual )
			 	{
		 		itemp |= EDualChannel;	// 	EDualChannel	= 0x4
			 	continue;
			 	}
			 if ( SupportedChannelModes[i] == CSbcEncoderIntfc::ESbcChannelStereo )
			 	{
				itemp |= EStereo;	// 	EStereo			= 0x2	 		
			 	continue;
				}
			 if ( SupportedChannelModes[i] == CSbcEncoderIntfc::ESbcChannelJointStereo )
			 	{
		 		itemp |= EJointStereo;	//	EJointStereo	= 0x1
			 	continue;
			 	}
		 	}
		 aSbc.SetChannelModes(itemp);
		 TRACE_INFO((_L("CBTSACStreamerController::FillCapabilities() Encoder Channel modes: %d"), itemp))
		}
	else
		{
		return ret; 
		}

    // Blocks
    RArray<TUint> SupportedNumOfBlocks; 
    ret = iEncoderCI->GetSupportedNumOfBlocks(SupportedNumOfBlocks); 
	if ( !ret )
		{
		 itemp = 0x00; 
	
		 for (TInt i=0; i<SupportedNumOfBlocks.Count(); i++ )
		 	{
			 if ( SupportedNumOfBlocks[i] ==  TUint(4))
			 	{
				itemp = EBlockLenFour;	//	 EBlockLenFour 		= 0x8		 		
				continue;		 		
			 	}
			 if ( SupportedNumOfBlocks[i] == TUint(8) )
			 	{
			 	itemp |= EBlockLenEight;	// 	EBlockLenEight 		= 0x4
			 	continue;
			 	}
			 if ( SupportedNumOfBlocks[i] == TUint(12) )
			 	{
			 	itemp |= EBlockLenTwelve;	// 	EBlockLenTwelve		= 0x2
			 	continue;
				}
			 if ( SupportedNumOfBlocks[i] == TUint(16) )
			 	{
			 	itemp |= EBlockLenSixteen;	// 	EBlockLenSixteen	= 0x1
			 	continue;
			 	}
		 	}
		 aSbc.SetBlockLengths(itemp);
		 TRACE_INFO((_L("CBTSACStreamerController::FillCapabilities() Encoder Blocks: %d"), itemp))
		}
	else
		{
		return ret; 
		}
		 
	// Subbands
	RArray<TUint> SupportedNumOfSubbands; 
	ret = iEncoderCI->GetSupportedNumOfSubbands(SupportedNumOfSubbands);
	if ( !ret )
		{
	    itemp = 0x0;
		for (TInt i=0; i<SupportedNumOfSubbands.Count(); i++ )
		 	{	
			 if ( SupportedNumOfSubbands[i] == TUint(4) )
			 	{
				itemp = EFourSubbands; // 	EFourSubbands  = 0x02
				continue; 
			 	}
			 if ( SupportedNumOfSubbands[i] == TUint(8) )
			 	{
			 	itemp |= EEightSubbands;	// 	EEightSubbands  = 0x01
			 	continue; 
			 	}
		 	}
		 aSbc.SetSubbands(itemp);
		 TRACE_INFO((_L("CBTSACStreamerController::FillCapabilities() Encoder SubBands: %d"), itemp))
		 }
	else
		{
		return ret; 
		}
		 
    // allocation method bitmask 
    RArray<CSbcEncoderIntfc::TSbcAllocationMethod> SupportedAllocationMethods;
    ret = iEncoderCI->GetSupportedAllocationMethods(SupportedAllocationMethods); 
	if ( !ret )
		{
	    itemp = 0x0;
		for (TInt i=0; i<SupportedAllocationMethods.Count(); i++ )
		 	{	
			 if ( SupportedAllocationMethods[i] == CSbcEncoderIntfc::ESbcAllocationSNR )
			 	{
				itemp = ESNR; // 	ESNR  = 0x02
				continue; 
			 	}
			 if ( SupportedAllocationMethods[i] == CSbcEncoderIntfc::ESbcAllocationLoudness )
			 	{
			 	itemp |= ELoudness;	// 	ELoudness  = 0x01
			 	continue; 
			 	}
		 	}
		 aSbc.SetAllocationMethods(itemp);
		 TRACE_INFO((_L("CBTSACStreamerController::FillCapabilities() Encoder Alloc method: %d"), itemp))
		 }
	else
		{
		return ret; 
		}
		 
	// bitpools
	TUint MinSupportedBitpoolSize; TUint MaxSupportedBitpoolSize; 
	ret = iEncoderCI->GetSupportedBitpoolRange(MinSupportedBitpoolSize, 
		                      				   MaxSupportedBitpoolSize);
	if ( !ret )
	 	{
		if (MaxSupportedBitpoolSize > MAXBITPOOLVALUEMAX)
	 	    {
	 	    MaxSupportedBitpoolSize = MAXBITPOOLVALUEMAX;
	 	    }
	 	// TUint8 to TInt
		aSbc.SetMaxBitpoolValue(MaxSupportedBitpoolSize);
		TRACE_INFO((_L("CBTSACStreamerController::FillCapabilities() Encoder Max bitpool: %d"), MaxSupportedBitpoolSize))
		aSbc.SetMinBitpoolValue(MinSupportedBitpoolSize);
		TRACE_INFO((_L("CBTSACStreamerController::FillCapabilities() Encoder Min bitpool: %d"), MinSupportedBitpoolSize))
	 	}
	else
		{
		return ret; 
		}	 
		 
    // save our local copy, 9 bytes on stack
    iLocalCap = aSbc;
	
	TRACE_INFO((_L("CBTSACStreamerController::FillCapabilities() Return: %d"), ret));	
	return ret;
	}
	
// -----------------------------------------------------------------------------
// CBTSACStreamerController::ConfigureSEP
// -----------------------------------------------------------------------------
//	
TInt CBTSACStreamerController::ConfigureSEP( TSBCCodecCapabilities& aDec )
	{
	TRACE_FUNC
	TInt err = KErrNone;
	TRAP(err, InitializeBitpoolDataL());
	if(err)
		return err;
	
	err = GetMatchingCaps(aDec);
	if(!err)
		{
		CheckAndAdjustBitpool(aDec);
		// Save the matching Caps
		iMatchCap = aDec;
		ReorganizeBitpoolTable(iMatchCap.MaxBitpoolValue());
		err = SetCurrentBitpoolData(iMatchCap.MaxBitpoolValue());
		if(!err)
			{
			iEncoderCI->SetBitpoolSize(iMatchCap.MaxBitpoolValue());
	  		// configure encoder
			err = iEncoderCI->ApplyConfig();
			}
		}
	TRACE_INFO((_L("CBTSACStreamerController::ConfigureSEP() return: %d"), err))
	return err;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::ConfigureSEPBitpool
// -----------------------------------------------------------------------------
//	
TInt CBTSACStreamerController::ConfigureSEPBitpool(EOngoingAction aAction)
	{
	TRACE_FUNC
    TInt err = KErrNone;
	TSBCCodecCapabilities newDecConfiguration; 
	memset(&newDecConfiguration, 0, sizeof(newDecConfiguration)); 
	newDecConfiguration = iMatchCap;
	TInt nextIndex = (aAction == EDowngradeBitrate) ? iCurrentBitpoolData->iDownBitpoolIndex : iCurrentBitpoolData->iUpBitpoolIndex;
	if(!IndexValid(nextIndex))
		{
		return KErrArgument;
		}
	TInt BitpoolValue = iBitpoolData[nextIndex].iMaxBitpoolValue;
	newDecConfiguration.SetMaxBitpoolValue(BitpoolValue);
	CheckAndAdjustBitpool(newDecConfiguration);
	
	TRACE_INFO((_L("CBTSACStreamerController::ConfigureSEPBitpool() Current FrameLength: %d"), iCurrentFrameLength))
	TRACE_INFO((_L("CBTSACStreamerController::ConfigureSEPBitpool() Current Bitrate: %d"), iCurrentBitrate))
	
	err = iBTStreamer->SetNewFrameLength(iCurrentFrameLength, iCurrentBitrate);
	if(!err)
		{
		TRACE_INFO((_L("CBTSACStreamerController::ConfigureSEPBitpool(), Reconfiguring the encoder...")))
		iEncoderCI->SetBitpoolSize(newDecConfiguration.MaxBitpoolValue());
		// configure encoder
		err = iEncoderCI->ApplyConfig();
		if(!err)
			{			
			err = SetCurrentBitpoolData(newDecConfiguration.MaxBitpoolValue());
			if(!err)
				{
				TRACE_INFO((_L("CBTSACStreamerController::ConfigureSEPBitpool(): Current BP: %d"), iCurrentBitpoolData->iMaxBitpoolValue))
				}			
			}
		TRACE_INFO((_L("CBTSACStreamerController::ConfigureSEPBitpool() The encoder reconfigured.")))
		}
	TRACE_INFO((_L("CBTSACStreamerController::ConfigureSEPBitpool() return: %d"), err))
	return err;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::FrameLength
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::FrameLength() const
	{
	TRACE_INFO((_L("CBTSACStreamerController::FrameLength() = %d"), iCurrentFrameLength))
	return iCurrentFrameLength;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::GetMatchingCaps
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::GetMatchingCaps(TSBCCodecCapabilities& aCaps)
	{
	TRACE_FUNC
	TInt ret = KErrNone; 
	TUint8 MatchingFreq, MatchingChnl, MatchingBlck, MatchingSub, MatchingAlloc;
	
	MatchingFreq = (iLocalCap.SamplingFrequencies() & aCaps.SamplingFrequencies());
	MatchingChnl = (iLocalCap.ChannelModes() & aCaps.ChannelModes()); 
	MatchingBlck = (iLocalCap.BlockLengths() & aCaps.BlockLengths());
	MatchingSub = (iLocalCap.Subbands() & aCaps.Subbands());
	MatchingAlloc = (iLocalCap.AllocationMethods() & aCaps.AllocationMethods());
	
	if ( 	 MatchingFreq  &&
			 MatchingChnl  &&
			 MatchingBlck 	&&
			 MatchingSub   &&
			 MatchingAlloc &&
		     ( iLocalCap.MinBitpoolValue() <= aCaps.MaxBitpoolValue() )  &&
			 ( iLocalCap.MaxBitpoolValue() >= aCaps.MinBitpoolValue() ) )
		{
		TRACE_INFO((_L("CBTSACStreamerController::GetMatchingCaps() accessory configuration suits us")))

		// Choose highest possible values that are supported
			
		// Sampling Frequency
		SetSamplingFrequency(aCaps, MatchingFreq);
		// Channel mode
		SetChannelMode(aCaps, MatchingChnl);
		// Blocks
		SetBlockLen(aCaps, MatchingBlck);			
		// Subbands			
		SetNumOfSubbands(aCaps, MatchingSub);	
		// Allocation method	
		SetAllocationMethod(aCaps, MatchingAlloc);		
		// Bitpool
		SetBitpoolValues(aCaps);
		}
	else // no match
		{
		TRACE_INFO((_L("CBTSACStreamerController::GetMatchingCaps() **No Matching Capabilities**")))
		ret = KErrNotFound; 
		}
	return ret;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::CheckAndAdjustBitpool
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::CheckAndAdjustBitpool(TSBCCodecCapabilities& aCap)
	{
  	TRACE_FUNC
	// ******* Adjust bitpool if necessary *****
	TInt targetbitrate; 
	TInt nChnls = GetNumOfChannels(aCap);
	TInt nSbands = GetNumOfSubbands(aCap);
	TInt nBlck  = GetBlockLen(aCap);
	TInt Freq = GetSamplingFrequency(aCap);
	TInt frameLength = FrameLengthFormula(aCap);
	// Note: this is the bitrate of the transmission. 
	TInt bitrateInt = 8 * frameLength * Freq / nSbands / nBlck; 
	
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), nChnls = %d"), nChnls))
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), nSbands = %d"), nSbands))
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), nBlck = %d"), nBlck))
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), Freq = %d"), Freq))
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), MaxBP = %d"), aCap.MaxBitpoolValue()))
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), Frame length = %d"), frameLength))
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), Bitrate = %d"), bitrateInt))

	// max bit rate:	320kb/s for mono, 
	//					512kb/s for two-channel modes
	if (aCap.ChannelModes() & EMono)
		{
		if (bitrateInt <= 320000.0)		
			{
			targetbitrate = bitrateInt;
		 	}
		 else // target bitrate=320kbps
		 	{
		 	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool() Adjusting...")))
		 	targetbitrate = 320000.0; 
		 	TInt BP = ( ((targetbitrate*nSbands*nBlck)/Freq) - 32 - (4*nSbands*nChnls) ) / (nBlck*nChnls); 
		 	aCap.SetMaxBitpoolValue(BP);
		 	}
		 }
	else
		 {
		 if (bitrateInt <= 512000.0)
		 	{
			targetbitrate = bitrateInt; 
		 	}
		else // target bitrate=512kbps
			{
			TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool() Adjusting...")))
			targetbitrate = 512000.0; 
			TInt BP = 0;
			if (aCap.ChannelModes() & EDualChannel)
		 		{
		 	  	BP = ( ((targetbitrate*nSbands*nBlck)/Freq) - 32 - (4*nSbands*nChnls) ) / (nBlck*nChnls); 	
	 	  		}
			else
				{
				TInt join = (aCap.ChannelModes() & EJointStereo) ? 1 : 0; 
		 	  	BP = ( ((targetbitrate*nSbands*nBlck)/Freq) - 32 - (4*nSbands*nChnls) -  (nSbands*join) ) / (nBlck);
				}
				
			aCap.SetMaxBitpoolValue(BP);
			// Calculate new frame length
			frameLength = FrameLengthFormula(aCap);
		 	}	
		}

	iCurrentFrameLength = frameLength;
	iCurrentBitrate = targetbitrate; 	
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), After Adjustment MaxBP: %d"), aCap.MaxBitpoolValue()))
	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), After Adjustment Frame length: %d"), iCurrentFrameLength))
  	TRACE_INFO((_L("CBTSACStreamerController::CheckAndAdjustBitpool(), After Adjustment Bitrate: %d"), iCurrentBitrate))
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::RunL
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::RunL()
	{
	TRACE_FUNC
	switch(iStatus.Int())
		{
   		case KErrNone:
      		{
			switch( iOngoingAction )
				{
				case EErrorSending:
					{
					TRACE_INFO((_L("CBTSACStreamerController::RunL(), ErrorSending")))
					
					TRACE_INFO((_L("CBTSACStreamerController::RunL(), TotalNbrOfDroppedPackets: %d"), iTotalNbrOfDroppedPackets))
					TInt TotalNbrOfDroppedBits = iTotalNbrOfDroppedPackets*8;
					TRACE_INFO((_L("CBTSACStreamerController::RunL(), Total dropped bits: %d"), TotalNbrOfDroppedBits))
					TRACE_INFO((_L("CBTSACStreamerController::RunL(), Current Bitrate: %d"), iCurrentBitrate))
					TInt ReconfigureLimit = KLimitCheckInterval*iCurrentBitrate*KReconfigureLimitFactor;
					TRACE_INFO((_L("CBTSACStreamerController::RunL(), Limit for reconfigure : %d bits"), ReconfigureLimit))
					TRACE_INFO((_L("CBTSACStreamerController::RunL(), PacketDropDeviation: %d (limit %d)"), iPacketDropDeviation, iCurrentBitpoolData->iMaxDeviation))
					
					if((TotalNbrOfDroppedBits > ReconfigureLimit) && (iPacketDropDeviation >= iCurrentBitpoolData->iMaxDeviation))
						{
						iPacketDropIoctl->Cancel();
						
						if(iCurrentBitpoolData->iMinimumMaxBitpool)
							{
							TRACE_INFO((_L("CBTSACStreamerController::RunL(), Minimum max bitpool reached.")))
							TRACE_INFO((_L("CBTSACStreamerController::RunL(), Stop receive packet drops.")))
							return;
							}												
						
						// Initialize values for upgrade timer. Only if setting min bitpool value fails, start retry timer.
						TInt delay = KStabilizationDelay;
						iOngoingAction = EStabilize;
						TInt err = ConfigureSEPBitpool(EDowngradeBitrate);							
						if(err)
							{
							iOngoingAction = EDowngradeBitrate;
							delay = KRetryTimerDelay;
							}

		    			iTimer.After(iStatus, delay);
		    			SetActive();
						}
					
					iPacketDropDeviation = 0;
					iTotalNbrOfDroppedPackets = 0;
					break;
					}					
				
				case EStabilize:
					// Start receive overflow indications again
					TRACE_INFO((_L("CBTSACStreamerController::RunL(), stabilize timer expired.")))
					iPacketDropIoctl->Start();
					break;

				case EDowngradeBitrate:
				case EUpgradeBitrate:
				case EStartStabilizingTimer:
					{
					TRACE_INFO((_L("CBTSACStreamerController::RunL(), Action = %d"), iOngoingAction))
					TInt err = KErrNone;
					TInt delay = KStabilizationDelay;
					if(iOngoingAction != EStartStabilizingTimer)
						{
						err = ConfigureSEPBitpool(iOngoingAction);
						}
					if(err)
						{
						delay = KRetryTimerDelay;
						}
					else
						{
	    				TRACE_INFO((_L("CBTSACStreamerController::RunL(), stabilize timer started")))
	    				iOngoingAction = EStabilize;
						}

    				iTimer.After(iStatus, delay);
    				SetActive();
					break;
					}				
				}
        	break;
      		}
      	
		case KErrGeneral: // We have received error from audio adaptation
			{
			TRACE_INFO((_L("CBTSACStreamerController::RunL(), KErrGeneral")))
			iAudioInput->Stop();
			iAudioInput->Disconnect();
			iObserver.NotifyError(KErrGeneral);
			break;
			}
			
   		default:
   			TRACE_INFO((_L("CBTSACStreamerController::RunL(), #default")))
      		break;
   		}  
    TRACE_INFO((_L("CBTSACStreamerController::RunL() completed")))
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::DoCancel
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::DoCancel()
	{
	TRACE_FUNC
    iTimer.Cancel();
    TRACE_INFO((_L("CBTSACStreamerController::DoCancel() completed")))
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::RunError
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::RunError(TInt /*aError*/)    
	{
	TRACE_FUNC
	return 0;
	}
    
// -----------------------------------------------------------------------------
// CBTSACStreamerController::Error
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::Error(const TInt aError) 
	{
	TRACE_INFO((_L("CBTSACStreamerController::Error() %d"), aError))
	TInt ret = KErrNone;
	switch(aError)
		{
		case KErrOverflow:
		case KErrUnderflow:
			{
			DoSelfComplete(EErrorSending, KErrNone);
			break;
			}
		case KErrGeneral:
			{
			if(iState != EStateError)
				{
				iState = EStateError;
				DoSelfComplete(iOngoingAction, KErrGeneral);
				}
			break;
			}
		default:
			{
			ret = KErrUnknown;
			break;
			}
		}
	return ret;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::GetCaps
// -----------------------------------------------------------------------------
//
TSBCCodecCapabilities& CBTSACStreamerController::GetCaps()
	{
	return iLocalCap;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::SetBlockLen
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::SetBlockLen(TSBCCodecCapabilities& aCap, TUint8 aBlockLen)
	{
	// Set Block Lenght, select highest one
	if ( aBlockLen & EBlockLenSixteen )
		{
		iEncoderCI->SetNumOfBlocks(TUint(16));
		aCap.SetBlockLengths(EBlockLenSixteen);
		TRACE_INFO((_L("CBTSACStreamerController::SetBlockLen(), BlockLength: EBlockLenSixteen")))
		}
	else if ( aBlockLen & EBlockLenTwelve )
		{
		iEncoderCI->SetNumOfBlocks(TUint(12));
		aCap.SetBlockLengths(EBlockLenTwelve);	
		TRACE_INFO((_L("CBTSACStreamerController::SetBlockLen(), BlockLength: EBlockLenTwelve")))
		}
	else if ( aBlockLen & EBlockLenEight )
		{
		iEncoderCI->SetNumOfBlocks(TUint(8));
		aCap.SetBlockLengths(EBlockLenEight);
		TRACE_INFO((_L("CBTSACStreamerController::SetBlockLen(), BlockLength: EBlockLenEight")))
		}
	else if ( aBlockLen & EBlockLenFour )
		{
		iEncoderCI->SetNumOfBlocks(TUint(4));
		aCap.SetBlockLengths(EBlockLenFour);
		TRACE_INFO((_L("CBTSACStreamerController::SetBlockLen(), BlockLength: EBlockLenFour")))
		}
	else
		{
		TRACE_INFO((_L("CBTSACStreamerController::SetBlockLen(), No matching Block Length")))
		}
	}
	
// -----------------------------------------------------------------------------
// CBTSACStreamerController::GetBlockLen
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::GetBlockLen(TSBCCodecCapabilities& aCap) const
	{
	TInt nBlck  = 0;
	switch( aCap.BlockLengths() )
		{
		case EBlockLenSixteen:
			nBlck = 16;
			break;
		case EBlockLenTwelve:
			nBlck = 12;
			break;
		case EBlockLenEight:
			nBlck = 8;
			break;
		case EBlockLenFour:
			nBlck = 4;
			break;
		default:
			TRACE_INFO((_L("CBTSACStreamerController::GetBlockLen(), #default")))
			nBlck = 16;
			break;
		}
	return nBlck;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::SetBitpoolValues
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::SetBitpoolValues(TSBCCodecCapabilities& aCap)
	{
	// Define max bitpool
	TInt MaxBP = 0;
	TBool ProperMaxBitpoolFound = EFalse;
	for(TInt i = 0 ; i < KNumOfBitpoolValues ; i++)
		{
		if(aCap.MaxBitpoolValue() >= KMaxBitpoolValues[i])
			{
			MaxBP = KMaxBitpoolValues[i];
			ProperMaxBitpoolFound = ETrue;
			break;
			}
		}
	if(!ProperMaxBitpoolFound)
		{
		// None of our proposed max bitpool values weren't suitable for accessory.
		// Let's use the one which was proposed by the accessory.
		MaxBP = aCap.MaxBitpoolValue();
		}
	
	// Define min bitpool. This bitpool value is negotiated with sink.
	TInt MinBP = (iLocalCap.MinBitpoolValue() < aCap.MinBitpoolValue()) ? aCap.MinBitpoolValue() : iLocalCap.MinBitpoolValue();
	
	// Define bitpool which is used for medium quality streaming (when streaming is interfered for some reason).
	// This is real lowest bitpool value which is used for streaming.
	
	// Check if selected max bitpool value is smaller than our default min bitpool value, if it is, set
	// minimum max bitpool value to be same as max value otherwise just update it with our default min value.
	if(MaxBP < MAXBITPOOLVALUEMIN)
		{
		// Update the minumum bitpool data
		iBitpoolData[iBitpoolData.Count()-1].iMaxBitpoolValue = MaxBP;
		}
	else if(MAXBITPOOLVALUEMIN < MinBP) // Check also that minimum max bitpool value is not smaller than bitpool which is negotiated for the link.
		{
		iBitpoolData[iBitpoolData.Count()-1].iMaxBitpoolValue = MinBP;
		}
	
	TRACE_INFO((_L("CBTSACStreamerController::SetBitpoolValues(), Remote Max Bitpool: %d"), aCap.MaxBitpoolValue()))
	TRACE_INFO((_L("CBTSACStreamerController::SetBitpoolValues(), Remote Min Bitpool: %d"), aCap.MinBitpoolValue()))
	
	aCap.SetMaxBitpoolValue(MaxBP);
	TRACE_INFO((_L("CBTSACStreamerController::SetBitpoolValues(), Max Bitpool: %d"), MaxBP))
	aCap.SetMinBitpoolValue(MinBP);
	TRACE_INFO((_L("CBTSACStreamerController::SetBitpoolValues(), Min Bitpool: %d"), MinBP))
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::SetAllocationMethod
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::SetAllocationMethod(TSBCCodecCapabilities& aCap, TUint8 aAllocationMethod)
	{
	// Set Allocation Method, select highest one
	if ( aAllocationMethod & ELoudness )
		{
		iEncoderCI->SetAllocationMethod(CSbcEncoderIntfc::ESbcAllocationLoudness);
		aCap.SetAllocationMethods(ELoudness);
		TRACE_INFO((_L("CBTSACStreamerController::SetAllocationMethod(), AllocationMethod: ELoudness")))
		}
	else if ( aAllocationMethod & ESNR )
		{
		iEncoderCI->SetAllocationMethod(CSbcEncoderIntfc::ESbcAllocationSNR);
		aCap.SetAllocationMethods(ESNR);
		TRACE_INFO((_L("CBTSACStreamerController::SetAllocationMethod(), AllocationMethod: ESNR")))
		}
	else
		{
		TRACE_INFO((_L("CBTSACStreamerController::SetAllocationMethod(), No matching Allocation Method")))
		}
	}
// -----------------------------------------------------------------------------
// CBTSACStreamerController::SetChannelMode
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::SetChannelMode(TSBCCodecCapabilities& aCap, TUint8 aChannelMode)
	{
	// Set Channel mode, select highest one
	if ( aChannelMode & EJointStereo )
		{
		iEncoderCI->SetChannelMode(CSbcEncoderIntfc::ESbcChannelJointStereo);
		aCap.SetChannelModes(EJointStereo);
		TRACE_INFO((_L("CBTSACStreamerController::SetChannelMode(), ChannelMode: EJointStereo")))
		}
	else if ( aChannelMode & EStereo )
		{
		iEncoderCI->SetChannelMode(CSbcEncoderIntfc::ESbcChannelStereo);
		aCap.SetChannelModes(EStereo);	
		TRACE_INFO((_L("CBTSACStreamerController::SetChannelMode(), ChannelMode: EStereo")))
		}
	else if ( aChannelMode & EDualChannel )
		{
		iEncoderCI->SetChannelMode(CSbcEncoderIntfc::ESbcChannelDual);
		aCap.SetChannelModes(EDualChannel);
		TRACE_INFO((_L("CBTSACStreamerController::SetChannelMode(), ChannelMode: EDualChannel")))
		}
	else if ( aChannelMode & EMono )
		{
		iEncoderCI->SetChannelMode(CSbcEncoderIntfc::ESbcChannelMono);
		aCap.SetChannelModes(EMono);
		TRACE_INFO((_L("CBTSACStreamerController::SetChannelMode(), ChannelMode: EMono")))
		}
	else
		{
		TRACE_INFO((_L("CBTSACStreamerController::SetChannelMode(), No matching Channel Mode")))
		}
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::SetSamplingFrequency
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::SetSamplingFrequency(TSBCCodecCapabilities& aCap, TUint8 aFrequency)
	{
	// Set Sampling Frequency, select highest one
	if ( aFrequency & E48kHz )
		{
		iEncoderCI->SetSamplingFrequency(TUint(48000));
		aCap.SetSamplingFrequencies(E48kHz);
		TRACE_INFO((_L("CBTSACStreamerController::SetSamplingFrequency(), Sampling Frequency: E48kHz")))
		}
	else if ( aFrequency & E44100Hz )
		{
		iEncoderCI->SetSamplingFrequency(TUint(44100));
		aCap.SetSamplingFrequencies(E44100Hz);
		TRACE_INFO((_L("CBTSACStreamerController::SetSamplingFrequency(), Sampling Frequency: E44100Hz")))
		}
	else if ( aFrequency & E32kHz )
		{
		iEncoderCI->SetSamplingFrequency(TUint(32000));
		aCap.SetSamplingFrequencies(E32kHz);
		TRACE_INFO((_L("CBTSACStreamerController::SetSamplingFrequency(), Sampling Frequency: E32kHz")))
		}
	else if ( aFrequency & E16kHz )
		{
		iEncoderCI->SetSamplingFrequency(TUint(16000));
		aCap.SetSamplingFrequencies(E16kHz);
		TRACE_INFO((_L("CBTSACStreamerController::SetSamplingFrequency(), Sampling Frequency: E16kHz")))
		}
	else
		{
		TRACE_INFO((_L("CBTSACStreamerController::SetSamplingFrequency(), No matching Sampling Frequency")))
		}
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::GetSamplingFrequency
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::GetSamplingFrequency(TSBCCodecCapabilities& aCap) const
	{
	TInt Freq  = 0;
	switch( aCap.SamplingFrequencies() )
		{
		case E48kHz:
			Freq = 48000;
			break;
		case E44100Hz:
			Freq = 44100;
			break;
		case E32kHz:
			Freq = 32000;
			break;
		case E16kHz:
			Freq = 16000;
			break;
		default:
			TRACE_INFO((_L("CBTSACStreamerController::GetSamplingFrequency(), #default")))
			Freq = 48000;
			break;
		}
	return Freq;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::GetNumOfChannels
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::GetNumOfChannels(TSBCCodecCapabilities& aCap) const
	{
	TInt nChnls = (aCap.ChannelModes() & EMono) ? 1 : 2;
	return nChnls;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::SetNumOfSubbands
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::SetNumOfSubbands(TSBCCodecCapabilities& aCap, TUint8 aNumOfSubbands)
	{	
	// Set Number of Subbands, select highest one
	if ( aNumOfSubbands & EEightSubbands )
		{
		iEncoderCI->SetNumOfSubbands(TUint(8));
		aCap.SetSubbands(EEightSubbands);
		TRACE_INFO((_L("CBTSACStreamerController::SetNumOfSubbands(), Subband: EEightSubbands")))
		}
	else if ( aNumOfSubbands & EFourSubbands )
		{
		iEncoderCI->SetNumOfSubbands(TUint(4));
		aCap.SetSubbands(EFourSubbands);
		TRACE_INFO((_L("CBTSACStreamerController::SetNumOfSubbands(), Subband: EFourSubbands")))
		}
	else
		{
		TRACE_INFO((_L("CBTSACStreamerController::SetNumOfSubbands(), No matching Subband")))
		}
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::GetNumOfSubbands
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::GetNumOfSubbands(TSBCCodecCapabilities& aCap) const
	{
	TInt nSbands = (aCap.Subbands() & EEightSubbands) ? 8 : 4;
	return nSbands;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::FrameLengthFormula
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::FrameLengthFormula(TSBCCodecCapabilities& aCap) const
	{
	TInt join = ( aCap.ChannelModes() & EJointStereo ) ? 1 : 0; 
	TInt nChnls = GetNumOfChannels(aCap);
	TInt nSbands = GetNumOfSubbands(aCap);
	TInt nBlck  = GetBlockLen(aCap);
	
	// For mono (and dual channel) mode the formula is slightly differend than for stereo mode(s). 
	TInt frameLength = 
		( (aCap.ChannelModes() & EMono) || (aCap.ChannelModes() & EDualChannel) ) ? 
		  4 + ((4 * nSbands * nChnls) / 8) + ( (nBlck * nChnls * aCap.MaxBitpoolValue()) / 8): 
		  4 + ((4 * nSbands * nChnls) / 8) + ( (join * nSbands + nBlck * aCap.MaxBitpoolValue()) / 8)/* + join*/; 

	// Note:	last "+ join" in latter version of the formula is for joint stereo. 
	//			Otherwise the formula doesn't calculate the extra byte that's in the header.
	return frameLength;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::PacketsDropped
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::PacketsDropped(TInt aError)
	{
	TRACE_FUNC
	if(!iTotalNbrOfDroppedPackets)
		{
		// This is first packet drop indication after previous sample interval
		iFirstPacketDropTime.UniversalTime();
		}
	TTime timeNow;
	timeNow.UniversalTime();
    iPacketDropDeviation = static_cast<TInt32>(timeNow.MicroSecondsFrom(iFirstPacketDropTime).Int64());
	
	iTotalNbrOfDroppedPackets += aError;
	
	if(!IsActive())
		{
		iOngoingAction = EErrorSending;
		iTimer.After(iStatus, KDataCollectDelay);
		SetActive();
		}
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::SetCurrentBitpoolData
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::SetCurrentBitpoolData(TInt aBitpool)
	{
	TRACE_FUNC
	TInt err = KErrNone;
	TInt Index = GetIndex(aBitpool);
	if(Index >= 0)
		{
		iCurrentBitpoolData = GetBitpoolData(Index);
		}
	else
		{
		err = KErrNotFound;
		}
	return err;
	}
// -----------------------------------------------------------------------------
// CBTSACStreamerController::GetIndex
// -----------------------------------------------------------------------------
//
TInt CBTSACStreamerController::GetIndex(TInt aBitpool)
	{
	TInt Bitpool = aBitpool;
	if(aBitpool > iMatchCap.MaxBitpoolValue())
		{
		Bitpool = iMatchCap.MaxBitpoolValue();
		}
	for(TInt i = 0 ; i < iBitpoolData.Count() ; i++)
		{
		if(Bitpool == iBitpoolData[i].iMaxBitpoolValue)
			{
			return i;
			}
		}
	return KErrNotFound;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::IndexValid
// -----------------------------------------------------------------------------
//
TBool CBTSACStreamerController::IndexValid(TInt aIndex)
	{
	if(aIndex >= 0 && aIndex < iBitpoolData.Count())
		return ETrue;
	else
		return EFalse;
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::InitializeBitpoolData
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::InitializeBitpoolDataL()
	{
	TRACE_FUNC
	TBitpoolData data;
	iBitpoolData.Reset();
	for(TInt i = 0 ; i < KNumOfBitpoolValues ; i++)
		{
		data.iMaxBitpoolValue = KMaxBitpoolValues[i];
		data.iMaxDeviation = KDeviationValues[i];
		//data.iUpgradeDelay = KUpgradeDelays[i];
		data.iUpBitpoolIndex = (i == 0) ? i : i - 1;
		data.iIndex = i;
		data.iDownBitpoolIndex = (i == KNumOfBitpoolValues - 1) ? i : i + 1;
		data.iMinimumMaxBitpool = (i == KNumOfBitpoolValues - 1) ? ETrue : EFalse;
		iBitpoolData.AppendL(data);
		}
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::GetBitpoolData
// -----------------------------------------------------------------------------
//
TBitpoolData* CBTSACStreamerController::GetBitpoolData(TInt aIndex)
	{
	if(aIndex < 0 || aIndex >= iBitpoolData.Count())
		return NULL;
	
	return &iBitpoolData[aIndex];
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::ReorganizeBitpoolTable
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::ReorganizeBitpoolTable(TInt aNegotiatedMaxBitpool)
	{
	TRACE_INFO((_L("CBTSACStreamerController::ReorganizeBitpoolTable(), Negotiated Max Bitpool %d"), aNegotiatedMaxBitpool))
	// Start checking from the lowest possible bitpool value	
	for(TInt i = (KNumOfBitpoolValues - 1) ; i > 0 ; i--)
		{
		if(aNegotiatedMaxBitpool <= KMaxBitpoolValues[i])
			{
			TInt ii;
			for(ii = 0 ; ii < i ; ii++)
				{
				iBitpoolData.Remove(0);				
				}
			TRACE_INFO((_L("CBTSACStreamerController::ReorganizeBitpoolTable(), Tables removed: %d"), ii))
			for(TInt j = 0 ; j < iBitpoolData.Count() ; j++)
				{
				if(j == 0)
					{
					iBitpoolData[j].iMaxBitpoolValue = aNegotiatedMaxBitpool;
					}
				iBitpoolData[j].iIndex = j;
				iBitpoolData[j].iUpBitpoolIndex = (j == 0) ? j : j - 1;
				iBitpoolData[j].iDownBitpoolIndex = (j == iBitpoolData.Count() - 1) ? j : j + 1;
				}
			break;
			}
		}
	TRACE_INFO((_L("CBTSACStreamerController::ReorganizeBitpoolTable(), Tables left: %d"), iBitpoolData.Count()))
	for(TInt k = 0 ; k < iBitpoolData.Count() ; k++)
		{
		TRACE_INFO((_L("CBTSACStreamerController::ReorganizeBitpoolTable(), Table[%d] MaxBP: %d"), k, iBitpoolData[k].iMaxBitpoolValue))
		}
	}

// -----------------------------------------------------------------------------
// CBTSACStreamerController::DoSelfComplete
// -----------------------------------------------------------------------------
//
void CBTSACStreamerController::DoSelfComplete(EOngoingAction aAction, TInt aError)
	{
	TRACE_FUNC
	if (IsActive())
    	{
    	// Stop timer
    	Cancel();
    	}
    if (!IsActive())
    	{
		// Just self complete here, handle error in RunL
		iOngoingAction = aAction;
		TRequestStatus *status = &iStatus;
		iStatus = KRequestPending;
 		iThread.RequestComplete(status, aError);
 		SetActive();
    	}
    else
    	{
    	TRACE_INFO((_L("CBTSACStreamerController::DoSelfComplete((), Couldn't do self complete")))
    	}
	}
//  End of File      
