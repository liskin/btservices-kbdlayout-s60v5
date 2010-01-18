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
* Description:
*
*/


#ifndef BTMCPROTOCOL_H
#define BTMCPROTOCOL_H

#include <e32base.h>
#include <etelmm.h>
#include <btengdiscovery.h>
#include "btmcactive.h"
#include "btmcprofileid.h"
#include "btmcprotdatabuf.h"
#include "HfpAtCmdHandler.h"

// forward declarations
class CATCommand;
class CATResult;
class TBtmcProtocolStatus;   
class MBtmcObserver;
class MATExtObserver;
class CBtmcMobileLine;
class CBtmcPhoneStatus;
class CBtmcCallStatus;
class CBtmcNumber;
class CBtmcOperator;
class CBtmcCallingHandler;
class TBtmcProtDataBuf;
class CDesC8ArrayFlat;


const TUint KBTHSRemoteAudioVolumeControl = 0x0302;

NONSHARABLE_CLASS(CBtmcProtocol) : public CBase, 
    public MBtmcActiveObserver, 
    public MBTEngSdpResultReceiver, 
    public MATExtObserver
      {
    public:
        static CBtmcProtocol* NewL(
            MBtmcObserver& aObserver, 
            TBtmcProfileId aProfile,
            const TDesC8& aBTDevAddr,
            TBool aAccessoryInitiated);

        ~CBtmcProtocol();
        
        /**
        * Handle an AT command from HF or HS unit.
        *
        */
        void NewProtocolDataL(const TDesC8& aData);

        void SendResponseL(const CATResult& aResult);
        
        void SendResponseL(const RPointerArray<CATResult>& aResults);
        
        void SendUnsoltResultL(const CATResult& aResult);
        
        void CmdHandlingCompletedL();
        
        TBtmcProtocolStatus& ProtocolStatus();
        
        void VoiceRecognitionError();
        
        void HandleNrecCompletedL(TInt aErr);

        void ActivateRemoteVolumeControl();

        void DeActivateRemoteVolumeControl();   

        TInt GetRemoteSupportedFeature();

        /**
         * Tells if AT+CHLD command handling is in progress
         */
        TBool ActiveChldHandling() const;
        
    private:        
        // from MBTEngSdpResultReceiver
        void ServiceSearchComplete( const RSdpRecHandleArray& /*aResult*/, 
		                                         TUint /*aTotalRecordsCount*/, TInt /*aErr*/ ) {}
		
        void AttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, 
		                                           const RSdpResultArray& /*aAttr*/, TInt /*aErr*/ ) {}
		
        void ServiceAttributeSearchComplete( TSdpServRecordHandle /*aHandle*/, 
		                                                  const RSdpResultArray& /*aAttr*/, 
		                                                  TInt /*aErr*/ );
		
		void DeviceSearchComplete( CBTDevice* /*aDevice*/, TInt /*aErr*/ ) {}

        
        // From MBtmcActiveObserver
    
        void RequestCompletedL(CBtmcActive& aActive, TInt aErr);
        
        void CancelRequest(TInt aServiceId);

		// From MATExtObserver
			
        void ATExtHandleCommandCompletedL(TInt aErr, const TDesC8& aReply);
        
        void UnsolicitedResultFromATExtL(TInt aErr, const TDesC8& aAT);

    private:
        CBtmcProtocol(MBtmcObserver& aObserver);
        
        // 2nd phase construction, called by NewL()
        void ConstructL(TBtmcProfileId aProfile, const TDesC8& aBTDevAddr, TBool aAccessoryInitiated);
        
        void DoHandleCommandL();
                
        void HandleTestCommandL(const CATCommand& aCmd);
        
        void HandleReadCommandL(const CATCommand& aCmd);
        
        void HandleWriteCommandL(const CATCommand& aCmd);
        
        void HandleActionCommandL(const CATCommand& aCmd);

        void DoSendProtocolDataL();

        void StartTimerL(TInt aService, TInt aTimeout);
        
        void StopTimer(TInt aService);

        TBool ServiceLevelConnected() const;
        
        void SetIndicatorL(TInt aIndicator, TInt aValue);

    private:
        MBtmcObserver& iObserver;  // unowned
        TBtmcProtocolStatus* iProtocolStatus; // owned
        RTelServer iServer; // owned
        RMobilePhone iPhone; // owned
        CBtmcPhoneStatus* iPhoneStatus;// owned
        CBtmcCallStatus* iCallStatus;// owned

        CBtmcNumber* iNumber; // owned
        CBtmcOperator* iOperator; // owned

        CBtmcCallingHandler* iCallingHandler;
        
        // Service Level Connection
        TInt iSlcMask;

        RTimer iTimer;// owned
        CBtmcActive* iTimerActive; // owned, SLC
        
        CBtmcActive* iEtelQuery; // owned
        
        CBtmcActive* iCmdHanldingActive;
 
        CHFPAtCmdHandler* iAtExt;
        
        // buffer of incoming At commands
        TBtmcProtDataBuf iInDataBuf;
        TBool iHandleCmdPending;
        CDesC8ArrayFlat* iOutgoPacketQueue; // owned
        TInt iCredit; // How many commands are allowed to acc
        TBool iVolumeSyncFromAccessory;
        CBTEngDiscovery* iBteng; // for volume query
        TBool iAccessoryInitiated; // who initiated the connection
        RMobilePhone::TMobilePhoneIdentityV1 iIdentity; // holds IMEI etc
        RMobilePhone::TMobilePhoneSubscriberId iId; // holds id
        };

#endif // BTMCPROTOCOL_H

// End of File
 
