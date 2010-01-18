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
* Description:  Contains an implementation of CBTAudioStreamSenderSBC class.
*
*/


#include "btaudiostreamsendersbc.h"
#include "btaudiostreamerdebug.h" 

const TInt KA2DPMediaPacketHeaderLength = 1; 
const TInt KA2DPMediaPacketHeaderIndex = 0; 
const TInt KRTPHeaderLength = 12; 

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// Constructor. 
// ---------------------------------------------------------------------------
//
CBTAudioStreamSenderSBC::CBTAudioStreamSenderSBC(MBTAudioStreamSenderObserver& aObserver, RRtpSession& aSession): 
        CBTAudioStreamSender(aObserver, aSession) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::CBTAudioStreamSenderSBC() ->")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::CBTAudioStreamSenderSBC() <-")));
    }

// ---------------------------------------------------------------------------
// Factory method.
// ---------------------------------------------------------------------------
//
/*static*/ CBTAudioStreamSenderSBC* CBTAudioStreamSenderSBC::NewL(MBTAudioStreamSenderObserver& aObserver, RRtpSession& aSession)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::NewL() ->")));

    CBTAudioStreamSenderSBC* self = new (ELeave) CBTAudioStreamSenderSBC(aObserver, aSession);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::NewL() <-")));
    return self;
    }

// ---------------------------------------------------------------------------
// Destructor. 
// ---------------------------------------------------------------------------
//
CBTAudioStreamSenderSBC::~CBTAudioStreamSenderSBC() 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::~CBTAudioStreamSenderSBC() ->")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::~CBTAudioStreamSenderSBC() <-")));
    }

// ---------------------------------------------------------------------------
// From class CBTAudioStreamSender.
// This method stores the frames to the frame buffer and 
// when we have enough frames. 
// ---------------------------------------------------------------------------
//
TInt CBTAudioStreamSenderSBC::AddHeaderToSendPacket(TPtr8& aPayloadDesC) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::AddHeaderToSendPacket() ->")));
    aPayloadDesC[KA2DPMediaPacketHeaderIndex] = iNumOfFramesInSendPacket; 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::AddHeaderToSendPacket() <-")));
    return KErrNone; 
    }

// ---------------------------------------------------------------------------
// From class CBTAudioStreamSender.
// This method stores the frames to the frame buffer and 
// when we have enough frames. 
// ---------------------------------------------------------------------------
//
TInt CBTAudioStreamSenderSBC::AddBufferToSendPacket(const TDesC8& aBuffer) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::AddBufferToSendPacket() ->")));
    // Copy data from the buffer to the outgoing packet. 
    // The return code means how many frames can still fit in the SendPacket. 
    //     - If there's more space, the value > 0. In this case we need more data. 
    //  - If the buffer is full and all data was consumed, the value == 0. In this case the packet must be sent. 
    //     - If the buffer is full and all data didn't fit in it, the value < 0. The packet must be sent and this method called again with the same frame. 

    // Calculate how many frames there's left in the buffer. Note that iFrameLength must always be greater than zero. 
    TInt numOfFramesLeftInBuffer = (aBuffer.Length() / iFrameLength) - iNumOfFramesAlreadyMoved ; 

    // If the whole buffer can fit in packet, move it completely, otherwise move as many frames as can fit. 
    TInt numOfFramesToMove = numOfFramesLeftInBuffer <= iMaxNumOfFrames - iNumOfFramesInSendPacket ? 
                            numOfFramesLeftInBuffer : 
                            iMaxNumOfFrames - iNumOfFramesInSendPacket; 

    // Some traces for seeing what's going on. 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Filling sendpacket...")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t %d frames can fit in packet."), iMaxNumOfFrames));    
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t %d frames have already been moved from current buffer to sendpacket."), iNumOfFramesAlreadyMoved));    
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t %d frames left to move from current buffer."), numOfFramesLeftInBuffer));    
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t %d frames already in current packet."), iNumOfFramesInSendPacket));    
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t => Moving %d frames."), numOfFramesToMove));    

    // Move the number of frames calculated above to the packet: 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t replacing sendpacket content from %d, length is %d..."), KA2DPMediaPacketHeaderLength + iNumOfFramesInSendPacket * iFrameLength, numOfFramesToMove * iFrameLength));    

    // Make the ptrNextFrameInBuffer point to the first frame that hasn't been moved. 
    const TUint8 * ptrNextFrameInBuffer = aBuffer.Ptr() + iNumOfFramesAlreadyMoved * iFrameLength; 	
    memcpy((void *)iPtrEndOfPayload, (void *)ptrNextFrameInBuffer, numOfFramesToMove * iFrameLength); 
    iPtrEndOfPayload += numOfFramesToMove * iFrameLength; 
    ptrNextFrameInBuffer += numOfFramesToMove * iFrameLength; 

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t %d frames moved from buffer to packet."), numOfFramesToMove));    

    // If we have now moved them all, reset the counter, otherwise update it for the next round. 
    iNumOfFramesAlreadyMoved = numOfFramesLeftInBuffer == numOfFramesToMove ? 0 : iNumOfFramesAlreadyMoved + numOfFramesToMove; 

    // update the information needed for building the header later (in sending phase): 
    iNumOfFramesInSendPacket += numOfFramesToMove; 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t iNumOfFramesInSendPacket incremented, it is now %d"), iNumOfFramesInSendPacket));    

    numOfFramesLeftInBuffer -= numOfFramesToMove; 

    // return the number of frames that will still fit in the packet: 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t returning: %d"), iMaxNumOfFrames - iNumOfFramesInSendPacket - numOfFramesLeftInBuffer));    
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::AddBufferToSendPacket() <-")));
    return (iMaxNumOfFrames - iNumOfFramesInSendPacket - numOfFramesLeftInBuffer); 
    }

