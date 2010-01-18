/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This class implements the SBC codec specific data handling. 
*
*/




#ifndef __BTAUDIOSTREAMSENDERSBC_H__
#define __BTAUDIOSTREAMSENDERSBC_H__
 
#include "btaudiostreamsender.h" 

/**
 *  SBC codec specific implementation of CBTAudioStreamSender base class 
 *
 *  This class wraps the handling of SBC frames and it can place the 
 *  media packet header in place. 

 *  @lib btaudiostreamer.lib
 *  @since S60 v3.1 
 */
NONSHARABLE_CLASS(CBTAudioStreamSenderSBC) : public CBTAudioStreamSender
    {
    public:
    static CBTAudioStreamSenderSBC* NewL(MBTAudioStreamSenderObserver& aObserver, RRtpSession& aSession);
    virtual ~CBTAudioStreamSenderSBC();

    private:
    CBTAudioStreamSenderSBC(MBTAudioStreamSenderObserver& aObserver, RRtpSession& aSession);

    /**
     * From CBTAudioStreamSender.
     * This method places to received buffer into the send packet in SBC specific way. 
     *
     * @since S60 v3.1
     * @param aBuffer contains the SBC frames. 
     */
    virtual TInt AddBufferToSendPacket(const TDesC8& aBuffer); 

    /**
     * From CBTAudioStreamSender.
     * This method places the SBC codec specific media packet header in its place. 
     *
     * @since S60 v3.1
     * @param void. 
     * @return a Symbian OS wide error code. 
     */
    virtual TInt AddHeaderToSendPacket(TPtr8& aPayloadDesC); 

    /**
     * From CBTAudioStreamSender.
     * This method does the cleanup work after the packet has been sent. 
     *
     * @since S60 v3.1
     * @param void. 
     * @return a Symbian OS wide error code. 
     */
    virtual TInt PacketSendingCompleted(TPtr8& aPayloadDesC); 

    /**
     * From CBTAudioStreamSender.
     * This method is used for calculating the required packet length. 
     * It takes the header's into account and tries to fit as many frames 
     * to the packet that can fit to the outgoing MTU. 
     *
     * @since S60 v3.1
     * @param aOutboundMTUSize The MTU size for outgoing L2CAP link. 
     * @param aFrameLength  The length of one SBC frame. 
     */
    virtual TInt CalculatePacketLength(TUint aOutboundMTUSize, TUint aFrameLength); 

    /**
     * From CBTAudioStreamSender.
     * This method prepares the class for sending packets. 
     *
     * @since S60 v3.1
     * @param void. 
     * @return a Symbian OS wide error code. 
     */
    virtual TInt InitSender(RRtpSendPacket& aSendPacket, TPtr8& aPayloadDesC, const TUint aTargetBitrate); 

    /**
     * From CBTAudioStreamSender.
     * This method corrects the timestamp value when incoming buffers can't fit in one sendpacket. 
     *
     * @since S60 v5.0
     * @param void. 
     * @return void.
     */
		virtual void AdjustTimestamp(TInt64 &aTimestamp); 

    /**
     * 
     * This method calculates the playing time of one frame in microseconds. 
     * Information is used also internally for correcting the timestamp values. 
     *
     * @since S60 v5.0
     * @param void. 
     * @return a duration of one frame. 
     */
    virtual TUint CalculateFrameDuration(const TUint aFrameLength, const TUint aTargetBitrate); 
		
	/**
     * This method returns the maximum number of frames in one outgoing packet.  
     *
     * @since S60 Timebox 9.2
     * @return number of frames in one send packet. 
     */
	virtual TUint MaxFramesPerPacket(); 

	
    private: 

    /**
     * Stores the corrected timestamp value. 
     */
		TInt64 iTimestampOfFirstFrameInSendPacket; 

    /**
     * Stores the length of one SBC frame. 
     */
    TInt iFrameLength; 

    /**
     * Stores the length of one SBC frame. Used when the frame length changes. 
     */
    TInt iNewFrameLength; 

    /**
     * Stores the duration of one SBC frame in microseconds. Used for correcting the timestamps in long buffers. 
     */
    TInt iDurationOfFrame; 

    /**
     * Used for calculating how many frames can fit in one SendPacket. 
     */
		TInt iSpaceNeededForBuffer; 
		
    /**
     * Stores the number of frames we have moved from buffer to packet so far. 
     * This variable is needed when the whole buffer can't fit in the current packet. 
     */
    TInt iNumOfFramesAlreadyMoved; 

    /**
     * Stores the maximum number of frames that we can place in one packet. 
     */
    TInt iMaxNumOfFrames; 
    
    /**
     * This stores the number of frames in send packet. 
     * This information is needed for A2DP media packet header. 
     */
    TInt iNumOfFramesInSendPacket; 

    /**
     * This stores the end position of the SendPacket's payload. 
     */
    const TUint8 *iPtrEndOfPayload; 

    /**
     * This stores the number of frames that haven't been sent yet. It's needed if 
     * there's more incoming frames than can fit in single SendPacket. 
     */
    TInt iNumOfFramesLeftInBuffer; 

    /**
     * This tells if it is time to start using the passed in timestamp as reference. 
     */
		TBool iTimestampChangeFlag; 

    };

#endif // __BTAUDIOSTREAMSENDERSBC_H__
