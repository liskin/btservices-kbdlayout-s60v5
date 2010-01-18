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
* Description:  Contains an implementation of CBTAudioStreamSender class.
*
*/


#include "btaudiostreamsender.h"
#include "btaudiostreamsendersbc.h"
#include "btaudiostreamerdebug.h" 
#include <btsockaddr.h> // TBTSockAddr 

// ---------------------------------------------------------------------------
// Constructor. 
// ---------------------------------------------------------------------------
//
CBTAudioStreamSender::CBTAudioStreamSender(MBTAudioStreamSenderObserver& aObserver, RRtpSession& aSession) :
    CActive(EPriorityHigh),     
    iObserver(aObserver), 
    iRtpSession(aSession), 
    iPayloadDesC(NULL,NULL), 
    iLinkOptimiserAvailable(EFalse)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::CBTAudioStreamSender() ->")));
    CActiveScheduler::Add(this);    
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::CBTAudioStreamSender() <-")));
    }

// ---------------------------------------------------------------------------
// Symbian OS second phase contruction. 
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::ConstructL()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::ConstructL() ->")));   

    iSendSrc = iRtpSession.NewSendSourceL();
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t New send source created.")));
    iSendSrc.PrivRegisterEventCallbackL(ERtpSendSucceeded, (TRtpCallbackFunction)CBTAudioStreamSender::PacketSent, this);
    iSendSrc.PrivRegisterEventCallbackL(ERtpSendFail, (TRtpCallbackFunction)CBTAudioStreamSender::SendError, this);
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Callbacks registered.")));

    iStreamerState = EStopped; 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t State changed to: EStopped")));

    iThread = RThread();

    iCurrentSendPacket = new (ELeave) RRtpSendPacket; 

    iSpareSendPacket = new (ELeave) RRtpSendPacket; 

    if(iA2DPOptimiser.Open() == KErrNone)
        {
        iLinkOptimiserAvailable = ETrue; 
        }

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::ConstructL() <-")));
    }

// ---------------------------------------------------------------------------
// Static factory method. 
// ---------------------------------------------------------------------------
//
CBTAudioStreamSender* CBTAudioStreamSender::NewL(MBTAudioStreamSenderObserver& aObserver, RRtpSession& aSession)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::NewL() ->")));
    if (!aSession.IsOpen())
        {
        User::Leave(KErrArgument);
        }
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::NewL() <-")));
    return CBTAudioStreamSenderSBC::NewL(aObserver, aSession); 
    }

// ---------------------------------------------------------------------------
// Destructor. 
// ---------------------------------------------------------------------------
//
CBTAudioStreamSender::~CBTAudioStreamSender()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::~CBTAudioStreamSender() ->")));
    Cancel();
    if(iCurrentSendPacket) 
        {
        if(iCurrentSendPacket->IsOpen() != EFalse)
            {
   			iCurrentSendPacket->Close();
            }
         delete iCurrentSendPacket; 
         iCurrentSendPacket = NULL; 
         }

    if(iSpareSendPacket) 
        {
        if(iSpareSendPacket->IsOpen() != EFalse)
            {
            iSpareSendPacket->Close(); 
            }
        delete iSpareSendPacket; 
        iSpareSendPacket = NULL; 
        }
    iSendSrc.Close();

    if(iLinkOptimiserAvailable)
        {
        iA2DPOptimiser.Close();
        }

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::~CBTAudioStreamSender() <-")));
    }

// ---------------------------------------------------------------------------
// This is the static callback method which CBTAudioStreamSender registers with the RtpSession to be called
// when a packet is _successfully_ sent.  It is a static method so simply calls a processing
// method on the class pointed to be aPtr (which is the CBTAudioStreamSender which registered the cb).
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::PacketSent(CBTAudioStreamSender* aPtr, const TRtpEvent& /*aEvent*/)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::PacketSent() ->")));
    aPtr->DoPacketSent();
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::PacketSent() <-")));
    }

