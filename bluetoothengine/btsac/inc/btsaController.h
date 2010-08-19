/*
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declaration for CBTSAController class. This class is advertised
*				 to E-com framework (@see proxy.cpp) so that it loads this class when 
*				 this plugin gets loaded. That is why this class implements the interface
*				 of the module (currently BT Accessory Server) which loads this plugin. Being
*				 main class, this class constructs other classes and also recieves callbacks,
*				 hence implementing their interfaces. 
*
*/


#ifndef BTSACONTROLLER_H
#define BTSACONTROLLER_H



//  INCLUDES

// Implement Interface provided by 'BT Accessory Server' component. E-com framework
// uses this interface to load this plugin
#include <es_sock.h>
#include <btaccPlugin.h>    
#include "btsacdefs.h"
#include "btsacStreamerController.h" 	//MBTAudioAdaptationObserver
#include "btsacActiveObserver.h" 		//MBtsacActiveObserver


// CONSTANTS

// DATA TYPES

// FORWARD DECLARATIONS
class CBtsacState;
class CBTSACGavdp;
class CBTSACSEPManager;
class CBTSACStreamerController; 

// CLASS DECLARATION

/**
*  
*  
*  
*/
NONSHARABLE_CLASS(CBTSAController) : public CBTAccPlugin, public MBTAudioAdaptationObserver, public MBtsacActiveObserver
     {
public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * After construction the instance is ready to serve requests.
        * Also starts listening to incoming connections from BT Stereo 
        * audio accessories.
        */
        static CBTSAController* NewL(TPluginParams& aParams);
        
        /**
        * Destructor.
        * Removes service and security registrations and stops listening to
        * incoming connections.
        */
        virtual ~CBTSAController();

	    /** 
	     * State transition
	     * 
	     * @since S60 v3.1
	     * @param aState the next state to be entered.
	     */
	    void ChangeStateL(CBtsacState* aState);

public:
		void ResetRemoteCache();
		void InitializeSEPManager();
		void CleanSockets();
		void StoreAccInfo();
		void DeleteAccInfo();
		TBool IsAccInfoAvailable();
		TInt AbortStream();
		
		void NewAccessory(const TBTDevAddr& aBDAddr);
        void DisconnectedFromRemote(const TBTDevAddr& aAddr, TInt aError );      
        void AccessoryOpenedAudio(const TBTDevAddr& aBDAddr);
		void AccessoryClosedAudio(const TBTDevAddr& aBDAddr);
    	void AccessorySuspendedAudio(const TBTDevAddr& aBDAddr);
		void CompletePendingRequests(TUint aCompleteReq, TInt aError);
		
		void SetRemoteAddr(const TBTDevAddr& aRemoteAddr);
		TBTDevAddr GetRemoteAddr() const;
		TBool IsOpenAudioReqFromAccFWPending() const;
		void SetSEPIndex(TInt aIndex);
		TInt GetSEPIndex() const;
		void SetResetAudioInput(TBool aReset);
		TBool GetResetAudioInput() const;
		CBtsacActive* GetGavdpErrorActive() const;
		void GAVDP_Error(TInt aError) ;
      
private:

        /**
        * C++ default constructor.
        */        
        CBTSAController(TPluginParams& aParams);

        /**
        * Symbian 2nd phase constructor.
        * Starts listening to incoming connections from BT Stereo 
        * audio accessories.
        * Gets ready to accept requests from clients.
        */
        void ConstructL(/*TPluginParams& aParams*/);
	
public: // From MBTAudioAdaptationObserver
	
		virtual void NotifyError(TInt aError); 
        
private: // Functions from CBTAccPlugin

   		 /** 
		* BT Acc Server learns if plugin supports mono audio or not 
		* while loading the plugin
		@param 	None.
		@return ETrue if plugin supports mono audio, TFalse otherwise.
		*/
    	virtual TProfiles PluginType();
    
    	/** 
		* Called by BT Acc Sever to connect to mono or stereo accessory
		* @param aAddr Bluetooth Device address of the remote device
		* @return result of operation
		*/
		virtual void ConnectToAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

		/** 
		* Called by BT Acc Sever to connect to mono or stereo accessory
		* @param aAddr Bluetooth Device address of the remote device
		* @return result of operation
		*/
		virtual void CancelConnectToAccessory(const TBTDevAddr& aAddr);
		
		/** 
		* Called by BT Acc Sever to connect to mono or stereo accessory
		* @param aAddr Bluetooth Device address of the remote device
		* @return result of operation
		*/
		virtual void DisconnectAccessory(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

		/** 
		* Called by BT Acc Sever to open audio link 
		* @param aAddr Bluetooth Device address of the remote device
		* @return result of operation
		*/
		virtual void OpenAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus);

    	/** 
    	* Called by BT Acc Sever to cancel opening of audio link
    	@param aAddr Bluetooth Device address of the remote device
    	@return result of operation
    	*/
    	void CancelOpenAudioLink(const TBTDevAddr& aAddr );

    	/** 
    	* Called by BT Acc Sever to connect to mono or stereo accessory
    	@param aAddr Bluetooth Device address of the remote device
    	@return result of operation
    	*/
    	void CloseAudioLink(const TBTDevAddr& aAddr, TRequestStatus& aStatus);
    	
        /** 
    	* Called by BT Acc Sever to cancel closing of audio link
    	@param aAddr Bluetooth Device address of the remote device
    	@return 
    	*/
    	void CancelCloseAudioLink(const TBTDevAddr& aAddr );

        /**
        * BTAccServer informs BTSAC to start streaming after getting permission
        * from Acc Fw
        */
        virtual void StartRecording();   
        
	   	/*
	   	 * Defaults from MBtsacActiveObserver
		 */	
		/**
	     * Handles the request completion event.
	     *
	     * @since S60 v3.1
	     * @param aActive the Active Object to which the request is assigned to.
	     */
		virtual void RequestCompletedL(CBtsacActive& aActive);

	    /**
	     * Handles the cancellation of an outstanding request.
	     *
	     * @since S60 v3.1
	     * @param aActive the Active Object to which the request is assigned to.
	     */
		virtual void CancelRequest(CBtsacActive& aActive);

		
private:
		
		/** 
		* Called by BT Acc Sever to inform plugin that accessory is in use. 
		@param  None.
		@return None.
		*/
		virtual void AccInUse();

public: // Data
        CBTSACGavdp* iGavdp;
		CBTSACStreamerController* iStreamer;
		CBTSACSEPManager* iLocalSEPs;
		CBTSACSEPManager* iRemoteSEPs;
		RArray<RSocket> iStreamingSockets;
		
private: // Data
	   /**
     	* the current state.
     	*/
    	CBtsacState* iState;
    	
    	/**
     	* AO for handling gavdp errors.
     	*/
    	CBtsacActive* iGavdpErrorActive;
    	
    	TBTDevAddr iRemoteAddr;
		TBool iNotifiedObserverOfNewAccessory;
    	TInt  iSBCSEPIndex;   // SBC SEP Index in iRemoteSEPs array
    	TBool iResetAudioInput;
    	TRequestStatus* iConnectStatus;
        TRequestStatus* iDisconnectStatus;
        TRequestStatus* iOpenAudioStatus;
        TRequestStatus* iCloseAudioStatus;
        RPointerArray<CBTSACSEPManager> iAccDb;    // one SEP manager for each accessory
    };


#endif      // BTSAC_CONTROLLER_H
            
// End of File