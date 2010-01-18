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
*  Version     : %version:  3.2.3 %
*
* Contributors:
*
* Description:  An abstract class implemented by classes which are communicating 
*                with Accessory Fw or DOS 
*
*/


#ifndef BTACC_FW_IF_H
#define BTACC_FW_IF_H

//  INCLUDES
#include <AccessoryTypes.h> // TAccAudioType

// FORWARD DECLARATION
class TAccInfo;

// CLASS DECLARATION

/**
*  CBTSession class represent session on server side
*
*/
NONSHARABLE_CLASS(CBasrvAccfwIf) : public CBase
    {
 
    public: // New Functions

         /**
        * Constructor.        
        */
        static CBasrvAccfwIf* NewL(const TAccInfo* aInfo = NULL); 

         /**
        * Informs the class that there is a request pending.
        * @param    aUse True if a request is pending, false otherwise.
        * return     None.
        */
        virtual void CompleteReq(TBool aUse) = 0;
    
        /**
        * Request Accessory Framework to attach accessory
        * @param    aBDAddr BT Device Address of accessory to be attached
        * @param     aStatus Success status of the operation.
        * return     success code of request to attach accessory
        */
        virtual void AttachAccessory(const TBTDevAddr& aBDAddr, TRequestStatus& aStatus) = 0;
        
        /**
        * Request Accessory Framework to cancel attach accessory
        * @param    aBDAddr BT Device Address of accessory to be attached
        * return     success code of request to attach accessory
        */
        virtual void CancelAttachAccessory(const TBTDevAddr& aBDAddr)= 0;
        
        /**
        * Request Accessory Framework to detatch accessory
        * @param    aBDAddr BT Device Address of accessory to be attached
        * @param     aStatus Success status of the operation.
        * return     None.
        */
        virtual void DetatchAccessory(const TBTDevAddr& aBDAddr, TRequestStatus& aStatus)= 0;
        
        /**
        * Register for notification when audio link is required to be opened.
        * @param aStatus                Success status of the operation.
        * @return                       None.
        */
          virtual void NotifyBTAudioLinkOpenReq(TBTDevAddr& aBDAddress, TRequestStatus& aStatus, TAccAudioType& aType )= 0;

        /**
        * Register for notification when audio link is required to be closed.
        * @param aStatus                Success status of the operation.
        * @return                       None.
        */
        virtual void NotifyBTAudioLinkCloseReq(TBTDevAddr& aBDAdress, TRequestStatus& aStatus, TAccAudioType& aType )= 0; 

        /**
        * Cancel registeration for notification when audio link is required to be opened.
        * @param aStatus                Success status of the operation.
        * @return                       None.
        */
          virtual void CancelNotifyBTAudioLinkOpenReq()= 0;

        /**
        * Cancel registeration for notification when audio link is required to be closed.
        * @param aStatus                Success status of the operation.
        * @return                       None.
        */
        virtual void CancelNotifyBTAudioLinkCloseReq()= 0; 

        /**
        * Informs that the audio connection open operation has completed.
        * @param aStatus                Success status of the operation.
        * @return                       None.
        */
        virtual void AudioConnectionOpenCompleteL( const TBTDevAddr& aBDAddress,
                                          TInt aResp, TInt aLatency = 0 )= 0;

        /**
        * Informs that the audio connection close operation has completed.
        * @param aStatus                Success status of the operation.
        * @return                       None.
        */
        virtual void AudioConnectionCloseCompleteL( const TBTDevAddr& aBDAddress, 
                                            TInt aResp )= 0;

        /**
        * Notifies the DOS that an audio connection to the currently connected
        * accessory has been opened.
        * @param aBDAddress             BT hardware address of the accessory.
        * @return                       None.
        */
        virtual void NotifyAudioLinkOpenL( const TBTDevAddr& aBDAddress, TAccAudioType aType )= 0;

        /**
        * Notifies the DOS that the audio connection to the currently connected
        * accessory has been closed.
        * @param aBDAddress             BT hardware address of the accessory.
        * @return                       None.
        */
        virtual void NotifyAudioLinkCloseL( const TBTDevAddr& aBDAddress, TAccAudioType aType )= 0;
       
  
        /**
        * Requests Accessory Framework or Dos to disable NREC
        * @param aBDAddress             BT hardware address of the accessory.
        * @return                       None.
        */
        virtual TInt DisableNREC(const TBTDevAddr& aBDAddress) = 0; 
    };
    
#endif
