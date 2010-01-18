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
* Description:  This class implements the audio data sending behaviour 
*                that is common to all codecs. 
*
*/



#ifndef __BTAUDIOSTREAMSENDER_H__
#define __BTAUDIOSTREAMSENDER_H__

#include <bluetooth/a2dpoptimisation.h>
#include <bluetooth/a2dpoptimisationparams.h>
 
#include "btaudiostreamer.h" 

/**
 *  This class handles the sending of RTP frames on general level. 
 *
 *  This class takes care of all the operations related to an RTP frame that are 
 *  common to all formats. Such operations are e.g. timestamp handling and 
 *  frame sending. 
 *  
 *  The child classes of this are used for handling the encoder specific 
 *  frame handling. This class specifies and calls pure virtual methods that 
 *  must be implemented by an encoder specific child class. 
 *
 *  @lib btaudiostreamer.lib
 *  @since S60 v3.1 
 */
NONSHARABLE_CLASS(CBTAudioStreamSender) : public CActive
    {
    public:
    static CBTAudioStreamSender* NewL(MBTAudioStreamSenderObserver& aObserver, RRtpSession& aSession);
    ~CBTAudioStreamSender();

    /**
     * A callback for registering with RTP API 
     *
     * @since S60 v3.1
     * @param aPtr  An pointer to an instance that handles the callback. 
     * @param aEvent Handle to the event that occurred. 
     * @return void. 
     */
    static void PacketSent(CBTAudioStreamSender* aPtr, const TRtpEvent& aEvent);
    /**
     * A callback for registering with RTP API 
     *
     * @since S60 v3.1
     * @param aPtr  An instance who handles the callback. 
     * @param aEvent Handle to the event that occurred. 
     * @return void. 
     */
    static void SendError(CBTAudioStreamSender* aPtr, const TRtpEvent& aEvent);

    /**
     * Method for actually handling the RTP API callbacks for sent packets. 
     *
     * @since S60 v3.1
     * @return void. 
     */
    virtual void DoPacketSent();

    /**
     * Method for actually handling the RTP API callback for send errors. 
     *
     * @since S60 v3.1
     * @return void. 
     */
    virtual void DoSendError();

    virtual TInt SetNewFrameLength(TUint aOutboundMTUSize, const TUint aFrameLength, const TUint aTargetBitrate);

    /**
     * This method prepares the Stream Sender for incoming audio buffers. 
     *
     * @since S60 v3.1
     * @param aOutboundMTUSize The MTU size for outgoing L2CAP link. 
     * @param aFrameLength  The length of one audio data frame. 
     * @return void. 
     */
    virtual void StartL(TUint aOutboundMTUSize, const TUint aFrameLength, const TUint aTargetBitrate);

    /**
     * This method contains the code for closing the RTP API and cleaning up. 
     *
     * @since S60 v3.1
     * @return void. 
     */
    virtual void Stop();

    /**
     * An implementation of the buffer handling method. 
     *
     * @since S60 v3.1
     * @param aBuffer contains the audio data frames. 
     * @param aTimestamp The timestamp of the audio data frames. 
     * @return a Symbian OS wide error code. 
     */
    virtual TInt SendThisBuffer(const TDesC8& aBuffer, TTimeIntervalMicroSeconds aTimestamp);

    protected: 
    void ConstructL();
    CBTAudioStreamSender(MBTAudioStreamSenderObserver& aObserver, RRtpSession& aSession);

    void RunL();
    void DoCancel();

    /**
     * A pure virtual method that must be implemented by the encoder specific child class. 
     * This method moves the audio frames from buffer to SendPacket in encoder specific way. 
     *
     * @since S60 v3.1
     * @param aBuffer contains the audio data frames. 
     * @return a Symbian OS wide error code. 
     */
    virtual TInt AddBufferToSendPacket(const TDesC8& aBuffer) = 0; 

    /**
     * A pure virtual method that must be implemented by the encoder specific child class. 
     * This method places a header to SendPacket in encoder specific way. 
     *
     * @since S60 v3.1
     * @param void. 
     * @return a Symbian OS wide error code. 
     */
    virtual TInt AddHeaderToSendPacket(TPtr8& aPayloadDesC) = 0; 

    /**
     * A pure virtual method that must be implemented by the encoder specific child class. 
     * This method handles the encoder specific operations that should be done after the packet 
     * has been sent. 
     *
     * @since S60 v3.1
     * @param void. 
     * @return a Symbian OS wide error code. 
     */
    virtual TInt PacketSendingCompleted(TPtr8& aPayloadDesC) = 0; 

    /**
     * A pure virtual method that must be implemented by the encoder specific child class. 
     * This method handles the encoder specific packet length calculation.  
     *
     * @since S60 v3.1
     * @param aOutboundMTUSize The MTU size for outgoing L2CAP link. 
     * @param aFrameLength  The length of one audio data frame. 
     * @return a Symbian OS wide error code. 
     */
    virtual TInt CalculatePacketLength(TUint aOutboundMTUSize, const TUint aFrameLength) = 0; 

    /**
     * A pure virtual method that must be implemented by the encoder specific child class. 
     * This method returns the maximum number of frames in one outgoing packet.  
     *
     * @since S60 Timebox 9.2
     * @return number of frames in one send packet. 
     */
    virtual TUint MaxFramesPerPacket() = 0; 

    /**
     * 
     * This method calculates the playing time of one frame in microseconds.
     * The result is encoder specific. 
     *
     * @since S60 Timebox 9.2
     * @param void. 
     * @return a duration of one frame. 
     */
    virtual TUint CalculateFrameDuration(const TUint aFrameLength, const TUint aTargetBitrate) = 0; 


    /**
     * A pure virtual method that must be implemented by the encoder specific child class. 
     * This method handles the encoder specific initialisation.  
     *
     * @since S60 v3.1
     * @param void. 
     * @return void. 
     */
    virtual TInt InitSender(RRtpSendPacket& aSendPacket, TPtr8& aPayloadDesC, const TUint aTargetBitrate) = 0; 

    /**
     * A pure virtual method that must be implemented by the encoder specific child class. 
     * This method adjusts the incoming timestamp if buffers don't fill the SendPacket exactly.  
     *
     * @since S60 v5.0
     * @param aTimestamp A 64 bit representation of the timestamp to adjust. This will change during the call. 
     * @return void. 
     */
	virtual void AdjustTimestamp(TInt64 &aTimestamp) = 0; 


    private: 
    /**
     * An inline method that empties the received buffer. 
     * This method uses encoder specific inherited class to handles the encoder specific data.  
     * and then decides whether to send the data out or request for more data. 
     *
     * @since S60 v3.1
     * @param void. 
     * @return void. 
     */
    inline void ConsumeBuffer(); 

    /**
     * An inline method that switches to new sendpacket when the length of frame has changed. 
     * This method also initializes the sender to use the new packet. 
     *
     * @since S60 v3.1
     * @param void. 
     * @return void. 
     */
		inline void SwitchToNewSendPacket(); 

    /**
     * An inline method that self completes this active object. 
     * The purpose of this is to set the data provider thread free 
     * and continue execution in this thread's context.  
     *
     * @since S60 v3.1
     * @param void. 
     * @return void. 
     */
    inline void ProceedToPacketSending(); 

    
    private: 

    /**
     * The possible states of the Streamer. 
     */
    enum TStreamerState
        {
        EStopped, 
        EBuffering,
        ESending, 
        EErrorSending 
        };

    /**
    * A2DP optimiser object that is needed for prioritising A2DP audio over other data. 
    */
   RA2dpOptimiser iA2DPOptimiser; 

   /**
    * This stores a reference to the observer of this class. 
    */
   MBTAudioStreamSenderObserver&    iObserver;

   /**
    * RTP session reference, needed for creating the SendSource. 
    */
   RRtpSession&    iRtpSession;

   /**
    * RRtpSendSoure is needed for SendPacket management. 
    */
   RRtpSendSource    iSendSrc;

   /**
    * This stores the timestamp which we must place in the header. 
    */
   TInt64 iTimestamp; 

    /**
     * This stores the timestamp which we must place in the header. 
     */
    TInt64 iAdjustedTimestamp; 


   /**
    * This stores the current state of the streamer. 
    */
   TStreamerState     iStreamerState; 

   /**
    * This stores a reference to the buffer until it has 
    * been emptied and can be given back to the audio adaptation. 
    */
   const TDesC8 *iBuffer; 

   /**
    * These assist with creating RTP payloads with proper header and content. 
    */
   RRtpSendPacket*   iCurrentSendPacket; 
   RRtpSendPacket*   iSpareSendPacket; 
   
   /**
    * A pointer to the send packet's contents. 
    * It is used to abstract away the container of the data. 
    */
    TPtr8            iPayloadDesC;

   /**
    * An instance of our thread. This is needed for self completing 
    * the requests, which forces the ActiveScheduler to call our RunL. 
    */
   RThread iThread; 

    /**
     * This stores the new framelength when the streaming parameters are changing. 
     */
    TInt iNewFrameLength; 

    /**
     * This stores the target bitrate. The reason for this is that it is not applied 
     * immediately, but after the current packet is sent. 
     */
    TUint iTargetBitrate; 

    /**
     * This tells if the frame length is changing. If it is, then we won't put 
     * frames of new length into the same packet with the old frames. 
     * After sending the old frames a new sendpacket is taken into use.   
     */
    TBool iChangingFrameLength; 
		
    /**
     * This tells whether we should keep the previous timestamp or overwrite 
     * it with the most recent one. 
     */
    TBool     iKeepPreviousTimestamp; 

    /**
     * This tells whether we should keep moving data from buffer to an incoming frame. 
     * The value is set to ETrue when new frame arrives and when the buffer was completely consumed 
     * and sent, this value is set to EFalse. 
     */
    TBool     iNonprocessedDataInBuffer; 

    /**
     * This tells if the streaming optimiser service is available. 
     */
    TBool     iLinkOptimiserAvailable; 

    };

#endif // __BTAUDIOSTREAMSENDER_H__