// ---------------------------------------------------------------------------
// An instance method which is called by the static callback function.  We simply notify the
// observer if all data has been sent. If it hasn't, then we send more data. 
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::DoPacketSent()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::DoPacketSent() ->")));

	// First check if we're Stopping. 
    if(iStreamerState == EStopped)
        {
	    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t Stopping and packet sending completed.")));
        iObserver.NotifyBufferSent(*iBuffer); // Return the buffer. 
		return; // No need to do more here, we're stopping. 
        }
	else
		{
		iStreamerState = EBuffering; 
    	BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t State changed to: EBuffering")));
		}

    PacketSendingCompleted(iPayloadDesC); 
 
    // If we're here, then there either is data in the buffer or it has been emptied completely. 
    // Check this by calling the method again. If we consumed the buffer completely, 
    // the method just signals the observer. 
    if(iNonprocessedDataInBuffer != EFalse) 
        {
				// Check if we need to start using new sendpacket. 
        if(iChangingFrameLength != EFalse)
            {
        	  SwitchToNewSendPacket(); 
		        }
            ConsumeBuffer(); 
        }
    else 
        {
        iObserver.NotifyBufferSent(*iBuffer); 
        }

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::DoPacketSent() <-")));
}

// ---------------------------------------------------------------------------
// This is the static callback method which CBTAudioStreamSender registers with the RtpSession to be called
// when a packet sending fails.
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::SendError(CBTAudioStreamSender* aPtr, const TRtpEvent& /*aEvent*/)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SendError() ->")));
    aPtr->DoSendError();
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SendError() <-")));
    }

// ---------------------------------------------------------------------------
// An instance method which is called by the static callback function.  
// We set the state to EErrorSending and handle the error next time in RunL. 
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::DoSendError()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::DoSendError() ->")));
    
    iStreamerState = EErrorSending; 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t State changed to: EErrorSending")));
    if (!IsActive())
        {
        TRequestStatus *status = &iStatus;
        iStatus = KRequestPending;
        iThread.RequestComplete(status, KErrNone);
        SetActive();
        }
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::DoSendError() <-")));
    }

// ---------------------------------------------------------------------------
// This method is for setting the sender up. 
// ---------------------------------------------------------------------------
//
TInt CBTAudioStreamSender::SetNewFrameLength(TUint aOutboundMTUSize, const TUint aFrameLength, const TUint aTargetBitrate)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SetNewFrameLength() ->")));
		TInt retVal = KErrGeneral;

		// Don't proceed if the previous change is not completed yet. 
		if(iChangingFrameLength != EFalse)
		    {
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Previous frame length change still ongoing!")));
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SetNewFrameLength() <-")));
		    return KErrInUse; 
		    }

    if(iSpareSendPacket) 
        {
        __ASSERT_DEBUG(!iSpareSendPacket->IsOpen(),User::Invariant());
        }

    TInt requiredPacketLength = CalculatePacketLength(aOutboundMTUSize, aFrameLength); 

    if(requiredPacketLength > 0) 
        {
		    iNewFrameLength = aFrameLength; 

    		BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Creating new sendpacket.")));
			__ASSERT_DEBUG(iSpareSendPacket, User::Invariant());
    		TRAPD(ret, *iSpareSendPacket = iSendSrc.NewSendPacketL(requiredPacketLength));
        	if( ret )
        		{
        		BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t ERROR! Creating new sendpacket failed.")));
        		return KErrGeneral;
        		}
        	iChangingFrameLength = ETrue; 
			retVal = KErrNone; 
      }
    else 
        {
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t Error! Packet length must be greater than zero!")));
        retVal = KErrArgument; 
        }
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SetNewFrameLength() <-")));

    iTargetBitrate = aTargetBitrate; 
    return retVal; 
    }


