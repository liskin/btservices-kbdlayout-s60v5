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
* ============================================================================
*  Name        : basrvaccman.h
*  Part of     : BTEng / Bluetooth Accessory Server
*  Description : 
*  Version     : %version: 3.1.4.1.2.2.14 %
*
*/


#ifndef C_BTASRVACCMAN_H
#define C_BTASRVACCMAN_H

#include <e32base.h> 
#include <bttypes.h>
#include <btnotif.h>
#include <btengconstants.h>
#include "basrvactive.h"
#include "btaccObserver.h"
#include "BTAccFwIf.h"
#include "basrvactivenotifier.h"

class CBasrvAcc;
class CBTAccSession;
class TAccInfo;
class CBasrvPluginMan;
class CBasrvAudio4Dos;

class CBasrvAccMan :
    public CBase,
    public MBTAccObserver,
    public MBasrvActiveObserver
    {
public:
    static CBasrvAccMan* NewL();

    ~CBasrvAccMan();
        
    void LoadServicesL();
        
    void ConnectL(CBTAccSession& aSession, const TBTDevAddr& aAddr);
    
    void CancelConnect(const TBTDevAddr& aAddr);
    
    void DisconnectL(CBTAccSession& aSession, const TBTDevAddr& aAddr);

    void DisconnectAllL(CBTAccSession& aSession);

    void SetProfileNotifySession(CBTAccSession& aSession);

    void ClearProfileNotifySession(CBTAccSession& aSession);

    const TAccInfo* AccInfo(const TBTDevAddr& aAddr);

    CBasrvPluginMan& PluginMan();
    
    void ConnectCompletedL(const TBTDevAddr& aAddr, TInt aErr, TInt aProfile, const RArray<TBTDevAddr>* aConflicts = NULL);
    
    void DisconnectCompletedL(const TBTDevAddr& aAddr, TInt aProfile, TInt aErr);
    
    TInt OpenAudioCompleted(const TBTDevAddr& aAddr, TAccAudioType aType, TInt aErr);
    
    TInt CloseAudioCompleted(const TBTDevAddr& aAddr, TAccAudioType aType, TInt aErr);

    void AccObsoleted(CBasrvAcc* aAcc, const TBTDevAddr& aAddr);
    
    CBasrvAccfwIf* AccfwConnectionL(const TAccInfo* aInfo = NULL);

    CBasrvAccfwIf* NewAccfwConnectionInstanceL();
    
    void ListenAudioRequestL();
    
    void CancelAudioRequestListen();
    
    void RemoveAudioRequest(const TBTDevAddr& aAddr);
    
    void ShowNote(TBTGenericInfoNoteType aNote, const TBTDevAddr& aAddr);
    
    void FilterProfileSupport(TAccInfo& aInfo);
    
    TInt NotifyAccFwAudioOpened(const TBTDevAddr& aAddr, TAccAudioType aType, TInt aLatency);

    TInt AccInfos(RPointerArray<const TAccInfo>& aInfos);

    void NotifyClientNewProfile(TInt aProfile, const TBTDevAddr& aAddr);
    void NotifyClientNoProfile(TInt aProfile, const TBTDevAddr& aAddr);

    TBTEngConnectionStatus ConnectionStatus4Client(const TBTDevAddr& aAddr) const;

    TBool IsAvrcpVolCTSupported();
    TBool IsAvrcpLegacyVolCTSupported(); 
    TBool IsAbsoluteVolumeSupported(const TBTDevAddr& aAddr);
    TBool DisconnectIfAudioOpenFails();

private: // From MBTAccObserver
    void NewAccessory(const TBTDevAddr& aAddr, TProfiles aProfile);
    
    void AccessoryDisconnected(const TBTDevAddr& aAddr, TProfiles aProfile);

    void RemoteAudioOpened(const TBTDevAddr& aAddr, TProfiles aProfile);

    void RemoteAudioClosed(const TBTDevAddr& aAddr, TProfiles aProfile);

    TInt DisableNREC(const TBTDevAddr& aAddr); 

    void AccessoryTemporarilyUnavailable(const TBTDevAddr& aAddr, TProfiles aType);
    
    TInt ConnectionStatus(const TBTDevAddr& aAddr);

    /**
     * Get the supported features of a profile in a device.
     * @param aAddr The address of the device.
     * @param aProfile the profile identifier
     * @return the supported features value.
     */
    TInt SupportedFeature( const TBTDevAddr& aAddr, TProfiles aProfile ) const;
    
    //TCleanupOperation callback function for TCleanupItem
    static void CleanupLastItemFromClientRequestsArray(TAny* aPtr);

private:
    // From MBasrvActiveObserver
    
    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void RequestCompletedL(CBasrvActive& aActive);
    
    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    void CancelRequest(CBasrvActive& aActive);

private:
    void DoShowNoteL(TBTGenericInfoNoteType aNote, const TBTDevAddr& aAddr);
    
protected:
    /**
    * Default constructor
    */
    CBasrvAccMan();

    void ConstructL();

    void HandleNewAudioRequestL();

    void DoAudioRequestL();

    TInt FindAcc(const TBTDevAddr& aRemote) const;

    TInt AudioLinkStatus();

    void RejectAudioLink(const TBTDevAddr& aAddr, TAccAudioType aType);

private:
    enum TRequestType
        {
        ECloseReqFromAudioPolicy,
        EOpenReqFromAudioPolicy,
        ERejectAudioOpenedByAcc,
        };
    
    class TAudioRequest
        {
    public:
        TBTDevAddr iAddr;
        TAccAudioType iAudioType;
        TRequestType iReqType;
        TBool iOngoing;
        };

    class TClientRequestCache
        {
    public:
        CBTAccSession* iSession;
        TInt iRequest;
        TBTDevAddr iAddr;
        };

protected:
    RArray<TClientRequestCache> iClientRequests;
    CBTAccSession* iSessionOfDisconnectAll;         // not own
    CBTAccSession* iProfileNotifySession;           // not own
    
    CBasrvAudio4Dos* iAudio4Dos;                    // not own
    
    RPointerArray<CBasrvAcc> iAccs;

    CBasrvPluginMan* iPluginMan;
    CBasrvAccfwIf* iAccfw;
    
    CBasrvActive* iOpenListener;
    CBasrvActive* iCloseListener;
    TAccAudioType iOpenType;
    TAccAudioType iCloseType;
    TBTDevAddr iOpenAddr;
    TBTDevAddr iCloseAddr;
    RArray<TAudioRequest> iAudioRequests;
    
    RPointerArray<CBasrvActive> iNotifierArray;
    TBuf8<sizeof(TInt)> iNoResult;
    
    TBool iDisconnectAllPending;
    
    CBasrvActive* iAsyncHandlingActive;
    
    TBool iAvrcpVolCTSupported;
    TBool iAvrcpLegacyVolCTSupported;
    TBool iDisconnectIfAudioOpenFails;
    };

#endif      // C_BTASRVACCMAN_H

// End of File
