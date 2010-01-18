/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This is the implementation of application class
 *
*/


#include "datasegmenter.h"

CDataSegmenter* CDataSegmenter::NewL()
    {
    CDataSegmenter* self = NewLC();
    CleanupStack::Pop(self);
    return self;
    }

CDataSegmenter* CDataSegmenter::NewLC()
    {
    CDataSegmenter* self = new (ELeave) CDataSegmenter();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

CDataSegmenter::CDataSegmenter()
    {
    // No work to do
    }

CDataSegmenter::~CDataSegmenter()
    {
    delete iPayloadBuf;

    delete iPacket;
    }

void CDataSegmenter::SegmentDataL(const TDesC8& aInitialHeader,
        const TDesC8& aPayload, TChar aAdditionalHeaderByte, TInt aMTU)
    {
    // Store the continuation packet header byte and MTU
    iAdditionalHeaderByte = aAdditionalHeaderByte;
    iMTU = aMTU;

    // Determine the payload that will go in the first packet
    TInt initialPayLoadSize = Min((iMTU - aInitialHeader.Length()),
            aPayload.Length());

    // The initial packet size
    TInt initialSize = aInitialHeader.Length() + initialPayLoadSize;

    TPtr8 dataPtr(0, 0);
    if (!iPacket)
        {
        // Allocate the packet buffer, if it doesn't exist
        iPacket = HBufC8::NewL(initialSize);
        dataPtr.Set(iPacket->Des());
        }
    else
        {
        // Ensure the existing buffer is large enough to hold the new data
        dataPtr.Set(iPacket->Des());
        if (dataPtr.MaxLength() < initialSize)
            {
            delete iPacket;
            iPacket = 0;
            iPacket = HBufC8::NewL(initialSize);
            dataPtr.Set(iPacket->Des());
            }
        }

    // Reset the packet buffer
    dataPtr.Zero();
    // Append the header and initial payload
    dataPtr.Append(aInitialHeader);
    dataPtr.Append(aPayload.Mid(0, initialPayLoadSize));

    // Reset the payload pointer to NULL
    iPayloadBufPtr.Set(TPtrC8(0, 0));

    // Determine the remaining payload
    TInt payloadRemaining = aPayload.Length() - initialPayLoadSize;

    // If there is some remaining payload, store it
    if (payloadRemaining > 0)
        {
        if (!iPayloadBuf)
            {
            // Allocate the payload buffer, if it doesn't exist
            iPayloadBuf = HBufC8::NewL(payloadRemaining);
            dataPtr.Set(iPayloadBuf->Des());
            }
        else
            {
            // Ensure the existing buffer is large enough to hold the new data
            dataPtr.Set(iPayloadBuf->Des());
            if (dataPtr.MaxLength() < payloadRemaining)
                {
                delete iPayloadBuf;
                iPayloadBuf = 0;
                iPayloadBuf = HBufC8::NewL(payloadRemaining);
                dataPtr.Set(iPayloadBuf->Des());
                }
            }

        // Reset the payload buffer
        dataPtr.Zero();
        // Copy the remaining payload
        dataPtr.Append(aPayload.Mid(initialPayLoadSize));
        // Initialise the payload pointer
        iPayloadBufPtr.Set(iPayloadBuf->Des());
        }
    }

const HBufC8* CDataSegmenter::FirstPacketL() const
    {
    // Leave if no packet created.
    User::LeaveIfNull(iPacket);

    // Return the initial packet created
    return iPacket;
    }

const HBufC8* CDataSegmenter::NextPacket()
    {
    HBufC8* result = iPacket;
    TPtr8 packetPtr = iPacket->Des();

    // If there is some payload left over
    if (iPayloadBufPtr.Length() > 0)
        {
        // Reset the packet buffer
        packetPtr.Zero();

        // Append the additional packet header
        packetPtr.Append(iAdditionalHeaderByte);

        // Determine how much payload to copy, minimum of what's left and what
        // can fit in the packet
        TInt dataToCopy = Min((iMTU - 1), iPayloadBufPtr.Length());

        // Copy payload into the packet buffer
        packetPtr.Append(iPayloadBufPtr.Mid(0, dataToCopy));

        // Update the payload pointer
        iPayloadBufPtr.Set(iPayloadBufPtr.Mid(dataToCopy));
        }
    else if (packetPtr.Length() == iMTU)
        {
        // No payload left, but last packet was full, so we must send
        // an additional packet with just the header
        packetPtr.Zero();
        packetPtr.Append(iAdditionalHeaderByte);
        }
    else
        {
        // No more packets
        result = 0;
        }

    return result;
    }

void CDataSegmenter::Reset()
    {
    // If a packet has been allocated, then reset the data
    if (iPacket)
        {
        TPtr8 packetPtr = iPacket->Des();
        packetPtr.Zero();
        }

    // Reset the payload pointer to NULL
    iPayloadBufPtr.Set(TPtrC8(0, 0));
    }

void CDataSegmenter::ConstructL()
    {
    // No work to do
    }