// ---------------------------------------------------------------------------
// This method is for setting the sender up. 
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::StartL(TUint aOutboundMTUSize, const TUint aFrameLength, const TUint aTargetBitrate)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::StartL() ->")));
    if(iCurrentSendPacket) 
        {
        __ASSERT_DEBUG(!iCurrentSendPacket->IsOpen(),User::Invariant());
        }
    TInt requiredPacketLength = CalculatePacketLength(aOutboundMTUSize, aFrameLength); 

    if(requiredPacketLength > 0) 
        {
		__ASSERT_DEBUG(iCurrentSendPacket, User::Invariant());
        *iCurrentSendPacket = iSendSrc.NewSendPacketL(requiredPacketLength); 
        iTargetBitrate = aTargetBitrate; 
        InitSender(*iCurrentSendPacket, iPayloadDesC, iTargetBitrate); 
        iStreamerState = EBuffering; 
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t State changed to: EBuffering")));
      }
    else 
        {
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t Error! Packet length must be greater than zero!")));
        User::Leave(KErrArgument); 
        }

    // Streaming started, optimise the ACL link if optimiser service is available. 
    if(iLinkOptimiserAvailable)
        {
        // The below code is to get the connected device's BT Address.
        TBTSockAddr addr; 
        iRtpSession.RtpSocket()->RemoteName(addr); 
        TUint accessLatency = MaxFramesPerPacket() * CalculateFrameDuration(aFrameLength, aTargetBitrate); 
        // Not checking the return code, because it doesn't matter if it fails. 
        iA2DPOptimiser.OptimiseAclForAudioStreaming(addr.BTAddr(), aTargetBitrate, accessLatency);
        }
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::StartL() <-")));
    }

// ---------------------------------------------------------------------------
// This method is for closing the sender down. 
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::Stop()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::Stop() ->")));
	// Make sure that the packet is not out at the moment. 
	Cancel(); 
    if(iCurrentSendPacket->IsOpen() != EFalse)
        {
   		iCurrentSendPacket->Close(); 
        }

    if(iSpareSendPacket->IsOpen() != EFalse)
        {
    	iSpareSendPacket->Close(); 
        }

    // Remove the ACL link optimisation if optimiser service is available. 
    if(iLinkOptimiserAvailable)
        {
	    TBTSockAddr addr; 
    	iRtpSession.RtpSocket()->RemoteName(addr); 
	    // Not checking the return code, because it doesn't matter if it fails. 
    	iA2DPOptimiser.RemoveAclOptimisation(addr.BTAddr());
		}
    iStreamerState = EStopped; 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t State changed to: EStopped")));

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::Stop() <-")));
    }

// ---------------------------------------------------------------------------
// From class CBTAudioStreamSender.
// RunL will be called when ConsumeBuffer has decided that we have enough 
// frames for one packet, or DoSendError was called. 
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::RunL()
    {        
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::RunL() ->")));
    switch(iStreamerState)
        {
        case ESending: 
            {    
            // Set A2DP media packet header. 
            AddHeaderToSendPacket(iPayloadDesC); 

            // Use the timestamp from the first buffer. 
            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Adjust and set timestamp.")));
            iCurrentSendPacket->SetTimestamp(iAdjustedTimestamp); 

            // Set the length and send the packet. 
            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Set length.")));
            iCurrentSendPacket->WritePayload().SetLength(iPayloadDesC.Length());

            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Send.")));
            iCurrentSendPacket->Send();

            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Packet has transferred to RTP API.")));
            }
            break; 
        case EBuffering: 
            {
            // Note: Here we will check that no packets were discarded while waiting for the sending to complete. 
            // If that happens, we could inform SAC and then try to decrease the bitrate. 
    
            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Packet sent succesfully.")));
            iPayloadDesC.Zero(); 
            }
            break; 
        case EErrorSending: 
            {
            // Note: Inform the observer 
            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Packet sending failed.")));
            iPayloadDesC.Zero(); 
            PacketSendingCompleted(iPayloadDesC); 
            iStreamerState = EBuffering; 
            
            // Tell client that we're ready for more data. Method name is misleading, but we will request for more 
            // until the observer tells us to stop. 
            iObserver.NotifyBufferSent(*iBuffer); 

            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t State changed to: EBuffering")));
            }
            break; 
        default: 
            {
          BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Error: RunL is in incorrect state!")));
            }
            break; 
        }
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::RunL() <-")));
    }

