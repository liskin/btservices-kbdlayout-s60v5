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
* Description:  Contains an implementation of CBTAudioStreamer class.
*
*/


#include <bt_sock.h> 
#include <in_sock.h>

#include "btaudiostreamerdebug.h" 
#include "btaudiostreamer.h"
#include "btaudiostreamsender.h"

const TInt KL2CAPDefaultMTUSize = 672; 

// ---------------------------------------------------------------------------
// Constructor. 
// ---------------------------------------------------------------------------
//
CBTAudioStreamer::CBTAudioStreamer() : iStarted(EFalse)  
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::CBTAudioStreamer() ->")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::CBTAudioStreamer() <-")));
    }

// ---------------------------------------------------------------------------
// Static factory method. 
// ---------------------------------------------------------------------------
//
EXPORT_C CBTAudioStreamer* CBTAudioStreamer::NewL()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::NewL() ->")));
    CBTAudioStreamer* self = new (ELeave) CBTAudioStreamer();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::NewL() <-")));
    return self;
    }

// ---------------------------------------------------------------------------
// Symbian OS second phase contruction. 
// ---------------------------------------------------------------------------
//
void CBTAudioStreamer::ConstructL()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::ConstructL() ->")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::ConstructL() <-")));
    }

// ---------------------------------------------------------------------------
// Destructor. 
// ---------------------------------------------------------------------------
//
CBTAudioStreamer::~CBTAudioStreamer()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::~CBTAudioStreamer() ->")));
    Stop(); 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::~CBTAudioStreamer() <-")));
    }

// ---------------------------------------------------------------------------
// This method prepares the streamer for incoming data. 
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CBTAudioStreamer::SetNewFrameLength(const TUint aFrameLength, const TUint aTargetBitrate) 
{
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::SetNewFrameLength() ->")));
	TInt retVal = KErrGeneral; 
	if(iStarted != EFalse) 
		{
	    TInt outboundMTUSize; 
	//  if(aSocket.GetOpt(KL2CAPGetOutboundMTU, KSolBtL2CAP, outboundMTUSize) != KErrNone) 
	        {
	        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Failed to get outbound MTU size, using L2CAP default value")));
	        outboundMTUSize = KL2CAPDefaultMTUSize; 
	        }
		retVal = iSender->SetNewFrameLength(outboundMTUSize, aFrameLength, aTargetBitrate); 
		}
	else 
		{
	    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Error: Not started!")));
	    retVal = KErrNotReady; 
		}
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::SetNewFrameLength() <-")));
	return retVal; 
}

// ---------------------------------------------------------------------------
// This method prepares the streamer for incoming data. 
// ---------------------------------------------------------------------------
//
EXPORT_C void CBTAudioStreamer::StartL(RSocket& aSocket, const TUint aFrameLength, CBTAudioStreamInputBase *aAudioInput, const TUint aTargetBitrate)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::StartL() ->")));
    iAudioInput = aAudioInput; 

    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FPrint(_L("[BTAudioStreamer]\t Framelength: %d"), aFrameLength));

    if(iStarted != EFalse) 
        {
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Error: Already started!")));
        User::Leave(KErrInUse); 
        }

    TInt inboundMTUSize = 0; 
    if(aSocket.GetOpt(KL2CAPInboundMTU, KSolBtL2CAP, inboundMTUSize) != KErrNone) 
        {
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Failed to get inbound MTU size, using L2CAP default value")));
        inboundMTUSize = KL2CAPDefaultMTUSize; 
        }

    iRtpSession.OpenL(aSocket, inboundMTUSize); 
    iSender = CBTAudioStreamSender::NewL(*this, iRtpSession); 

    TInt outboundMTUSize; 
    if(aSocket.GetOpt(KL2CAPGetOutboundMTU, KSolBtL2CAP, outboundMTUSize) != KErrNone) 
        {
        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Failed to get outbound MTU size, using L2CAP default value")));
        outboundMTUSize = KL2CAPDefaultMTUSize; 
        }

    iSender->StartL(outboundMTUSize, aFrameLength, aTargetBitrate); 

		iStarted = ETrue; 
		
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::StartL() <-")));
    }

// ------------------------------------------------------------------------------------------
// This method cleans up the streamer and releases the resources it reserved for streaming. 
// ------------------------------------------------------------------------------------------
//
EXPORT_C void CBTAudioStreamer::Stop()
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::Stop() ->")));
    
    if(iStarted != EFalse) // Don't delete things unless we have started, thus created those. 
        {
        if(iSender) 
            {
            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Stopping and deleting the stream sender.")));
            iSender->Stop();
            delete iSender; 
            iSender = NULL; 
            }
        else 
            {
            BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Error! Stream sender was deleted while the streamer was running!")));
            }

        BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t Closing RTP session.")));
        iRtpSession.Close(); 
        }

    iAudioInput = NULL; 
	iStarted = EFalse;
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::Stop() <-")));
    }

TInt CBTAudioStreamer::Receive(const TDesC8& /*aBuffer*/)
    {
    return KErrNotSupported; 
    }

TInt CBTAudioStreamer::Receive(const TDesC8& aBuffer, TTimeIntervalMicroSeconds aTimestamp)
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::Receive() ->")));
    TInt retval = iSender->SendThisBuffer(aBuffer, aTimestamp); 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::Receive() <-")));
    return retval; 
    }

void CBTAudioStreamer::NotifyBufferSent( const TDesC8 &aBuffer ) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::NotifyBufferSent() ->")));
    if(iAudioInput) iAudioInput->BufferEmptied(aBuffer); 
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::NotifyBufferSent() <-")));
    }

void CBTAudioStreamer::NotifyErrorSending( const TDesC8 &/*aBuffer*/ ) 
    {
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::NotifyErrorSending() ->")));
    BT_AUDIO_STREAMER_TRACE_OPT( KPRINTFTRACE, FLOG(_L("[BTAudioStreamer]\t CBTAudioStreamer::NotifyErrorSending() <-")));
    }
