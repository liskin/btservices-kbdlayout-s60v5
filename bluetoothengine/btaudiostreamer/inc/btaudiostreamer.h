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
* Description:  This is an interface class that exports streamer's functions 
*                for its clients. The functionality is implemented in 
*                BTAudioStreamerSender, this class forwards the function 
*                calls to it. 
*
*/


#ifndef __BTAUDIOSTREAMER_H__
#define __BTAUDIOSTREAMER_H__

#include <in_sock.h>
#include <rtp.h> 

#include "btaudiostreaminputbase.h" 

class CBTAudioStreamSender; 

/**
 *  An observer pattern. 
 *
 *  This class defines an observer interface for BT Audio Stream Sender. 
 *
 *  @lib btaudiostreamer.lib
 *  @since S60 v3.1 
 */
NONSHARABLE_CLASS(MBTAudioStreamSenderObserver) 
    {
    public:
    virtual void NotifyBufferSent(const TDesC8& aBuffer ) = 0;         // Buffer sent. 
    virtual void NotifyErrorSending(const TDesC8& aBuffer ) = 0;    // Could not send a packet at all. 
    };

/**
 *  BT Audio Streamer class. 
 *
 *  This class implements the audio streaming over A2DP. 
 *  It acts as an interface between the controller, BT Audio Adaptation and RTP API. 
 *  It uses helper class CBTAudioStreamSender to handle 
 *  the frame contruction and sending. 
 *
 *  @lib btaudiostreamer.lib
 *  @since S60 v3.1 
 */
class CBTAudioStreamer : public CBase, public MBTAudioStreamObserver, public MBTAudioStreamSenderObserver 
    {
    public:

    IMPORT_C static CBTAudioStreamer* NewL(); 
    
    /**
     * A method for preparing the BT Audio Streamer for incoming data. 
     *
     * @since S60 v3.1
     * @param aSocket Bluetooth socket instance, needed for getting the RTP sending class instance. 
     * @param aFrameLength Length of a single audio data frame, needed for calculating the send packet size. 
     * @param aAudioInput The API where the data is received and where the sending of packets will be confirmed. 
     * @return void. 
     */
    IMPORT_C TInt SetNewFrameLength(const TUint aFrameLength, const TUint aTargetBitrate);

    /**
     * A method for preparing the BT Audio Streamer for incoming data. 
     *
     * @since S60 v3.1
     * @param aSocket Bluetooth socket instance, needed for getting the RTP sending class instance. 
     * @param aFrameLength Length of a single audio data frame, needed for calculating the send packet size. 
     * @param aAudioInput The API where the data is received and where the sending of packets will be confirmed. 
     * @return void. 
     */
    IMPORT_C void StartL(RSocket& aSocket, const TUint aFrameLength, CBTAudioStreamInputBase* aAudioInput, const TUint aTargetBitrate);

    /**
     * A method for cleaning up the BT Audio Streamer after streaming data. 
     *
     * @since S60 v3.1
     * @param void. 
     * @return void. 
     */
    IMPORT_C void Stop(); 

    virtual ~CBTAudioStreamer();

    /**
     * From MBTAudioStreamObserver.
     * This method receives data from the data producer. 
     *
     * @since S60 v3.1
     * @param aBuffer contains the audio data frames. 
     * @return a Symbian OS wide error code. 
     */
    TInt Receive(const TDesC8& aBuffer);

    /**
     * From MBTAudioStreamObserver.
     * This method receives data and timestamps from the data producer. 
     *
     * @since S60 v3.1
     * @param aBuffer contains the audio data frames. 
     * @param aTimestamp The timestamp of the audio data frames. 
     *                   34 minutes max. interval should not be a problem to us. 
     * @return a Symbian OS wide error code. 
     */
    TInt Receive(const TDesC8& aBuffer, TTimeIntervalMicroSeconds aTimestamp);

    /**
     * From MBTAudioStreamSenderObserver.
     * This method is used for reporting that the referred buffer has been sent. 
     * This tells the data producer that we're ready to receive more data. 
     *
     * @since S60 v3.1
     * @param aBuffer refers to the original bufffer that we received in Receive method. 
     * @return void. 
     */
    void NotifyBufferSent( const TDesC8& aBuffer );  

    /**
     * From MBTAudioStreamSenderObserver.
     * This method is used for reporting that the buffer wasn't sent because of an error. 
     * This method then informs the error observer class. 
     *
     * @since S60 v3.1
     * @param aBuffer refers to the original bufffer that we received in Receive method. 
     * @return void. 
     */
    void NotifyErrorSending( const TDesC8& aBuffer );  

    private:
    CBTAudioStreamer();
    void ConstructL();

    private:
    /**
     * This stores a pointer to the audio input interface. 
     * After a buffer has been processed, this interface must be informed. 
     */
    CBTAudioStreamInputBase* iAudioInput; 

    /**
     * This points to a helper class that handles the sending of a frame. 
     * Own.  
     */
    CBTAudioStreamSender* iSender; 

    /**
     * RTP session object that is needed for sending RTP frames. 
     */
    RRtpSession iRtpSession;

    /**
     * This stores the state of the streamer. 
     */
		TBool iStarted;

    }; 

#endif // __BTAUDIOSTREAMER_H__