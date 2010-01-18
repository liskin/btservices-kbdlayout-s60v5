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
* Description:  Contains definition of CBTSACGavdp class which uses GAVDP API. 
*				 Two other classes CAVStreamEndPoint and CAVSEPManager are helper 
*				 classes to use GAVDP. This class also constructs CBTSACStreamerController
*				 class.
*
*/


#ifndef BTSAC_GAVDP_H
#define BTSAC_GAVDP_H



//  INCLUDES
#include <btengdiscovery.h>	// Register SDP record for A2DP
#include <gavdp.h>			// symbian Bluetooth GAVDP API


// CONSTANTS

// DATA TYPES

// FORWARD DECLARATIONS
class CBTSACStreamerController;
class CBTSACSEPManager;
class MInternalGavdpUser; 


// CLASS DECLARATION

/**
*  
*  
*  
*/
NONSHARABLE_CLASS(CBTSACGavdp) : public CBase, public MGavdpUser
                       
    {
    public:  // Constructors and destructor
        
        
        /**
        * Two-phased constructor.
        * 
        * 
        * 
        */
        static CBTSACGavdp* NewL( MInternalGavdpUser* aObserver );
        
        /**
        * 
        * 
        * 
        */
        virtual ~CBTSACGavdp();
        
   public: // New Functions
   
			/*
			 *
			 *
			 */	 
			void RegisterObserver(MInternalGavdpUser* aObserver, const TDesC& aName);
			
			/*
			 *
			 *
			 */
			TInt StartSrc(CBTSACSEPManager& aLocalSEPs, TSBCCodecCapabilities& aSbc);
	
	 		/*
			 *
			 *
			 */
	 		void StopSrc(); // disconnect from accessory
					
			 /*
			 * Connect with remote accessory
			 * @param 	aAddr	bluetooth address of the device to be connected
			 * @return	KErrNone if connection is successfull, error otherwise.
			 */
			 void Connect( const TBTDevAddr& aAddr );  		
			 
			 /*
			 *
			 *
			 */
			 TInt Listen();
 
			 /*
			 *
			 *
			 */	 			
     		 void Cancel();
    		 
    		 /*
			 *
			 *
			 */
    		 void DiscoverSEPs();
      		 
    		 /*
    		 * Stop streaming to the connected remote accessory
    		 * @param   None.
    		 * @return  None.
    		 */
    		 void AbortStreaming(TSEID& aRemoteSEPId);
    		 
    		 
    		 /*
    		 * Disconnects the RGavdp session from the signalling channel, but does not destroy the local SEPs owned
			 * by the signalling channel. Must only be called if there is no stream present -
			 * if a stream is present AbortStream() should be called.
    		 * @param   None.
    		 * @return  None.
    		 */
    		 TInt CBTSACGavdp::Shutdown();
    		
    		 /**
    		 * Suspend (Pause) streaming to remote accessory
    		 * @ param None.
    		 * @return KErrNone if streaming could be suspended successfully, error otherwise
    		 */
    		 void SuspendStreams(TSEID& aRemoteSEPId);
    		 
    		 /*
			 *
			 *
			 */
    		 TInt ConnectAsSrc();	// Not implemented yet
			 
			 /*
			 *
			 *
			 */
			 TInt CreateBearers(TSEID& aRemoteSEPId);
			 
			 /*
			 *
			 *
			 */
			 TInt ConfigureSEP(TSEID aLocalSEPId, TSEID aRemoteSEPId, 
			 					TSBCCodecCapabilities& aSBCCaps, TAvdtpMediaTransportCapabilities& aMedTranCaps  );
			 
			 /*
			 *
			 *
			 */
			 void DoSink();				// Not implemented yet
			 
			 /*
			 *
			 *
			 */
			 void StartStreams(TSEID& aRemoteSEPId); 
			 
			 /*
			 *
			 *
			 */
			 void GetCapabilities(TSEID& aId); 
			 
			 /*
			 *
			 *
			 */
			 void SendSecurityControl();
			 
			 /*
			 * Closes the stream
			 */
			 void Close();
			 
			 /*
			 * Opens a RGavdp session
			 */
			 TInt Open();
			 
			/*
			* Registers Stream endpoint to gavdp
			*
			*/    
			TInt RegisterSEPs(CBTSACSEPManager& aLocalSEPs, TSBCCodecCapabilities& aSbc );

   private:

        /**
        * C++ default constructor.
        */
        CBTSACGavdp();
        
        /**
        * C++ default constructor.
        */
        CBTSACGavdp( MInternalGavdpUser* aObserver );

        /**
        * Symbian 2nd phase constructor.
        * 
        * 
        * 
        * 
        */
        void ConstructL();

		

   private:		// GAVDP callbacks - @see gavdp.h for definitions
		
		virtual void GAVDP_SEPDiscovered(const TAvdtpSEPInfo& aSEP);
		virtual void GAVDP_SEPDiscoveryComplete();
		virtual void GAVDP_SEPCapability(TAvdtpServiceCapability* aCapability);	
		virtual void GAVDP_SEPCapabilityComplete();
		virtual void GAVDP_AbortStreamConfirm();
		virtual void GAVDP_StartStreamsConfirm();
		virtual void GAVDP_SuspendStreamsConfirm();
		virtual void GAVDP_SecurityControlConfirm(const TDesC8& aResponseData);
		virtual void GAVDP_ConfigurationConfirm();	// configuration complete and SEP selected *AND* reconfigure confirm
		virtual void GAVDP_Error(TInt aError, const TDesC8& aErrorData);
		virtual void GAVDP_ConnectConfirm(const TBTDevAddr& aAddr);
		virtual void GAVDP_ConfigurationStartIndication(TSEID aLocalSEID, TSEID aRemoteSEID);	
		virtual TInt GAVDP_ConfigurationIndication(TAvdtpServiceCapability* aCapability);
		virtual TInt GAVDP_ConfigurationEndIndication();
		virtual TInt GAVDP_StartIndication(TSEID aSEID);
		virtual TInt GAVDP_SuspendIndication(TSEID aSEID);
		virtual TInt GAVDP_SecurityControlIndication(TSEID aSEID, TDes8& aSecurityData);
		virtual void GAVDP_AbortIndication(TSEID aSEID);
		virtual void GAVDP_ReleaseIndication(TSEID aSEID);
		virtual void GAVDP_BearerReady(RSocket aNewSocket, const TAvdtpSockAddr& aAddr);
	    
		
    private:    // Data
        
	    MInternalGavdpUser* iObserver; 
      
	    CBTSACStreamerController* iStreamer;
		RSocketServ	iSockServ;
		RGavdp iGavdp;	

		// Handle to the AVDTP SOURCE SDP service record, which is 
    	// created to our local SDP database
    	TSdpServRecordHandle iA2dpSourceSDPrecordHandle;
    	CBTEngDiscovery* iBTConnectionPtr;
	   };


#endif      // BTSAC_STREAMER_H
            
// End of File