// ---------------------------------------------------------------------------
// For sending the frames. 
// ---------------------------------------------------------------------------
//
TInt CBTAudioStreamSender::SendThisBuffer(const TDesC8& aBuffer, TTimeIntervalMicroSeconds aTimestamp)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SendThisBuffer() ->")));

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t Received timestamp low: %d"), I64LOW(aTimestamp.Int64()))); 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t Received timestamp high: %d"), I64HIGH(aTimestamp.Int64()))); 

    // Store the buffer's address for later reference. 
    iBuffer = &aBuffer; 

		if(iChangingFrameLength != EFalse) 
		{
			// Check the new framelength... 
			if(aBuffer.Length() == iNewFrameLength)  // Check if the buffer exactly of the new length. 
			{
						// If we have data with previous frame length in buffer, send it first. 
            if(iNonprocessedDataInBuffer != EFalse)
                {
	              ProceedToPacketSending(); 
                return KErrNone; 
                }
            else	// Else we can switch to new packet immediately and continue processing this buffer. 
                {
								SwitchToNewSendPacket(); 
                }
			}
			else if(aBuffer.Length() > iNewFrameLength) // Check if it's safe to peek at index iNewFrameLength. 
			{
				if(aBuffer[iNewFrameLength] == 0x9c) // Check if the index iNewFrameLength the syncword. 
				    {
						// If we have data with previous frame length in buffer, send it first. 
            if(iNonprocessedDataInBuffer != EFalse)
                {
	              ProceedToPacketSending(); 
                return KErrNone; 
                }
            else	// Else we can switch to new packet immediately and continue processing this buffer. 
                {
								SwitchToNewSendPacket(); 
                }
						}
				else 
				{
            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender: Still waiting for packets of the new frame length...")));
				}
			}
			else 
			{
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender: Still waiting for packets of the new frame length...")));
				// Encoder has not yet started to use the new frame length. 
			}
		}
    else if(iStreamerState != EBuffering) 
        {
        // Previous sending is not yet completed or there is was an error that is not yet handled. 
        // Just discard the incoming frame. 
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender error: Previous buffer sending hasn't completed - discarding this buffer.")));
        iObserver.NotifyBufferSent(aBuffer); 
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SendThisBuffer() <-")));
        return KErrOverflow; 
        }

		// Store the timestamp for later use. 
    iTimestamp = aTimestamp.Int64(); 

    iNonprocessedDataInBuffer = ETrue; 
    ConsumeBuffer(); 

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SendThisBuffer() <-")));
    return KErrNone; 
    }

// ---------------------------------------------------------------------------
// From class CActive.
// Cancellation. 
// ---------------------------------------------------------------------------
//
void CBTAudioStreamSender::DoCancel()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::DoCancel() ->")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::DoCancel() <-")));
    }

inline void CBTAudioStreamSender::ConsumeBuffer()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::ConsumeBuffer() ->")));
    if (!IsActive())
        {    
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::ConsumeBuffer() ok to send")));				

        TInt spaceLeftInSendPacket = AddBufferToSendPacket(*iBuffer); 
				iAdjustedTimestamp = iTimestamp;  // take a copy, because we want to keep the original timestamp. 
        if(spaceLeftInSendPacket >= 0) 
            {
            iNonprocessedDataInBuffer = EFalse; // This indicates that there's no need to call this method after sending. 
            }
        ProceedToPacketSending(); 
        }
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::ConsumeBuffer() <-")));
    }


inline void CBTAudioStreamSender::ProceedToPacketSending()
{
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::ProceedToPacketSending() ->")));
    iStreamerState = ESending; 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTSTATE, FLOG(_L("[BTAudioStreamer]\t State changed to: ESending")));
    TRequestStatus *status = &iStatus;
    iStatus = KRequestPending;
    iThread.RequestComplete(status, KErrNone);
    SetActive();
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::ProceedToPacketSending() <-")));
}

inline void CBTAudioStreamSender::SwitchToNewSendPacket()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SwitchToNewSendPacket() ->")));

		// Make the iCurrentSendPacket point to the spare packet opened when the frame length change came. 
    RRtpSendPacket* oldSendPacket; 
    oldSendPacket = iCurrentSendPacket; 
    iCurrentSendPacket = iSpareSendPacket; 
		iSpareSendPacket = oldSendPacket; 
		
    // The previously used send packet (which has now become the spare send packet) can be closed. 
		oldSendPacket->Close(); 

		// The rest of the initializations: 
    InitSender(*iCurrentSendPacket, iPayloadDesC, iTargetBitrate); 

		iChangingFrameLength = EFalse; 

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamSender::SwitchToNewSendPacket() <-")));
	  }
