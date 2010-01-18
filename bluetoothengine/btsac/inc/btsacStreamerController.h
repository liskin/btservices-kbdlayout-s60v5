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
* Description:  Declaration for CBTSACStreamerController class. This class constructs and
*				 communicates with 'BT Audio Streamer' component to do RTP streaming.

*
*/


#ifndef BTSAC_STREAMER_H
#define BTSAC_STREAMER_H

//  INCLUDES

#include <btaudiostreamer.h>  
#include <btaudiostreaminputbase.h> 
#include <bluetoothav.h>		// Protocol definitions, e.g., TSBCCodecCapabilities
#include "btsacActivePacketDropIoctl.h" 
#include <SbcEncoderIntfc.h>  // uuid of SBC Encoder


class MBTAudioAdaptationObserver
    {
    public:
    virtual void NotifyError(TInt aError) = 0; 
    };

// CONSTANTS

// DATA TYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  
*  
*  
*/

class TBitpoolData
	{
public:
	TInt iMaxBitpoolValue;
	TInt iMaxDeviation;
	//TInt iUpgradeDelay;
	TInt iIndex;
	TInt iDownBitpoolIndex;
	TInt iUpBitpoolIndex;
	TBool iMinimumMaxBitpool;
	};

NONSHARABLE_CLASS(CBTSACStreamerController) : public CActive, MBTAudioErrorObserver, MActivePacketDropIoctlObserver
    {
 public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * 
        * 
        * 
        */
        static CBTSACStreamerController* NewL(MBTAudioAdaptationObserver& aObserver);
        
        /**
        * Destructor.
        * 
        * 
        */
        virtual ~CBTSACStreamerController();
        
  public:
		// From MBTAudioErrorObserver
		virtual TInt Error(const TInt aError); 

		// From MActivePacketDropIoctlObserver
    	virtual void PacketsDropped(TInt aPacketCount);

  
  public: // New functions
        
        /**
        * 
        * 
        * @param aStatus            
        * @return                   None.
        */
        TInt StartStream(RSocket& aSocket, TInt aFrameLength);
        
        /**
        * 
        * 
        * @param aStatus            
        * @return                   None.
        */
        TInt StopStream();
        
        /**
        * 
        * 
        * @param aStatus            
        * @return                   None.
        */
        void ResetAudioInput();
        
        /**
        * 
        * 
        * @param aStatus            
        * @return                   None.
        */
        TInt FillCapabilities( TSBCCodecCapabilities& aSbc );
        
        /**
        * 
        * 
        * @param aStatus            
        * @return                   None.
        */
        TInt ConfigureSEP( TSBCCodecCapabilities& aDec );
        
        /**
        * 
        * 
        * @param aStatus            
        * @return                   None.
        */
        TInt FrameLength() const;
        
        /**
        * 
        * 
        * @param none
        * @return                   TSBCCodecCapabilities.
        */
        TSBCCodecCapabilities &GetCaps();
        
    protected:  // Functions from CActive
    
        /**
        * RunL is called by framework after request is being completed.
        * @param nones
        * @return none 
        */    
    	void RunL();
    	
        /**
        * DoCancel() has to be implemented by all the classes deriving CActive.
        * Request cancellation routine.
        * @param none
        * @return none 
        */
    	void DoCancel();

    	/**
    	* RunError is called if RunL() leaves
    	*/
        TInt RunError(TInt aError);

                                 
	private: // Data definitions

        enum TBTSACStreamingState
        	{
        		EStateStreaming,
        		EStateIdle,
        		EStateError
        	};
 		
 		typedef enum EOngoingAction
			{
			ENone,
			EErrorSending,
			EDowngradeBitrate,
			EUpgradeBitrate,
			EStartStabilizingTimer,
			EStabilize
			};
			
     private:

        /**
        * C++ default constructor.
        */
        CBTSACStreamerController(MBTAudioAdaptationObserver& aObserver);

        /**
        * Symbian 2nd phase constructor.
        * 
        * 
        * 
        * 
        */
        void ConstructL();
		
		void CheckAndAdjustBitpool(TSBCCodecCapabilities& aDec);
		TInt GetMatchingCaps(TSBCCodecCapabilities& aCaps);
		TInt ConfigureSEPBitpool(EOngoingAction aAction);
		void DoSelfComplete(EOngoingAction aAction, TInt aError);
		
		void SetBlockLen(TSBCCodecCapabilities& aCap, TUint8 aBlockLen);
		TInt GetBlockLen(TSBCCodecCapabilities& aCap) const;
		void SetSamplingFrequency(TSBCCodecCapabilities& aCap, TUint8 aFrequency);
		TInt GetSamplingFrequency(TSBCCodecCapabilities& aCap) const;
		TInt FrameLengthFormula(TSBCCodecCapabilities& aCap) const;
		TInt GetNumOfChannels(TSBCCodecCapabilities& aCap) const;
		void SetNumOfSubbands(TSBCCodecCapabilities& aCap, TUint8 aNumOfSubbands);
		TInt GetNumOfSubbands(TSBCCodecCapabilities& aCap) const;		
		void SetBitpoolValues(TSBCCodecCapabilities& aCap);
		void SetAllocationMethod(TSBCCodecCapabilities& aCap, TUint8 aAllocationMethod);
		void SetChannelMode(TSBCCodecCapabilities& aCap, TUint8 aChannelMode);
		
		TInt SetCurrentBitpoolData(TInt aBitpool);
		void InitializeBitpoolDataL();
		void ReorganizeBitpoolTable(TInt aNegotiatedMaxBitpool);
		TBitpoolData* GetBitpoolData(TInt aIndex);
		TInt GetIndex(TInt aBitpool);
		TBool IndexValid(TInt aIndex);
        
	private:    // Data
        
		CBTAudioStreamer* iBTStreamer;  // Bluetooth Audio Streamer component
		CBTAudioStreamInputBase* iAudioInput;
    	TBTSACStreamingState iState;
    	EOngoingAction iOngoingAction;
    	TSBCCodecCapabilities iLocalCap; // local copy of Audio Adapatation capabilities
    	TSBCCodecCapabilities iMatchCap; // local copy of capabilites used to configure accessory
    	RLibrary iLib;  // for loading BTAudioStreamer and BTAudioAdaptation dll
    	TInt iCurrentBitrate; 
    	TInt iCurrentFrameLength;
		RTimer iTimer;
		CActivePacketDropIoctl* iPacketDropIoctl; 
		RThread iThread;
		CSbcEncoderIntfc *iEncoderCI;
		MBTAudioAdaptationObserver& iObserver;
		TInt iTotalNbrOfDroppedPackets;
		TTime iFirstPacketDropTime; 
		TInt iPacketDropDeviation;
		RArray<TBitpoolData> iBitpoolData;
		TBitpoolData* iCurrentBitpoolData;
    };


#endif      // BTSAC_STREAMER_H
            
// End of File