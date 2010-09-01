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
* Description:  The Connected state declaration
*
*/


#ifndef C_BTASRVACCSTATECONNECTED_H
#define C_BTASRVACCSTATECONNECTED_H

#include "basrvaccstate.h"
#include "btengprivatepskeys.h"

class CBasrvAccStateAttached : public CBasrvAccState
    {
public:

    static CBasrvAccStateAttached* NewL(CBasrvAcc& aParent, TBool aShowNote = EFalse);

    virtual ~CBasrvAccStateAttached();
    
    /**
     * Entry of this state.
     *
     * @since S60 v3.1
     */
    void EnterL();
    
    /**
     * Returns the next state if EnterL leaves.
     *
     * @since S60 v3.1
     * @return the next state
     */
    CBasrvAccState* ErrorOnEntry(TInt aReason);

    TBTEngConnectionStatus ConnectionStatus() const;

    void ConnectL(const TBTDevAddr& aAddr);
        
    /**
    * Disconnect accessory.
    */
    void DisconnectL();

    /**
    * 
    * @return an error. Default implementation returns KErrNotFound
    */
    void AccessoryConnected(TProfiles aProfile);
    
    /**
    * 
    * @return an error. Default implementation returns KErrNotFound
    */
    void AccOpenedAudio(TProfiles aProfile);
    
    /**
    * 
    * @return an error. Default implementation returns KErrNotFound
    */
    void AccClosedAudio(TProfiles aProfile);
    
    /**
    * Called when the connection is disconnected from remote side.
    * @param aErr the error code of event
    */
    void AccessoryDisconnectedL(TProfiles aProfile);
    
    /**
    * Called when an audio open request comes from Acc FW.
    */
    void OpenAudioL(TAccAudioType aType);

    /**
    * Called when an audio close request comes from Acc FW.
    */
    void CloseAudioL(TAccAudioType aType);
    
protected:
    // From MBasrvActiveObserver
    
    /**
     * Handles the request completion event.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void RequestCompletedL(CBasrvActive& aActive);
    
    /**
     * Handles the cancellation of an outstanding request.
     *
     * @since S60 v3.1
     * @param aActive the Active Object to which the request is assigned to.
     */
    virtual void CancelRequest(CBasrvActive& aActive);

protected:

    /**
    * Default constructor
    */
    CBasrvAccStateAttached(CBasrvAcc& aParent, TBool aShowNote);
    
    void HandleAccOpenedAudio(TProfiles aProfile);
 
    void DoCloseAudioL(TAccAudioType aType);
    
    void DoConnectRemConCtIfNeededL();
    
private:

	void UpdateAudioState(TBTAudioLinkState aState, TAccAudioType aType);

    RPointerArray<CBasrvActive> iActives;
    CBasrvActive* iAudioOpener;
    CBasrvActive* iAudioCloser;
    
    RTimer iTimer;
    CBasrvActive* iTimerActive;
    TProfiles iCloseAudioProfile;
    
    TBool iShowNote;
    
    CBasrvActive* iRemConTGConnector;
    };

#endif      // C_BTASRVACCSTATECONNECTED_H

// End of File
