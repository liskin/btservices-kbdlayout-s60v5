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
*                to, e.g. get permission to attach/detach a remote bluetooth
*                device and also recieves requests to open/close audio connections.
*  Version     : %version:  1.2.4 %
*
*/


#ifndef BTACC_FW_CONNECTION_H
#define BTACC_FW_CONNECTION_H

//  INCLUDES
#include <e32base.h>        // Declaration of CActive
#include <AccessoryServer.h> 
#include <AccessoryBTControl.h>
#include "BTAccFwIf.h"

/**
*  CBTSession class represent session on server side
*
*/
NONSHARABLE_CLASS(CBasrvAccfwConn) : public CBasrvAccfwIf
                                           
{
public:  // Constructors and destructor

    /**
    * Constructor.
    */
    static CBasrvAccfwIf* NewL();

    /**
    * Destructor.
    */
    ~CBasrvAccfwConn();

public: // from BTAccFwIf

    /**
    * 
    * @param
    * return     
    */
    virtual void CompleteReq(TBool aUse);

    /**
    * Request Accessory Framework to attach accessory
    * @param    aBDAddr BT Device Address of accessory to be attached
    * @param     aStatus Success status of the operation.
    * return     success code of request to attach accessory
    */
    virtual void AttachAccessory(const TBTDevAddr& aBDAddr, TRequestStatus& aStatus);
    
    
    /**
    * Request Accessory Framework to cancel attach accessory
    * @param    aBDAddr BT Device Address of accessory to be attached
    * return     success code of request to attach accessory
    */
    virtual void CancelAttachAccessory(const TBTDevAddr& aBDAddr);
    
    /**
    * Request Accessory Framework to detatch accessory
    * @param    aBDAddr BT Device Address of accessory to be attached
    * @param     aStatus Success status of the operation.
    * return     None.
    */
    virtual void DetatchAccessory(const TBTDevAddr& aBDAddr, TRequestStatus& aStatus);
    
    /**
    * Register for notification when audio link is required to be opened.
    * @param aStatus                Success status of the operation.
    * @return                       None.
    */
    virtual void NotifyBTAudioLinkOpenReq(TBTDevAddr& aBDAddress, TRequestStatus& aStatus, TAccAudioType& aType );

    /**
    * Register for notification when audio link is required to be closed.
    * @param aStatus                Success status of the operation.
    * @return                       None.
    */
    virtual void NotifyBTAudioLinkCloseReq(TBTDevAddr& aBDAdress, TRequestStatus& aStatus, TAccAudioType& aType ); 

    /**
    * Cancel registeration for notification when audio link is required to be opened.
    * @param aStatus                Success status of the operation.
    * @return                       None.
    */
    virtual void CancelNotifyBTAudioLinkOpenReq();

    /**
    * Cancel registeration for notification when audio link is required to be closed.
    * @param aStatus                Success status of the operation.
    * @return                       None.
    */
    virtual void CancelNotifyBTAudioLinkCloseReq(); 

    /**
    * Informs that the audio connection open operation has completed.
    * @param aStatus                Success status of the operation.
    * @return                       None.
    */
    virtual void AudioConnectionOpenCompleteL( const TBTDevAddr& aBDAddress,
                                      TInt aResp, TInt aLatency = 0 );

    /**
    * Informs that the audio connection close operation has completed.
    * @param aStatus                Success status of the operation.
    * @return                       None.
    */
    virtual void AudioConnectionCloseCompleteL( const TBTDevAddr& aBDAddress, 
                                        TInt aResp );

    /**
    * Notifies the DOS that an audio connection to the currently connected
    * accessory has been opened.
    * @param aBDAddress             BT hardware address of the accessory.
    * @return                       None.
    */
    virtual void NotifyAudioLinkOpenL( const TBTDevAddr& aBDAddress, TAccAudioType aType );

    /**
    * Notifies the DOS that the audio connection to the currently connected
    * accessory has been closed.
    * @param aBDAddress             BT hardware address of the accessory.
    * @return                       None.
    */
    virtual void NotifyAudioLinkCloseL( const TBTDevAddr& aBDAddress, TAccAudioType aType );

        /**
    * Requests Accessory Framework to disable NREC
    * @param aBDAddress             BT hardware address of the accessory.
    * @return                       None.
    */
    virtual TInt DisableNREC(const TBTDevAddr& aBDAddress); 
    
private: // New Functions

    /**
    * C++ default constructor.
    */
    CBasrvAccfwConn();
    
    /**
    * 2nd Phase Construction
    */
    void ConstructL();
    

private: // Data

    RAccessoryServer iAccessoryServerSession;
    RAccessoryBTControl    iAccessoryBTControlSession;

};

#endif      // BTACC_FW_CONNECTION_H

// End of File