// ---------------------------------------------------------------------------
// From class CBTAudioStreamSender.
// This method stores the frames to the frame buffer and 
// when we have enough frames. 
// ---------------------------------------------------------------------------
//
TInt CBTAudioStreamSenderSBC::PacketSendingCompleted(TPtr8& aPayloadDesC) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::PacketSendingCompleted() ->")));
    iNumOfFramesInSendPacket = 0; 
    iPtrEndOfPayload = aPayloadDesC.Ptr() + KA2DPMediaPacketHeaderLength; 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::PacketSendingCompleted() <-")));
    return KErrNone; 
    }

// ---------------------------------------------------------------------------
// From class CBTAudioStreamSender.
// This method sets up the sender and calculates the required packet  
// length according to its own implementation. 
// ---------------------------------------------------------------------------
//
TInt CBTAudioStreamSenderSBC::InitSender(RRtpSendPacket& aSendPacket, TPtr8& aPayloadDesC, const TUint aTargetBitrate) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::InitSender() ->")));
    iFrameLength = iNewFrameLength; 
    iMaxNumOfFrames = iSpaceNeededForBuffer / iFrameLength; 

    // Make the descriptor point in the beginning of the send packet. 
    aPayloadDesC.Set(const_cast<TUint8*>(aSendPacket.WritePayload().Ptr()), iSpaceNeededForBuffer, iSpaceNeededForBuffer);
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t Descriptor inited, space needed: %d bytes."), iSpaceNeededForBuffer));    

    iPtrEndOfPayload = aPayloadDesC.Ptr() + KA2DPMediaPacketHeaderLength; 

    iNumOfFramesInSendPacket = 0; 
		
		iTimestampOfFirstFrameInSendPacket = 0; 

		iDurationOfFrame = CalculateFrameDuration(iFrameLength, aTargetBitrate); 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t iDurationOfFrame: %d"), iDurationOfFrame));    

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::InitSender() <-")));
    return KErrNone; 
    }

TInt CBTAudioStreamSenderSBC::CalculatePacketLength(TUint aOutboundMTUSize, const TUint aFrameLength) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::CalculatePacketLength() <-")));
		if(aFrameLength > 0) // Must not be zero, and cannot be according to A2DP spec. 
		    {
        iNewFrameLength = aFrameLength; 

				iSpaceNeededForBuffer = ((aOutboundMTUSize - KA2DPMediaPacketHeaderLength - KRTPHeaderLength) / iNewFrameLength) * iNewFrameLength + KA2DPMediaPacketHeaderLength; 

    		BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::CalculatePacketLength() <-")));
        return iSpaceNeededForBuffer; 
        }
    else 
        {
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t Length parameter is zero!")));    
		    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::CalculatePacketLength() <-")));
        return 0; 
        }
    }

TUint CBTAudioStreamSenderSBC::CalculateFrameDuration(const TUint aFrameLength, const TUint aTargetBitrate) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::CalculateFrameInterval() ->")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::CalculateFrameInterval() <-")));
    return static_cast<TUint>(static_cast<TReal>(aFrameLength * 8) / static_cast<TReal>(aTargetBitrate)  * 1000000.0); 
    }

TUint CBTAudioStreamSenderSBC::MaxFramesPerPacket()  
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::FramesPerPacket() ->")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::FramesPerPacket() <-")));
    return iMaxNumOfFrames; 
    }

void CBTAudioStreamSenderSBC::AdjustTimestamp(TInt64& aTimestamp) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::AdjustTimestamp() ->")));

		// The buffer has been emptied, but there may be space for additional frames in sendpacket. 
		// So keep the current timestamp until the sendpacket becomes full. 
	  if(iNumOfFramesAlreadyMoved == 0) 
			  {
		    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Buffer became empty.")));
				iTimestampChangeFlag = ETrue; 
			  }

		if(iNumOfFramesInSendPacket == iMaxNumOfFrames) // Adjust the timestamp only when the sendpacket is full. 
				{
				if(iTimestampChangeFlag != EFalse) 
						{
						// When the buffer been emptied earlier, take the new timestamp into use and adjust it by the amount of frames we just moved. 
						iTimestampOfFirstFrameInSendPacket = aTimestamp + iDurationOfFrame * iNumOfFramesAlreadyMoved; 
				    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t New timestamp %d (low) is in use."), I64LOW(aTimestamp))); 
				    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t New timestamp %d (high) is in use."), I64HIGH(aTimestamp))); 
						iTimestampChangeFlag = EFalse; 
						}
		    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t Timestamp is %d (low)"), I64LOW(iTimestampOfFirstFrameInSendPacket)));    
		    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t Timestamp is %d (high)"), I64HIGH(iTimestampOfFirstFrameInSendPacket)));    

				// Use the timestamp first and then do a normal adjustment for the next round. 
				aTimestamp = iTimestampOfFirstFrameInSendPacket; 
		    iTimestampOfFirstFrameInSendPacket = iTimestampOfFirstFrameInSendPacket + iMaxNumOfFrames * iDurationOfFrame; 
				}
	  BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSenderSBC::AdjustTimestamp() <-")));
    }
